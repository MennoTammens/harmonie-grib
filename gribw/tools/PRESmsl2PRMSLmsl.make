prog=PRESmsl2PRMSLmsl

${prog}:	${prog}.c
	gcc -o ${prog} -O2 -Wall ${prog}.c -I.. -L.. -lgribw -lm

