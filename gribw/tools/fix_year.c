#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <math.h>
#include <float.h>

#include "gribw.h"
#include "pds4.h"

/*
 * 1/2001
 *		simple fix of year
 */

#define NEW_YEAR 2000
#define OLD_YEAR 1900

int main(int argc, char **argv) {

    long int len_grib, pos = 0;
    unsigned char *pds, *gds, *bms, *bds, *gds2;
    FILE *input, *output;
    int count, cnvt, year;

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

    pos = count = cnvt = 0;
    for(;;) {

	/* read the grib file */

        len_grib = rd_grib_msg(input, pos, &pds, &gds, &bms, &bds);
        if (len_grib <= 0) break;
	year = PDS_Year4(pds);
	if (year == OLD_YEAR) {
	    cnvt++;
	    year = ((NEW_YEAR - 1) % 100) + 1;
	    PDS_Year(pds) = year;
	    PDS_Century(pds) = (NEW_YEAR - year) / 100 + 1;
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
