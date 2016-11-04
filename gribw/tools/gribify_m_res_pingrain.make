
p=gribify_m_res_pingrain

$p:	$p.c  ../gribwlib.h ../pdstool.h ../gdstool.h
	cc -o $p $p.c -I.. -L.. -lgribw -lm
