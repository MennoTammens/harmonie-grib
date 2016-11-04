#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <math.h>
#include <float.h>

#include "gribwlib.h"

/*
 * copy a grib file, convert the subcenter type to 2
 * (ie reanalysis)
 *
 * modified to use PDStool
 *                              Wesley Ebisuzaki
 */


void main(int argc, char **argv) {

    long int len_grib, pos = 0;
    unsigned char *pds, *gds, *bms, *bds;
    FILE *input, *output;
    int count = 0;

    /* preliminaries .. open up all files */

    if (argc != 3) {
	fprintf(stderr, "%s [in gribfile] [out gribfile]\n", argv[0]);
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

    while ((len_grib = rd_grib_msg(input, pos, &pds, &gds, &bms, &bds)) > 0) {
	pds = PDStool(pds,P_subcenter(2));
	wrt_grib_msg(output, pds, gds, bms, bds);
	pos += len_grib;
	count++;
    }
    printf("%d records converted\n", count);
    fclose(input);
    fclose(output);
}
