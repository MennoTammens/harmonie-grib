obj=pdstool.o std_ncep_gds.o wrt_grib_rec.o wrt_grib_msg.o \
  append_24section.o cpGRIBsec.o mk_bds_bms.o \
  gribw_aux.o gribw_private.o intpower.o ibm2flt.o \
  flt2ibm.o flist2bit.o gdstool.o rd_grib_msg.o rd_grib_rec.o \
  seekgrib.o readgrib.o get_nxny.o BDSunpk.o  unpk_bds.o gribw_time.o \
  inv_scan.o

cflags=-q64 -O2
lflags=-X64 -q

# cflags=-O2
# lflags=-q

libgribw.a:	${obj} pdstool.h gribwlib.h ${obj}
	touch libgribw.a
	rm libgribw.a
	ar $(lflags) libgribw.a $(obj)

clean:
	rm libgribw.a

.c.o:
	cc -c $(cflags) -o $*.o $*.c
