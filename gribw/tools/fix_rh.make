
fix_rh:	fix_rh.c
	gcc -o fix_rh -O2 -Wall fix_rh.c -I.. -L.. -lgribw -lm

