#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "gribwlib.h"
#include "ncepopn.h"

#define NXNY		360*180
#define LOCAL_UNDEF	-999.0

/*
 * gribify an SST field 360x180 
 *  
 * uses PDStool(), NCEP_GDS()
 * v1.1 convert from Celcius to Kelvin
 */


int main(int argc, char **argv) {

    unsigned char *pds, *pds_debug, *gds;
    FILE *input, *output;
    int count = 0, yyyymmdd, i, header;
    float data[NXNY];

    /* preliminaries .. open up all files */

    if (argc != 4) {
	fprintf(stderr, "%s [in bin-file] [out gribfile] [YYYYMMDD]\n", argv[0]);
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
    yyyymmdd = atoi(argv[3]);
    printf("initial YYYYMMDD is %d\n", yyyymmdd);

    /* generate a PDS */

    pds = PDStool(New_PDS, 		/* new PDS */
	NCEP_opn, 			/* center, subcenter */
        P_param_table(2), 		/* parameter table */
	P_process(0),			/* process that made data */
	P_param(TMP), P_sfc,		/* variable and level */
	P_date(yyyymmdd),		/* initial date yyyymmdd */
	P_hour(0),
	P_init_analysis,
	P_dec_scale(3),			/* scale numbers by 10**3 */
	P_end);				/* end of arguments */

    /* generate a GDS */

    /* use NCEP grid number 2 */

    /*  gds = NCEP_GDS(pds,GDS_GRID);	/* use 144x73 grid */

    /* or make a user defined GDS for a 2.5 x 2.5 grid */

    gds = new_LatLon_GDS(pds,360,180,0.5,-89.5,359.5,89.5,1.0,1.0);

    /* skip 40 bytes header */

    fseek(input, 40L, SEEK_SET);

    if (i=fread(&header, sizeof(int), 1, input) == 1) {
	if (i != 1 || header != NXNY * sizeof(float)) {
	    fprintf(stderr,"missing f77 header\n");
	    exit(8);
	}
        if (i=fread(&(data[0]), sizeof(float), NXNY, input) != NXNY) {
	    fprintf(stderr,"missing data\n");
	    exit(8);
	}

	for (i = 0; i < NXNY; i++) {
		if (data[i] == LOCAL_UNDEF) data[i] = UNDEFINED;
		else data[i] += 273.15;
	}
	wrt_grib_rec(pds, gds, data, NXNY, output);
	count++;

    }
    free(pds);
    free(gds);

    printf("%d records converted\n", count);

    fclose(input);
    fclose(output);
    exit((count == 1) ? 0 : 8);
}
