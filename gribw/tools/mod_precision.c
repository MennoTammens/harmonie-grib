#include <stdio.h>
#include "pdstool.h"
#include "gribwlib.h"

/*
 * need to write out data with a fixed precision (#bits)
 *
 * 10/2003                             Wesley Ebisuzaki
 */

int main(int argc, char **argv) {

    long int pos = 0;
    int len_grib;
    unsigned char *pds, *gds;
    unsigned char *bms, *bds;
    float *data, tmp[144*73];
    int ndata, ix, iy;
    FILE *input, *output;
    int nbits;

    /* preliminaries .. open up all files */

    if (argc != 4) {
	fprintf(stderr, "%s [in gribfile] [out gribfile] [#bits]\n", argv[0]);
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
    nbits = atoi(argv[3]);
    if (nbits < 1 || nbits > 24) {
	fprintf(stderr,"illegal nbits %d\n", argv[3]);
	exit(8);
    }

    /* main loop */
    printf("number of bits %d\n", nbits);

    while ((len_grib = rd_grib_rec(input, pos, &pds, &gds, &data, &ndata)) > 0) {

	/* go to ECMWF style */
 	pds = PDStool(pds,P_dec_scale(0),P_end);

	set_BDSMaxBits(nbits);
	set_BDSMinBits(nbits);

	wrt_grib_rec(pds, gds, data, ndata, output);
	pos += len_grib;
    }
    exit(0);
    return 0;
}
