#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <math.h>
#include <float.h>

#include "gribw.h"
#include "pds4.h"

/*
 * 7/2007
 *
 * simple program to convert PRESmsl to PRMSLmsl
 *
 *                              Wesley Ebisuzaki
 *
 */


int main(int argc, char **argv) {

    long int len_grib, pos = 0;
    unsigned char *pds, *gds, *bms, *bds, *gds2;
    FILE *input, *output;
    int count, cnvt, grid;

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

        grid = PDS_Grid(pds);
        if (grid == 98 || grid == 2) {
	    gds2 = NCEP_GDS(pds,grid);	/* get the right grid defn */
            wrt_grib_msg(output, pds, gds2, bms, bds);
	    free(gds2);
	    cnvt++;
            wrt_grib_msg(output, pds, gds, bms, bds);
	}
	else {
            wrt_grib_msg(output, pds, gds, bms, bds);
	}
	pos += len_grib;
	count++;
    }
    fclose(output);
    fclose(input);
    fprintf(stderr,"fixed %d records out of %d\n", cnvt, count);
    return 0;
}
