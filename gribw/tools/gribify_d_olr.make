
gribify_d_olr: gribify_d_olr.c ../gribwlib.h ../pdstool.h ../gdstool.h
	cc -o gribify_d_olr gribify_d_olr.c -I.. -L.. -lgribw -lm
