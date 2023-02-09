#!/usr/bin/env python3

import os
import shutil
import sys
from datetime import datetime
from pathlib import Path
import pygrib

DATA_DIR = Path('/data')
TMP_DIR = DATA_DIR / 'tmp'

DATASET_PRODUCT = int(os.getenv('data_product'))
HOUR_MAX = int(os.getenv('hour_max'))

# specify filters
type_of_level = str(os.getenv('type_of_level'))
value_of_level = float(os.getenv('level_value'))


def convert(tmpdirname):
    tmp_dir = Path(tmpdirname)
    files = sorted(tmp_dir.glob('*_GB'))

    if len(files) != (HOUR_MAX + 1):
        print(f'Verkeerd aantal GRIB-bestanden in {tmp_dir}, exiting')
        sys.exit(1)

    tmp_grib_wind = tmp_dir / 'temp_wind.grb'

    with tmp_grib_wind.open('wb') as gribout_wind:
        def writeGribMessage(message, wind=False):
            message['generatingProcessIdentifier'] = 96

            # https://apps.ecmwf.int/codes/grib/format/grib1/centre/0/
            message['centre'] = 'kwbc'

            # TODO: include information on rotated lat/lon for DP2?
            # https://apps.ecmwf.int/codes/grib/format/grib1/ctable/6/
            # https://apps.ecmwf.int/codes/grib/format/grib2/templates/3/1
            #
            # N.B. DP2 is GRIB1 (https://www.knmidata.nl/data-services/knmi-producten-overzicht/atmosfeer-modeldata/data-product-2)

            if wind:
                gribout_wind.write(message.tostring())

        for filename in files:
            with pygrib.open(str(filename)) as grbs:
                # U-wind
                msg_u = grbs.select(indicatorOfParameter=33,
                                    typeOfLevel=type_of_level,
                                    level=value_of_level,
                                    )[0]
                writeGribMessage(msg_u, wind=True)

                # V-wind
                msg_v = grbs.select(indicatorOfParameter=34,
                                    typeOfLevel=type_of_level,
                                    level=value_of_level,
                                    )[0]
                writeGribMessage(msg_v, wind=True)

    run_time = str(files[0])[-21:-11]
    run_time_date = datetime.strptime(run_time, '%Y%m%d%H').strftime('%Y-%m-%d_%H')
    print(f'Run time date: {run_time_date}')

    dst_dir = DATA_DIR / f'{run_time_date}'
    dst_dir.mkdir(exist_ok=True)

    filename_fmt = f'harmonie_xy_dp{DATASET_PRODUCT}_' \
                   f'{run_time_date}_wind_' \
                   f'{type_of_level}{int(value_of_level)}_' \
                   f'{HOUR_MAX}hours.grb'

    shutil.move(tmp_grib_wind, dst_dir / filename_fmt.format('wind'))

    (DATA_DIR / 'new').symlink_to(dst_dir.relative_to(DATA_DIR))
    (DATA_DIR / 'new').rename(DATA_DIR / 'latest')

    # delete old files
    for path in DATA_DIR.glob('*'):
        if path.is_dir() and path not in (dst_dir, DATA_DIR / 'latest'):
            print(f'removing {path}')
            shutil.rmtree(path)


if __name__ == "__main__":
    convert(TMP_DIR)
