obj=cp_grib.o rd_grib_msg.o wrt_grib_msg.o readgrib.o seekgrib.o size_24section.o \
    append_24section.o NMC_table.o gribw_aux.o gio.o  unpak_bds.o get_nxny.o \
    mk_bds_bms.o intpower.o flt2ibm.o list2bit.o gribw_time.o \
    gribw_private.o ibm2flt.o con1x1.o rd_GDS.o

cp_grib: ${obj} gribw.h
	gcc -o cp_grib ${obj} -lm

.c.o:	$*.c
	gcc -c -O2 -Wall $*.c
