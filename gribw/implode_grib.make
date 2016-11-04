obj=size_24section.o implode_grib.o rd_24section.o wrt_grib_msg.o \
    append_24section.o NMC_table.o gribw_aux.o rd_GDS.o gio.o \
    mk_bds_bms.o intpower.o flt2ibm.o list2bit.o gribw_time.o gribw_private.o

implode_grib: ${obj} gribw.h
	gcc -o implode_grib ${obj} -lm

.c.o:	$*.c
	gcc -c -O2 -Wall $*.c
