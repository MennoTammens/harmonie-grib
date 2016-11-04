
gribify_nmc0: gribify_nmc0.c ../gribwlib.h ../pdstool.h ../gdstool.h
	cc -o gribify_nmc0 gribify_nmc0.c -I.. -L.. -lgribw -lm
