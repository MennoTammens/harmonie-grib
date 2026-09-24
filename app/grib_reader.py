"""GRIB2 header extraction and filtering for Harmonie 46 files.

Reads GRIB2 message headers directly from local files or remote HTTP URLs
via byte Range requests without downloading message bodies or full archives.
Derived from and compatible with example-scripts/grib2_header_dump_parallel.py.
"""

import asyncio
import datetime
import logging
import os
from pathlib import Path
import sys
import time
from typing import Any, Dict, List, Optional, Set, Tuple

from app.config import (
    CHUNK_SIZE,
    CONCURRENCY,
    HTTP_TIMEOUT,
    TARGET_RECORDS,
)

logger = logging.getLogger(__name__)

TIME_UNIT_HOURS = {
    0: 1 / 60,     # minute
    1: 1,          # hour
    2: 24,         # day
    10: 3,         # 3 hours
    11: 6,         # 6 hours
    12: 12,        # 12 hours
    13: 1 / 3600,  # second
}


def format_forecast_datetime(ref_dt, time_unit, fcst_time, end_dt):
    """Format reference time plus forecast offset(s) as +HHH[-HHH]."""
    hours = round(fcst_time * TIME_UNIT_HOURS.get(time_unit, 1))
    text = "%sZ+%03d" % (ref_dt.strftime("%Y-%m-%dT%H%M"), hours)
    if end_dt is not None:
        end_hours = round((end_dt - ref_dt).total_seconds() / 3600)
        text += "-%03d" % end_hours
    return text


PARAMETERS = {
    (0, 0, 0): ("TMP", "Temperature", "K"),
    (0, 0, 6): ("DPT", "Dew Point Temperature", "K"),
    (0, 0, 11): ("SHTFL", "Sensible Heat Net Flux", "W m-2"),
    (0, 0, 30): ("LHFLXE", "Latent Heat Net Flux Due to Evaporation", "W m-2"),
    (0, 1, 1): ("RH", "Relative Humidity", "%"),
    (0, 1, 11): ("SNOD", "Snow Depth", "m"),
    (0, 1, 52): ("TPRATE", "Total Precipitation Rate", "kg m-2 s-1"),
    (0, 1, 60): ("SDWE", "Snow Depth Water Equivalent", "kg m-2"),
    (0, 1, 65): ("RPRATE", "Rain Precipitation Rate", "kg m-2 s-1"),
    (0, 1, 66): ("SPRATE", "Snow Precipitation Rate", "kg m-2 s-1"),
    (0, 1, 74): ("TCOLG", "Total Column Integrated Graupel", "kg m-2"),
    (0, 1, 75): ("GPRATE", "Graupel (Snow Pellets) Precipitation Rate", "kg m-2 s-1"),
    (0, 2, 2): ("UGRD", "U-Component of Wind", "m s-1"),
    (0, 2, 3): ("VGRD", "V-Component of Wind", "m s-1"),
    (0, 2, 23): ("UGUST", "U-Component of Wind (Gust)", "m s-1"),
    (0, 2, 24): ("VGUST", "V-Component of Wind (Gust)", "m s-1"),
    (0, 3, 0): ("PRES", "Pressure", "Pa"),
    (0, 3, 4): ("GP", "Geopotential", "m2 s-2"),
    (0, 3, 18): ("HPBL", "Planetary Boundary Layer Height", "m"),
    (0, 4, 7): ("DSWRF", "Downward Shortwave Radiation Flux", "W m-2"),
    (0, 4, 9): ("NSWRF", "Net Shortwave Radiation Flux", "W m-2"),
    (0, 5, 5): ("NLWRF", "Net Longwave Radiation Flux", "W m-2"),
    (0, 6, 1): ("TCDC", "Total Cloud Cover", "%"),
    (0, 6, 3): ("LCDC", "Low Cloud Cover", "%"),
    (0, 6, 4): ("MCDC", "Medium Cloud Cover", "%"),
    (0, 6, 5): ("HCDC", "High Cloud Cover", "%"),
    (0, 6, 11): ("CDCB", "Cloud Base", "m"),
    (0, 19, 0): ("VIS", "Visibility", "m"),
    (2, 0, 0): ("LAND", "Land Cover", "Proportion"),
}


