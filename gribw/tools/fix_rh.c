/*
 * multiple rh by 100 to fix processing bug 
 *
 * 08/2007 W. Ebisuzaki
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <math.h>
#include <float.h>

#include "gribwlib.h"
#include "pds4.h"
#include "ncepopn.h"

int main(int argc, char **argv) {

    unsigned char *pds, *gds;
    FILE *input, *output;
    float *data;
    int ndata, i, ok, count, cnvt;

    long int pos, len_grib;

    /* preliminaries .. open up all files */

    if (argc != 3) {
	fprintf(stderr, "%s [in gribfile] [out]\n", argv[0]);
	exit(8);
    }
    if ((input = fopen(argv[1],"rb")) == NULL) {
        fprintf(stderr,"could not open file: %s\n", argv[1]);
        exit(7);
    }
    if ((output = fopen(argv[2],"wb+")) == NULL) {
        fprintf(stderr,"could not open file: %s\n", argv[2]);
        exit(7);
    }

    for(count = cnvt = pos = 0;;) {

	len_grib = rd_grib_rec(input, pos, &pds, &gds, &data, &ndata);
	if (len_grib <= 0) break;

        ok = 1; 
	if (PDS_PARAM(pds) == RH) {
	    for (i = 0; i < ndata; i++) {
		if (!UNDEFINED_VAL(data[i]) && data[i] > 2.0) break;
	    }
	    ok = 0;
	}
	if (ok == 0) {
		cnvt++;
		for (i = 0; i < ndata; i++) {
		    if (!UNDEFINED_VAL(data[i])) {
		        data[i] *= 100.0;
		    }
		}
	        pds = PDStool(pds, P_dec_scale(PDS_DecimalScale(pds)-2), P_end);
	}
	wrt_grib_rec(pds, gds, data, ndata, output);
	count++;
	pos += len_grib;
    }
    fclose(output);
    fclose(input);
    printf("%d RH  records converted out of %d\n", cnvt, count);
    return 0;
}
