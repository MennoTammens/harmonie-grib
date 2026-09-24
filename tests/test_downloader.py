"""Unit tests for app.downloader module."""

import asyncio
from pathlib import Path
import tempfile
import unittest

from app.downloader import coalesce_ranges, download_filtered_gribs


class TestDownloader(unittest.TestCase):
    def test_coalesce_ranges(self):
        records = [
            {"offset": 1000, "total_length": 500, "param_abbrev": "TMP", "surface": "2 m above ground"},
            {"offset": 1500, "total_length": 500, "param_abbrev": "RH", "surface": "2 m above ground"},
            {"offset": 3000, "total_length": 400, "param_abbrev": "UGRD", "surface": "10 m above ground"},
            {"offset": 3400, "total_length": 400, "param_abbrev": "VGRD", "surface": "10 m above ground"},
            {"offset": 3800, "total_length": 200, "param_abbrev": "UGUST", "surface": "10 m above ground"},
            {"offset": 5000, "total_length": 100, "param_abbrev": "VIS", "surface": "0 m above ground"},
        ]

        coalesced = coalesce_ranges(records)
        # Should merge [1000, 1500] -> [1000, len=1000]
        # Should merge [3000, 3400, 3800] -> [3000, len=1000]
        # [5000, len=100] stays separate
        self.assertEqual(len(coalesced), 3)

        self.assertEqual(coalesced[0]["offset"], 1000)
        self.assertEqual(coalesced[0]["length"], 1000)
        self.assertEqual(len(coalesced[0]["messages"]), 2)

        self.assertEqual(coalesced[1]["offset"], 3000)
        self.assertEqual(coalesced[1]["length"], 1000)
        self.assertEqual(len(coalesced[1]["messages"]), 3)

        self.assertEqual(coalesced[2]["offset"], 5000)
        self.assertEqual(coalesced[2]["length"], 100)
        self.assertEqual(len(coalesced[2]["messages"]), 1)

    def test_coalesce_empty(self):
        self.assertEqual(coalesce_ranges([]), [])


if __name__ == "__main__":
    unittest.main()
