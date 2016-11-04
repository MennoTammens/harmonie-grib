glib=../libgribw.a


all:	fix_gds ${glib}


${glib}:
	cd `dirname ${glib}` ; make -f gribwlib.make

fix_gds:	fix_gds.c ${glib}
	gcc -o fix_gds -O2 -Wall fix_gds.c -I.. -L.. -lgribw -lm

