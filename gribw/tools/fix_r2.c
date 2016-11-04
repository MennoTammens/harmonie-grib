#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <math.h>
#include <float.h>

#include "gribw.h"
#include "pds4.h"

/*
 * 9/99
 * fix r2 flux gaussian grid fields (gds)
 *
 *  The w3lib software had a problem with creating the correct
 *   gds (grid definition section) for a T62 Gaussian grid.
 *  The problem is fixed in the current version of the w3lib.
 *
 *  This program fixes the definition of a T62 grid using the gribw library
 *
 *                              Wesley Ebisuzaki
 *
 * 1/01
 * fix r2 lat-long/gaussian grid field 
 *  The w3lib software had a problem from translating from one
 *   gds representation to another (kgds -> igds).  Yeh, great
 *   software engineering to have two fortran representations
 *   of the GDS.  Anyways the translator for the two representations
 *   was out-dated producing which resulted in the
 *    (1) wrong scan mode for 2.5 x 2.5 grids
 *    (2) wrong longitude increments for Gaussian grids
 *
 *                              Wesley Ebisuzaki
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
