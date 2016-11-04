:
#
# fix r2 flux file
#
if [ $# -ne 2 ] ; then
   echo "usage: $0 in.grib out.grb"
   echo "fixes r2 flux file"
   exit 0
fi

in=$1
out=$2
t=$3

# temp files
tmp1=/tmp/gribw.$$.1
tmp2=/tmp/gribw.$$.2


wgrib -ncep_rean -PDS -GDS -o $tmp1 $in -d all | sed 's/.*PDS/PDS/' | \
 sed 's/GDS=00002000ff0400c0005e0159de000000808159de800753002f07530000000000/GDS=00002000ff0400c0005e0159de000000808159de8007530753002f0000000000/' >$tmp2
   
gribw -o $out $tmp1 <$tmp2

# clean up
rm $tmp1 $tmp2
exit 0
