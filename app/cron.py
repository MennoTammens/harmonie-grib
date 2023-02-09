#!/usr/bin/env python3

import os
import json
from urllib.request import urlopen, Request
from datetime import datetime, timedelta
import tarfile
import tempfile
from pathlib import Path
import convert


DATA_DIR = Path('/data')
TMP_DIR = DATA_DIR / 'tmp'

API_URL = "https://api.dataplatform.knmi.nl/open-data"
API_KEY = os.getenv('KNMI_API_KEY')

DATASET_PRODUCT = int(os.getenv('data_product'))
DATASET_NAME = f"harmonie_arome_cy40_p{DATASET_PRODUCT}"
DATASET_VERSION = "0.2"

HOUR_MAX = int(os.getenv('hour_max'))


def file_list():
    yesterday = (datetime.now() - timedelta(days=1)).strftime("%Y%m%d")

    req = Request(
        f"{API_URL}/datasets/{DATASET_NAME}/versions/{DATASET_VERSION}/files?"
        f"startAfterFilename=harm40_v1_p{DATASET_PRODUCT}_{yesterday}00.tar",
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
    run_time = filename[-14:-4]
    run_time_date = datetime.strptime(run_time, '%Y%m%d%H').strftime('%Y-%m-%d_%H')

    if (DATA_DIR / run_time_date).exists() or (TMP_DIR / f'HA40_N25_{run_time}00_00000_GB').exists():
        print(f"Skipping download, {filename} already downloaded")
    else:
        with tempfile.TemporaryDirectory() as tmpdirname:
            get_file(filename, tmpdirname, HOUR_MAX)
            convert.convert(tmpdirname)
    print('DONE')


if __name__ == "__main__":
    cron()
