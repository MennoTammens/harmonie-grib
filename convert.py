#!/usr/bin/env python

import sys
import os
import glob
import subprocess
import bz2
import pygrib
from numpy import sqrt

files = sorted(glob.glob1('work/','*_GB'))

if len(files) != 49:
    print('Verkeerd aantal GRIB-bestanden')
    sys.exit(1)

def writeGribMessage(message, out):
    message['generatingProcessIdentifier'] = 96
    message['centre'] = 'kwbc'
    out.write(message.tostring())

gribout = open('temp.grb', 'wb')

for file in files:
    grbs = pygrib.open('work/'+file)
    
    # Mean sea level pressure
    msg_mslp = grbs.select(indicatorOfParameter=1)[0]
    msg_mslp.indicatorOfParameter = 2
    msg_mslp.indicatorOfTypeOfLevel = 'sfc'
    msg_mslp.typeOfLevel = 'meanSea'
    writeGribMessage(msg_mslp, gribout)
    
    # Relative humidity
    msg_rh = grbs.select(indicatorOfParameter=52)[0]
    msg_rh.values = msg_rh.values * 100
    writeGribMessage(msg_rh, gribout)
    
    # Temperature 2m
    msg_t = grbs.select(indicatorOfParameter=11)[0]
    writeGribMessage(msg_t, gribout)
    
    # U-wind
    msg_u = grbs.select(indicatorOfParameter=33)[0]
    writeGribMessage(msg_u, gribout)
    
    # V-wind
    msg_v = grbs.select(indicatorOfParameter=34)[0]
    writeGribMessage(msg_v, gribout)
    
    # Precipication Intensity
    msg_ip = grbs.select(indicatorOfParameter=61, level=456)[0]
    msg_ip.typeOfLevel = 'surface'
    msg_ip.level = 0
    msg_ip.values = msg_ip.values * 3600 # mm/s => mm/h
    writeGribMessage(msg_ip, gribout)
    
    # Wind gusts
    msg_ug = grbs.select(indicatorOfParameter=162)[0]
    msg_vg = grbs.select(indicatorOfParameter=163)[0]
    msg_ug.values = sqrt(msg_ug.values**2 + msg_vg.values**2)
    msg_ug.indicatorOfParameter = 180
    msg_ug.typeOfLevel = 'surface'
    writeGribMessage(msg_ug, gribout)

gribout.close()

DEVNULL = open(os.devnull, 'wb')
subprocess.call(['/usr/local/bin/ggrib', 'temp.grb', 'temp_nl.grb', '3.071', '50.748', '7.252', '53.761'], stdout=DEVNULL)
subprocess.call(['bzip2', 'temp.grb'])
subprocess.call(['bzip2', 'temp_nl.grb'])

filename = files[0][:-6]+'zygrib.grb.bz2'
filename_nl = files[0][:-6]+'zygrib_nl.grb.bz2'
for file in glob.glob('harm36_v1_*.grb.bz2'):
    os.remove(file)
os.rename('temp.grb.bz2', filename)
os.rename('temp_nl.grb.bz2', filename_nl)
