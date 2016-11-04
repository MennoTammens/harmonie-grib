
gribify_rr_sst: gribify_rr_sst.c ../gribwlib.h ../pdstool.h ../gdstool.h
	cc -o gribify_rr_sst gribify_rr_sst.c -I.. -L.. -lgribw -lm
