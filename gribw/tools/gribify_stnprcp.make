
gribify_stnprcp: gribify_stnprcp.c ../gribwlib.h ../pdstool.h ../gdstool.h
	cc -o gribify_stnprcp gribify_stnprcp.c -I.. -L.. -lgribw -lm
