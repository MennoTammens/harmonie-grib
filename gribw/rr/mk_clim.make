
g=..

all:	mk_clim_fast.x mk_clim_slow.x

mk_clim_fast.x:	mk_clim.c
	cc -o mk_clim_fast.x -O2 mk_clim.c -I$g -L$g -lgribw -lm -D IN_ORDER -Wall

mk_clim_slow.x:	mk_clim.c
	cc -o mk_clim_slow.x -O2 mk_clim.c -I$g -L$g -lgribw -lm -Wall
