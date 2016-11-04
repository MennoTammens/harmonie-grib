
vertint_rr: vertint_rr.c ../gribwlib.h ../pdstool.h ../gdstool.h 
	cc -o vertint_rr vertint_rr.c -I.. -L.. -lgribw -lm
