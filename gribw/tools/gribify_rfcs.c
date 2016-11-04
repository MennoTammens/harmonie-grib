#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "gribwlib.h"
#include "ncepopn.h"

#define NXNY		224*464
#define LOCAL_UNDEF	-9999.0

/*
 * gribify a field (use undef variable name)
 *  
 * uses PDStool(), NCEP_GDS()
 * v1.1 convert from Celcius to Kelvin
 */


int main(int argc, char **argv) {

    unsigned char *pds, *pds_debug, *gds;
    FILE *input, *output;
    int count = 0, yyyymm, i, header;
    float data[NXNY];

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
	NCEP_opn, 			/* center, subcenter */
        P_param_table(255), 		/* parameter table */
	P_process(255),			/* process that made data */

	P_param(255), P_sfc,		/* variable and level */
	P_date(yyyymm*100+1),		/* initial date yyyymmdd */
	P_hour(0),

	P_ave_mon(0,1),			/* averaged from month 0 to 1 */

	P_dec_scale(0),			/* scale numbers by 10**2 */
	P_end);				/* end of arguments */

    /* generate a GDS */

    /* or make a user defined GDS for a 2.5 x 2.5 grid */
    /* gds = new_LatLon_GDS(pds,360,180,-179.5,-89.5,179.5,89.5,1.0,1.0); */

    gds = new_LatLon_GDS(pds,464,224,235.0625,25.0625,292.9375,52.9375,0.125,0.125);

    /* loop through all the data */

    while (i=fread(&(data[0]), sizeof(float), NXNY, input) == NXNY) {
        
	for (i = 0; i < NXNY; i++) {
		if (data[i] == LOCAL_UNDEF) data[i] = UNDEFINED;
	}
	wrt_grib_rec(pds, gds, data, NXNY, output);

	/* change date code in the PDS */

	if (++yyyymm % 100 == 13) {
	    yyyymm += (101 - 13);
	}
	pds = PDStool(pds, P_date(yyyymm*100+1),P_end);

	count++;

    }
    free(pds);
    free(gds);

    printf("%d records converted\n", count);

    fclose(input);
    fclose(output);
    return 0;
}

