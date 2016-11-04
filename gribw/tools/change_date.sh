:
#
# change initial date of grib file
#
# uses gribw and wgrib                 w. ebisuzaki
# requires bash shell
#

tohex() {
  d1=$[$1 / 16 + 1]
  d2=$[$1 % 16 + 1]
  hex=$(expr substr '0123456789abcdef' $d1 1)$(expr substr '0123456789abcdef' $d2 1)
  echo "$hex"
}

if [ $# -ne 3 ] ; then
   echo "usage: $0 in.grib out.grb YYYYMMDDHH"
   echo "changes time stamp on a grib file"
   exit 0
fi

in=$1
out=$2
t=$3

# temp files
tmp1=/tmp/gribw.$$.1
tmp2=/tmp/gribw.$$.2


hour=$(expr $t % 100)
t=$(expr $t / 100)
day=$(expr $t % 100)
t=$(expr $t / 100)
month=$(expr $t % 100)
t=$(expr $t / 100)
year=$(expr $t % 100)
t=$(expr $t / 100)
century=$(expr $t % 100)

if [ $year -eq 0 ] ; then
   year=100
else
   century=$(expr $century + 1)
fi

# convert to hex

hour=`tohex $hour`
day=`tohex $day`
month=`tohex $month`
year=`tohex $year`
century=`tohex $century`

# make pds/gds string

wgrib -PDS -GDS -o $tmp1 $in -d all | sed 's/.*PDS/PDS/' >$tmp2
   
sed "s/\(PDS=.\{30\}\)../\1$hour/" <$tmp2 | \
sed "s/\(PDS=.\{28\}\)../\1$day/" | \
sed "s/\(PDS=.\{26\}\)../\1$month/" | \
sed "s/\(PDS=.\{24\}\)../\1$year/" | \
sed "s/\(PDS=.\{48\}\)../\1$century/" | \
gribw -o $out $tmp1

# clean up
rm $tmp1 $tmp2
exit 0
