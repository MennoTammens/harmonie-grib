obj=to144x73grb.o rd_grib_msg.o wrt_grib_msg.o readgrib.o seekgrib.o \
    append_24section.o gribw_aux.o unpk_bds.o get_nxny.o \
    mk_bds_bms.o intpower.o flt2ibm.o flist2bit.o gds_grid2.o \
    gribw_private.o ibm2flt.o con1x1.o rd_GDS.o rd_24section.o

to144x73grb: ${obj} gribw.h
	gcc -o to144x73grb ${obj} -lm

.c.o:	$*.c
	gcc -c -O2 -Wall $*.c
