#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <math.h>
#include <float.h>

#include "gribw.h"
#include "pds4.h"


/*
 * Takes a gds-less file and adds a gds
 *   -- only for NCEP files --
 *
 *                              Wesley Ebisuzaki
 *
 */

int main(int argc, char **argv) {

    long int len_grib, pos = 0;
    unsigned char *pds, *gds, *bms, *bds, *gds0;
    FILE *input, *output;
    int nrec, ngds;

    /* preliminaries .. open up all files */

    if (argc != 3) {
	fprintf(stderr, "%s [in gribfile] [out gribfile]\n", argv[0]);
	fprintf(stderr, "adds GDS to NCEP files\n");
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

    nrec = ngds = pos = 0;
    for(;;) {				/* loop over all records */

        len_grib = rd_grib_msg(input, pos, &pds, &gds, &bms, &bds);
        if (len_grib <= 0) break;

	nrec++;
        if (PDS_HAS_GDS(pds) || PDS_Center(pds) != NMC) {
            wrt_grib_msg(output, pds, gds, bms, bds);
	}
	else {
	    gds0 = NCEP_GDS(pds, PDS_Grid(pds));
            wrt_grib_msg(output, pds, gds0, bms, bds);
	    if (gds0) free(gds0);
	    ngds++;
	}
	pos += len_grib;
    }
    fclose(output);
    fclose(input);
    printf("%d GDS sections added out of %d records\n", ngds, nrec);
    return 0;
}
