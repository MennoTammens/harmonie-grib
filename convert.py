#!/usr/bin/env python

from __future__ import print_function

import glob
import os
import subprocess
import sys

from numpy import sqrt

import pygrib

WORKDIR = 'work/'

# bounds to create sliced versions for:
BOUNDS = {
    # label: [sw_corner, ne_corner]
    'nl': [[50.748, 3.071], [53.761, 7.252]],
    'ijsselmeer': [[52.3, 4.94], [53.12, 5.74]]
}

# discover ggrib location
GGRIB_BIN = None
for location in ('/usr/local/bin/ggrib', 'ggrib'):
    if os.path.isfile(location) and os.access(location, os.X_OK):
        GGRIB_BIN = location

files = sorted(glob.glob1(WORKDIR, '*_GB'))

if len(files) != 49:
    print('Verkeerd aantal GRIB-bestanden in {}, exiting'.format(WORKDIR))
    sys.exit(1)


gribout = open('temp.grb', 'wb')
gribout_wind = open('temp_wind.grb', 'wb')


def writeGribMessage(message, wind=False):
    message['generatingProcessIdentifier'] = 96
    message['centre'] = 'kwbc'

    gribout.write(message.tostring())
    if wind:
        gribout_wind.write(message.tostring())


for filename in files:
    grbs = pygrib.open(os.path.join('work/', filename))

    # Mean sea level pressure
    msg_mslp = grbs.select(indicatorOfParameter=1)[0]
    msg_mslp.indicatorOfParameter = 2
    msg_mslp.indicatorOfTypeOfLevel = 'sfc'
    msg_mslp.typeOfLevel = 'meanSea'
    writeGribMessage(msg_mslp)

    # Relative humidity
    msg_rh = grbs.select(indicatorOfParameter=52)[0]
    msg_rh.values = msg_rh.values * 100
    writeGribMessage(msg_rh)

    # Temperature 2m
    msg_t = grbs.select(indicatorOfParameter=11)[0]
    writeGribMessage(msg_t)

    # U-wind
    msg_u = grbs.select(indicatorOfParameter=33)[0]
    writeGribMessage(msg_u, wind=True)

    # V-wind
    msg_v = grbs.select(indicatorOfParameter=34)[0]
    writeGribMessage(msg_v, wind=True)

    # Precipication Intensity
    msg_ip = grbs.select(indicatorOfParameter=61, level=456)[0]
    msg_ip.typeOfLevel = 'surface'
    msg_ip.level = 0
    msg_ip.values = msg_ip.values * 3600  # mm/s => mm/h
    writeGribMessage(msg_ip)

    # Wind gusts
    msg_ug = grbs.select(indicatorOfParameter=162)[0]
    msg_vg = grbs.select(indicatorOfParameter=163)[0]
    msg_ug.values = sqrt(msg_ug.values ** 2 + msg_vg.values ** 2)
    msg_ug.indicatorOfParameter = 180
    msg_ug.typeOfLevel = 'surface'
    msg_ug.level = 0
    writeGribMessage(msg_ug, wind=True)

date_base = files[0][:-6]

filename = date_base + 'zygrib.grb.bz2'
filename_fmt = date_base + 'zygrib_{}.grb.bz2'

# delete old files
for file in glob.glob('harm36_v1_*.grb.bz2'):
    os.remove(file)

if GGRIB_BIN is None:
    print('ggrib binary not found, please install by typing `make ggrib`')
else:
    DEVNULL = open(os.devnull, 'wb')

    for name, bounds in BOUNDS.items():
        out_file = 'temp_{}.grb'.format(name)
        print('Writing to', out_file)

        cmd = [GGRIB_BIN, 'temp.grb', out_file]
        cmd.extend(map(str, [bounds[0][1], bounds[0][0], bounds[1][1], bounds[1][0]]))

        subprocess.call(cmd)
        subprocess.call(['bzip2', out_file])

        os.rename(out_file + '.bz2', filename_fmt.format(name))

subprocess.call(['bzip2', 'temp.grb'])
os.rename('temp.grb.bz2', filename)
