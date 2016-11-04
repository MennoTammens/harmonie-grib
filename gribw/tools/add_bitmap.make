p=add_bitmap
$p:	$p.c
	cc -o $p $p.c -I.. -L.. -lgribw -lm
