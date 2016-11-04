g=/home/ebis/gribw

daily_interpolate:	daily_interpolate.c tojulian.c
	cc -o daily_interpolate -O2 daily_interpolate.c tojulian.c -I$g -L$g -lgribw -lm