def format_param(discipline, category, number):
    """Look up Section 4 parameter in PARAMETERS table."""
    entry = PARAMETERS.get((discipline, category, number))
    if entry is None:
        return "discipline=%d:category=%d:number=%d" % (discipline, category, number)
    abbrev, name, units = entry
    return "%s %s (%s)" % (abbrev, name, units)


UNDEFINED = 9.999e20
NCEP = 7
KMA = 40

LEVEL_TABLE = [
    "reserved", "surface", "cloud base", "cloud top", "0C isotherm",
    "level of adiabatic condensation from sfc", "max wind", "tropopause",
    "top of atmosphere", "sea bottom", "entire atmosphere", "cumulonimbus base",
    "cumulonimbus top", "lowest level %g%% integrated cloud cover",
    "level of free convection", "convection condensation level",
    "level of neutral buoyancy", "Departure level of most unstable parcel of air (MUDL)",
    "Departure level of mixed layer parcel of air with specified layer depth %g (Pa)",
    "reserved", "%g K level", "lowest level > %g kg/m^3", "highest level > %g kg/m^3",
    "lowest level > %g Bq/m^3", "highest level > %g Bg/m^3", "highest level > %g dBZ",
    "convective cloud layer base", "convective clout layer top", "reserved", "reserved",
    "specified radius from center of sun", "solar photosphere", "ionosphere D-region level",
    "ionosphere E-region level", "ionosphere F1-region level", "ionosphere F2-region level",
    "stratopause",
] + ["reserved"] * (37 - 37 + 63) + [
    "%g mb", "mean sea level", "%g m above mean sea level", "%g m above ground",
    "%g sigma level", "%g hybrid level", "%g m underground", "%g K isentropic level",
    "%g mb above ground", "PV=%g (Km^2/kg/s) surface", "reserved", "%g Eta level",
    "reserved", "%g logarithmic hybrid level", "snow level", "%g sigma height level",
    "reserved", "mixed layer depth", "%g hybrid height level", "%g hybrid pressure level",
] + ["reserved"] * 30 + [
    "%g generalized vertical height coordinate", "soil level %g", "sea-ice level",
] + ["reserved"] * 7 + [
    "%g m below sea level", "%g m below water surface", "lake or river bottom",
    "bottom of sediment layer", "bottom of thermally active sediment layer",
    "bottom of sediment layer penetrated by thermal wave", "mixing layer",
    "bottom of root zone", "ocean model level %g",
    "ocean level %g (kg*m-3) density difference to near surface",
    "ocean level %g (K) pot. temperature difference to near surface",
    "ocean level %g (m*m/s) vertical eddy diffusivity difference from near surface",
    "mixed layer depth (delta rho=%g)", "reserved",
    "top surface of ice on sea, lake or river",
    "top surface of ice, und snow on sea, lake or river",
    "bottom surface ice on sea, lake or river", "deep soil", "reserved",
    "top surface of glacier ice and inland ice", "deep inland or glacier ice",
    "grid tile land fraction as a model surface", "grid tile water fraction as a model surface",
    "grid tile ice fraction on sea, lake or river as a model surface",
    "grid tile glacier ice and inland ice fraction as a model surface",
] + ["reserved"] * 7

