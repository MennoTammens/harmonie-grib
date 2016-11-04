
p=gribify_zonal_o3

$p:	$p.c  ../gribwlib.h ../pdstool.h ../gdstool.h
	cc -o $p $p.c -I.. -L.. -lgribw -lm
