"""Low-memory asynchronous GRIB Range downloader.

Downloads only filtered GRIB2 byte ranges over HTTP Range requests using
temporary chunk files to keep memory usage under 10 MB RAM, and collates them
sequentially into both all-parameter and wind-parameter GRIB files.
"""

import asyncio
import logging
import os
from pathlib import Path
import shutil
from typing import Any, Dict, List, Optional
import uuid

import aiohttp

from app.config import (
    DOWNLOAD_CONCURRENCY,
    HTTP_TIMEOUT,
    TMP_DIR,
    WIND_RECORDS,
)

logger = logging.getLogger(__name__)


def coalesce_ranges(records: List[Dict[str, Any]]) -> List[Dict[str, Any]]:
    """Coalesce contiguous message byte ranges to reduce HTTP request count.

    Returns a list of items:
      {
        'offset': int,
        'length': int,
        'messages': [record, ...]
      }
    """
    if not records:
        return []

    sorted_records = sorted(records, key=lambda r: r["offset"])
    coalesced: List[Dict[str, Any]] = []

    for rec in sorted_records:
        rec_offset = rec["offset"]
        rec_len = rec["total_length"]

        if coalesced and (coalesced[-1]["offset"] + coalesced[-1]["length"] == rec_offset):
            coalesced[-1]["length"] += rec_len
            coalesced[-1]["messages"].append(rec)
        else:
            coalesced.append({
                "offset": rec_offset,
                "length": rec_len,
                "messages": [rec],
            })

    return coalesced


async def _download_chunk_to_file(
    session: aiohttp.ClientSession,
    sem: asyncio.Semaphore,
    url: str,
    offset: int,
    length: int,
    out_path: Path,
):
    """Download single byte range and stream directly to temporary file."""
    end = offset + length - 1
    headers = {"Range": f"bytes={offset}-{end}"}
    async with sem:
        async with session.get(url, headers=headers) as resp:
            resp.raise_for_status()
            with open(out_path, "wb") as f:
                async for chunk in resp.content.iter_chunked(65536):
                    f.write(chunk)


async def download_filtered_gribs(
    download_url: str,
    records: List[Dict[str, Any]],
    all_path: Path,
    wind_path: Path,
    tmp_dir: Optional[Path] = None,
    concurrency: int = DOWNLOAD_CONCURRENCY,
) -> Dict[str, Any]:
    """Download filtered GRIB messages via Range requests with <10 MB RAM footprint.

    Writes all target records to `all_path` and wind records to `wind_path`.
    """
    all_path = Path(all_path)
    wind_path = Path(wind_path)
    all_path.parent.mkdir(parents=True, exist_ok=True)
    wind_path.parent.mkdir(parents=True, exist_ok=True)

    parent_tmp = Path(tmp_dir) if tmp_dir else TMP_DIR
    parent_tmp.mkdir(parents=True, exist_ok=True)
    job_tmp_dir = parent_tmp / f"dl_{uuid.uuid4().hex[:8]}"
    job_tmp_dir.mkdir(parents=True, exist_ok=True)

    coalesced_items = coalesce_ranges(records)
    logger.info(
        "Downloading %d records coalesced into %d HTTP Range requests (concurrency=%d)...",
        len(records),
        len(coalesced_items),
        concurrency,
    )

    sem = asyncio.Semaphore(concurrency)
    connector = aiohttp.TCPConnector(limit=concurrency * 2)
    timeout = aiohttp.ClientTimeout(total=HTTP_TIMEOUT * 4)

    try:
        async with aiohttp.ClientSession(connector=connector, timeout=timeout) as session:
            tasks = []
            for seq, item in enumerate(coalesced_items):
                part_path = job_tmp_dir / f"{seq:05d}.part"
                task = asyncio.create_task(
                    _download_chunk_to_file(
                        session=session,
                        sem=sem,
                        url=download_url,
                        offset=item["offset"],
                        length=item["length"],
                        out_path=part_path,
                    )
                )
                tasks.append(task)

            await asyncio.gather(*tasks)

        logger.info("All %d Range chunks downloaded to temp storage. Collating to disk...", len(coalesced_items))

        # Collate chunks sequentially into all_path and wind_path
        tmp_all_path = all_path.with_suffix(all_path.suffix + ".tmp")
        tmp_wind_path = wind_path.with_suffix(wind_path.suffix + ".tmp")

        all_bytes = 0
        wind_bytes = 0
        all_count = 0
        wind_count = 0

        with open(tmp_all_path, "wb") as f_all, open(tmp_wind_path, "wb") as f_wind:
            for seq, item in enumerate(coalesced_items):
                part_path = job_tmp_dir / f"{seq:05d}.part"
                with open(part_path, "rb") as f_part:
                    chunk_bytes = f_part.read()

                local_offset = 0
                for msg in item["messages"]:
                    msg_len = msg["total_length"]
                    msg_data = chunk_bytes[local_offset : local_offset + msg_len]
                    local_offset += msg_len

                    f_all.write(msg_data)
                    all_bytes += len(msg_data)
                    all_count += 1

                    is_wind = (msg.get("param_abbrev"), msg.get("surface")) in WIND_RECORDS
                    if is_wind:
                        f_wind.write(msg_data)
                        wind_bytes += len(msg_data)
                        wind_count += 1

        tmp_all_path.replace(all_path)
        tmp_wind_path.replace(wind_path)

        logger.info(
            "Wrote all-params: %s (%d msgs, %.2f MB); wind-params: %s (%d msgs, %.2f MB)",
            all_path.name,
            all_count,
            all_bytes / (1024 * 1024),
            wind_path.name,
            wind_count,
            wind_bytes / (1024 * 1024),
        )

        return {
            "all_path": all_path,
            "wind_path": wind_path,
            "all_bytes": all_bytes,
            "wind_bytes": wind_bytes,
            "all_count": all_count,
            "wind_count": wind_count,
        }

    finally:
        shutil.rmtree(job_tmp_dir, ignore_errors=True)