NCEP_LEVEL_TABLE = [
    "reserved", "reserved", "reserved", "reserved", "reserved", "reserved", "reserved", "reserved",
    "entire atmosphere (considered as a single layer)", "entire ocean (considered as a single layer)",
    "reserved", "reserved", "highest tropospheric freezing level", "reserved",
    "grid scale cloud bottom level", "grid scale cloud top level", "reserved",
    "boundary layer cloud bottom level", "boundary layer cloud top level",
    "boundary layer cloud layer", "low cloud bottom level", "low cloud top level",
    "low cloud layer", "cloud ceiling", "effective layer top level %gm",
    "effective layer bottom level %gm", "effective layer %gm", "reserved",
    "planetary boundary layer", "layer between two hybrid levels", "middle cloud bottom level",
    "middle cloud top level", "middle cloud layer",
] + ["reserved"] * 7 + [
    "high cloud bottom level", "high cloud top level", "high cloud layer",
    "%gC ocean isotherm", "layer between two depths below ocean surface",
    "bottom of ocean mixed layer", "bottom of ocean isothermal layer",
    "layer ocean surface and 26C ocean isothermal level", "ocean mixed layer",
    "%g in sequence", "convective cloud bottom level", "convective cloud top level",
    "convective cloud layer", "lowest level of the wet bulb zero",
    "maximum equivalent potential temperature level", "equilibrium level",
    "shallow convective cloud bottom level", "shallow convective cloud top level",
    "reserved", "deep convective cloud bottom level", "deep convective cloud top level",
    "lowest bottom level of supercooled liquid water layer",
    "highest top level of supercooled liquid water layer", "missing",
]

KMA_LEVEL_TABLE = list(NCEP_LEVEL_TABLE)
KMA_LEVEL_TABLE[195 - 192 : 200 - 192] = [
    "hybrid sigma pressure", "Height from ground KFT", "canopy",
    "ICAO convective cloud top level", "ICAO convective cloud base level",
]
KMA_LEVEL_TABLE[216 - 192 : 219 - 192] = ["reserved", "reserved", "reserved"]


def sign_magnitude_byte(b: int) -> int:
    """Decode a GRIB2 sign-and-magnitude byte (high bit = sign)."""
    return -(b & 0x7F) if b & 0x80 else b


def sign_magnitude_int32(data: bytes) -> int:
    """Decode a 4-byte GRIB2 sign-and-magnitude integer (high bit = sign)."""
    if data[0] & 0x80:
        return -(((data[0] & 0x7F) << 24) | (data[1] << 16) | (data[2] << 8) | data[3])
    return (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3]


def decode_surface(type_byte: int, scale_byte: int, value_bytes: bytes) -> Tuple[int, float]:
    """Decode a Section 4 fixed-surface triple (type, scale factor, scaled value)."""
    if type_byte == 255 or scale_byte == 255 or value_bytes == b"\xff\xff\xff\xff":
        return type_byte, UNDEFINED
    scale_factor = sign_magnitude_byte(scale_byte)
    raw_value = sign_magnitude_int32(value_bytes)
    value = float(raw_value) if scale_factor == 0 else raw_value * (10.0 ** -scale_factor)
    return type_byte, value


def format_level1(type_: int, val: float, center: Optional[int]) -> Optional[str]:
    """Format single fixed surface matching wgrib2's level1()."""
    if type_ < 192:
        v = val * 0.01 if type_ in (100, 108) else val
        text = LEVEL_TABLE[type_]
        return text % v if "%" in text else text
    if type_ == 255:
        return None
    table = NCEP_LEVEL_TABLE if center == NCEP else (KMA_LEVEL_TABLE if center == KMA else None)
    if table is not None:
        v = val * 0.1 if type_ == 235 else val
        text = table[type_ - 192]
        return text % v if "%" in text else text
    if val != UNDEFINED:
        return "local level type %d %g" % (type_, val)
    return "local level type %d" % type_


