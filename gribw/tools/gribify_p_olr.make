
gribify_p_olr: gribify_p_olr.c ../gribwlib.h ../pdstool.h ../gdstool.h
	cc -o gribify_p_olr gribify_p_olr.c -I.. -L.. -lgribw -lm
