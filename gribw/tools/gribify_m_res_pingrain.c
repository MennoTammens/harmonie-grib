#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "gribwlib.h"
#include "ncepopn.h"

#define NXNY		144*72
#define GDS_GRID	255
#define LOCAL_UNDEF	-999.0

/*
 * gribify an ping-ping's monthly rain
 *
 * uses PDStool(), NCEP_GDS()
 */


int main(int argc, char **argv) {

    unsigned char *pds, *pds_debug, *gds;
    FILE *input, *output;
    int count = 0, yyyymm, i;
    float data[NXNY], factor;

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
        P_param_table(2), 		/* parameter table */
	P_process(0),			/* process that made data */

	P_param(PRATE), P_sfc,		/* variable and level */
	P_date(yyyymm*100+1),		/* initial date yyyymmdd */
	P_hour(0),

	P_ave_mon(0,1),			/* averaged from month 0 to 1 */

	P_dec_scale(7),			/* scale numbers by 10**7 */
	P_end);				/* end of arguments */

    /* generate a GDS */

    gds = new_LatLon_GDS(pds,144,72,1.25,-88.75,-1.25,88.75,2.5,2.5);


    /* loop through all the data */

    for (;;) {
        if (fread(&(data[0]), sizeof(float), NXNY, input) != NXNY) break;

        factor = 1.0 / 86400.0;

	for (i = 0; i < NXNY; i++) {
		if (data[i] == LOCAL_UNDEF) {
			data[i] = UNDEFINED;
		}
		else {
			data[i] *= factor;
		}
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
