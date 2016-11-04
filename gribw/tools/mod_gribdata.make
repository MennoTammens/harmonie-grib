
mod_gribdata:	mod_gribdata.c 
	cc -o mod_gribdata mod_gribdata.c -I.. -L.. -lgribw -lm
