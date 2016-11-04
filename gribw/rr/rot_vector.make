
rot_vector: rot_vector.c ../gribwlib.h ../pdstool.h ../gdstool.h 
	cc -o rot_vector rot_vector.c -I.. -L.. -lgribw -lm
