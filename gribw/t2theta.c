#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <math.h>
#include <float.h>

#include "gribw.h"
#include "pds4.h"


/*
 * tmp 2 theta
 *
 *                              Wesley Ebisuzaki
 *
 */

#define N 100000
#define TMP 11
#define POT 13
#define PRS 100


void main(int argc, char **argv) {

    long int len_grib, pos = 0;
    unsigned char *pds, *gds, *bms, *bds;
    FILE *input, *output;
    int i, nxny;
    float *array;
    double prs, factor;
    int array_size;

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

    array = (float *) malloc(N * sizeof(float));
    array_size = N;
    if (array == NULL) {
	fprintf(stderr,"memeory allocation failed\n");
	exit(8);
    }

    pos = 0;
    for(;;) {				/* loop over all records */

            len_grib = rd_grib_msg(input, pos, &pds, &gds, &bms, &bds);
            if (len_grib <= 0) break;

	    if (PDS_PARAM(pds) == TMP && PDS_L_TYPE(pds) == PRS ) {
		nxny = get_nxny(pds, gds, bms, bds);
	        if (nxny > array_size) {
		    free(array);
		    array_size = nxny;
		    array = (float *) malloc(array_size * sizeof(float));
		    if (array == NULL) {
			fprintf(stderr,"memory allocation failed\n");
			exit(8);
		    }
	        }
                unpk_bds(array, pds, gds, bms, bds, nxny);
                prs = PDS_KPDS7(pds);	/* pressure in mb */
		factor = pow(1000.0/(double) prs, 0.286);
	        for (i = 0; i < nxny; i++) {
		    if (! UNDEFINED_VAL(array[i])) {
			array[i] = array[i] * factor;
		    }
	        }
		pds[8] = POT;
                bms = mk_BMS(pds, array, &nxny, UNDEFINED_LOW, UNDEFINED_HIGH);
	        bds = mk_BDS(pds, array, nxny);
                wrt_grib_msg(output, pds, gds, bms, bds);
	        free(bds);
	        if (bms) free(bms);
	    }
	    else {
                wrt_grib_msg(output, pds, gds, bms, bds);
	    }
	    pos += len_grib;
    }
    exit(0);
}
