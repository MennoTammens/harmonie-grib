
p=gribify_m_lw_sfc

$p:	$p.c  ../gribwlib.h ../pdstool.h ../gdstool.h
	cc -o $p $p.c -I.. -L.. -lgribw -lm
