#include <stdio.h>
#include "gribwlib.h"
#include "pds4.h"

/*
 * Add a bitmap to some grib files NCEP style grib
 *
 * 11/07                             Wesley Ebisuzaki
 */


#define MISSING      -9999.0
#define MISSING_LOW  -9999.9
#define MISSING_HIGH -9998.9

int main(int argc, char **argv) {

    long int pos = 0, pos_fixed = 0;
    int len_grib, i, j, ndata;
    unsigned char *pds, *gds, *bms, *bds;
    float *data;
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
	for (i =j = 0; i < ndata; i++) {
	    if (data[i] > MISSING_LOW && data[i] < MISSING_HIGH) {
		data[i] = UNDEFINED;
		j++;
	    }
	}
	wrt_grib_rec(pds, gds, data, ndata, output);
	printf("bitmap used for %d points\n",j);
	pos += len_grib;
    }
    return 0;
}
