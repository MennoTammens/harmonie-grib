
fix_rr_mask: fix_rr_mask.c ../gribwlib.h ../pdstool.h ../gdstool.h 
	cc -o fix_rr_mask fix_rr_mask.c -I.. -L.. -lgribw -lm