def format_surface(type1: int, val1: float, type2: int, val2: float, center: Optional[int]) -> str:
    """Format the fixed surface(s) as a level or layer matching wgrib2's level2()."""
    if type1 == 100 and type2 == 100:
        return "%g-%g mb" % (val1 / 100, val2 / 100)
    if type1 == 102 and type2 == 102:
        return "%g-%g m above mean sea level" % (val1, val2)
    if type1 == 103 and type2 == 103:
        return "%g-%g m above ground" % (val1, val2)
    if type1 == 104 and type2 == 104:
        return "%g-%g sigma layer" % (val1, val2)
    if type1 == 105 and type2 == 105:
        return "%g-%g hybrid layer" % (val1, val2)
    if type1 == 106 and type2 == 106:
        return "%g-%g m below ground" % (val1, val2)
    if type1 == 107 and type2 == 107:
        return "%g-%g K isentropic layer" % (val1, val2)
    if type1 == 108 and type2 == 108:
        return "%g-%g mb above ground" % (val1 / 100, val2 / 100)
    if type1 == 111 and type2 == 111:
        return "%g-%g Eta layer" % (val1, val2)
    if type1 == 115 and type2 == 115:
        return "%g-%g sigma height layer" % (val1, val2)
    if type1 == 118 and type2 == 118:
        return "%g-%g hybrid height layer" % (val1, val2)
    if type1 == 119 and type2 == 119:
        return "%g-%g hybrid pressure layer" % (val1, val2)
    if type1 == 150 and type2 == 150:
        return "%g-%g generalized vertical height coordinate" % (val1, val2)
    if type1 == 160 and type2 == 160:
        return "%g-%g m below sea level" % (val1, val2)
    if type1 == 161 and type2 == 161:
        return "%g-%g m ocean layer" % (val1, val2)
    if type1 == 1 and type2 == 8:
        return "atmos col"
    if type1 == 9 and type2 == 1:
        return "ocean column"
    if center == NCEP and type1 == 235 and type2 == 235:
        return "%g-%gC ocean isotherm layer" % (val1 / 10, val2 / 10)
    if center == NCEP and type1 == 236 and type2 == 236:
        return "%g-%g m ocean layer" % (val1 * 10, val2 * 10)
    if type1 == 255 and type2 == 255:
        return "no_level"
    text = format_level1(type1, val1, center) or ""
    if type2 != 255:
        text2 = format_level1(type2, val2, center)
        if text2:
            text += " - " + text2
    return text


class FileReader:
    """Reads byte ranges from a local file without moving file offset."""

    def __init__(self, path: str):
        self._fd = os.open(path, os.O_RDONLY)
        self.reads = 0
        self.bytes_read = 0

    async def read_at(self, offset: int, size: int) -> bytes:
        buf = await asyncio.to_thread(os.pread, self._fd, size, offset)
        self.reads += 1
        self.bytes_read += len(buf)
        return buf

    async def close(self):
        os.close(self._fd)


class HttpRangeReader:
    """Reads byte ranges from a URL via HTTP Range requests."""

    def __init__(self, url: str):
        import aiohttp

        self.url = url
        self.reads = 0
        self.bytes_read = 0
        self.bytes_downloaded = 0
        self._warned_no_range = False
        self.session = aiohttp.ClientSession(
            connector=aiohttp.TCPConnector(limit=CONCURRENCY),
            timeout=aiohttp.ClientTimeout(total=HTTP_TIMEOUT),
        )

    async def read_at(self, offset: int, size: int) -> bytes:
        end = offset + size - 1
        headers = {"Range": f"bytes={offset}-{end}"}
        async with self.session.get(self.url, headers=headers) as resp:
            resp.raise_for_status()
            content = await resp.read()
            status = resp.status
        self.reads += 1
        self.bytes_downloaded += len(content)
        if status != 206:
            if not self._warned_no_range:
                self._warned_no_range = True
                logger.warning("Server did not honor Range request for %s", self.url)
            content = content[offset : offset + size]
        self.bytes_read += len(content)
        return content

    async def close(self):
        await self.session.close()


def open_reader(path: str):
    if path.startswith("http://") or path.startswith("https://"):
        return HttpRangeReader(path)
    return FileReader(path)


def find_message_start(buf: bytes) -> Optional[int]:
    """Return the offset within buf of first valid GRIB2 magic, or None."""
    pos = 0
    while True:
        idx = buf.find(b"GRIB", pos)
        if idx == -1 or idx + 16 > len(buf):
            return None
        if buf[idx + 7] == 2:  # GRIB edition number must be 2
            return idx
        pos = idx + 1


