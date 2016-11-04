
gribify_tcdc: gribify_tcdc.c ../gribwlib.h ../pdstool.h ../gdstool.h
	cc -o gribify_tcdc gribify_tcdc.c -I.. -L.. -lgribw -lm
