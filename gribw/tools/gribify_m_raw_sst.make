
gribify_m_raw_sst: gribify_m_raw_sst.c ../gribwlib.h ../pdstool.h ../gdstool.h
	cc -o gribify_m_raw_sst gribify_m_raw_sst.c -I.. -L.. -lgribw -lm
