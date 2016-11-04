
gribify_gpi: gribify_gpi.c ../gribwlib.h ../pdstool.h ../gdstool.h
	cc -o gribify_gpi gribify_gpi.c -I.. -L.. -lgribw -lm
