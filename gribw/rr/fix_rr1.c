#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <math.h>
#include <float.h>

#include "gribw.h"
#include "pds4.h"
#include "bds.h"

#include "nceptab_131.h"
#define PREC 1

/*
 *
 *                              Wesley Ebisuzaki
 */

#define NXNY (349*277)

int main(int argc, char **argv) {

    long int len_grib, pos = 0;
    unsigned char *pds, *gds, *bms, *bds, *gds2;
    FILE *input, *output, *maskfile;
    int count, cnvt, grid, scale10, scale2;
    int ndata, time_range, p1, p2;
    float *array = NULL;
    float undef, undef_low, undef_hi;
    int array_size = 0, i, fld, level_type;
    float mask[NXNY];

    /* preliminaries .. open up all files */

    if (argc != 4) {
	fprintf(stderr, "%s [in gribfile] [out gribfile] awipland.grb\n", argv[0]);
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

    /* open land mask */
    if ((maskfile = fopen(argv[3], "rb")) == NULL) {
	fprintf(stderr,"could not land mask open file: %s\n", argv[3]);
	exit(7);
    }
    pos = 0;
    len_grib = rd_grib_rec(maskfile, pos, &pds, &gds, &array, &ndata);
    fclose(maskfile);
    if (len_grib == 0 || ndata != NXNY) {
	fprintf(stderr,"unexpected data in land mask file: %s\n", "veg.grb");
	exit(7);
    }
    /* array will get reused */
    for (i = 0; i < NXNY; i++) {
	mask[i] = array[i];
    }
    printf("finished reading land mask: %s\n", argv[3]);

    set_BDSMinBits(-1);	/* ETA-style mode */

    pos = count = cnvt = 0;
    for(;;) {

	/* read the grib file */
        len_grib = rd_grib_rec2(input, pos, &pds, &gds, &array, &ndata, 
          &scale10, &scale2);
	if (len_grib <= 0) break;
	if (ndata != NXNY) {
	    fprintf(stderr,"error in grid size\n");
	    exit(8);
	}

	p1 = PDS_P1(pds);
	p2 = PDS_P2(pds);
        time_range = PDS_TimeRange(pds);

	/* printf("scale 10 %d scale-2 %d\n", scale10, scale2); */
	set_def_power2(-scale2);
	

	/* fixup data */
	fld = PDS_PARAM(pds);
	level_type = PDS_L_TYPE(pds);

#ifdef PREC
	/* change precision */
	if (fld == HGT) {
	  set_def_power2(1);
	}
	if (fld == POT) {
	  set_def_power2(4);
	}
	if (fld == TSOIL) {
	  set_def_power2(4);
	}
	if (fld == TMP && scale2 == -7) {
	  set_def_power2(6);
	}
	if (fld == TMP && level_type != 100) {
	  set_def_power2(4);
	}
	if (fld == DPT) {
	  set_def_power2(4);
	}
	if (fld == UGRD && scale2 == -7) {
	  set_def_power2(6);
	}
	if (fld == VGRD && scale2 == -7) {
	  set_def_power2(6);
	}
#endif

	/* mask out land quantities */
	if (fld == BGRUN || fld == CCOND || fld == CNWAT || fld == RCS
		|| fld == RCQ || fld == RCSOL || fld == RCT || fld == SOILL
		|| fld == SOILM || fld == SOILW || fld == TSOIL
		|| fld == VEG || fld == GFLUX || fld == MSTAV || fld == SSRUN) {
	    for (i = 0; i < NXNY; i++) {
		if (mask[i] == 0.0) array[i] = UNDEFINED;
	    }
	    cnvt++;
	}

	/* cloud top/bottom heights */
	if ((fld == HGT && level_type == 2) || (fld == HGT && level_type == 3)) {
	    for (i = 0; i < NXNY; i++) {
		if (array[i] < 0.0) array[i] = UNDEFINED;
	    }
	    cnvt++;
	}
	/* cloud top/bottom pressure */
	if ((fld == PRES && level_type == 2) || (fld == HGT && level_type == 3)) {
	    for (i = 0; i < NXNY; i++) {
		if (array[i] < 0.0) array[i] = UNDEFINED;
	    }
	    cnvt++;
	}


	/* output data */

	if (fld != 209 && fld != PRMSL && fld != FLX && (time_range != 4 || 
                    p1 != 0 || p2 != 0)) {
	    wrt_grib_rec(pds, gds, array, ndata, output);
	}

	pos += len_grib;
	count++;
    }
    fclose(output);
    fclose(input);
    fprintf(stderr,"fixed %d records out of %d\n", cnvt, count);
    return 0;
}
