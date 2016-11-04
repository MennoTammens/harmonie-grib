
g=/home/ebis/gribw

mk_mean.x:	mk_mean.c
	cc -o mk_mean.x -O2 mk_mean.c -I$g -L$g -lgribw -lm

