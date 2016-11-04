#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <math.h>
#include <float.h>

#include "gribwlib.h"
#include "gribw.h"


/*
 * does a simple grib merge
 *
 * if all files have same order .. easy to average!!!
 */

#define NMAX	31
int main(int argc, char **argv) {

    long int len_grib, pos[NMAX];
    unsigned char *pds, *gds, *bds, *bms;
    int n, i, j;

    FILE *input[NMAX], *output;

    if (argc <= 2) {
	fprintf(stderr, "usage: %s [out gribfile] [list of grib files]\n", argv[0]);
	exit(8);
    }

    if ((output = fopen(argv[1],"wb")) == NULL) {
        fprintf(stderr,"could not open file: %s\n", argv[1]);
        exit(7);
    }
    for (i = 2; i < argc; i++) {
        if ((input[i-2]  = fopen(argv[i],"rb")) == NULL) {
            fprintf(stderr,"could not open file: %s\n", argv[i]);
            exit(7);
        }
        printf("opening %s\n",argv[i]);
    }
    printf("\n");

    n = argc - 2;
    if (n > NMAX) {
	fprintf(stderr,"fatal error .. increase NMAX\n");
	exit(8);
    }

    for (i = 0 ;  i < n; i++) {
	pos[i] = 0;
    }

    for (j=0;;j++) {
	for (i = 0; i < n; i++) {
	    len_grib = rd_grib_msg(input[i], pos[i], &pds, &gds, &bms, &bds);
	    if (len_grib <= 0) {
		printf("finished at record %d file %d\n", j, i);
		exit(8);
	    }
	    wrt_grib_msg(output, pds, gds, bms, bds);
	    pos[i] += len_grib;
         }
     }
}
