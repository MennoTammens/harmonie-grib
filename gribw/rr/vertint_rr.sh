#!/bin/sh

wgrib -s $1  | egrep ':(PRES:sfc|TMP:.* mb):' | sed 's/PRES/AAAA/' | \
   sort -k4,4 -nk5,5 -t : | wgrib $1 -i -s -grib
