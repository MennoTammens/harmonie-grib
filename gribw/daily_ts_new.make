obj=daily_ts_new.o rd_grib_msg.o wrt_grib_msg.o readgrib.o seekgrib.o \
    append_24section.o gribw_aux.o unpk_bds.o get_nxny.o \
    mk_bds_bms.o intpower.o flt2ibm.o flist2bit.o \
    gribw_private.o gribw_time.o ibm2flt.o rd_GDS.o rd_24section.o BDSunpk.o

daily_ts_new: ${obj} gribw.h
	gcc -o daily_ts_new ${obj} -lm

.c.o:	$*.c
	gcc -c -O2 -Wall $*.c
