:
#
# change time range on grib file
#  example on how to change forecast time unit, time_range, p1 and p2
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

#input and output files
in=test.grb
out=out.grb

# temp files
tmp1=/tmp/gribw.$$.1
tmp2=/tmp/gribw.$$.2


# make pds/gds string
# these number must be in hex
funit='02'
p1='01'
p2='06'
tr='03'

wgrib -PDS -GDS -o $tmp1 $in -d all | sed 's/.*PDS/PDS/' >$tmp2
   
sed "s/\(PDS=.\{34\}\)../\1$funit/" <$tmp2 | \
sed "s/\(PDS=.\{36\}\)../\1$p1/" | \
sed "s/\(PDS=.\{38\}\)../\1$p2/" | \
sed "s/\(PDS=.\{40\}\)../\1$tr/" | \
gribw -o $out $tmp1

# clean up
rm $tmp1 $tmp2
exit 0
