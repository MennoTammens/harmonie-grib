#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <math.h>
#include <float.h>

#include "gribw.h"
#include "pds4.h"


/*
 * takes 6 hour time series, and makes average
 *  for pentad and monthly means
 *
 *                              Wesley Ebisuzaki
 *
 */

#define N 100000

int main(int argc, char **argv) {

    long int len_grib, pos = 0;
    unsigned char *pds, *gds, *bms, *bds, *pds0, *gds0;
    FILE *input, *output;
    int i, nxny;
    float *array, *sum;
    int *count;
    int array_size;
    int tr, p1, p2, units, nave, nmissing;
    unsigned int field, time;
    int ndays, day_count, dhour;

    /* preliminaries .. open up all files */

    if (argc != 4) {
	fprintf(stderr, "%s [in gribfile] [out gribfile] [n days]\n", argv[0]);
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
    ndays = atoi(argv[3]);

    count = (int *) malloc(N * sizeof(int));
    sum = (float *) malloc(N * sizeof(float));
    array = (float *) malloc(N * sizeof(float));
    array_size = N;
    if (count == NULL || sum == NULL || array == NULL) {
	fprintf(stderr,"memeory allocation failed\n");
	exit(8);
    }

    pos = 0;
    for(;;) {				/* loop over all records */

	for(day_count=0;;) {		/* loop over summation */

            len_grib = rd_grib_msg(input, pos, &pds, &gds, &bms, &bds);
            if (len_grib <= 0) break;

	    if (day_count == 0) {
	        time = get_InitTime(pds);
		nxny = get_nxny(pds, gds, bms, bds);
	        if (nxny > array_size) {
		    free(count);
		    free(sum);
		    free(array);
		    array_size = nxny;
		    count = (int *) malloc(array_size * sizeof(int));
		    sum = (float *) malloc(array_size * sizeof(float));
		    array = (float *) malloc(array_size * sizeof(float));
		    if (count == NULL || sum == NULL || array == NULL) {
			fprintf(stderr,"memeory allocation failed\n");
			exit(8);
		    }
	        }
                for (i = 0; i < nxny; i++) {
	            count[i] = 0;
	            sum[i] = 0.0;
	        }
		field = PDS_Field(pds);
		pds0 = cpGRIBsec(pds);
		gds0 = cpGRIBsec(gds);
	    }
	    else {
		if (field != PDS_Field(pds)) break;
		if (nxny != get_nxny(pds, gds, bms, bds)) {
		    fprintf(stderr,"resolution change?\n");
		    exit(8);
		}
	    }

            unpk_bds(array, pds, gds, bms, bds, nxny);
	    for (i = 0; i < nxny; i++) {
		if (! UNDEFINED_VAL(array[i])) {
		    sum[i] += array[i];
		    count[i]++;
		}
	    }
	    pos += len_grib;
	    day_count++;
	}
        if (day_count == 0) break;
	if (day_count > ndays) {
	    fprintf(stderr,"big problems: number of records > number of days\n");
	    fprintf(stderr,"records %d >days %d\n",day_count, ndays);
	    exit(8);
	}
	for (i = 0; i < nxny; i++) {
	    sum[i] = count[i] != 0 ? sum[i]/count[i] : UNDEFINED;
	}

	printf("processing %d\n",time);

	get_TimeRange(pds0, &tr, &p1, &p2, &units, &nave, &nmissing);

	if (tr == 10) {
	    tr = 113;
            p1 = p1;
	    if (p2 != 0) {
		fprintf(stderr,"help .. need to improve program ..\n");
		fprintf(stderr,"bad grib head will be written\n");
	    }
            p2 = 6;
            units = HOUR;
	    nmissing = (ndays - day_count);
	    nave = day_count;
	}
	else {
	    fprintf(stderr,"unexpected time range %d\n", tr);
	    exit(8);
	}
	
	set_TimeRange(pds0, tr, p1, p2, units, nave, nmissing);
	/* if more than 20 days, increase resolution */
	if (ndays > 20) set_DecScale(pds0, get_DecScale(pds0) + 1);
        bms = mk_BMS(pds0, sum, &nxny, UNDEFINED_LOW, UNDEFINED_HIGH);
	bds = mk_BDS(pds0, sum, nxny);
        wrt_grib_msg(output, pds0, gds0, bms, bds);
	free(bds);
	if (bms) free(bms);
	free(gds0);
	free(pds0);
    }
    fclose(output);
    fclose(input);
    return 0;
}
