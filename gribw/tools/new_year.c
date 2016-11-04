#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <math.h>
#include <float.h>

#include "gribw.h"
#include "pds4.h"

/*
 * 10/2001
 *  set grib file to new year
 *
 * mod of fix_year.c
 */


int main(int argc, char **argv) {

    long int len_grib, pos = 0;
    unsigned char *pds, *gds, *bms, *bds, *gds2;
    FILE *input, *output;
    int count, cnvt, year, oldyear, yy;


    /* preliminaries .. open up all files */

    if (argc != 4) {
	fprintf(stderr, "%s [in gribfile] [out gribfile] year\n", argv[0]);
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
    year = atoi(argv[3]);

    pos = count = cnvt = 0;
    for(;;) {

	/* read the grib file */

        len_grib = rd_grib_msg(input, pos, &pds, &gds, &bms, &bds);
        if (len_grib <= 0) break;
	oldyear = PDS_Year4(pds);
	if (year != oldyear) {
	    cnvt++;
	    yy = ((year - 1) % 100) + 1;
	    PDS_Year(pds) = yy;
	    PDS_Century(pds) = (year - yy) / 100 + 1;
	}
        wrt_grib_msg(output, pds, gds, bms, bds);
	pos += len_grib;
	count++;
    }
    fclose(output);
    fclose(input);
    fprintf(stderr,"fixed %d records out of %d\n", cnvt, count);
    return 0;
}
