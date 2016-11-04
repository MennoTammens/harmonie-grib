
gribify_fits: gribify_fits.c ../gribwlib.h ../pdstool.h ../gdstool.h
	cc -o gribify_fits gribify_fits.c -I.. -L.. -lgribw -lm
