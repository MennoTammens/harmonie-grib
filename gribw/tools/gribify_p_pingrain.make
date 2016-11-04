
gribify_p_pingrain: gribify_p_pingrain.c ../gribwlib.h ../pdstool.h ../gdstool.h
	cc -o gribify_p_pingrain gribify_p_pingrain.c -I.. -L.. -lgribw -lm
