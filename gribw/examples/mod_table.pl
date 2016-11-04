#!/usr/bin/perl -w
#
# This programs modifies the table and subcenter of
# a grib file.  It is easily modified to change other
# values of the PDS.
#
# can be considered an example of a quick and dirty
# grib file modification
#
# 2/98 w. ebisuzaki
#
# uses:  utilities wgrib and gribw
#        files: oldPDSGDS newPDSGDS dump
#

$table=2;
$subcenter=1;

if ($#ARGV != 0) {
   print "usage: (cmdname) gribfile\n";
   exit 8;
}
$file=$ARGV[0];

`wgrib $file -PDS -GDS -d all -o dump >oldPDSGDS`;
open(oldPDSGDS,"oldPDSGDS");
open(newPDSGDS,">newPDSGDS");

@hexdigit = ('0' .. '9', 'a' .. 'f');

#
# convert to hexidecimal
#

if (defined $table) {$table = &hex($table)};
if (defined $subcenter) {$subcenter = &hex($subcenter)};

#
# take the PDS/GDS info and modify it
#

while (<oldPDSGDS>) {
  /PDS=([0-9a-f]*).*GDS=([0-9a-f]*)/;
  $pds=$1;
  $gds=$2;

  if (defined $table) {substr($pds,6,2) = $table};
  if (defined $subcenter) {substr($pds,50,2) = $subcenter};
  print newPDSGDS "PDS=$pds:GDS=$gds\n";
}
close(newPDSGDS);

#
# rewrite a new GRIB file new PDS/GDS
#

`gribw -i dump -o $file.new <newPDSGDS`;
`rm oldPDSGDS newPDSGDS dump`;
exit 0;

sub hex {
    $hexdigit[$_[0] / 16] . $hexdigit[$_[0] % 16];
}
