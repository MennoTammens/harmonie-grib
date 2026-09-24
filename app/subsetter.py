"""Regional subsetting using wgrib2 -small_grib."""

import json
import logging
from pathlib import Path
import subprocess
from typing import Any, Dict, List, Optional, Tuple

from app.config import (
    BOUNDS_FILE,
    GRIB_EXT,
    GRIB_PREFIX,
    WGRIB2_BIN,
)

logger = logging.getLogger(__name__)


def load_areas(bounds_file: Path = BOUNDS_FILE) -> List[Dict[str, Any]]:
    """Load regional bounding boxes from JSON configuration."""
    bounds_path = Path(bounds_file)
    if not bounds_path.exists():
        raise FileNotFoundError(f"Bounds configuration file not found: {bounds_path}")

    with open(bounds_path, "r", encoding="utf-8") as f:
        areas = json.load(f)
    return areas


def build_subset_args(
    areas: List[Dict[str, Any]],
    output_dir: Path,
    run_time: str,
    is_wind: bool = False,
    prefix: str = GRIB_PREFIX,
    ext: str = GRIB_EXT,
) -> Tuple[List[str], List[Path]]:
    """Build wgrib2 -small_grib command arguments and expected output paths."""
    output_dir = Path(output_dir)
    output_dir.mkdir(parents=True, exist_ok=True)

    args = []
    output_files = []

    suffix = "_wind" if is_wind else ""
    for area in areas:
        abbr = area["abbr"]
        sw_lon, sw_lat = area["bounds"][0]
        ne_lon, ne_lat = area["bounds"][1]

        filename = f"{prefix}_{run_time}_{abbr}{suffix}{ext}"
        dst_path = output_dir / filename
        output_files.append(dst_path)

        lon_range = f"{sw_lon}:{ne_lon}"
        lat_range = f"{sw_lat}:{ne_lat}"
        args.extend(["-small_grib", lon_range, lat_range, str(dst_path)])

    return args, output_files


def generate_subsets(
    input_grib: Path,
    output_dir: Path,
    run_time: str,
    is_wind: bool = False,
    areas: Optional[List[Dict[str, Any]]] = None,
    wgrib2_bin: str = WGRIB2_BIN,
    prefix: str = GRIB_PREFIX,
    ext: str = GRIB_EXT,
) -> List[Path]:
    """Generate regional GRIB2 subsets for all areas in a single wgrib2 execution."""
    input_grib = Path(input_grib)
    if not input_grib.is_file():
        raise FileNotFoundError(f"Input GRIB file does not exist: {input_grib}")

    if areas is None:
        areas = load_areas()

    subset_args, output_files = build_subset_args(
        areas=areas,
        output_dir=output_dir,
        run_time=run_time,
        is_wind=is_wind,
        prefix=prefix,
        ext=ext,
    )

    cmd = [wgrib2_bin, str(input_grib), "-set_grib_type", "same"] + subset_args
    logger.info("Executing wgrib2 subsetting for %d areas on %s...", len(areas), input_grib.name)

    res = subprocess.run(
        cmd,
        stdout=subprocess.DEVNULL,
        stderr=subprocess.PIPE,
        text=True,
    )

    if res.returncode != 0:
        logger.error("wgrib2 error (code %d): %s", res.returncode, res.stderr)
        raise RuntimeError(f"wgrib2 subsetting failed with exit code {res.returncode}: {res.stderr}")

    logger.info("Successfully generated %d regional subsets in %s", len(output_files), output_dir)
    return output_files
