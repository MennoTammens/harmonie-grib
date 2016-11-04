#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <math.h>
#include <float.h>

#include "gribw.h"
#include "pds4.h"


/*
 * copy a grib file, convert 360x181 to 144x73 resolution
 * when there is no bitmap
 *
 *                              Wesley Ebisuzaki
 *
 */

#define N 100000

void con1x1(float *grd1x1, float *grd25);

int main(int argc, char **argv) {

    long int len_grib, pos = 0;
    unsigned char *pds, *gds, *bms, *bds, *gds2;
    FILE *input, *output;
    int count, cnvt, nxny;
    float array[N];
    float array25[144*73];

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

    /* read gds.2 = gds for grid type 2 */
    gds2 = gds_grid2();

    pos = count = cnvt = 0;
    for(;;) {
        len_grib = rd_grib_msg(input, pos, &pds, &gds, &bms, &bds);
        if (len_grib <= 0) break;

	/* convert 1x1 grid to 2.5x2.5 grid only if not bitmap */
	/* conversion code doesn't like undefined values */
        if (PDS_Grid(pds) == 3 && PDS_HAS_BMS(pds) == 0) {
	    nxny = unpk_bds(array, pds, gds, bms, bds, N);
	    if (nxny != 360*181) {
        	fprintf(stderr,"unexpected grid size, record %d\n", count);
		exit(8);
	    }
	    /* convert to 144x73 */
	    con1x1(array, array25);
	    set_PDSGridType(pds, 2);
	    bds = mk_BDS(pds, array25, 144*73);
            wrt_grib_msg(output, pds, gds2, bms, bds);
	    cnvt++;
	    free(bds);
	}
	else {
            wrt_grib_msg(output, pds, gds, bms, bds);
	}
	pos += len_grib;
	count++;
    }
    fclose(output);
    fclose(input);
    fprintf(stderr,"converted %d records out of %d to 144x73\n", cnvt, count);
    return 0;
}
