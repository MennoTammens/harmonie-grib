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
 * another fix for the Regional Reanalysis AWIP fields
 *
 * some vector fields are not rotated 
 *  rotate vector fields
 *  note rotation angles are slightly off
 *   as the interpolation from e-grid to AWIPS grid
 *     used unrotated vectors
 *
 *                              Wesley Ebisuzaki
 */

#define NXNY		(349*277)

int main(int argc, char **argv) {

    long int len_grib, pos = 0;
    unsigned char *pds, *gds, *pds_u, *gds_u, *bms, *bds;
    FILE *input, *output, *rot;
    int count, cnvt, scale10, scale2;
    int ndata;
    float *array = NULL, *v, u[NXNY];
    double c, s, uu, vv,f ;
    int i, fld;
    float cos_theta[NXNY], sin_theta[NXNY];

    /* preliminaries .. open up all files */

    if (argc != 4) {
	fprintf(stderr, "%s [in gribfile] [out gribfile] [U1000_rot.grb]\n", argv[0]);
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

    /* read rotation file */
    if ((rot = fopen(argv[3], "rb")) == NULL) {
	fprintf(stderr,"no rotation file\n");
	exit(9);
    }
    pos = 0;
    len_grib = rd_grib_rec(rot, pos, &pds, &gds, &array, &ndata);
    if (len_grib <= 0 || ndata != NXNY) {
	fprintf(stderr,"bad rot file\n");
	exit(9);
    }
    for (i = 0; i < NXNY; i++) {
	cos_theta[i] = array[i];
    }
    pos += len_grib;
    len_grib = rd_grib_rec(rot, pos, &pds, &gds, &array, &ndata);
    if (len_grib <= 0 || ndata != NXNY) {
	fprintf(stderr,"bad rot file\n");
	exit(9);
    }
    for (i = 0; i < NXNY; i++) {
	sin_theta[i] = array[i];
    }
    fclose(rot);

    /* fix up rotation data */
    for (i = 0; i < NXNY; i++) {
        if (! UNDEFINED_VAL(cos_theta[i])) {
            c = cos_theta[i];
            s = sin_theta[i];
	    f = 1.0 / sqrt(c*c + s*s);
	    cos_theta[i] = f*c;
	    sin_theta[i] = -s*f;
	}
    }


    set_BDSMinBits(-1);	/* ETA-style mode */

    pos = count = cnvt = 0;
    for(;;) {

	/* read the grid record without decoding */

	len_grib = rd_grib_msg(input, pos, &pds, &gds, &bms, &bds);
	if (len_grib <= 0) break;

        fld = PDS_PARAM(pds);
        
	if (fld != WVUFLX && fld != WCUFLX && fld != UFLX) {
	    (void) wrt_grib_msg(output, pds, gds, bms, bds);
	    pos += len_grib;
	    count++;
	    continue;
	}

	/* read grib record with decoding */
        len_grib = rd_grib_rec2(input, pos, &pds, &gds, &array, &ndata, 
          &scale10, &scale2);
	if (len_grib <= 0) break;
        set_def_power2(-scale2);

	if (ndata != NXNY) {
	    fprintf(stderr,"error in grid size\n");
	    exit(8);
	}
        fld = PDS_PARAM(pds);

	/* save data */
	for (i = 0; i < NXNY; i++) {
	    u[i] = array[i];
	}

	/* save pds gds */
	pds_u = cpGRIBsec(pds);
	gds_u = cpGRIBsec(gds);

	/* read next field */
	pos += len_grib;

        len_grib = rd_grib_rec2(input, pos, &pds, &gds, &v, &ndata, 
          &scale10, &scale2);
	if (len_grib <= 0) {
	    fprintf(stderr,"error expecting %d vector field, EOF\n", fld+1);
	    break;
	}

        if (ndata != NXNY) {	
            fprintf(stderr,"error in vector grid size\n");
	    exit(8);
	}
        if (fld+1 != PDS_PARAM(pds)) {
	    fprintf(stderr,"error expecting %d vector field\n", fld+1);
	    break;
	}

	/* now to rotate winds */
	for (i = 0; i < NXNY; i++) {
	    uu = u[i];
	    vv = v[i];
	    c = cos_theta[i];
	    s = sin_theta[i];

            if (UNDEFINED_VAL(c) || UNDEFINED_VAL(uu) || UNDEFINED_VAL(vv)) {
		u[i] =  v[i] = UNDEFINED;
	    }
	    else {
		u[i] = c*uu + s*vv;
		v[i] = -s*uu + c*vv;
	    }
	}
	wrt_grib_rec(pds_u, gds_u, u, ndata, output);
	wrt_grib_rec(pds, gds, v, ndata, output);
	free(pds_u);
	free(gds_u);

	pos += len_grib;
	cnvt++;
	count++;
    }
    fclose(output);
    fclose(input);
    fprintf(stderr,"fixed %d records out of %d\n", cnvt, count);
    return 0;
}
