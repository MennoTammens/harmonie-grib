
fix_t40_flx:	fix_t40_flx.c
	gcc -o fix_t40_flx -O2 -Wall fix_t40_flx.c -I.. -L.. -lgribw -lm

