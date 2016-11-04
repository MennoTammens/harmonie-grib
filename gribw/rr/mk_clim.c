#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <math.h>
#include <float.h>

#include "gribw.h"
#include "gribwlib.h"
#include "pds4.h"

#define HOUR 1
#define DAY 2
#define HOURS3  10
#define HOURS6  11
#define HOURS12  12

/* #define IN_ORDER */

/*
 * make climatologies
 *
 * based on mk_ave_eta.c
 *
 * v0.1 3/2004
 */

#define N 100000


void update_data(float *sum, int *count,float *data, int ndata);
int write_clim(float *sum, int *count, unsigned char *pds,
	unsigned char *gds, int ndata, 
	int NumAve, int NumMissing, int scale2, FILE *output);


int main(int argc, char **argv) {

    long int len_grib, pos = 0;
    unsigned char *pds, *gds, *old_pds, *old_gds;
    FILE *input, *output;
    float *sum;
    int *count;
    int array_size;
    unsigned int field, old_field;
    int new_mean,  eof, i;
    int scale10, scale2, old_scale2=0;
    char record[400];

    float *data;
    int ndata, old_ndata, NumAve, NumMissing;

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
    if (count == NULL || sum == NULL) {
	fprintf(stderr,"memeory allocation failed\n");
	exit(8);
    }
    array_size = N;
    for (i = 0; i < array_size; i++) {
	count[i] = 0;
	sum[i] = 0.0;
    }

    pos = 0;
    new_mean = 1;
    old_field = -1;
    NumAve = NumMissing = 0;
    old_ndata = 0;
    eof = 0;
    old_pds = old_gds = NULL;

    for(;;) {
#ifdef IN_ORDER
	len_grib = rd_grib_rec2(input, pos, &pds, &gds, &data, &ndata,
		&scale10, &scale2);
	if (len_grib <= 0) eof = 1;
#else
	if (fgets(record, sizeof(record), stdin) == NULL) eof = 1;
	if (sscanf(record, "%d:%ld:", &i, &pos) != 2) eof = 1;
	if (eof == 0) {
	    len_grib = rd_grib_rec2(input, pos, &pds, &gds, &data, &ndata,
		&scale10, &scale2);
	    if (len_grib <= 0) eof = 1;
	}
#endif

	/* check to see if we have a new variable/month */

        if (eof == 1) new_mean = 1;

	field = PDS_Field(pds);
	if (old_field != field) new_mean = 1;

	if (new_mean == 1 && NumAve > 0) {
	    /* printf("field %d nave=%d\n",field, NumAve); */
	    write_clim(sum,count,old_pds,old_gds,
	    old_ndata,NumAve,NumMissing,old_scale2,output);
	    if (old_pds == NULL) printf("old_pds == NULL\n");
	    if (old_pds) free(old_pds);
	    if (old_gds) free(old_gds);
	    old_pds = old_gds = NULL;
	}

        if (eof) break;

	if (new_mean) {
	    /* initialize for new mean */
            if (ndata > array_size) {
                count = (int *) realloc(count, ndata * sizeof(int));
                sum = (float *) realloc(sum, ndata * sizeof(float));
                array_size = ndata;
                if (count == NULL || sum == NULL) {
                    fprintf(stderr,"memeory allocation failed\n");
                    exit(8);
                }
            }
            for (i = 0; i < ndata; i++) {
                count[i] = 0;
                sum[i] = 0.0;
            }
	    old_field = field;
	    NumAve = NumMissing = new_mean = 0;
	    old_ndata = ndata;
	    old_pds = cpGRIBsec(pds);
	    old_gds = cpGRIBsec(gds);
	    old_scale2 = scale2;
	}

	if (ndata != old_ndata) {
	    fprintf(stderr,"fatal error .. size of data array changed!"
		" %d %d\n",ndata,old_ndata);
	    exit(8);
	}
        update_data(sum,count,data,ndata);
	NumAve++;

	pos += len_grib;
   }
   fclose(input);
   fclose(output);
   exit (0);
}

void update_data(float *sum, int *count, float *data, int ndata) 
{
int i;

    for (i = 0; i < ndata; i++) {
	if (! UNDEFINED_VAL(data[i])) {
	    sum[i] += data[i];
	    count[i]++;
	}
    }
}


