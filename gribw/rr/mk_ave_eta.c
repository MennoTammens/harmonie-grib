#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <math.h>
#include <float.h>

#include "gribwlib.h"
#include "pds4.h"

#define HOUR 1
#define DAY 2
#define HOURS3  10
#define HOURS6  11
#define HOURS12  12

/* #define IN_ORDER */

/*
 * take a 6-hourly time series, and make a monthly mean
 *
 * v0.1 10/99
 * v0.2 4/00 handles averages of averages
 * v0.3 9/00 fixed time_range = 113 -> time_range == 113
 * v0.4 12/01 added time_range = 4 -> time_range == 113A
 * v0.5 1/02 minor stuff .. overwriting data not found?
 * v0.6 12/03 fixup remove monthly mean hard coding
 */

#define N 100000


void update_data(float *sum, int *count,float *data, int ndata);
int write_mean(float *sum, int *count, unsigned char *pds,
	unsigned char *gds, int ndata, int dhr,
	int NumAve, int NumMissing, int scale2, FILE *output);


int main(int argc, char **argv) {

    long int len_grib, pos = 0;
    unsigned char *pds, *gds, *old_pds, *old_gds;
    FILE *input, *output;
    float *sum;
    int *count;
    int array_size;
    unsigned int field, old_field;
    int time;
    int new_mean, hour, dhr, eof, i;
    int scale10, scale2, old_scale2;
    char record[400];

    float *data;
    int ndata, old_ndata, NumAve, NumMissing;

    /* preliminaries .. open up all files */

    if (argc != 4) {
	fprintf(stderr, "%s [in gribfile] [out gribfile] dhr\n", argv[0]);
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
    dhr = atoi(argv[3]);
    if (dhr > 24) {
	fprintf(stderr,"dhr must be less than or equal to 24 (hours)\n");
	exit(8);
    }
    if (24 % dhr != 0) {
	fprintf(stderr,"dhr hours must be multiple of 24\n");
	exit(8);
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

	time = get_InitYYYYMMDDHH(pds);

	if (new_mean == 1 && NumAve > 0) {
		/* printf("field %d nave=%d\n",field, NumAve); */
		write_mean(sum,count,old_pds,old_gds,
		old_ndata,dhr,NumAve,NumMissing,old_scale2,output);
		if (old_pds == NULL) printf("old_pds == NULL\n");
		if (old_gds == NULL) printf("old_gds == NULL\n");
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
	hour = PDS_Hour(pds);
        update_data(sum,count,data,ndata);
	NumAve += (PDS_NumAve(pds) > 1 ? PDS_NumAve(pds) : 1);
	NumMissing += PDS_NumMissing(pds);

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


static int days_in_month(int yyyymm) {

    static int days[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
    int year, mo = yyyymm % 100;


    if (mo <= 0 || mo > 12) exit(9);

    if (mo != 2) return days[mo-1];
    year = yyyymm / 100;

    if (year % 4 != 0) return 28;
    if (year % 100 != 0) return 29;
    if (year % 400 != 0) return 28;
    return 29;
}

int write_mean(float *sum, int *count, unsigned char *pds,
	unsigned char *gds, int ndata, int dhr,
	int NumAve, int NumMissing, int scale2, FILE *output) {

    int dec_scale, i, count_max, p1, p2, time_range, time_units, new_time_range, nhour;
    static int m_count = 0;

    if (NumAve == 0) return 0;

    time_range = PDS_TimeRange(pds);

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

    /* increase precision by 1 digit */
    /* if (NumAve > 10) dec_scale = PDS_DecimalScale(pds) + 1; */
    /* pds = PDStool(pds, P_dec_scale(dec_scale), P_end); */

    time_range = PDS_TimeRange(pds);
    p1 = PDS_P1(pds);
    p2 = PDS_P2(pds);
    time_units =  PDS_ForecastTimeUnit(pds);

    if (time_units != HOUR) {
	fprintf(stderr,"help .. Forecast Time Unit not hours\n");
	fprintf(stderr,"extend code time_unit = %d\n",time_units);
	exit(8);
    }

    if (time_range == 0) {
	new_time_range = 113;
	p2 = dhr;
    }
    else if (time_range == 1) {
	new_time_range = 113;
	p1 = 0;
	p2 = dhr;
    }
    else if (time_range == 3 && dhr == 24) {
	new_time_range = 130;
    }
    else if (time_range == 3 && dhr == p2-p1) {
	new_time_range = 131;
    }
    else if (time_range == 4 && dhr == 24) {
	new_time_range = 128;
    }
    else if (time_range == 4 && dhr == p2-p1) {
	new_time_range = 129;
    }
    else if (time_range == 10) {
	new_time_range = 113;
	if (p1 != 0) {
	     fprintf(stderr,"forecast hour too big 255 used\n");
	     p1 = 255;
	}
	else {
	    p1 = p2;
	}
	p2 = dhr;
    }
    else if (time_range == 113) {
	new_time_range = 113;
	p2 = dhr < p2 ? dhr : p2;
    }
    else if (time_range == 128 && dhr == p2 - p1) {
	new_time_range = 129;
    }
    else if (time_range == 130 && dhr == p2 - p1) {
	new_time_range = 131;
    }
    else {
	fprintf(stderr,"help .. mean with time range %d p1 %d p2 %d\n",time_range,p1,p2);
	fprintf(stderr,"record not written\n");
	return 0;
    }
    pds = PDStool(pds, P_time_range(new_time_range),P_p1(p1),P_p2(p2),P_end);

    /* eta style output - works with mrf */
    set_BDSMinBits(-1);
    set_def_power2(-scale2);

    /* dont take averages of certain fields */
    if (time_range != 113 || (PDS_KPDS6(pds) != 2 && PDS_KPDS6(pds) != 3)) {
        wrt_grib_rec(pds, gds, sum, ndata, output);
    }
    return 0;
}
