#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <math.h>
#include <float.h>

#include "gribwlib.h"
#include "pds4.h"

#define HOUR	1
#define DAY	2
#define HOURS3  10
#define HOURS6  11
#define HOURS12	12

#define AVE_TIME_RANGE			113
#define DAILY_AVE_TIME_RANGE		129
#define SUCCESSIVE_AVE_TIME_RANGE	130
#define DAILY_ACC_TIME_RANGE		131
#define SUCCESSIVE_ACC_TIME_RANGE	132


/*
 * take a 3/6 hour data and make means of all sorts
 *  daily, monthy, stratified monthly
 *
 * v0.1 10/99
 * v0.2 4/00 handles averages of averages
 * v0.3 9/00 fixed time_range = 113 -> time_range == 113
 * v0.4 12/01 added time_range = 4 -> time_range == 113
 * v0.5 1/02 minor stuff .. overwriting data not found?
 * v0.6 4/03 use new time range codes
 *           eta-style precision
 *
 * stdin = sorted inventory
 */

#define N 100000

/* special code for TMIN and TMAX .. center PDS dependant numbers */
/* set TMIN and TMAX to -1 for no TMIN/TMAX processing */

#define TMAX 15
#define TMIN 16

void update_data(float *sum, int *count,float *data, int ndata);


int write_mean(float *sum, int *count, unsigned char *pds,
	unsigned char *gds, int ndata, int scale2, int min_hour, int dhr,
	int NumAve, FILE *output);


int main(int argc, char **argv) {

    long int len_grib, pos = 0;
    unsigned char *pds, *gds, *old_pds, *old_gds;
    FILE *input, *output;
    float *sum;
    int *count;
    int array_size;
    unsigned int field, old_field;
    int time, old_yyyymm, old_yyyymmdd;
    int time_range, old_time_range;
    int new_mean, min_hour, hour, dhr, eof, i;
    int scale10, scale2, old_scale2;
    char record[400];

    float *data;
    int ndata, old_ndata, NumAve, ntime;

    /* preliminaries .. open up all files */

    if (argc != 5) {
	fprintf(stderr, "usage: %s [in gribfile] [out gribfile] dhr ntime\n", argv[0]);
	fprintf(stderr, "  ntime = number of time intervals\0");
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
    ntime = atoi(argv[4]);
    if (ntime <= 0) {
	fprintf(stderr,"ntime must be > 0\n");
	exit(8);
    }

    /* arrays for averaging */
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
    old_yyyymm = -1;
    old_field = -1;
    old_time_range = -1;
    old_time = -1;
    NumAve = 0;
    old_ndata = 0;
    old_scale2 = 0;
    min_hour = -1;
    eof = 0;
    old_pds = old_gds = NULL;

    for(;;) {

	/* get next (sorted) record */
	if (fgets(record, sizeof(record), stdin) == NULL) eof = 1;
	if (sscanf(record, "%d:%ld:", &i, &pos) != 2) eof = 1;

	/* read record */
	if (eof == 0) {
            len_grib = rd_grib_rec2(input, pos, &pds, &gds, &data, &ndata,
		&scale10, &scale2);
	    if (len_grib <= 0) eof = 1;
	}

	/* check to see if we need to start a new mean */

	/* if no more data, make sure to make mean of previous data */
        if (eof == 1) new_mean = 1;

	field = PDS_Field(pds);
	if (old_field != field) new_mean = 1;

	time_range = PDS_TimeRange(pds);
	if (old_time_range != time_range) new_mean = 1;

	time = get_InitYYYYMMDDHH(pds);

	if (new_mean == 1 && NumAve > 0) {
	    /* printf("field %d nave=%d\n",field, NumAve); */
	    write_mean(sum,count,old_pds,old_gds,
		old_ndata,old_scale2,min_hour,dhr,NumAve,output);
	    if (old_pds == NULL) printf("old_pds == NULL\n");
	    if (old_gds == NULL) printf("old_gds == NULL\n");
	    if (old_pds) free(old_pds);
	    if (old_gds) free(old_gds);
	    old_pds = old_gds = NULL;
	}

        if (eof) break;

	/* check time code */

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
	    old_time_range = time_range;
	    NumAve = new_mean = 0;
	    old_ndata = ndata;
	    old_pds = cpGRIBsec(pds);
	    old_gds = cpGRIBsec(gds);
	    min_hour = PDS_Hour(pds);
	    old_scale2 = scale2;
	}

	if (ndata != old_ndata) {
	    fprintf(stderr,"fatal error .. size of data array changed!"
		" %d %d\n",ndata,old_ndata);
	    exit(8);
	}
	hour = PDS_Hour(pds);
        update_data(sum,count,data,ndata);
	min_hour = min_hour < hour ? min_hour : hour;
	NumAve += (PDS_NumAve(pds) > 1 ? PDS_NumAve(pds): 1);
   }
   fclose(input);
   fclose(output);
   exit (0);
}

