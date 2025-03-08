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
DATASET_NAME = "harmonie_arome_cy43_p1"
DATASET_VERSION = "1.0"


def file_list():
    yesterday = (datetime.now() - timedelta(hours=8)).strftime("%Y%m%d%H")
    req = Request(
        f"{API_URL}/datasets/{DATASET_NAME}/versions/{DATASET_VERSION}/files?startAfterFilename=HARM43_V1_P1_{yesterday}.tar",
        headers={"Authorization": API_KEY}
    )
    with urlopen(req) as list_files_response:
        files = json.load(list_files_response).get("files")

    return files


def get_file(filename, tmpdirname):
    req = Request(
        f"{API_URL}/datasets/{DATASET_NAME}/versions/{DATASET_VERSION}/files/{filename}/url",
        headers={"Authorization": API_KEY}
    )
    with urlopen(req) as get_file_response:
        url = json.load(get_file_response).get("temporaryDownloadUrl")

    print(f'downloading {filename}')
    with urlopen(url) as remote:
        with tarfile.open(fileobj=remote, mode='r|*') as tar:
            for tarinfo in tar:
                print(tarinfo.name, flush=True)
                tar.extract(tarinfo, tmpdirname)
    print('downloaded '+filename, flush=True)


def cron():
    latest = file_list()[-1]
    filename = latest["filename"]
    run_time = filename[-14:-4]
    run_time_date = datetime.strptime(run_time, '%Y%m%d%H').strftime('%Y-%m-%d_%H')

    if (DATA_DIR / run_time_date).exists() or (TMP_DIR / f'HA40_N25_{run_time}00_00000_GB').exists():
        print(f"Skipping download, {filename} already downloaded")
    else:
        with tempfile.TemporaryDirectory() as tmpdirname:
            get_file(filename, tmpdirname)
            convert.convert(tmpdirname)
    print('DONE')


if __name__ == "__main__":
    cron()
