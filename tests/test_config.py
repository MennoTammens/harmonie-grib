"""Unit tests for app.config module."""

import os
from pathlib import Path
import tempfile
import unittest

from app.config import (
    DATASET_NAME,
    DATASET_VERSION,
    MQTT_BROKER,
    MQTT_PORT,
    TARGET_RECORDS,
    load_dotenv,
)


class TestConfig(unittest.TestCase):
    def test_default_dataset_info(self):
        self.assertEqual(DATASET_NAME, "harmonie_arome_cy46_p1")
        self.assertEqual(DATASET_VERSION, "1.0")

    def test_default_mqtt_info(self):
        self.assertEqual(MQTT_BROKER, "mqtt.dataplatform.knmi.nl")
        self.assertEqual(MQTT_PORT, 443)

    def test_target_records_count_and_content(self):
        expected = {
            ("TPRATE", "surface"),
            ("PRES", "mean sea level"),
            ("UGRD", "10 m above ground"),
            ("VGRD", "10 m above ground"),
            ("VGUST", "10 m above ground"),
            ("UGUST", "10 m above ground"),
            ("RH", "2 m above ground"),
            ("TMP", "2 m above ground"),
            ("VIS", "0 m above ground"),
        }
        self.assertEqual(TARGET_RECORDS, expected)
        self.assertEqual(len(TARGET_RECORDS), 9)

    def test_wind_records_count_and_content(self):
        from app.config import WIND_RECORDS
        expected = {
            ("UGRD", "10 m above ground"),
            ("VGRD", "10 m above ground"),
            ("VGUST", "10 m above ground"),
            ("UGUST", "10 m above ground"),
        }
        self.assertEqual(WIND_RECORDS, expected)
        self.assertTrue(WIND_RECORDS.issubset(TARGET_RECORDS))

    def test_retention_runs_default(self):
        from app.config import RETENTION_RUNS
        self.assertEqual(RETENTION_RUNS, 1)

    def test_load_dotenv(self):
        with tempfile.NamedTemporaryFile("w+", delete=False) as f:
            f.write("# comment\nTEST_CUSTOM_VAR=my_custom_value\nQUOTED_VAR=\"hello_world\"\n")
            f_path = f.name
        try:
            load_dotenv(f_path)
            self.assertEqual(os.environ.get("TEST_CUSTOM_VAR"), "my_custom_value")
            self.assertEqual(os.environ.get("QUOTED_VAR"), "hello_world")
        finally:
            os.unlink(f_path)
            os.environ.pop("TEST_CUSTOM_VAR", None)
            os.environ.pop("QUOTED_VAR", None)


if __name__ == "__main__":
    unittest.main()
