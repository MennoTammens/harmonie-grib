#!/usr/bin/env python3

import os
import shutil
import subprocess
import sys
from datetime import datetime
import json
from pathlib import Path
from math import atan2, degrees

from numpy import sqrt

import pygrib

DATA_DIR = Path('/data')
TMP_DIR = DATA_DIR / 'tmp'

# bounds to create sliced versions for:
BOUNDS = json.load(open('bounds.json'))

# location from meteogram
lat = float(os.getenv('METEOGRAM_LAT', 52.634))
lat = 49+round((lat-49)/0.023)*0.023
lon = float(os.getenv('METEOGRAM_LON', 5.402))
lon = round(lon/0.037)*0.037
lat1 = lat-0.001
lat2 = lat+0.001
lon1 = lon-0.001
lon2 = lon+0.001


# discover ggrib location
GGRIB_BIN = None
for location in ('/usr/local/bin/ggrib', './ggrib'):
    if os.path.isfile(location) and os.access(location, os.X_OK):
        GGRIB_BIN = location


def convert(tmpdirname):
    tmp_dir = Path(tmpdirname)
    files = sorted(tmp_dir.glob('*_GB'))

    if len(files) != 49:
        print(f'Verkeerd aantal GRIB-bestanden in {tmp_dir}, exiting')
        sys.exit(1)

    tmp_grib = tmp_dir / 'temp.grb'
    tmp_grib_wind = tmp_dir / 'temp_wind.grb'
    meteogram = []

    with tmp_grib.open('wb') as gribout, tmp_grib_wind.open('wb') as gribout_wind:
        def writeGribMessage(message, wind=False):
            message['generatingProcessIdentifier'] = 96
            message['centre'] = 'kwbc'

            gribout.write(message.tostring())
            if wind:
                gribout_wind.write(message.tostring())

        for filename in files:
            with pygrib.open(str(filename)) as grbs:
                # Mean sea level pressure
                msg_mslp = grbs.select(indicatorOfParameter=1)[0]
                msg_mslp.indicatorOfParameter = 2
                msg_mslp.indicatorOfTypeOfLevel = 'sfc'
                msg_mslp.typeOfLevel = 'meanSea'
                writeGribMessage(msg_mslp)
                analDate = msg_mslp.analDate.isoformat()
                validDate = datetime.strptime(f'{msg_mslp["validityDate"]}{msg_mslp["validityTime"]:04d}', '%Y%m%d%H%M').isoformat()
                [[mslp]], _, _ = msg_mslp.data(lat1=lat1,lat2=lat2,lon1=lon1,lon2=lon2)
                mslp = mslp / 100

                # Relative humidity
                msg_rh = grbs.select(indicatorOfParameter=52)[0]
                msg_rh.values = msg_rh.values * 100
                writeGribMessage(msg_rh)
                [[rh]], _, _ = msg_rh.data(lat1=lat1,lat2=lat2,lon1=lon1,lon2=lon2)

                # Temperature 2m
                msg_t = grbs.select(indicatorOfParameter=11)[0]
                writeGribMessage(msg_t)
                [[t]], _, _ = msg_t.data(lat1=lat1,lat2=lat2,lon1=lon1,lon2=lon2)
                t = t - 273.15

                # U-wind
                msg_u = grbs.select(indicatorOfParameter=33)[0]
                writeGribMessage(msg_u, wind=True)

                # V-wind
                msg_v = grbs.select(indicatorOfParameter=34)[0]
                writeGribMessage(msg_v, wind=True)
                [[u]], _, _ = msg_u.data(lat1=lat1,lat2=lat2,lon1=lon1,lon2=lon2)
                [[v]], _, _ = msg_v.data(lat1=lat1,lat2=lat2,lon1=lon1,lon2=lon2)
                wind_dir = (degrees(atan2(u, v))+180)%360
                wind_speed = sqrt(u**2 + v**2) * 3.6 / 1.852

                # Precipication Intensity
                msg_ip = grbs.select(indicatorOfParameter=181, level=0, stepType='instant')[0]
                msg_ip.indicatorOfParameter = 61
                msg_ip.typeOfLevel = 'surface'
                #msg_ip.level = 0
                msg_ip.values = msg_ip.values * 3600  # mm/s => mm/h
                if msg_ip['P2'] > 0:
                    msg_ip['P1'] = msg_ip['P2'] - 1
                writeGribMessage(msg_ip)
                [[ip]], _, _ = msg_ip.data(lat1=lat1,lat2=lat2,lon1=lon1,lon2=lon2)

                # Wind gusts
                msg_ug = grbs.select(indicatorOfParameter=162)[0]
                msg_vg = grbs.select(indicatorOfParameter=163)[0]
                msg_ug.values = sqrt(msg_ug.values ** 2 + msg_vg.values ** 2)
                msg_ug.indicatorOfParameter = 180
                msg_ug.typeOfLevel = 'surface'
                msg_ug.level = 0
                if msg_ug['P2'] > 0:
                    msg_ug['P1'] = msg_ug['P2'] - 1
                writeGribMessage(msg_ug, wind=True)
                [[ug]], _, _ = msg_ug.data(lat1=lat1,lat2=lat2,lon1=lon1,lon2=lon2)
                [[vg]], _, _ = msg_vg.data(lat1=lat1,lat2=lat2,lon1=lon1,lon2=lon2)
                gust_dir = (degrees(atan2(ug, vg))+180)%360
                gust_speed = sqrt(ug**2 + vg**2) * 3.6 / 1.852

                meteogram.append({
                    'analDate': analDate,
                    'validDate': validDate,
                    'mslp': round(float(mslp)),
                    'rh': round(float(rh)),
                    't': round(float(t), 1),
                    'wind_dir': round(float(wind_dir)),
                    'wind_speed': round(float(wind_speed),1),
                    'gust_dir': round(float(gust_dir)),
                    'gust_speed': round(float(gust_speed),1),
                    'ip': round(float(ip),2),
                })


    run_time = str(files[0])[-21:-11]
    run_time_date = datetime.strptime(run_time, '%Y%m%d%H').strftime('%Y-%m-%d_%H')
    print(run_time_date)

    dst_dir = DATA_DIR / f'{run_time_date}'
    dst_dir.mkdir(exist_ok=True)

    with open(dst_dir / 'meteogram.json', 'w') as fp:
        json.dump({'location': {'lat': lat, 'lon': lon}, 'meteogram': meteogram}, fp, indent=2)

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
        for area in BOUNDS:
            bounded_slice(tmp_grib, dst_dir, area['abbr'], area['bounds'])
            bounded_slice(tmp_grib_wind, dst_dir, area['abbr'] + '_wind', area['bounds'])

    shutil.move(tmp_grib, dst_dir / f'harmonie_xy_{run_time_date}.grb')
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
