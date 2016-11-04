#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <math.h>
#include <float.h>

#include "gribw.h"
#include "gribwlib.h"
#include "pds4.h"
#include "bds.h"

#include "nceptab_131.h"

/*
 * Make a pressure weighted vertical integral of RR fields
 *
 * easily changed to any other model
 *
 * input: sfc pressure
 * 
 * v0.1
 *                              Wesley Ebisuzaki
 */

#define NXNY (349*277)
#define NLEV 29

int level[NLEV] = { 100, 125, 150, 175, 200, 225, 250, 275, 300, 350, 
  400, 450, 500, 550, 600, 650, 700, 725, 750, 775, 800, 825, 
  850, 875, 900, 925, 950, 975, 1000};

int main(int argc, char **argv) {


    long int len_grib, pos = 0;
    unsigned char *pds, *gds;
    FILE *input, *output;
    float sfc_pres[NXNY];
    float *data[NLEV];
    float sum, vertint[NXNY], weight, sfc, dp, ddp, ave_val, v0, v1;

    int kpds5, ilev;

    int scale10, scale2;
    int ndata;
    float *array = NULL;
    int i;


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
    
    for (i = 0; i < NLEV; i++) {
	data[i] = malloc( NXNY * sizeof(float));
        if (data[i] == NULL) {
	    fprintf(stderr,"lack of memory\n");
	    exit(7);
	}
    }

    set_BDSMinBits(-1);	/* ETA-style mode */

    /* read the surface pressure */

    len_grib = rd_grib_rec2(input, pos, &pds, &gds, &array, &ndata, 
          &scale10, &scale2);
    pos += len_grib;

    if (len_grib <= 0 || ndata != NXNY) {
	fprintf(stderr,"wrong dimensions\n");
	exit(8);
    }
    if (PDS_PARAM(pds) != PRES) {
	fprintf(stderr,"expecting surface pressure\n");
	exit(8);
    }
    if (PDS_L_TYPE(pds) != 1) {
	fprintf(stderr,"expecting surface pressure\n");
	exit(8);
    }

    for (i = 0; i < NXNY; i++) sfc_pres[i] = array[i];
    printf("read surface pressure\n");

    /* read the vertical stack */

    for (ilev = 0; ilev < NLEV; ilev++) {
        len_grib = rd_grib_rec2(input, pos, &pds, &gds, &array, &ndata, 
            &scale10, &scale2);
        pos += len_grib;
        if (len_grib <= 0 || ndata != NXNY) {
            fprintf(stderr,"wrong dimensions\n");
            exit(8);
	}

        /* check variable type */

	if (ilev == 0) {
            kpds5 = PDS_KPDS5(pds);
	}
	else if (kpds5 != PDS_KPDS5(pds)) {
            fprintf(stderr,"mixed variables\n");
            exit(8);
	}

	if (PDS_KPDS6(pds) != 100) {
            fprintf(stderr,"unexpected level type %d\n", PDS_KPDS6(pds));
            exit(8);
	}
	if (PDS_KPDS7(pds) != level[ilev]) {
            fprintf(stderr,"unexpected level %d expected %d\n", 
		PDS_KPDS7(pds), level[ilev]);
            exit(8);
	}
        for (i = 0; i < NXNY; i++) data[ilev][i] = array[i];
    }
    printf("finished reading data\n");

    /* init sum and weights */

    /* do vertical integral */
    for (i = 0; i < NXNY; i++) {
	sfc = sfc_pres[i];
	sum = 0.0;
	weight = 0.0;
	if (! UNDEFINED_VAL(sfc)) {
	    sfc = sfc / 100.0;	/* Pa -> mb */
	    for (ilev = 1; ilev < NLEV && level[ilev-1] < sfc; ilev++) {
	        dp = level[ilev]-level[ilev-1];
		v0 = data[ilev-1][i];
		v1 = data[ilev][i];

	        if (level[ilev] < sfc) {
		    sum += 0.5*(v0 + v1)*dp;
		    weight += dp;
	        }
	        else {
		    ddp = sfc - level[ilev-1];
		    ave_val = v1*0.5*ddp/dp + 
			v0*(1.0 - 0.5*ddp/dp);
		    sum += ave_val*ddp;
		    weight += ddp;
	        }
	    }
	    /* surface is below vertical stack */
	    /* extrapolate */
	    if (level[NLEV-1] < sfc) {
		dp = sfc - level[NLEV-1];
		sum += data[NLEV-1][i]*dp;
		weight += dp;
	    }
	}
	vertint[i] = weight > 0.0 ? sum / weight : UNDEFINED;
    }

    pds = PDStool(pds, P_atmos_clm,  P_end);
    set_def_power2(-scale2);

    wrt_grib_rec(pds, gds, vertint, ndata, output);


    fclose(output);
    fclose(input);
    return 0;
}
