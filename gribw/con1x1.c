#include <math.h>
#include <float.h>

/* 					1/96 Wesley Ebisuzaki
 *
 * 	convert 1x1 grid to 2.5x2.5 grid
 *
 * 	assumptions:
 * 		(1) no bitmap
 * 		(2) bilinear interpolation
 * 		(3) grid1 and grid2 do not share same memory space
 *              (4) fortran order
 * 	base point (90N, 0E)
 */


void con1x1(float *grd1x1, float *grd25)
{
    int ix, iy, ix1, lat;
    float row[360];


    for (lat = 0; lat <= 1800; lat += 25) {
	iy = lat / 10;
	if (lat % 10 == 0) {
	    for (ix = 0; ix <= 359; ++ix) {
		row[ix] = grd1x1[ix + iy * 360];
	    }
	}
        else {
	    for (ix = 0; ix <= 359; ++ix) {
		row[ix] = (grd1x1[ix + iy * 360] + grd1x1[ix + (iy + 1) * 360]
			) * (float) 0.5;
	    }
	}

	iy = lat / 25;
	ix1 = 0;
	for (ix = 0; ix <= 143; ix += 2) {
	    grd25[ix + iy * 144] = row[ix1];
	    ix1 += 5;
	}

	ix1 = 2;
	for (ix = 1; ix <= 143; ix += 2) {
	    grd25[ix + iy * 144] = (row[ix1] + row[ix1 + 1]) * (float) 0.5;
	    ix1 += 5;
	}
    }
}
