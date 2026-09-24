"""Unit tests for app.subsetter module."""

from pathlib import Path
import unittest

from app.subsetter import build_subset_args, load_areas


class TestSubsetter(unittest.TestCase):
    def test_load_areas(self):
        areas = load_areas("app/bounds.json")
        self.assertEqual(len(areas), 7)
        abbrs = [a["abbr"] for a in areas]
        self.assertIn("ijmg", abbrs)
        self.assertIn("wad", abbrs)
        self.assertIn("zld", abbrs)
        self.assertIn("nl", abbrs)
        self.assertIn("nz", abbrs)
        self.assertIn("db", abbrs)
        self.assertIn("dov", abbrs)

    def test_build_subset_args(self):
        areas = [
            {"abbr": "ijmg", "name": "IJsselmeer", "bounds": [[4.107, 52.082], [5.994, 53.416]]},
            {"abbr": "wad", "name": "Waddenzee", "bounds": [[4.218, 52.795], [7.548, 53.876]]},
        ]
        out_dir = Path("/tmp/test_subsets")
        args, files = build_subset_args(
            areas=areas,
            output_dir=out_dir,
            run_time="2026-09-24_13",
            is_wind=False,
            prefix="harmonie_xy",
            ext=".grb2",
        )

        self.assertEqual(len(files), 2)
        self.assertEqual(files[0], out_dir / "harmonie_xy_2026-09-24_13_ijmg.grb2")
        self.assertEqual(files[1], out_dir / "harmonie_xy_2026-09-24_13_wad.grb2")

        # Check wgrib2 arg syntax: -small_grib LonW:LonE LatS:LatN <file>
        self.assertEqual(args[0], "-small_grib")
        self.assertEqual(args[1], "4.107:5.994")
        self.assertEqual(args[2], "52.082:53.416")
        self.assertEqual(args[3], str(files[0]))

        self.assertEqual(args[4], "-small_grib")
        self.assertEqual(args[5], "4.218:7.548")
        self.assertEqual(args[6], "52.795:53.876")
        self.assertEqual(args[7], str(files[1]))

    def test_build_subset_args_wind(self):
        areas = [
            {"abbr": "ijmg", "name": "IJsselmeer", "bounds": [[4.107, 52.082], [5.994, 53.416]]},
        ]
        out_dir = Path("/tmp/test_subsets")
        args, files = build_subset_args(
            areas=areas,
            output_dir=out_dir,
            run_time="2026-09-24_13",
            is_wind=True,
            prefix="harmonie_xy",
            ext=".grb2",
        )
        self.assertEqual(files[0].name, "harmonie_xy_2026-09-24_13_ijmg_wind.grb2")


if __name__ == "__main__":
    unittest.main()
