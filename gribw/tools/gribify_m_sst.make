
gribify_m_sst: gribify_m_sst.c ../gribwlib.h ../pdstool.h ../gdstool.h
	cc -o gribify_m_sst gribify_m_sst.c -I.. -L.. -lgribw -lm
