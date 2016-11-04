
gribify_us_daily_precipi: gribify_us_daily_precip.c ../gribwlib.h ../pdstool.h ../gdstool.h
	cc -o gribify_us_daily_precip gribify_us_daily_precip.c -I.. -L.. -lgribw -lm
