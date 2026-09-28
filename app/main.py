"""Main application entry point for Harmonie 46 GRIB indexer, downloader & subsetter."""

import argparse
import asyncio
from datetime import datetime
import logging
from pathlib import Path
import re
import shutil
import signal
import sys
import time
from typing import Any, Dict, List, Optional

from app.api_client import OpenDataAPIClient
from app.config import (
    DATA_DIR,
    DATASET_NAME,
    DATASET_VERSION,
    GRIB_EXT,
    GRIB_PREFIX,
    KNMI_NOTIFICATION_SERVICE_API_KEY,
    KNMI_OPEN_DATA_API_KEY,
    MQTT_TOPIC,
    RETENTION_RUNS,
    TARGET_RECORDS,
    TMP_DIR,
)
from app.downloader import download_filtered_gribs
from app.grib_reader import (
    filter_target_messages,
    scan_file_or_url,
    write_index_file,
)
from app.mqtt_subscriber import NotificationSubscriber
from app.subsetter import generate_subsets

logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s [%(levelname)s] %(name)s: %(message)s",
    datefmt="%Y-%m-%d %H:%M:%S",
)
logger = logging.getLogger("harmonie46-indexer")


def parse_run_time_date(filename: str) -> str:
    """Extract forecast runtime from filename (e.g. HARM46_V1_P1_2026092413.tar -> 2026-09-24_13)."""
    match = re.search(r"(\d{10})", filename)
    if match:
        raw_ts = match.group(1)
        try:
            return datetime.strptime(raw_ts, "%Y%m%d%H").strftime("%Y-%m-%d_%H")
        except ValueError:
            pass
    # Fallback to date stamp or sanitized name
    clean = Path(filename).stem
    return clean


