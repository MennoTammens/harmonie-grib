/*
 * rh  and temp -> spfh
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <math.h>
#include <float.h>

#include "gribwlib.h"
#include "pds4.h"
#include "ncepreanl.h"

double spfh(float temp, float relhum, float press);


int main(int argc, char **argv) {

    unsigned char *pds, *gds;
    FILE *input, *inv, *output;
    int i, iscale;

    float *data, *tmp;
    double shmax, shmin;
    int ndata, nmax, date;

    long int pos_tmp, pos_rh, len_grib;
    int lev, press;
    char *levels[] = {"1000 mb","925 mb","850 mb",
	"700 mb","600 mb","500 mb","400 mb","300 mb"};

    /* preliminaries .. open up all files */

    if (argc != 4) {
	fprintf(stderr, "%s [in gribfile] [inv] [out gribfile]\n", argv[0]);
	exit(8);
    }
    if ((input = fopen(argv[1],"rb")) == NULL) {
        fprintf(stderr,"could not open file: %s\n", argv[1]);
        exit(7);
    }
    if ((inv = fopen(argv[2],"rb")) == NULL) {
        fprintf(stderr,"could not open file: %s\n", argv[2]);
        exit(7);
    }
    if ((output = fopen(argv[3],"wb+")) == NULL) {
        fprintf(stderr,"could not open file: %s\n", argv[3]);
        exit(7);
    }

    nmax = 0;
    tmp = NULL;

    for (lev = 0; lev < sizeof(levels) / sizeof(char *); lev++) {
	rewind(inv);

	/* read the temperature */

	pos_tmp = scan3(inv, 0, "TMP", levels[lev]);
	if (pos_tmp < 0) continue;

	len_grib = rd_grib_rec(input, pos_tmp, &pds, &gds, &data, &ndata);
	if (len_grib <= 0) {
	    fprintf(stderr,"bad record\n");
	    exit(8);
	}
	if (ndata > nmax) {
	    if (tmp != NULL) free(tmp);    
	    nmax = ndata;
	    tmp = malloc(nmax*sizeof (float));
	    if (tmp == NULL) {
		fprintf(stderr,"malloc error\n");
		exit(8);
	    }
	}
	for (i = 0; i < ndata; i++) tmp[i] = data[i];

	/* read for the same date */

	date = PDS_Hour(pds) + 100*PDS_Day(pds) + 10000*PDS_Month(pds) +
		1000000*PDS_Year4(pds);

	/* read RH */

	pos_rh = scan3(inv, date, "RH", levels[lev]);
	if (pos_rh < 0) {
	    rewind(inv);
	    pos_rh = scan3(inv, 0, "RH", levels[lev]);
	}
	printf("%ld %ld\n", pos_tmp, pos_rh);
	len_grib = rd_grib_rec(input, pos_rh, &pds, &gds, &data, &ndata);
	if (len_grib <= 0) {
	    fprintf(stderr,"bad record\n");
	    exit(8);
	}

	/* compute SPFH */
	press = atoi(levels[lev]);
	for (i = 0; i < ndata; i++) {
	    if(!UNDEFINED_VAL(data[i]) && !UNDEFINED_VAL(tmp[i])) 
	        data[i] = spfh(tmp[i], data[i], (float) press);
	}

	/* figure out a good decimal scaling
	 * use what was used in NCEP/NCAR Reanalysis
	 * assume data is all defined
	 */
	shmax = 0;
	shmin = 1e20;
	for (i = 1; i < ndata; i++) {
	    if (!UNDEFINED_VAL(data[i])) {
	        shmin = shmin <= data[i] ? shmin : data[i];
	        shmax = shmax >= data[i] ? shmax : data[i];
	    }
	}
	shmax = shmax - shmin;
	iscale = 0;
	if (shmax != 0.0) {
	    while (shmax < 999.9) {
		iscale++;
		shmax *= 10.0;
	    }
	    while (shmax > 9999.9) {
		iscale--;
		shmax /= 10.0;
	    }
	}

	pds = PDStool(pds,P_param(SPFH),P_dec_scale(iscale), P_end);
	wrt_grib_rec(pds, gds, data, ndata, output);
    }

    fclose(output);
    fclose(inv);
    fclose(input);
    return 0;
}
