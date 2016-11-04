#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <math.h>
#include <float.h>

#include "gribw.h"
#include "pds4.h"


/*
 * new_trange.c
 *
 * changes the time range of the grib records
 *  prelimary version .. change all records
 *  next version .. selective change
 *
 *                              Wesley Ebisuzaki
 *
 */

void main(int argc, char **argv) {

    long int len_grib, pos;
    unsigned char *pds, *gds, *bms, *bds;
    FILE *input, *output;

    /* preliminaries .. open up all files */

    if (argc != 3) {
	fprintf(stderr, "%s [in] [out] (time range)\n", argv[0]);
	exit(8);
    }
    if ((input = fopen(argv[1],"rb")) == NULL) {
        fprintf(stderr,"could not open file: %s\n", argv[1]);
        exit(7);
    }
    if ((output = fopen(argv[2],"wb")) == NULL) {
        fprintf(stderr,"could not open file: %s\n", argv[2]);
        exit(7);
    }

    pos = 0;
    while ((len_grib = rd_grib_msg(input, pos, &pds, &gds, &bms, &bds)) > 0) {

            /* change time range */

            wrt_grib_msg(output, pds, gds, bms, bds);

	    pos += len_grib;
    }
    exit(0);
}
