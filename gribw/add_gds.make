obj=add_gds.o wrt_grib_msg.o rd_grib_msg.o  \
  seekgrib.o readgrib.o cpGRIBsec.o \
  gribw_private.o gribw_aux.o append_24section.o std_ncep_gds.o

add_gds: ${obj} gribw.h
	gcc -o add_gds ${obj} -lm

.c.o:	$*.c
	gcc -c -O2 -Wall $*.c
