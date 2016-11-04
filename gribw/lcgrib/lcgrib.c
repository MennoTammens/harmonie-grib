#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "gribwlib.h"
#include "gribw.h"
#include "pds4.h"
#include "gds.h"
#include "bds.h"


void w3fb11(double alat, double elon, double alat1, double elon1, 
    double dx, double elonv, double alatan, double *xi, double *xj);

void w3fb12(double xi, double xj, double alat1, double elon1, double dx, 
    double elonv, double alatan, double *alat, double *elon, int *ierr);


/*
 * 11/2002
 *
 * this program makes a geographic subdomain of a tangent grid
 * (lambert conformal grids which are tangent to the earth)
 *
 * wesley ebisuzaki
 *
 * v1.1 4/2003
 */

#define VERSION "lcgrib v1.1 4/2003  Wesley Ebisuzaki, Jun Wang"

int main(int argc, char **argv) {

    long int len_grib, pos = 0;
    unsigned char *pds, *gds, *bms, *bds;
    float *data, *tdata;
    int ndata, cvrt, scale10, scale2;

    int nx, ny, mode, scan, copy, ierr;
    double xi[8], xj[8], lat1, lon1, lov, dx, dy, latin1, latin2, lat, lon;
    double north, south, east, west;
    int inorth, isouth, ieast, iwest;
    int i1, i2, j1, j2;
    int fortran_mode, i, j, n, k, new_nxny;
 
    FILE *input, *output;
    int count, cnvt = 0;

    /* preliminaries .. open up all files */

    if (argc != 7) {
	fprintf(stderr, "%s\n", VERSION);
	fprintf(stderr, "  usage: %s [in_grib] [out_grib] [west] [south] [east] [north]\n", argv[0]);
	fprintf(stderr, "  makes subdomains of lambert conformal grids\n");
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
    west = atof(argv[3]);
    south = atof(argv[4]);
    east = atof(argv[5]);
    north = atof(argv[6]);


    pos = count = cvrt = 0;
    while ((len_grib = rd_grib_rec2(input,pos,&pds,&gds,&data,&ndata,
		&scale10, &scale2)) > 0) {

	count++;
	copy = 1;

	if (gds != NULL && GDS_Lambert(gds)) {
	    nx = GDS_Lambert_nx(gds);
	    ny = GDS_Lambert_ny(gds);
	    lat1 = GDS_Lambert_La1(gds)/1000.0;
	    lon1 = GDS_Lambert_Lo1(gds)/1000.0;
	    mode = GDS_Lambert_mode(gds);
	    lov = GDS_Lambert_Lov(gds)/1000.0;
	    dx = GDS_Lambert_dx(gds);
	    dy = GDS_Lambert_dx(gds);
	    scan = GDS_Lambert_scan(gds);
	    latin1 = GDS_Lambert_Latin1(gds)/1000.0;
	    latin2 = GDS_Lambert_Latin2(gds)/1000.0;
	    /* check for tangent grid */
	    if (dx == dy && latin1 == latin2) copy = 0;
	    if (scan & 128) {
		copy = 1;
		fprintf(stderr,"scan %d not supported\n", scan);
	    }
	    if ((scan & 64) == 0) {
		copy = 1;
		fprintf(stderr,"scan %d not supported\n", scan);
	    }
	    fortran_mode = (scan & 32) == 0;
	}


	if (copy == 1) {
	    /* copy messages that can not be subsetted */
	    /* read message as binary and write out */
	    rd_grib_msg(input,  pos, &pds, &gds, &bms, &bds);
	    wrt_grib_msg(output, pds, gds, bms, bds);
	    pos += len_grib;
	    continue;
	}

	/* must subset the grib message */

	/* need to find rectangular grid which encloses the
           north/south east/west specification

           find i, j of four corners
           find i, j of four mid points of lines

           and look for max/min of i's and j's
           
	*/

	w3fb11(south, west, lat1, lon1, dx, lov, latin1, xi, xj);
	w3fb11(north, west, lat1, lon1, dx, lov, latin1, xi+1, xj+1);
	w3fb11(south, east, lat1, lon1, dx, lov, latin1, xi+2, xj+2);
	w3fb11(north, east, lat1, lon1, dx, lov, latin1, xi+3, xj+3);
	w3fb11((north+south)/2, east, lat1, lon1, dx, lov, latin1, xi+4, xj+4);
	w3fb11((north+south)/2, west, lat1, lon1, dx, lov, latin1, xi+5, xj+5);
	w3fb11(north, (east+west)/2, lat1, lon1, dx, lov, latin1, xi+6, xj+6);
	w3fb11(south, (east+west)/2, lat1, lon1, dx, lov, latin1, xi+7, xj+7);

	ieast = iwest = (int) floor(xi[0] + 0.5);
	inorth = isouth = (int) floor(xj[0] + 0.5);

	for (i = 1; i < 8; i++) {
	    j = (int) floor(xi[i] + 0.5);
	    ieast = ieast > j ? ieast : j;
	    iwest = iwest < j ? iwest : j;
	    j = (int) floor(xj[i] + 0.5);
	    inorth = inorth > j ? inorth : j;
	    isouth = isouth < j ? isouth : j;
	}
	if (iwest < 1) iwest = 1;
	if (iwest > nx) iwest = nx;
	if (ieast < 1) ieast = 1;
	if (ieast > nx) ieast = nx;
	if (isouth < 1) isouth = 1;
	if (isouth > ny) isouth = ny;
	if (inorth < 1) inorth = 1;
	if (inorth > ny) inorth = ny;

	new_nxny = (inorth - isouth + 1) * (ieast - iwest + 1);
	if ((tdata = (float *) malloc(new_nxny * sizeof(float))) == NULL) {
	    fprintf(stderr, "ran out of memory\n");
	    exit(8);
	}

	if (fortran_mode) {
	    i1 = iwest;
	    i2 = ieast;
	    j1 = isouth;
	    j2 = inorth;
	    n = nx;
	}
	else {
	    j1 = iwest;
	    j2 = ieast;
	    i1 = isouth;
	    i2 = inorth;
	    n = ny;
	}

	k = 0;
	for (j = j1; j <= j2; j++) {
	    for (i = i1; i <= i2; i++) {
		tdata[k++] = data[(i-1) + (j-1)*n];
	    }
	}

	/* set pds grid to 255 - user */
	pds[6] = 255;

	/* set new gds */
	set_int2(gds+6, ieast - iwest + 1);	/* nx */
	set_int2(gds+8, inorth - isouth + 1);	/* ny */

	/* get lat lon of (1,1) grid point */

	w3fb12((double) iwest, (double) isouth,  lat1, lon1, dx, 
	    	lov, latin1, &lat, &lon, &ierr);
	if (ierr != 0) {
	    fprintf(stderr,"error %d\n", ierr);
	    exit (8);
	}

	/* get lon closest to lov */
	if (fabs(lov-lon) > fabs(lov-lon-360)) {
		lon += 360.0;
	}
	if (fabs(lov-lon) > fabs(lov-lon+360)) {
		lon -= 360.0;
	}

	set_signed_int3(gds+10, (int) floor(lat*1000+0.5));
	set_signed_int3(gds+13, (int) floor(lon*1000+0.5));

	nx = GDS_Lambert_nx(gds);
	ny = GDS_Lambert_ny(gds);

	/* worry about precision here */
	/* decimal scaling is preserved from the PDS */
	/* need to keep binary scaling */
	set_BDSMinBits(-1);	/* ETA-style mode */
	set_def_power2(-scale2);

	wrt_grib_rec(pds,gds,tdata,new_nxny,output);
	cnvt++;

	free(tdata);
	pos += len_grib;
    }
    printf("read %d messages and converted %d grids\n", count, cnvt);
    return 0;
}



