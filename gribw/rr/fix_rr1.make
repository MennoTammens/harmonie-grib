
fix_rr1: fix_rr1.c ../gribwlib.h ../pdstool.h ../gdstool.h 
	cc -o fix_rr1 fix_rr1.c -I.. -L.. -lgribw -lm