class IndexerService:
    """Orchestrates MQTT notifications, Open Data API calls, GRIB indexing, downloading & subsetting."""

    def __init__(self, data_dir: Path = DATA_DIR):
        self.data_dir = Path(data_dir)
        self.data_dir.mkdir(parents=True, exist_ok=True)
        self.api_client = OpenDataAPIClient()
        self.queue: asyncio.Queue[Dict[str, Any]] = asyncio.Queue()
        self.subscriber: Optional[NotificationSubscriber] = None
        self._running = False
        self._worker_task: Optional[asyncio.Task] = None
        self._loop: Optional[asyncio.AbstractEventLoop] = None

    def on_mqtt_message(self, event: Dict[str, Any]):
        """Thread-safe enqueue of received event."""
        loop = self._loop
        if loop is None:
            try:
                loop = asyncio.get_running_loop()
            except RuntimeError:
                pass

        if loop and loop.is_running():
            loop.call_soon_threadsafe(self.queue.put_nowait, event)
        else:
            logger.error("Cannot enqueue MQTT event: event loop is not available or running.")

    def update_latest_symlink(self, target_dir: Path):
        """Atomically point data/latest symlink to the new forecast run directory."""
        latest_symlink = self.data_dir / "latest"
        tmp_symlink = self.data_dir / f".latest_new_{int(time.time())}"
        try:
            if tmp_symlink.is_symlink() or tmp_symlink.exists():
                tmp_symlink.unlink()
            rel_target = target_dir.relative_to(self.data_dir)
            tmp_symlink.symlink_to(rel_target)
            tmp_symlink.replace(latest_symlink)
            logger.info("Updated symlink: data/latest -> %s", rel_target)
        except Exception as e:
            logger.warning("Could not create/update latest symlink: %s", e)
            if tmp_symlink.is_symlink() or tmp_symlink.exists():
                tmp_symlink.unlink(missing_ok=True)

    def prune_old_runs(self, retention: int = RETENTION_RUNS):
        """Remove older run directories beyond the retention count."""
        if retention <= 0:
            return

        run_dirs = []
        pattern = re.compile(r"^\d{4}-\d{2}-\d{2}_\d{2}$")
        for item in self.data_dir.iterdir():
            if item.is_dir() and pattern.match(item.name):
                run_dirs.append(item)

        run_dirs.sort(key=lambda d: d.name)
        if len(run_dirs) > retention:
            to_remove = run_dirs[:-retention]
            for d in to_remove:
                logger.info("Pruning old forecast directory: %s", d.name)
                shutil.rmtree(d, ignore_errors=True)

    async def process_file(
        self,
        filename: str,
        download_url: Optional[str] = None,
        dataset_name: str = DATASET_NAME,
        dataset_version: str = DATASET_VERSION,
        index_only: bool = False,
    ) -> Dict[str, Any]:
        """Index headers, download filtered messages, generate regional subsets, and organize outputs."""
        started = time.monotonic()
        run_time_date = parse_run_time_date(filename)
        run_dir = self.data_dir / run_time_date
        run_dir.mkdir(parents=True, exist_ok=True)

        logger.info("=== Starting processing for %s (run: %s) ===", filename, run_time_date)

        # 1. Obtain pre-signed download URL if needed
        if not download_url:
            logger.info("Fetching temporary download URL from Open Data API for %s...", filename)
            download_url = await self.api_client.get_temporary_download_url(
                dataset_name=dataset_name,
                dataset_version=dataset_version,
                filename=filename,
            )

        # 2. Scan GRIB headers via Range requests
        logger.info("Scanning GRIB2 headers from: %s...", download_url[:80])
        scan_t0 = time.monotonic()
        messages = await scan_file_or_url(download_url)
        scan_elapsed = time.monotonic() - scan_t0
        logger.info("Scanned %d total messages in %.2fs", len(messages), scan_elapsed)

        # 3. Filter target parameters (all 9)
        matched = filter_target_messages(messages, TARGET_RECORDS)
        logger.info("Found %d matching target records (out of %d)", len(matched), len(messages))

        # 4. Write index file
        idx_path = run_dir / f"{GRIB_PREFIX}_{run_time_date}.idx"
        write_index_file(matched, idx_path)
        # Also maintain a root-level copy for backward compatibility
        root_idx = self.data_dir / f"{filename}.idx"
        write_index_file(matched, root_idx)
        logger.info("Index files written to %s and %s", idx_path.name, root_idx.name)

        if index_only:
            logger.info("Index-only mode specified. Skipping GRIB downloading and subsetting.")
            return {"run_dir": run_dir, "idx_path": idx_path}

        # 5. Download filtered GRIBs with low-memory chunk streaming
        all_path = run_dir / f"{GRIB_PREFIX}_{run_time_date}{GRIB_EXT}"
        wind_path = run_dir / f"{GRIB_PREFIX}_{run_time_date}_wind{GRIB_EXT}"

        dl_t0 = time.monotonic()
        dl_stats = await download_filtered_gribs(
            download_url=download_url,
            records=matched,
            all_path=all_path,
            wind_path=wind_path,
            tmp_dir=TMP_DIR,
        )
        dl_elapsed = time.monotonic() - dl_t0
        logger.info(
            "Download & collation completed in %.2fs (All: %.2f MB, Wind: %.2f MB)",
            dl_elapsed,
            dl_stats["all_bytes"] / (1024 * 1024),
            dl_stats["wind_bytes"] / (1024 * 1024),
        )

        # 6. Generate regional subsets using wgrib2 -small_grib
        sub_t0 = time.monotonic()
        subsets_all = generate_subsets(
            input_grib=all_path,
            output_dir=run_dir,
            run_time=run_time_date,
            is_wind=False,
            prefix=GRIB_PREFIX,
            ext=GRIB_EXT,
        )
        subsets_wind = generate_subsets(
            input_grib=wind_path,
            output_dir=run_dir,
            run_time=run_time_date,
            is_wind=True,
            prefix=GRIB_PREFIX,
            ext=GRIB_EXT,
        )
        sub_elapsed = time.monotonic() - sub_t0
        logger.info(
            "Generated %d regional subsets in %.2fs (%d all-params, %d wind-params)",
            len(subsets_all) + len(subsets_wind),
            sub_elapsed,
            len(subsets_all),
            len(subsets_wind),
        )

        # 7. Update data/latest symlink
        self.update_latest_symlink(run_dir)

        # 8. Prune older forecast runs
        self.prune_old_runs()

        total_elapsed = time.monotonic() - started
        logger.info(
            "=== Finished processing for %s in %.2fs (Files in %s) ===",
            filename,
            total_elapsed,
            run_dir,
        )

        return {
            "run_dir": run_dir,
            "all_path": all_path,
            "wind_path": wind_path,
            "subsets_all": subsets_all,
            "subsets_wind": subsets_wind,
            "total_elapsed": total_elapsed,
        }

    async def worker_loop(self):
        """Asynchronously processes queued file events."""
        logger.info("Event processing worker started.")
        while self._running:
            try:
                event = await self.queue.get()
            except asyncio.CancelledError:
                break

            try:
                data = event.get("data", {})
                filename = data.get("filename")
                ds_name = data.get("datasetName", DATASET_NAME)
                ds_version = data.get("datasetVersion", DATASET_VERSION)

                if not filename:
                    logger.warning("Received event with missing filename: %s", event)
                    continue

                logger.info("Dequeued event for dataset %s v%s: %s", ds_name, ds_version, filename)
                await self.process_file(
                    filename=filename,
                    dataset_name=ds_name,
                    dataset_version=ds_version,
                )
            except Exception as e:
                logger.exception("Error processing event: %s", e)
            finally:
                self.queue.task_done()

    async def run_daemon(self):
        """Run the service continuously listening for notifications."""
        self._loop = asyncio.get_running_loop()
        self._running = True
        self._worker_task = asyncio.create_task(self.worker_loop())

        self.subscriber = NotificationSubscriber(event_callback=self.on_mqtt_message)
        self.subscriber.start()

        logger.info(
            "Harmonie 46 GRIB indexer & subsetter daemon is running. Subscribed to %s",
            MQTT_TOPIC,
        )

        stop_event = asyncio.Event()

        def _signal_handler():
            logger.info("Shutdown signal received.")
            stop_event.set()

        loop = asyncio.get_running_loop()
        for sig in (signal.SIGINT, signal.SIGTERM):
            try:
                loop.add_signal_handler(sig, _signal_handler)
            except NotImplementedError:
                pass

        await stop_event.wait()
        await self.shutdown()

    async def shutdown(self):
        """Gracefully stop subscriber and workers."""
        logger.info("Stopping indexer service...")
        self._running = False
        if self.subscriber:
            self.subscriber.stop()
        if self._worker_task:
            self._worker_task.cancel()
            try:
                await self._worker_task
            except asyncio.CancelledError:
                pass
        await self.api_client.close()
        logger.info("Indexer service stopped cleanly.")


