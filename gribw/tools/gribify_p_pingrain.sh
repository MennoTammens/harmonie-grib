#!/bin/sh

# get data from workstation

if [ 1 -eq 2 ] ; then
scp wd51we@special:/export-2/lnx193/xping2/PRODUCTS/CMAP/pentad/current/cmap_pen.lnx .
fi

./gribify_p_pingrain cmap_pen.lnx cmap_pen.grb 1979
