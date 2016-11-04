
gribify_rt_p_pingrain: gribify_rt_p_pingrain.c ../gribwlib.h ../pdstool.h ../gdstool.h
	cc -o gribify_rt_p_pingrain gribify_rt_p_pingrain.c -I.. -L.. -lgribw -lm
