#!/bin/sh

# this program takes oiv2 weekly SST and makes daily grib files

dir=/tmp/tmp3

gribify=/home/ebis/gribw/rr/gribify_wkly_sst
cd $dir

# convert ieee files to grib

rm sst.grib
for f in oisst.????????
do
   # big to little endian
   swap $f $f.swap
   date=`echo $f | sed 's/.*\.//'`
   # date=`add_day 3 $date`
   echo $date
   $gribify $f.swap $f.grb $date
   cat $f.grb >>sst.grib
done

# convert weekly files to daily
/home/ebis/gribw/rr/daily_interpolate sst.grib daily_grib
