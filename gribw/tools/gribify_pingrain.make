
gribify_pingrain: gribify_pingrain.c ../gribwlib.h ../pdstool.h ../gdstool.h
	cc -o gribify_pingrain gribify_pingrain.c -I.. -L.. -lgribw -lm
