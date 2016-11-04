#include <stdio.h>
#include "gribwlib.h"
#include "pds4.h"
#include "ncepreanl.h"

/*
 * A problem with some of flx files didn't have bitmaps
 *   this program adds the bitmaps - Reanalysis 2
 *
 * 3/99                             Wesley Ebisuzaki
 */


#define is_cld(x)	((x) == 212 || (x) == 213 || (x) == 222 || (x) == 223 || \
	(x) == 232 || (x) == 233)
#define NGAU	(192*94)

void main(int argc, char **argv) {

    long int pos = 0, pos_fixed = 0;
    int len_grib, i, ndata;
    unsigned char *pds, *gds, *bms, *bds;
    float *data;
    float land[NGAU], ice[NGAU], tmpdata[NGAU];
    float tcdc_low[NGAU], tcdc_mid[NGAU], tcdc_hi[NGAU];
    int land_date = 0, ice_date = 0, date;
    int low_date = 0, mid_date = 0, hi_date=0;
    FILE *input, *output;

    /* preliminaries .. open up all files */

    if (argc != 3) {
	fprintf(stderr, "%s [in gribfile] [out gribfile]\n", argv[0]);
	fprintf(stderr, " modifies grib file\n");
	exit(6);
    }
    if ((input = fopen(argv[1],"rb")) == NULL) {
        fprintf(stderr,"could not open file: %s\n", argv[1]);
        exit(7);
    }
    if ((output = fopen(argv[2],"wb")) == NULL) {
        fprintf(stderr,"could not open file: %s\n", argv[2]);
        exit(8);
    }

    /* main loop */
    while ((len_grib = rd_grib_rec(input, pos, &pds, &gds, &data, &ndata)) > 0) {
	printf("len_grib %d pos %d\n",len_grib,pos);
	if (ndata != NGAU) {
		fprintf(stderr,"wrong grib file!!!\n");
		exit(8);
	}
	date = get_InitYYYYMMDDHH(pds);
	while (date != land_date || date != ice_date ||
		date != low_date || date != mid_date || date != hi_date) {

		len_grib = rd_grib_rec(input,pos_fixed,&pds,&gds,&data,&ndata);
		printf("len_grib %d at search\n",len_grib);
		if (len_grib == -1) {
			fprintf(stderr,"land or ice not found for %d\n",date);
			exit(8);
		}
		pos_fixed += len_grib;
		if (get_InitYYYYMMDDHH(pds) != date) continue;

		if (PDS_PARAM(pds) == LAND) {
			for (i = 0; i < NGAU; i++) {
				land[i] = data[i];
			}
			land_date = date;
		}
		else if (PDS_PARAM(pds) == ICEC) {
			for (i = 0; i < NGAU; i++) {
				ice[i] = data[i];
			}
			ice_date = date;
		}
		if (PDS_PARAM(pds) == TCDC && PDS_L_TYPE(pds) == 214) {
			for (i = 0; i < NGAU; i++) {
				tcdc_low[i] = data[i];
			}
			low_date = date;
		}
		if (PDS_PARAM(pds) == TCDC && PDS_L_TYPE(pds) == 224) {
			for (i = 0; i < NGAU; i++) {
				tcdc_mid[i] = data[i];
			}
			mid_date = date;
		}
		if (PDS_PARAM(pds) == TCDC && PDS_L_TYPE(pds) == 234) {
			for (i = 0; i < NGAU; i++) {
				tcdc_hi[i] = data[i];
			}
			hi_date = date;
		}
	}
	/* too lazy to save CPU cycles */
	len_grib = rd_grib_rec(input, pos, &pds, &gds, &data, &ndata);

	/* ground flux, pot evapor. over land and sea-ice */
	if (PDS_PARAM(pds) == GFLUX || PDS_PARAM(pds) == PEVPR) {
		for (i = 0; i < ndata; i++) {
			if (land[i] == 0 && ice[i] == 0) data[i] = UNDEFINED;
		}
	}
	/* runoff, ground water runoff .. weird */
	else if (PDS_PARAM(pds) == RUNOF || PDS_PARAM(pds) == BGRUN) {
		for (i = 0; i < ndata; i++) {
			if (land[i] == 0 && ice[i] == 0) data[i] = UNDEFINED;
		}
	}
	/* soil wetness */
	else if (PDS_PARAM(pds) == SOILW) {
		for (i = 0; i < ndata; i++) {
			if (land[i] == 0) data[i] = UNDEFINED;
		}
	}
	/* soil temperature */
	else if (PDS_PARAM(pds) == TMP && (PDS_L_TYPE(pds) == 111 ||
			PDS_L_TYPE(pds) == 112)) {
		for (i = 0; i < ndata; i++) {
			if (land[i] == 0) data[i] = UNDEFINED;
		}
	}
	/* low cloud pressure, temperature */
	else if ((PDS_PARAM(pds) == TMP || PDS_PARAM(pds) == PRES) &&
			(PDS_L_TYPE(pds) == 212 || PDS_L_TYPE(pds) == 213)) {
		for (i = 0; i < ndata; i++) {
			if (tcdc_low[i] == 0) data[i] = UNDEFINED;
		}
	}
	/* mid cloud pressure, temperature */
	else if ((PDS_PARAM(pds) == TMP || PDS_PARAM(pds) == PRES) &&
			(PDS_L_TYPE(pds) == 222 || PDS_L_TYPE(pds) == 223)) {
		for (i = 0; i < ndata; i++) {
			if (tcdc_mid[i] == 0) data[i] = UNDEFINED;
		}
	}
	/* hi cloud pressure, temperature */
	else if ((PDS_PARAM(pds) == TMP || PDS_PARAM(pds) == PRES) &&
			(PDS_L_TYPE(pds) == 232 || PDS_L_TYPE(pds) == 233)) {
		for (i = 0; i < ndata; i++) {
			if (tcdc_hi[i] == 0) data[i] = UNDEFINED;
		}
	}

	wrt_grib_rec(pds, gds, data, ndata, output);
	pos += len_grib;
    }
    exit(0);
}