def parse_message(buf: bytes, idx: int, file_offset: int) -> Dict[str, Any]:
    """Parse Section 0, 1, and 4 header fields for GRIB2 message at buf[idx:]."""
    discipline = buf[idx + 6]
    msg = {
        "offset": file_offset,
        "total_length": int.from_bytes(buf[idx + 8 : idx + 16], "big"),
        "d": None,
        "param": None,
        "param_abbrev": None,
        "surface": None,
        "member": None,
    }

    ref_dt = None
    center = None
    pos = idx + 16  # end of Section 0
    while pos + 5 <= len(buf):
        if buf[pos : pos + 4] == b"7777":
            break
        sec_len = int.from_bytes(buf[pos : pos + 4], "big")
        if sec_len <= 0:
            break
        sec_num = buf[pos + 4]
        sec = buf[pos : pos + min(sec_len, len(buf) - pos)]

        if sec_num == 1 and len(sec) >= 17:
            center = int.from_bytes(sec[5:7], "big")
            year = int.from_bytes(sec[12:14], "big")
            month, day, hour = sec[14], sec[15], sec[16]
            ref_dt = datetime.datetime(year, month, day, hour)
        elif sec_num == 4:
            if len(sec) >= 28 and ref_dt is not None:
                cat, num = sec[9], sec[10]
                msg["param"] = format_param(discipline, cat, num)
                entry = PARAMETERS.get((discipline, cat, num))
                msg["param_abbrev"] = entry[0] if entry else f"{discipline}:{cat}:{num}"

                product_def_template = int.from_bytes(sec[7:9], "big")
                time_unit = sec[17]
                fcst_time = int.from_bytes(sec[18:22], "big")
                end_dt = None
                if product_def_template == 8 and len(sec) >= 41:
                    end_dt = datetime.datetime(
                        int.from_bytes(sec[34:36], "big"),
                        sec[36],
                        sec[37],
                        sec[38],
                        sec[39],
                        sec[40],
                    )
                msg["d"] = format_forecast_datetime(ref_dt, time_unit, fcst_time, end_dt)
                type1, val1 = decode_surface(sec[22], sec[23], sec[24:28])
                if len(sec) >= 34:
                    type2, val2 = decode_surface(sec[28], sec[29], sec[30:34])
                else:
                    type2, val2 = 255, UNDEFINED
                msg["surface"] = format_surface(type1, val1, type2, val2, center)
            break

        pos += sec_len

    return msg


async def scan_range(reader, start: int, end: Optional[int], chunk_size: int = CHUNK_SIZE, member_name: Optional[str] = None) -> List[Dict[str, Any]]:
    """Return a header dict for every GRIB2 message in byte range [start, end)."""
    messages = []
    pos = start
    while end is None or pos < end:
        want = chunk_size if end is None else min(chunk_size, end - pos)
        buf = await reader.read_at(pos, want)
        if not buf:
            break
        idx = find_message_start(buf)
        if idx is None:
            break
        msg = parse_message(buf, idx, pos + idx)
        if member_name:
            msg["member"] = member_name
        messages.append(msg)
        if msg["total_length"] <= 0:
            pos = pos + idx + 1
        else:
            pos = msg["offset"] + msg["total_length"]
    return messages


BLOCK_SIZE = 512
TAR_WINDOW = 3 * BLOCK_SIZE
TAR_METADATA_TYPES = b"xgLK"


def is_tar(buf: bytes) -> bool:
    """True if buf looks like a tar archive."""
    return len(buf) >= 262 and buf[257:262] == b"ustar"


def parse_tar_size(field: bytes) -> int:
    """Decode a tar header's 12-octet size field (octal or GNU base-256)."""
    if field[0] & 0x80:
        value = field[0] & 0x7F
        for byte in field[1:]:
            value = (value << 8) | byte
        return value
    text = field.rstrip(b"\0 ").lstrip(b" ")
    if not text:
        return 0
    return int(text, 8)


def round_up_blocks(size: int) -> int:
    """Round a member size up to a whole number of 512-byte tar blocks."""
    return (size + BLOCK_SIZE - 1) // BLOCK_SIZE * BLOCK_SIZE


