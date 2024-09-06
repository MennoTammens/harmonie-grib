#!/usr/bin/env python3

import os
import json
from urllib.request import urlopen, Request
from datetime import datetime, timedelta
import tarfile
import tempfile
from pathlib import Path
import convert

API_URL = "https://api.dataplatform.knmi.nl/open-data"
API_KEY = os.getenv('KNMI_API_KEY')

DATASET_ID = int(os.getenv("DATASET_ID"))
DATASET_PRODUCT = int(os.getenv("DATASET_PRODUCT"))
DATASET_VERSION = os.getenv("DATASET_VERSION")
DATASET_NAME = f"harmonie_arome_cy{DATASET_ID}_p{DATASET_PRODUCT}"

HOUR_MAX = int(os.getenv('HOUR_MAX'))

DATA_DIR = Path(f'/data/dp{DATASET_PRODUCT}')


def file_list():
    yesterday = (datetime.now() - timedelta(days=1)).strftime("%Y%m%d")
    yesterday = "20240513"

    req = Request(
        f"{API_URL}/datasets/{DATASET_NAME}/versions/{DATASET_VERSION}/files?"
        f"startAfterFilename=HARM43_V1_P{DATASET_PRODUCT}_{yesterday}00.tar",
        headers={"Authorization": API_KEY}
    )
    with urlopen(req) as list_files_response:
        files = json.load(list_files_response).get("files")

    return files


def get_file(filename, tmpdirname, hour_max: int):
    req = Request(
        f"{API_URL}/datasets/{DATASET_NAME}/versions/{DATASET_VERSION}/files/{filename}/url",
        headers={"Authorization": API_KEY}
    )
    with urlopen(req) as get_file_response:
        url = json.load(get_file_response).get("temporaryDownloadUrl")

    print(f'Downloading {filename}')
    with urlopen(url) as remote:
        with tarfile.open(fileobj=remote, mode='r|*') as tar:
            for tarinfo in tar:
                hour_idx = tarinfo.name.split('_')[-2]
                hour_idx = int(int(hour_idx) / 100)

                if hour_idx <= hour_max:
                    print(tarinfo.name, flush=True)
                    tar.extract(tarinfo, tmpdirname)
                else:
                    break

    print(f'Downloaded {filename} for first {hour_max} hours of forecast'
          f' ({hour_max+1} files)', flush=True)


def cron():
    latest = file_list()[-1]
    filename = latest["filename"]
    if "HARM43" in filename:
        # Harmonie cy43 does include an additional subfolder-layer.
        subfolder = None
    else:
        subfolder = filename.strip('.tar')
    run_time = filename[-14:-4]
    run_time_date = datetime.strptime(run_time, '%Y%m%d%H').strftime('%Y-%m-%d_%H')
    now = datetime.now()

    os.makedirs(DATA_DIR, exist_ok=True)
    print(DATA_DIR / run_time_date)
    if (DATA_DIR / run_time_date).exists():
        print(f"[{now}] Skipping download, {filename} already downloaded.")
    else:
        with tempfile.TemporaryDirectory() as tmpdirname:
            get_file(filename, tmpdirname, HOUR_MAX)
            convert.convert(tmpdirname, subfolder)
        print(f'[{now}] Done with downloading and processing data.')


if __name__ == "__main__":
    cron()
