grib_merge:	grib_merge.c
	gcc -O2 -o grib_merge  -I.. grib_merge.c -L.. -lgribw -lm
