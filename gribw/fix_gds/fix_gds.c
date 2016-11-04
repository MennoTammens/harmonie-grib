#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <math.h>
#include <float.h>

#include "gribw.h"
#include "pds4.h"
#include "gds.h"
#include "gdstool.h"

/*
 * variant of fix_t40
 *
 * 10/2005                             Wesley Ebisuzaki
 * 
 */

/*
   for fortran grid: real data(nlon,nlat)

   nlat/nlon = number of lat/lon rows/columns
   lat1/lon1 = lat/lon of data(1,1)
   lat2/lon2 = lat/lon of data(nlon,nlat)

   for S or W, use negative value

   dlon, dlat = degrees of lon/lat of grid interval

 */

#define NLAT 73
#define NLON 144
#define LAT1 90.0
#define LAT2 -90.0
#define LON1 0.0
#define LON2 357.5
#define DLAT -2.5
#define DLON 2.5

int main(int argc, char **argv) {

    long int len_grib, pos = 0;
    unsigned char *pds, *gds, *bms, *bds, *new_gds;
    FILE *input, *output;
    int count;

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



    pos = count = 0;
    for(;;) {

	/* read the grib file */

        len_grib = rd_grib_msg(input, pos, &pds, &gds, &bms, &bds);
        if (len_grib <= 0) break;

        /* MAKE NEW GDS HERE */
        new_gds = new_LatLon_GDS(pds,NLON,NLAT,LON1,LAT1,LON2,LAT2,DLON,DLAT);

        wrt_grib_msg(output, pds, new_gds, bms, bds);
	free(new_gds);

	pos += len_grib;
	count++;
    }
    fclose(output);
    fclose(input);
    fprintf(stderr,"fixed %d records\n", count);
    return 0;
}
