#!/usr/bin/env python3

import os
import shutil
import subprocess
import sys
from datetime import datetime
import json
from pathlib import Path

from numpy import sqrt

import pygrib

DATA_DIR = Path('/data')
TMP_DIR = DATA_DIR / 'tmp'

# bounds to create sliced versions for:
AREAS = json.load(open('bounds.json'))

# discover ggrib location
GGRIB_BIN = None
for location in ('/usr/local/bin/ggrib', './ggrib'):
    if os.path.isfile(location) and os.access(location, os.X_OK):
        GGRIB_BIN = location


def convert(tmpdirname):
    tmp_dir = Path(tmpdirname)
    files = sorted(tmp_dir.glob('**/*_GB'))

    if len(files) != 49:
        print(f'Verkeerd aantal GRIB-bestanden in {tmp_dir}, exiting')
        #sys.exit(1)

    tmp_grib = tmp_dir / 'temp.grb'
    tmp_grib_wind = tmp_dir / 'temp_wind.grb'

    with tmp_grib.open('wb') as gribout, tmp_grib_wind.open('wb') as gribout_wind:
        def writeGribMessage(message, wind=False):
            message['generatingProcessIdentifier'] = 96
            message['centre'] = 'kwbc'
            message['subCentre'] = 0
            message['table2Version'] = 1

            gribout.write(message.tostring())
            if wind:
                gribout_wind.write(message.tostring())

        for filename in files:
            with pygrib.open(str(filename)) as grbs:
                # Mean sea level pressure
                msg_mslp = grbs.select(indicatorOfParameter=1, typeOfLevel='heightAboveSea')[0]
                msg_mslp.indicatorOfParameter = 2
                msg_mslp.indicatorOfTypeOfLevel = 'sfc'
                msg_mslp.typeOfLevel = 'meanSea'
                writeGribMessage(msg_mslp)

                # Relative humidity
                msg_rh = grbs.select(indicatorOfParameter=52, level=2)[0]
                msg_rh.values = msg_rh.values * 100
                writeGribMessage(msg_rh)

                # Temperature 2m
                msg_t = grbs.select(indicatorOfParameter=11, level=2)[0]
                writeGribMessage(msg_t)

                # U-wind
                msg_u = grbs.select(indicatorOfParameter=33, level=10)[0]
                writeGribMessage(msg_u, wind=True)

                # V-wind
                msg_v = grbs.select(indicatorOfParameter=34, level=10)[0]
                writeGribMessage(msg_v, wind=True)

                # Precipication Intensity
                msg_ip = grbs.select(indicatorOfParameter=181, level=0, stepType='instant')[0]
                msg_ip.indicatorOfParameter = 61
                msg_ip.typeOfLevel = 'surface'
                #msg_ip.level = 0
                msg_ip.values = msg_ip.values * 3600  # mm/s => mm/h
                if msg_ip['P2'] > 0:
                    msg_ip['P1'] = msg_ip['P2'] - 1
                writeGribMessage(msg_ip)

                # Wind gusts
                msg_ug = grbs.select(indicatorOfParameter=162, level=10)[0]
                msg_vg = grbs.select(indicatorOfParameter=163, level=10)[0]
                msg_ug.values = sqrt(msg_ug.values ** 2 + msg_vg.values ** 2)
                msg_ug.indicatorOfParameter = 180
                msg_ug.typeOfLevel = 'surface'
                msg_ug.level = 0
                if msg_ug['P2'] > 0:
                    msg_ug['P1'] = msg_ug['P2'] - 1
                writeGribMessage(msg_ug, wind=True)

                #os.remove(filename)


    run_time = str(files[0])[-21:-11]
    run_time_date = datetime.strptime(run_time, '%Y%m%d%H').strftime('%Y-%m-%d_%H')
    print(run_time_date)

    dst_dir = DATA_DIR / f'{run_time_date}'
    dst_dir.mkdir(exist_ok=True)

    filename_fmt = f'harmonie_xy_{run_time_date}_{{}}.grb'

    def bounded_slice(src, dst_dir, name, bounds):
        dst = dst_dir / filename_fmt.format(name)
        print(f'Writing {name} to {dst}')

        cmd = [GGRIB_BIN, src, dst]
        cmd.extend(map(str, bounds[0] + bounds[1]))
        subprocess.call(cmd)

    if GGRIB_BIN is None:
        print('ggrib binary not found, please install by typing `make ggrib`')
    else:
        for area in AREAS:
            bounded_slice(tmp_grib, dst_dir, area['abbr'], area['bounds'])
            bounded_slice(tmp_grib_wind, dst_dir, area['abbr'] + '_wind', area['bounds'])

    shutil.move(tmp_grib, dst_dir / f'harmonie_xy_{run_time_date}.grb')
    shutil.move(tmp_grib_wind, dst_dir / filename_fmt.format('wind'))

    (DATA_DIR / 'new').symlink_to(dst_dir.relative_to(DATA_DIR))
    (DATA_DIR / 'new').rename(DATA_DIR / 'latest')

    # delete old files
    for dir in DATA_DIR.glob('*'):
        if dir not in (dst_dir, DATA_DIR / 'latest'):
            print(f'removing {dir}')
            shutil.rmtree(dir)

if __name__ == "__main__":
    convert(TMP_DIR)
