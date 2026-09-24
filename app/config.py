"""Configuration and environment management for Harmonie 46 GRIB indexer."""

import os
from pathlib import Path
import uuid


def load_dotenv(dotenv_path=".env"):
    """Load environment variables from a .env file without external dependencies."""
    path = Path(dotenv_path)
    if not path.is_file():
        return
    try:
        with open(path, "r", encoding="utf-8") as f:
            for line in f:
                line = line.strip()
                if not line or line.startswith("#") or "=" not in line:
                    continue
                key, val = line.split("=", 1)
                key = key.strip()
                val = val.strip().strip('"').strip("'")
                if key and key not in os.environ:
                    os.environ[key] = val
    except Exception:
        pass


# Ensure environment variables are loaded
load_dotenv()

# Dataset settings
DATASET_NAME = os.environ.get("DATASET_NAME", "harmonie_arome_cy46_p1")
DATASET_VERSION = os.environ.get("DATASET_VERSION", "1.0")

# MQTT Notification Service settings
MQTT_BROKER = os.environ.get("MQTT_BROKER", "mqtt.dataplatform.knmi.nl")
MQTT_PORT = int(os.environ.get("MQTT_PORT", "443"))
MQTT_TRANSPORT = os.environ.get("MQTT_TRANSPORT", "websockets")
MQTT_TOPIC = os.environ.get(
    "MQTT_TOPIC", f"dataplatform/file/v1/{DATASET_NAME}/{DATASET_VERSION}/#"
)
KNMI_NOTIFICATION_SERVICE_API_KEY = os.environ.get(
    "KNMI_NOTIFICATION_SERVICE_API_KEY", ""
)

# Open Data API settings
OPEN_DATA_BASE_URL = os.environ.get(
    "OPEN_DATA_BASE_URL", "https://api.dataplatform.knmi.nl/open-data/v1"
)
KNMI_OPEN_DATA_API_KEY = os.environ.get("KNMI_OPEN_DATA_API_KEY", "")

# Output directories
DATA_DIR = Path(os.environ.get("DATA_DIR", "data"))
DATA_DIR.mkdir(parents=True, exist_ok=True)


def get_client_id() -> str:
    """Return persistent client ID or generate a new one stored in data directory."""
    env_id = os.environ.get("MQTT_CLIENT_ID")
    if env_id:
        return env_id
    client_id_file = DATA_DIR / ".mqtt_client_id"
    if client_id_file.exists():
        try:
            stored_id = client_id_file.read_text(encoding="utf-8").strip()
            if stored_id:
                return stored_id
        except Exception:
            pass
    new_id = str(uuid.uuid4())
    try:
        client_id_file.write_text(new_id, encoding="utf-8")
    except Exception:
        pass
    return new_id


MQTT_CLIENT_ID = get_client_id()

# Performance settings
CONCURRENCY = int(os.environ.get("CONCURRENCY", "64"))
CHUNK_SIZE = int(os.environ.get("CHUNK_SIZE", "2048"))
HTTP_TIMEOUT = int(os.environ.get("HTTP_TIMEOUT", "30"))

# Target (param, surface) tuples to include in index
TARGET_RECORDS = {
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

# Wind subset tuples
WIND_RECORDS = {
    ("UGRD", "10 m above ground"),
    ("VGRD", "10 m above ground"),
    ("VGUST", "10 m above ground"),
    ("UGUST", "10 m above ground"),
}

# Subsetting and output configuration
BOUNDS_FILE = Path(os.environ.get("BOUNDS_FILE", "app/bounds.json"))
GRIB_PREFIX = os.environ.get("GRIB_PREFIX", "harmonie_xy")
GRIB_EXT = os.environ.get("GRIB_EXT", ".grb2")
RETENTION_RUNS = int(os.environ.get("RETENTION_RUNS", "1"))
WGRIB2_BIN = os.environ.get("WGRIB2_BIN", "/usr/local/bin/wgrib2")
DOWNLOAD_CONCURRENCY = int(os.environ.get("DOWNLOAD_CONCURRENCY", "32"))
TMP_DIR = Path(os.environ.get("TMP_DIR", str(DATA_DIR / "tmp")))
TMP_DIR.mkdir(parents=True, exist_ok=True)