async def iter_tar_members(reader, errors: list):
    """Yield (name, data_offset, size) for regular files in tar archive."""
    offset = 0
    while True:
        window = await reader.read_at(offset, TAR_WINDOW)
        if len(window) < BLOCK_SIZE:
            return
        pos = 0
        while pos + BLOCK_SIZE <= len(window):
            header = window[pos : pos + BLOCK_SIZE]
            if not header.strip(b"\0"):
                return
            if not is_tar(header):
                errors.append(f"bad tar header at offset {offset + pos}; stopping tar walk")
                return
            size = parse_tar_size(header[124:136])
            data_offset = offset + pos + BLOCK_SIZE
            next_offset = data_offset + round_up_blocks(size)
            type_flag = header[156]
            if type_flag in TAR_METADATA_TYPES:
                pos = next_offset - offset
                continue
            if type_flag in b"0\0":
                name = header[0:100].split(b"\0", 1)[0].decode("utf-8", "replace")
                yield name, data_offset, size
            offset = next_offset
            break
        else:
            offset += pos


async def dump_tar(reader) -> Tuple[List[Dict[str, Any]], bool]:
    """Scan every member of tar archive concurrently; return messages in archive order."""
    tasks = []
    errors = []
    try:
        async for name, data_offset, size in iter_tar_members(reader, errors):
            task = asyncio.create_task(
                scan_range(reader, data_offset, data_offset + size, member_name=name)
            )
            tasks.append((name, task))
    except Exception:
        for _, task in tasks:
            task.cancel()
        raise

    for error in errors:
        logger.error("%s", error)

    messages = []
    failed = bool(errors)
    results = await asyncio.gather(*(task for _, task in tasks), return_exceptions=True)
    for (name, _), result in zip(tasks, results):
        if isinstance(result, Exception):
            logger.error("Error reading member %s: %s", name, result)
            failed = True
            continue
        messages.extend(result)
    return messages, failed


async def scan_file_or_url(path_or_url: str) -> List[Dict[str, Any]]:
    """Scan headers for all GRIB2 messages in a local file or remote HTTP URL."""
    reader = open_reader(path_or_url)
    try:
        head = await reader.read_at(0, TAR_WINDOW)
        if is_tar(head):
            messages, _ = await dump_tar(reader)
        else:
            messages = await scan_range(reader, 0, None)
        return messages
    finally:
        await reader.close()


def filter_target_messages(
    messages: List[Dict[str, Any]],
    targets: Set[Tuple[str, str]] = TARGET_RECORDS,
) -> List[Dict[str, Any]]:
    """Filter parsed messages to retain only those matching (param, surface) target pairs."""
    matched = []
    for msg in messages:
        abbrev = msg.get("param_abbrev")
        surface = msg.get("surface")
        if (abbrev, surface) in targets:
            matched.append(msg)
    return matched


FIELDS = (
    ("offset", "offset"),
    ("length", "total_length"),
    ("d", "d"),
    ("param", "param"),
    ("surface", "surface"),
)


def format_record(index: int, msg: Dict[str, Any], include_member: bool = False) -> str:
    """Format single index record matching example-scripts/grib2_header_dump_parallel.py."""
    parts = ["%s=%s" % (label, msg.get(key, "")) for label, key in FIELDS]
    if include_member and msg.get("member"):
        parts.append(f"member={msg['member']}")
    return f"{index}:{':'.join(parts)}"


def write_index_file(
    messages: List[Dict[str, Any]],
    output_path: Path,
    include_member: bool = False,
) -> int:
    """Write filtered messages to index file. Returns number of records written."""
    output_path = Path(output_path)
    output_path.parent.mkdir(parents=True, exist_ok=True)
    temp_path = output_path.with_suffix(output_path.suffix + ".tmp")
    with open(temp_path, "w", encoding="utf-8") as f:
        for idx, msg in enumerate(messages, start=1):
            line = format_record(idx, msg, include_member=include_member)
            f.write(line + "\n")
    temp_path.replace(output_path)
    return len(messages)
