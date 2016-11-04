
fix_rr_conv: fix_rr_conv.c ../gribwlib.h ../pdstool.h ../gdstool.h 
	cc -O2 -o fix_rr_conv fix_rr_conv.c -I.. -L.. -lgribw -lm