async def run_check_mqtt():
    """Verify MQTT connection and subscription."""
    logger.info("Running MQTT connection verification...")
    if not KNMI_NOTIFICATION_SERVICE_API_KEY:
        logger.error("KNMI_NOTIFICATION_SERVICE_API_KEY is not set.")
        sys.exit(1)

    connected = asyncio.Event()

    def dummy_callback(event):
        pass

    sub = NotificationSubscriber(event_callback=dummy_callback, clean_start=True)
    sub.start()
    try:
        for _ in range(20):
            if sub.is_connected():
                connected.set()
                break
            await asyncio.sleep(0.5)

        if connected.is_set():
            logger.info("MQTT connection check PASSED! Connected and subscribed successfully.")
        else:
            logger.error("MQTT connection check FAILED: Could not connect within 10 seconds.")
            sys.exit(1)
    finally:
        sub.stop()


async def run_process_latest(service: IndexerService, index_only: bool = False):
    """Fetch the latest dataset file from Open Data API and process it."""
    logger.info("Fetching latest file info for dataset %s v%s...", DATASET_NAME, DATASET_VERSION)
    if not KNMI_OPEN_DATA_API_KEY:
        logger.error("KNMI_OPEN_DATA_API_KEY is not set.")
        sys.exit(1)

    latest_filename = await service.api_client.get_latest_filename(DATASET_NAME, DATASET_VERSION)
    if not latest_filename:
        logger.error("No files found in dataset %s v%s.", DATASET_NAME, DATASET_VERSION)
        sys.exit(1)

    logger.info("Latest file found: %s", latest_filename)
    await service.process_file(latest_filename, index_only=index_only)


async def run_process_file(service: IndexerService, filename: str, index_only: bool = False):
    """Index and process a specific file by filename via Open Data API."""
    logger.info("Processing specified file: %s", filename)
    await service.process_file(filename, index_only=index_only)


async def run_process_url(service: IndexerService, url: str, index_only: bool = False):
    """Index and process from a direct URL or local path."""
    logger.info("Processing from URL / path: %s", url)
    fname = Path(url.split("?")[0]).name or "custom_file.tar"
    await service.process_file(filename=fname, download_url=url, index_only=index_only)


def main():
    parser = argparse.ArgumentParser(
        description="Harmonie 46 GRIB Notification Service, Downloader & Subsetter"
    )
    parser.add_argument(
        "--check-mqtt",
        action="store_true",
        help="Check connection and topic subscription to KNMI MQTT broker and exit.",
    )
    parser.add_argument(
        "--latest",
        action="store_true",
        help="Fetch, filter, download, and subset the latest available file from KNMI and exit.",
    )
    parser.add_argument(
        "--file",
        type=str,
        help="Process a specific dataset filename via KNMI Open Data API and exit.",
    )
    parser.add_argument(
        "--url",
        type=str,
        help="Process a direct download URL or local file path and exit.",
    )
    parser.add_argument(
        "--index-only",
        action="store_true",
        help="Only scan headers and generate the index file without downloading message bodies.",
    )

    args = parser.parse_args()
    service = IndexerService()

    try:
        if args.check_mqtt:
            asyncio.run(run_check_mqtt())
        elif args.latest:
            asyncio.run(run_process_latest(service, index_only=args.index_only))
        elif args.file:
            asyncio.run(run_process_file(service, args.file, index_only=args.index_only))
        elif args.url:
            asyncio.run(run_process_url(service, args.url, index_only=args.index_only))
        else:
            asyncio.run(service.run_daemon())
    except KeyboardInterrupt:
        logger.info("Interrupted by user.")
    except Exception as exc:
        logger.exception("Fatal error: %s", exc)
        sys.exit(1)
    finally:
        asyncio.run(service.api_client.close())


if __name__ == "__main__":
    main()
