:
# v1.1  ok a better version
# makes .h file from a wgrib gribtable

echo "/* $1 .. parameter table .. include file for pdstool */"
echo 
cat $1 | grep "^[0-9]" | sed 's/[-_]//g' | cut -f1-2 -d: | \
  sed 's/:\([0-9]\)/:_\1/' | awk -F: '{print "#define " $2 "\t" $1}'
