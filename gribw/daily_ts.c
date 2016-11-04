#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <math.h>
#include <float.h>

#include "gribw.h"
#include "pds4.h"


/*
 * take a 6-hourly time series, and make a daily time series
 *
 * 5/96                             Wesley Ebisuzaki
 *
 * 11/97: added time range = 2
 */

#define N 200000
#define DHOUR 6
#define TMAX 15
#define TMIN 16

int main(int argc, char **argv) {

    long int len_grib, pos = 0;
    unsigned char *pds, *gds, *bms, *bds;
    FILE *input, *output;
    int i, nxny;
    float *array, *sum;
    int *count;
    int array_size, hr;
    int tr, p1, p2, units, nave, nmissing, expected_count;
    unsigned int field;
    unsigned int time0, time;

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

    count = (int *) malloc(N * sizeof(int));
    sum = (float *) malloc(N * sizeof(float));
    array = (float *) malloc(N * sizeof(float));
    array_size = N;
    if (count == NULL || sum == NULL || array == NULL) {
	fprintf(stderr,"memeory allocation failed\n");
	exit(8);
    }

    pos = 0;
    for(;;) {
	hr = 0;
	while (hr < 24) {
            len_grib = rd_grib_msg(input, pos, &pds, &gds, &bms, &bds);
            if (len_grib <= 0) break;
	    time = get_InitYYYYMMDDHH(pds);
	    /* time = get_InitTime(pds); */

	    /* is it the expected time */
	    if (hr == 0) {
		if (time % 100 != 0) {
		    fprintf(stderr,"expecting 00Z not %d\n", time);
	    	    pos += len_grib;
		    continue;
		}
	    }
	    else {
	        if (time != time0 + hr) {
		    fprintf(stderr,"missing time %d %d\n",time,time0+hr);
		    hr = 0;
		    continue;
		}
	    }

            if (hr == 0) {
		time0 = time;
		nxny = get_nxny(pds, gds, bms, bds);
	        if (nxny > N) {
		    fprintf(stderr,"recompile with bigger N\n");
		    exit(8);
	        }
                for (i = 0; i < nxny; i++) {
	            count[i] = 0;
	            sum[i] = 0.0;
	        }
		field = PDS_Field(pds);
	    }
	    else {
		if (field != PDS_Field(pds)) {
		    fprintf(stderr,"changed field parameter\n");
		    hr = 0;
		    continue;
		}
		if (nxny != get_nxny(pds, gds, bms, bds)) {
		    fprintf(stderr,"resolution change?\n");
		    exit(8);
		}
	    }

            unpk_bds(array, pds, gds, bms, bds, nxny);
	    if (PDS_PARAM(pds) == TMAX) {
	        for (i = 0; i < nxny; i++) {
		    if (! UNDEFINED_VAL(array[i])) {
			if (count[i] == 0) {
			    sum[i] = array[i];
			    count[i] = 1;
			}
			else {
			    if (sum[i] < array[i]) sum[i] = array[i];
			}
		    }
	        }
	    }
            else if (PDS_PARAM(pds) == TMIN) {
                for (i = 0; i < nxny; i++) {
                    if (! UNDEFINED_VAL(array[i])) {
                        if (count[i] == 0) {
                            sum[i] = array[i];
			    count[i] = 1;
                        }
                        else {
                            if (sum[i] > array[i]) sum[i] = array[i];
                        }
                    }
                }
            }  
	    else {
	        for (i = 0; i < nxny; i++) {
		    if (! UNDEFINED_VAL(array[i])) {
		        sum[i] += array[i];
		        count[i]++;
		    }
	        }
	    }
	    pos += len_grib;
	    hr += DHOUR;
	}
        if (len_grib <= 0) break;

        expected_count = 24 / DHOUR;
        if (PDS_PARAM(pds) == TMIN || PDS_PARAM(pds) == TMAX) 
		expected_count = 1;

	for (i = 0; i < nxny; i++) {
	    if (count[i] == expected_count) {
		sum[i] /= (double) expected_count;
	    }
	    else {
		sum[i] = UNDEFINED;
	    }
	}

	printf("processing %d\n",time0);
	set_InitYYYYMMDDHH(pds, time0);

	get_TimeRange(pds, &tr, &p1, &p2, &units, &nave, &nmissing);
	if (tr == 3) {
	    p1 = 0;
	    p2 = 24;
            units = HOUR;
	    nave = (24/DHOUR);
	    nmissing = 0;
	}
	else if (tr == 10) {
	    tr = 113;
	    p1 = p2;
	    p2 = DHOUR;
            units = HOUR;
	    nave = (24/DHOUR);
	    nmissing = 0;
	}
	else if (tr == 2 && p1 == 0 && p2 == 6 && units == HOUR) {
	    tr = 2;
	    p1 = 0;
	    p2 = 24;
            units = HOUR;
	    nave = 0;
	    nmissing = 0;
	}
	else {
	    fprintf(stderr,"unexpected time range %d p=(%d,%d) units=%d\n", 
                  tr, p1, p2, units);
	    continue;
	}
	
	set_TimeRange(pds, tr, p1, p2, units, nave, nmissing);

        bms = mk_BMS(pds, sum, &nxny, UNDEFINED_LOW, UNDEFINED_HIGH);
	bds = mk_BDS(pds, sum, nxny);
        wrt_grib_msg(output, pds, gds, bms, bds);
	free(bds);
	if (bms) free(bms);

    }
    fclose(output);
    fclose(input);
    return 0;
}
