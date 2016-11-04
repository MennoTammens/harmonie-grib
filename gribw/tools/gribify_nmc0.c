#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "gribwlib.h"
#include "ncepopn.h"

#define LOCAL_UNDEF	-999.0

/* defines for grid geometry */

#define NX		49
#define NY		51
#define NXNY		(NX*NY)
#define GDS_GRID	255
#define LOCAL_UNDEF	-999.0

#define LON1	-122.614
#define LAT1	-4.86069
#define LOV	-80.0
#define DX	381.0
#define DY	381.0

/*
 * gribify a nmc grid 0 
 *
 * uses PDStool()
 */


void main(int argc, char **argv) {

    unsigned char *pds, *pds_debug, *gds;
    FILE *input, *output;
    int count = 0, yyyymm, i, j, k;
    float factor;
    float data[NY][NX];		/* data is in fortran order */

    /* preliminaries .. open up all files */

    if (argc != 4) {
	fprintf(stderr, "%s [in bin-file] [out gribfile] [YYYYMM]\n", argv[0]);
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
    yyyymm = atoi(argv[3]);
    printf("initial YYYYMM is %d\n", yyyymm);

    /* generate a PDS */

    pds = PDStool(New_PDS, 		/* new PDS */
	P_center(7),			/* your center */
	P_subcenter(0),			/* your subcenter */
        P_param_table(2), 		/* parameter table */
	P_process(0),			/* process that made data */

	P_param(HGT), P_mb(500),	/* variable and level */
	P_date(yyyymm*100+1),		/* initial date yyyymmdd */
	P_hour(0),

	P_ave_mon(0,1),			/* averaged from month 0 to 1 */

	P_dec_scale(0),			/* scale numbers by 10**0 */
	P_end);				/* end of arguments */

    /* generate a GDS */

    gds = new_polar_GDS(pds, NX, NY, LON1, LAT1, LOV, DX, DY, 0, 1);


    /* loop through all the data */

    for (k=0; k < 3; k++) {
        /* should read the data here */
	/*
        if (fread(&(data[0][0]), sizeof(float), NXNY, input) == NXNY) break;
	*/

	/* but will initialize the data here instead */
	for (i = 0; i < NX; i++) {
	    for (j = 0; j < NY; j++) {
		data[j][i] = 5000.0 + j + k;
	    }
	}
	data[0][0] = UNDEFINED;

	wrt_grib_rec(pds, gds, &(data[0][0]), NXNY, output);

	/* change date code in the PDS */

	if (++yyyymm % 100 == 13) {
	    yyyymm += (101 - 13);
	}
	pds = PDStool(pds, P_date(yyyymm*100+1),P_end);

	count++;
    }
    free(pds);
    free(gds);

    printf("%d records written\n", count);

    fclose(input);
    fclose(output);
}

