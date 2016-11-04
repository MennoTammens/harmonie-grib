#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <math.h>
#include <float.h>

#include "gribwlib.h"
#include "pds4.h"

/*
 * interpolates by date
 * convert weekly sst to daily
 *
 * assume data is sorted by date and only has one type of field
 *
 * algorithm
 *   read GRIB1
 *   write GRIB1
 *   while (read(GRIB2)) {
 *      ndays = diff jdate(GRIB2) - jdate(GRIB1)
 *      do jdate = jdate(GRIB1)+1, jdate(GRIB2)
 *         GRIB3 = (i*GRIB2+(ndays-i)*GRIB1)/ndays
 *         write GRIB3
 *      enddo
 *   }
 *   
 */
int tojul(int month, int day, int year);
void unjul(int julday, int *month, int *day, int *years);


int main(int argc, char **argv) {

    FILE *input, *output;
    long int len_grib, pos = 0;
    unsigned char *pds, *gds;
    int ndata, i, ndays, j;
    int scale10, scale2;
    int ndata0, kpds5, kpds6, kpds7;
    float *data0, *d_data, *data;
    int julian0, julian, year, month, day;
    int d_day, d_month, d_year;

    if (argc != 3) {
	fprintf(stderr, "%s [in gribfile] [out gribfile]", argv[0]);
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

    /* read and write 1st record */

    pos = 0;
    len_grib = rd_grib_rec(input, pos, &pds, &gds, &data, &ndata);
    if (len_grib <= 0) {
	fprintf(stderr,"not a grib file\n");
	exit(8);
    }

    data0 = (float *) malloc(ndata * sizeof(float));
    d_data = (float *) malloc(ndata * sizeof(float));
    for (i = 0; i < ndata; i++) {
	data0[i] = data[i];
    }

    wrt_grib_rec(pds, gds, data, ndata, output);
    pos += len_grib;

    /* save data */
    ndata0=ndata;
    year = PDS_Year4(pds);
    month = PDS_Month(pds);
    day = PDS_Day(pds);
    julian0 = tojul(month, day, year);

    kpds5 = PDS_KPDS5(pds);
    kpds6 = PDS_KPDS6(pds);
    kpds7 = PDS_KPDS7(pds);


    while ((len_grib = rd_grib_rec(input, pos, &pds, &gds, &data, &ndata)) > 0) {
	if (ndata != ndata0) {
	    fprintf(stderr,"ndata differs\n");
	    exit(9);
        }
	if (kpds5 != PDS_KPDS5(pds)) {
	    fprintf(stderr,"variable differs\n");
	    exit(9);
	}
	if (kpds6 != PDS_KPDS6(pds)) {
	    fprintf(stderr,"level type differs\n");
	    exit(9);
	}
	if (kpds7 != PDS_KPDS7(pds)) {
	    fprintf(stderr,"level value differs\n");
	    exit(9);
	}
        year = PDS_Year4(pds);
        month = PDS_Month(pds);
        day = PDS_Day(pds);
        julian = tojul(month, day, year);
	ndays = julian - julian0;
	printf("%d %d %d ndays %d pos %ld %ld\n", year, month, day, ndays, pos, len_grib);

	if (julian <= julian0) {
	    fprintf(stderr,"dates are not ordered: %d %d\n",julian0,julian);
	    exit(9);
	}

	for (i = 1; i <= ndays; i++) {
	    for (j = 0; j <= ndata; j++) {
		if (UNDEFINED_VAL(data[j])  || UNDEFINED_VAL(data0[j])) {
		    d_data[j] = UNDEFINED;
		}
		else {
		    d_data[j] = (i*data[j] + (ndays-i)*data0[j])/ndays;
		}
	    }
	    unjul(julian0 + i, &d_month, &d_day, &d_year);
	    pds = PDStool(pds,P_day(d_day),P_month(d_month),
		P_year(d_year),P_end);
	    wrt_grib_rec(pds, gds, d_data, ndata, output);
	}
	for (i = 0; i < ndata; i++) {
	    data0[i] = data[i];
	}
	julian0 = julian;
	pos += len_grib;
    }
}
