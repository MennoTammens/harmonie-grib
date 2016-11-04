
gribify_gpcp: gribify_gpcp.c ../gribwlib.h ../pdstool.h ../gdstool.h
	cc -o gribify_gpcp gribify_gpcp.c -I.. -L.. -lgribw -lm
