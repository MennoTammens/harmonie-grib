
g=..

all:	mk_ave_eta.x mk_ave_eta_slow.x

mk_ave_eta.x:	mk_ave_eta.c
	cc -o mk_ave_eta_fast.x -O2 mk_ave_eta.c -I$g -L$g -lgribw -lm -D IN_ORDER

mk_ave_eta_slow.x:	mk_ave_eta.c
	cc -o mk_ave_eta_slow.x -O2 mk_ave_eta.c -I$g -L$g -lgribw -lm
