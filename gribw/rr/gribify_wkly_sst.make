
gribify_wkly_sst: gribify_wkly_sst.c ../gribwlib.h ../pdstool.h ../gdstool.h
	cc -o gribify_wkly_sst gribify_wkly_sst.c -I.. -L.. -lgribw -lm
