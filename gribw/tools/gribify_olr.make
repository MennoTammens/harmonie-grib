
gribify_olr: gribify_olr.c ../gribwlib.h ../pdstool.h ../gdstool.h
	cc -o gribify_olr gribify_olr.c -I.. -L.. -lgribw -lm
