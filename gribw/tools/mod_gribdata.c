#include <stdio.h>
#include "gribwlib.h"

/*
 * Some grib data was written incorrectly: the PDS had the wrong
 *  time description, and the data was S-N rather than N-S.
 *
 * This program fixes up these grib files
 *   Isn't this program too easy?
 *
 * 5/98                             Wesley Ebisuzaki
 */

void main(int argc, char **argv) {

    long int pos = 0;
    int len_grib;
    unsigned char *pds, *gds;
    unsigned char *bms, *bds;
    float *data, tmp[144*73];
    int ndata, ix, iy;
    FILE *input, *output;

    /* preliminaries .. open up all files */

    if (argc != 3) {
	fprintf(stderr, "%s [in gribfile] [out gribfile]\n", argv[0]);
	fprintf(stderr, " modifies grib file\n");
	exit(6);
    }
    if ((input = fopen(argv[1],"rb")) == NULL) {
        fprintf(stderr,"could not open file: %s\n", argv[1]);
        exit(7);
    }
    if ((output = fopen(argv[2],"wb")) == NULL) {
        fprintf(stderr,"could not open file: %s\n", argv[2]);
        exit(8);
    }

    /* main loop */

    while ((len_grib = rd_grib_rec(input, pos, &pds, &gds, &data, &ndata)) > 0) {
	if (ndata == 144*73) {
	    /* got the right data */
	    pds[6] = 2;
	    pds[20] = 4;
	    for (iy = 0; iy < 73; iy++) {
		for (ix = 0; ix < 144; ix++) {
		    tmp[ix+iy*144] = data[ix + (72-iy)*144];
		}
	    }
	    wrt_grib_rec(pds, gds, tmp, ndata, output);
        }
	pos += len_grib;
    }
    exit(0);
}
