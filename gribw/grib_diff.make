obj=grib_diff.o rd_grib_msg.o readgrib.o seekgrib.o \
    gribw_aux.o unpk_bds.o get_nxny.o \
    intpower.o flt2ibm.o flist2bit.o \
    gribw_private.o ibm2flt.o rd_24section.o

grib_diff: ${obj} gribw.h
	cc -o grib_diff ${obj} -lm

.c.o:	$*.c
	cc -c -O2 $*.c
