#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <math.h>
#include <float.h>

#include "gribw.h"
#include "pds4.h"
#include "gds.h"
#include "gdstool.h"

/*
 * fix t40 flux gaussian grid fields (gds)
 *
 *  The w3lib software had a problem with creating the correct
 *   gds (grid definition section) for a T62 Gaussian grid.
 *  The problem is fixed in the current version of the w3lib.
 *
 *  This program fixes the definition of a T40 grid using the gribw library
 *
 *                              Wesley Ebisuzaki
 * 9/99
 */


int main(int argc, char **argv) {

    long int len_grib, pos = 0;
    unsigned char *pds, *gds, *bms, *bds, *gds2;
    FILE *input, *output;
    int count, cnvt;

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

        if (PDS_Grid(pds) == 255 && GDS_Gaussian(gds) &&
		GDS_LatLon_nx(gds) == 128 && GDS_LatLon_ny(gds) == 64 ) {
 	    gds2 = new_Gaussian_GDS(pds, 128, 64, 0.0, 87.864);
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