void update_data(float *sum, int *count, float *data, int ndata) {
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

/*
 * assume time unit = hours,
 * change units to try to make it fit
 */

int normalize(int *p1, int *p2, unsigned char *pds) {

    if (*p2 < 256) return;

    if (*p1 % 24 == 0 && *p2 % 24 == 0 && *p1/24 < 256) {
	*p1 /= 24;
	*p2 /= 24;
        pds = PDStool(pds, P_fcst_unit(DAY), P_end);
	return 1;
    }
    if (*p1 % 12 == 0 && *p2 % 12 == 0 && *p1/12 < 256) {
	*p1 /= 12;
	*p2 /= 12;
        pds = PDStool(pds, P_fcst_unit(HOURS12), P_end);
	return 1;
    }
    if (*p1 % 6 == 0 && *p2 % 6 == 0 && *p1/6 < 256) {
	*p1 /= 6;
	*p2 /= 6;
        pds = PDStool(pds, P_fcst_unit(HOURS6), P_end);
	return 1;
    }
    if (*p1 % 3 == 0 && *p2 % 3 == 0 && *p1/3 < 256) {
	*p1 /= 3;
	*p2 /= 3;
        pds = PDStool(pds, P_fcst_unit(HOURS3), P_end);
	return 1;
    }

    fprintf(stderr,"help .. normalize\n");
    fprintf(stderr,"p1 %d, p2 %d\n", p1, p2);
    fprintf(stderr,"record not written\n");
    return 0;
}

/* use new grib time ranges */


int write_mean(float *sum, int *count, unsigned char *pds,
   unsigned char *gds, int ndata, int scale2, int min_hour, int dhr,
    int NumAve, FILE *output) {

    int dec_scale, i, count_max, p1, p2, time_range, time_units, 
	new_time_range, nhour;
    static int m_count = 0;

    if (NumAve == 0) return 0;

    time_range = PDS_TimeRange(pds);
    printf("%d: writing ave out %d points time_range %d NumAve %d\n", 
		++m_count, ndata, time_range, NumAve);

    for (count_max = i = 0; i < ndata; i++) {
	count_max = count_max < count[i] ? count[i] : count_max;
	sum[i] = count[i] > 0 ? sum[i]/count[i] : UNDEFINED;
    }


    /* number of missing obs */
    /* i = days_in_month(PDS_Year4(pds)*100 + PDS_Month(pds))*24/dhr - NumAve; */
    /* pds = PDStool(pds,P_used(NumAve),P_missing(i),P_end); */
    pds = PDStool(pds,P_used(NumAve),P_missing(0),P_end);


    /* set day, hour minute */
    pds = PDStool(pds,P_day(1),P_hour(min_hour),P_minute(0),P_end);

    /* increase precision by 1 digit */
    if (NumAve > 10) dec_scale = PDS_DecimalScale(pds) + 1;
    pds = PDStool(pds, P_dec_scale(dec_scale), P_end);

    time_range = PDS_TimeRange(pds);
    p1 = PDS_P1(pds);
    p2 = PDS_P2(pds);
    time_units = PDS_ForecastTimeUnit(pds);
    if (time_units != HOUR) {
	fprintf(stderr,"help .. Forecast Time Unit not hours\n");
	fprintf(stderr,"extend code\n");
	exit(8);
    }

    /* after p1 = forecast hour, p2 = dhr */

    if (time_range == 0) {
	p2 = dhr;
        new_time_range = AVE_TIME_RANGE;
    }
    else if (time_range == 1) {
	p1 = 0;
	p2 = dhr;
        new_time_range = AVE_TIME_RANGE;
    }
    else if (time_range == 2 || time_range == 3) {
	if (dhr == 24) {
	    new_time_range = DAILY_AVE_TIME_RANGE;
	}
	else if (p2 - p1 == dhr) {
	    new_time_range = SUCCESSIVE_AVE_TIME_RANGE;
	}
	else {
	    fprintf(stderr,"help..problem with time range\n");
	    fprintf(stderr,"timerange=%d p1=%d p2=%d\n", time_range, p1, p2);
	    return 8;
	}
    }
    else if (time_range == 4) {
	if (dhr == 24) {
	    new_time_range = DAILY_ACC_TIME_RANGE;
	}
	else if (p2 - p1 == dhr) {
	    new_time_range = SUCCESSIVE_ACC_TIME_RANGE;
	}
	else {
	    fprintf(stderr,"help..problem with time range\n");
	    fprintf(stderr,"timerange=%d p1=%d p2=%d\n", time_range, p1, p2);
	    return 8;
	}
    }
    else if (time_range == 10) {
        if (p1 != 0) {
	    fprintf(stderr,"forecast hour too big 255 used\n");
	    p1 = 255;
	}
	else p1 = p2;
	p2 = dhr;
        new_time_range = AVE_TIME_RANGE;
    }

    /* cases of averages of averages */

    else if (time_range == AVE_TIME_RANGE && time_units == HOUR && p2 == 24) {
	p2 = dhr;
	new_time_range = AVE_TIME_RANGE;
    }
    else if (time_range == DAILY_ACC_TIME_RANGE && (time_units == HOUR) &&
	(p2 - p1 == dhr)) {
	new_time_range = SUCCESSIVE_ACC_TIME_RANGE;
    }
    else if (time_range == DAILY_AVE_TIME_RANGE && (time_units == HOUR) &&
	(p2 - p1 == dhr)) {
	new_time_range = SUCCESSIVE_AVE_TIME_RANGE;
    }

    else {
	fprintf(stderr,"help .. mean with time range %d\n",time_range);
	fprintf(stderr,"record not written\n");
	return 0;
    }

    pds = PDStool(pds, P_time_range(new_time_range),P_p1(p1),P_p2(p2),P_end);

    /* eta style output - works with mrf */
    set_BDSMinBits(-1);
    set_def_power2(-scale2);

    wrt_grib_rec(pds, gds, sum, ndata, output);
    return 0;
}
