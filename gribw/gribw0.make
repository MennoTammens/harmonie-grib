obj=gribw0main.o mk_bds_bms.o wrt_grib_msg.o flt2ibm.o append_24section.o \
   flist2bit.o intpower.o gribw_aux.o gribw_private.o inv_pds.o f77_io.o \
   wrt_grib_rec.o

gribw: ${obj} gribw.h
	gcc -o gribw ${obj} -lm

.c.o:	$*.c gribw.h grib.h
	gcc -c -O2 -Wall $*.c
