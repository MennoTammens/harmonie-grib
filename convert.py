#!/usr/bin/env python

import sys
import os
import glob
import subprocess
import bz2
import pygrib
from numpy import sqrt

files = sorted(glob.glob1('work', '*_GB'))

if len(files) != 49:
    print('Verkeerd aantal GRIB-bestanden')
    sys.exit(1)

def writeGribMessage(message, out):
    message['generatingProcessIdentifier'] = 96
    message['centre'] = 'kwbc'
    out.write(message.tostring())


gribout = open('temp.grb', 'wb')
gribout_wind = open('temp_wind.grb', 'wb')

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
    writeGribMessage(msg_u, gribout_wind)
    
    # V-wind
    msg_v = grbs.select(indicatorOfParameter=34)[0]
    writeGribMessage(msg_v, gribout)
    writeGribMessage(msg_v, gribout_wind)
    
    # Precipication Intensity
    msg_ip = grbs.select(indicatorOfParameter=61, level=456)[0]
    msg_ip.typeOfLevel = 'surface'
    msg_ip.level = 0
    msg_ip.values = msg_ip.values * 3600
    writeGribMessage(msg_ip, gribout)
    
    # Wind gusts
    msg_ug = grbs.select(indicatorOfParameter=162)[0]
    msg_vg = grbs.select(indicatorOfParameter=163)[0]
    msg_ug.values = sqrt(msg_ug.values**2 + msg_vg.values**2)
    msg_ug.indicatorOfParameter = 180
    msg_ug.typeOfLevel = 'surface'
    msg_ug.level = 0
    writeGribMessage(msg_ug, gribout)
    writeGribMessage(msg_ug, gribout_wind)

gribout.close()
gribout_wind.close()

DEVNULL = open(os.devnull, 'wb')
subprocess.call(['/usr/local/bin/ggrib', 'temp.grb', 'temp_nl.grb', '3.071', '50.748', '7.252', '53.761'], stdout=DEVNULL)
subprocess.call(['/usr/local/bin/ggrib', 'temp_wind.grb', 'temp_wind_nl.grb', '3.071', '50.748', '7.252', '53.761'], stdout=DEVNULL)
subprocess.call(['/usr/local/bin/ggrib', 'temp.grb', 'temp_ijmwad.grb', '4.363', '52.294', '5.903', '53.411'], stdout=DEVNULL)
subprocess.call(['/usr/local/bin/ggrib', 'temp.grb', 'temp_zld.grb', '2.6', '51.2', '5.0', '52.0'], stdout=DEVNULL)
subprocess.call(['/usr/local/bin/ggrib', 'temp_wind.grb', 'temp_wind_zld.grb', '2.6', '51.2', '5.0', '52.0'], stdout=DEVNULL)
subprocess.call(['bzip2', 'temp.grb'])
subprocess.call(['bzip2', 'temp_nl.grb'])
subprocess.call(['bzip2', 'temp_wind.grb'])
subprocess.call(['bzip2', 'temp_wind_nl.grb'])
subprocess.call(['bzip2', 'temp_zld.grb'])
subprocess.call(['bzip2', 'temp_wind_zld.grb'])
subprocess.call(['bzip2', 'temp_ijmwad.grb'])

filename = files[0][:-6]+'zygrib.grb.bz2'
filename_nl = files[0][:-6]+'zygrib_nl.grb.bz2'
filename_wind = files[0][:-6]+'zygrib_wind.grb.bz2'
filename_wind_nl = files[0][:-6]+'zygrib_wind_nl.grb.bz2'
filename_zld = files[0][:-6]+'zygrib_zld.grb.bz2'
filename_wind_zld = files[0][:-6]+'zygrib_wind_zld.grb.bz2'
filename_ijmwad = files[0][:-6]+'zygrib_ijmwad.grb.bz2'
for file in [f for f in os.listdir('.') if f.startswith('harm36_v1')]:
    os.remove(file)
os.rename('temp.grb.bz2', filename)
os.rename('temp_nl.grb.bz2', filename_nl)
os.rename('temp_wind.grb.bz2', filename_wind)
os.rename('temp_wind_nl.grb.bz2', filename_wind_nl)
os.rename('temp_zld.grb.bz2', filename_zld)
os.rename('temp_wind_zld.grb.bz2', filename_wind_zld)
os.rename('temp_ijmwad.grb.bz2', filename_ijmwad)