int write_clim(float *sum, int *count, unsigned char *pds,
	unsigned char *gds, int ndata, 
	int NumAve, int NumMissing, int scale2, FILE *output) {

    int i, count_max, p1, p2, time_range, time_units, nave;
    static int m_count = 0;

    if (NumAve == 0) return 0;

    time_range = PDS_TimeRange(pds);
    nave = PDS_NumAve(pds);

    for (count_max = i = 0; i < ndata; i++) {
	count_max = count_max < count[i] ? count[i] : count_max;
	if (count[i] > 0) {
	    sum[i] /= count[i];
	}
	else {
	    sum[i] = UNDEFINED;
	}
    }

    printf("%d: writing ave out %d points time_range %d nave=%d\n", ++m_count, ndata, time_range,count_max);


    /* number of available and missing obs */
    pds = PDStool(pds,P_used(NumAve),P_missing(NumMissing),P_end);

    time_range = PDS_TimeRange(pds);
    p1 = PDS_P1(pds);
    p2 = PDS_P2(pds);
    time_units =  PDS_ForecastTimeUnit(pds);

    if (time_units != HOUR) {
	fprintf(stderr,"help .. Forecast Time Unit not hours\n");
	fprintf(stderr,"extend code time_unit = %d\n",time_units);
	exit(8);
    }

    /*
     * convention:
     *   minute = 0 +  0 => ave of analyses
     *   minute = 0 + 12 => ave of 3 hour forecasts
     *   minute = 1 + 12 => ave of 0..3 hour average forecasts
     *   minute = 2 + 12 => ave of 0..3 hour accumulated forecasts
     */

    pds = PDStool(pds, P_time_range(51), P_end);

    if (time_range == 0) {
	/* fcst: daily average output */
        pds = PDStool(pds, P_p1(1), P_p2(p1), P_minute(p1*4), P_fcst_unit(HOUR), P_end);
    }
    else if (time_range == 3) {
	/* ave: assume 3 hour average output */
        pds = PDStool(pds, P_p1(1), P_p2(p2), P_minute(p2*4+1), P_fcst_unit(HOUR), P_end);
    }
    else if (time_range == 4) {
	/* acc: assume 3 hour accumulation */
        pds = PDStool(pds, P_p1(1), P_p2(p2), P_minute(p2*4+2), P_fcst_unit(HOUR), P_end);
    }
    else if (time_range == 113) {
	/* average of forecasts or analyses */
	if ((p2 == 24 && time_units == HOUR) || (p2 == 1 && time_units == DAY)) {
            pds = PDStool(pds, P_p1(1), P_p2(nave), P_minute(p1*4), P_fcst_unit(DAY), P_end);
	}
	else {
	    i = nave*p2;
	    if (i % 24 == 0) {
	        i = i / 24;
                pds = PDStool(pds, P_fcst_unit(DAY), P_end);
	    }
            pds = PDStool(pds, P_p1(0), P_p2(i), P_minute(p1*4), P_end);
	}
    }
    else if (time_range == 129) {
	/* average of successive forecast accumulations */
	i = nave*p2;
	if (i % 24 == 0) {
	    i = i / 24;
            pds = PDStool(pds, P_fcst_unit(DAY), P_end);
	}
        pds = PDStool(pds, P_p1(0), P_p2(i), P_minute(p2*4+2), P_end);
    }
    else if (time_range == 131) {
	/* average of successive forecast accumulations */
	i = nave*p2;
	if (i % 24 == 0) {
	    i = i / 24;
            pds = PDStool(pds, P_fcst_unit(DAY), P_end);
	}
        pds = PDStool(pds, P_p1(0), P_p2(i), P_minute(p2*4+1), P_end);
    }
    else if (time_range == 128) {
	/* average of daily forecast accumulations */
        pds = PDStool(pds, P_p1(1), P_p2(nave), P_fcst_unit(DAY),  P_minute(p2*4+2), P_end);
    }
    else if (time_range == 130) {
	/* average of daily forecast accumulations */
        pds = PDStool(pds, P_p1(1), P_p2(nave), P_fcst_unit(DAY), P_minute(p2*4+1), P_end);
    }
    else {
	fprintf(stderr,"help .. mean with time range %d p1 %d p2 %d\n",time_range,p1,p2);
	fprintf(stderr,"record not written\n");
	return 0;
    }

    /* eta style output - works with mrf */
    set_BDSMinBits(-1);
    set_def_power2(-scale2);

    /* dont take averages of certain fields */
    wrt_grib_rec(pds, gds, sum, ndata, output);
    return 0;
}
