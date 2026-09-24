"""Unit tests for app.grib_reader module."""

import datetime
from pathlib import Path
import tempfile
import unittest

from app.grib_reader import (
    decode_surface,
    filter_target_messages,
    find_message_start,
    format_forecast_datetime,
    format_level1,
    format_param,
    format_record,
    format_surface,
    is_tar,
    parse_message,
    parse_tar_size,
    round_up_blocks,
    sign_magnitude_byte,
    sign_magnitude_int32,
    write_index_file,
)


class TestGribReader(unittest.TestCase):
    def test_sign_magnitude(self):
        self.assertEqual(sign_magnitude_byte(0x05), 5)
        self.assertEqual(sign_magnitude_byte(0x85), -5)
        self.assertEqual(sign_magnitude_int32(bytes([0x00, 0x00, 0x00, 0x0A])), 10)
        self.assertEqual(sign_magnitude_int32(bytes([0x80, 0x00, 0x00, 0x0A])), -10)

    def test_decode_surface(self):
        # 10 m above ground (scale=0, val=10)
        t, v = decode_surface(103, 0, bytes([0, 0, 0, 10]))
        self.assertEqual(t, 103)
        self.assertEqual(v, 10.0)

        # 2 m above ground (scale=1, val=20 -> 2.0)
        t, v = decode_surface(103, 1, bytes([0, 0, 0, 20]))
        self.assertEqual(t, 103)
        self.assertEqual(v, 2.0)

        # undefined / missing
        t, v = decode_surface(255, 0, bytes([0, 0, 0, 0]))
        self.assertEqual(t, 255)

    def test_format_surface(self):
        self.assertEqual(format_surface(1, 0, 255, 0, None), "surface")
        self.assertEqual(format_surface(101, 0, 255, 0, None), "mean sea level")
        self.assertEqual(format_surface(103, 10, 255, 0, None), "10 m above ground")
        self.assertEqual(format_surface(103, 2, 255, 0, None), "2 m above ground")
        self.assertEqual(format_surface(103, 0, 255, 0, None), "0 m above ground")

    def test_format_param(self):
        self.assertEqual(format_param(0, 0, 0), "TMP Temperature (K)")
        self.assertEqual(format_param(0, 1, 52), "TPRATE Total Precipitation Rate (kg m-2 s-1)")
        self.assertEqual(format_param(0, 2, 2), "UGRD U-Component of Wind (m s-1)")
        self.assertEqual(format_param(0, 19, 0), "VIS Visibility (m)")

    def test_format_forecast_datetime(self):
        ref = datetime.datetime(2026, 9, 24, 12, 0)
        # 3 hours forecast
        res = format_forecast_datetime(ref, time_unit=1, fcst_time=3, end_dt=None)
        self.assertEqual(res, "2026-09-24T1200Z+003")

        # with end datetime
        end = datetime.datetime(2026, 9, 24, 15, 0)
        res2 = format_forecast_datetime(ref, time_unit=1, fcst_time=0, end_dt=end)
        self.assertEqual(res2, "2026-09-24T1200Z+000-003")

    def test_find_message_start(self):
        # Section 0 is 16 bytes long: "GRIB" (4) + discipline (1) + edition (1) + total_len (8) + 2 reserved = 16
        buf = b"junk padding... " + b"GRIB\x00\x00\x00\x02" + b"\x00" * 8
        idx = find_message_start(buf)
        self.assertEqual(idx, 16)
        self.assertIsNone(find_message_start(b"random bytes without magic"))

    def test_tar_helpers(self):
        self.assertEqual(round_up_blocks(500), 512)
        self.assertEqual(round_up_blocks(512), 512)
        self.assertEqual(round_up_blocks(513), 1024)

        # Tar size octal
        self.assertEqual(parse_tar_size(b"00000001000 \x00"), 512)

        # Not tar
        self.assertFalse(is_tar(b"\x00" * 300))

    def test_filter_target_messages(self):
        messages = [
            {"param_abbrev": "TMP", "surface": "2 m above ground", "total_length": 100},
            {"param_abbrev": "TMP", "surface": "850 mb", "total_length": 100},  # should be excluded
            {"param_abbrev": "TPRATE", "surface": "surface", "total_length": 200},
            {"param_abbrev": "PRES", "surface": "mean sea level", "total_length": 300},
            {"param_abbrev": "UGRD", "surface": "10 m above ground", "total_length": 400},
            {"param_abbrev": "VGRD", "surface": "10 m above ground", "total_length": 500},
            {"param_abbrev": "VGUST", "surface": "10 m above ground", "total_length": 600},
            {"param_abbrev": "UGUST", "surface": "10 m above ground", "total_length": 700},
            {"param_abbrev": "RH", "surface": "2 m above ground", "total_length": 800},
            {"param_abbrev": "VIS", "surface": "0 m above ground", "total_length": 900},
            {"param_abbrev": "LAND", "surface": "surface", "total_length": 1000},  # should be excluded
        ]
        filtered = filter_target_messages(messages)
        self.assertEqual(len(filtered), 9)
        abbrevs = {m["param_abbrev"] for m in filtered}
        self.assertNotIn("LAND", abbrevs)
        # Ensure TMP at 850 mb was excluded
        tmp_surfaces = [m["surface"] for m in filtered if m["param_abbrev"] == "TMP"]
        self.assertEqual(tmp_surfaces, ["2 m above ground"])

    def test_write_index_file(self):
        msgs = [
            {
                "offset": 1024,
                "total_length": 45000,
                "d": "2026-09-24T1200Z+000",
                "param": "TMP Temperature (K)",
                "surface": "2 m above ground",
            },
            {
                "offset": 50000,
                "total_length": 25000,
                "d": "2026-09-24T1200Z+000",
                "param": "RH Relative Humidity (%)",
                "surface": "2 m above ground",
            },
        ]
        with tempfile.TemporaryDirectory() as tmpdir:
            out_file = Path(tmpdir) / "test.idx"
            count = write_index_file(msgs, out_file)
            self.assertEqual(count, 2)
            self.assertTrue(out_file.exists())
            lines = out_file.read_text().splitlines()
            self.assertEqual(len(lines), 2)
            self.assertTrue(lines[0].startswith("1:offset=1024:length=45000:d=2026-09-24T1200Z+000:param=TMP Temperature (K):surface=2 m above ground"))


if __name__ == "__main__":
    unittest.main()
