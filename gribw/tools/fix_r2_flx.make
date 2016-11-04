
fix_r2_flx:	fix_r2_flx.c
	gcc -o fix_r2_flx -O2 -Wall fix_r2_flx.c -I.. -L.. -lgribw -lm

