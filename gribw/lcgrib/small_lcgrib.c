#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "gribwlib.h"
#include "gribw.h"
#include "pds4.h"
#include "gds.h"
#include "bds.h"


/*
 * 9/2003
 *
 * this program takes every nth point of a tangent grid
 * (lambert conformal grids which are tangent to the earth)
 *
 * wesley ebisuzaki
 *
 * v0.9 9/2003
 */

#define VERSION "small_lcgrib v0.9 9/2003  Wesley Ebisuzaki"

int main(int argc, char **argv) {

    long int len_grib, pos = 0;
    unsigned char *pds, *gds, *bms, *bds;
    float *data, *tdata, dx, dy;
    int ndata,  scale10, scale2;

    int nx, ny, scan;
    int fortran_mode, i, j;
    int inc, new_nx, new_ny;
 
    FILE *input, *output;
    int count, cvrt = 0;

    /* preliminaries .. open up all files */

    if (argc != 4) {
	fprintf(stderr, "%s\n", VERSION);
	fprintf(stderr, "  usage: %s [in_grib] [out_grib] [INC]\n", argv[0]);
	fprintf(stderr, "  skip by INC .. makes smaller lambert conformal grids\n");
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
    inc = atoi(argv[3]);
    if (inc <= 0) {
	fprintf(stderr,"illegal INC factor\n");
	exit(8);
    }

    pos = count = 0;
    while ((len_grib = rd_grib_rec2(input,pos,&pds,&gds,&data,&ndata,
		&scale10, &scale2)) > 0) {

	count++;

	if (gds == NULL || GDS_Lambert(gds)) {
	    /* copy messages that can not be subsetted */
	    /* read message as binary and write out */
	    rd_grib_msg(input,  pos, &pds, &gds, &bms, &bds);
	    wrt_grib_msg(output, pds, gds, bms, bds);
	    pos += len_grib;
	    continue;

	}

	nx = GDS_Lambert_nx(gds);
	ny = GDS_Lambert_ny(gds);
	scan = GDS_Lambert_scan(gds);
	fortran_mode = (scan & 32) == 0;
        dx = GDS_Lambert_dx(gds);
        dy = GDS_Lambert_dx(gds);

	new_nx = (nx - 1) / inc + 1;
	new_ny = (ny - 1) / inc + 1;

        if ((tdata = (float *) malloc(new_nx * new_ny * sizeof(float))) == NULL) {
            fprintf(stderr, "ran out of memory\n");
            exit(8);
        }

	if (fortran_mode) {
	    for (j = 0; j < new_ny; j++) {
		for (i = 0; i < new_nx; i++) {
		    tdata[i+j*new_nx] = data[i*inc+j*inc*nx];
		}
	    }
	}
	else {
	    for (j = 0; j < new_nx; j++) {
		for (i = 0; i < new_ny; i++) {
		    tdata[i+j*new_ny] = data[i*inc+j*inc*ny];
		}
	    }
	}

	/* set pds grid to 255 - user */
	pds[6] = 255;

	gds = GDStool(pds, gds, g_2bytes,6,new_nx, g_2bytes,8,new_ny,
	g_s3bytes,20,RINT(1000.0*(dx/inc)),
        g_s3bytes,23,RINT(1000.0*(dy/inc)),g_end);



	/* worry about precision here */
	/* decimal scaling is preserved from the PDS */
	/* need to keep binary scaling */
	set_BDSMinBits(-1);	/* ETA-style mode */
	set_def_power2(-scale2);

	wrt_grib_rec(pds,gds,tdata,new_nx*new_ny, output);

	free(tdata);
	pos += len_grib;
	cvrt++;
    }
    printf("read %d messages and converted %d grids\n", count, cvrt);
    return 0;
}

