#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <math.h>
#include <float.h>

#include "gribw.h"
#include "pds4.h"


/*
 * grib_diff.c find differences in 2 files
 *
 *                              Wesley Ebisuzaki
 *
 */

#define N 100000

int main(int argc, char **argv) {

    long int len_grib, pos1, pos2;

    unsigned char *pds, *gds, *bms, *bds;
    FILE *input1, *input2;
    int i, count, nxny1, nxny2, n, ndiff;
    float array1[N], array2[N], maxerr, err, rms_err;
    float minx, maxx;

    /* preliminaries .. open up all files */

    if (argc != 3) {
	fprintf(stderr, "%s [gribfile1] [gribfile2]\n", argv[0]);
	exit(8);
    }
    if ((input1 = fopen(argv[1],"rb")) == NULL) {
        fprintf(stderr,"could not open file: %s\n", argv[1]);
        exit(7);
    }
    if ((input2 = fopen(argv[2],"rb")) == NULL) {
        fprintf(stderr,"could not open file: %s\n", argv[2]);
        exit(7);
    }

    pos1 = pos2 = 0;
    for(count = 1;;count++) {
        len_grib = rd_grib_msg(input1, pos1, &pds, &gds, &bms, &bds);
        if (len_grib <= 0) break;
	nxny1 = unpk_bds(array1, pds, gds, bms, bds, N);
	pos1 += len_grib;
        if (nxny1 == -1) {
	    fprintf(stderr,"array too small\n");
	    exit(8);
	}

        len_grib = rd_grib_msg(input2, pos2, &pds, &gds, &bms, &bds);
        if (len_grib <= 0) break;
	nxny2 = unpk_bds(array2, pds, gds, bms, bds, N);
	pos2 += len_grib;
        if (nxny2 == -1) {
	    fprintf(stderr,"array too small\n");
	    exit(8);
	}

        if (nxny2 != nxny1) {
	    fprintf(stderr,"nxny1 != nxny2\n");
	    exit(8);
	}

        rms_err = maxerr = 0.0;
	minx = maxx = array1[0];
        ndiff = 0;
        for (n = i = 0; i < nxny1; i++) {
	    if (!UNDEFINED_VAL(array1[i]) && !UNDEFINED_VAL(array2[i])) {
	        if (n == 0) {
		    minx = maxx = array1[i];
		}
	        if (minx > array1[i]) minx = array1[i];
	        if (maxx < array1[i]) maxx = array1[i];

	        err = fabs(array1[i] - array2[i]);
	        if (err > maxerr) maxerr = err;
	        rms_err += err*err;
                n++;
                if (array1[i] != array2[i]) ndiff++;
            }
	}
	if (maxerr != 0) 
           printf("%d: max %g rms %g scaled max %g scaled rms %g m/m %g/%g ndiff %d\n", 
           count, maxerr, sqrt(rms_err/n), maxerr/(maxx-minx),
           sqrt(rms_err/n)/(maxx-minx),minx,maxx,ndiff);
    }
    fclose(input1);
    fclose(input2);
    return 0;
}
