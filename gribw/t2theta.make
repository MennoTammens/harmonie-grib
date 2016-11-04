obj=t2theta.o wrt_grib_msg.o rd_grib_msg.o get_nxny.o unpk_bds.o \
  mk_bds_bms.o flt2ibm.o ibm2flt.o intpower.o seekgrib.o readgrib.o \
  gribw_private.o gribw_aux.o flist2bit.o append_24section.o

t2theta: ${obj} gribw.h
	gcc -o t2theta ${obj} -lm

.c.o:	$*.c
	gcc -c -O2 -Wall $*.c
