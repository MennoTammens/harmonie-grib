#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "gribwlib.h"
#include "ncepopn.h"

#define NXNY		144*72
#define GDS_GRID	255
#define LOCAL_UNDEF	-999.0

/*
 * gribify ping-ping pentad rain 
 *
 * uses PDStool(), NCEP_GDS()
 */

int pentad_day1[73] = {
     101,  106,  111,  116,  121,  126,  131,  205,  210,  215,  220,  225, 
     302,  307,  312,  317,  322,  327,  401,  406,  411,  416,  421,  426, 
     501,  506,  511,  516,  521,  526,  531,  605,  610,  615,  620,  625, 
     630,  705,  710,  715,  720,  725,  730,  804,  809,  814,  819,  824, 
     829,  903,  908,  913,  918,  923,  928, 1003, 1008, 1013, 1018, 1023, 
    1028, 1102, 1107, 1112, 1117, 1122, 1127, 1202, 1207, 1212, 1217, 1222, 
    1227};

void main(int argc, char **argv) {

    unsigned char *pds, *pds_debug, *gds;
    FILE *input, *output;
    int count = 0, yyyy, mmdd, i, pentad=0;
    float data[NXNY], factor;

    /* preliminaries .. open up all files */

    if (argc != 4) {
	fprintf(stderr, "%s [in bin-file] [out gribfile] [YYYY]\n", argv[0]);
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

    yyyy = atoi(argv[3]);
    printf("initial YYYY is %d\n", yyyy);
    mmdd=pentad_day1[0];

    /* generate a PDS */

    pds = PDStool(New_PDS, 		/* new PDS */
	NCEP_opn, 			/* center, subcenter */
        P_param_table(2), 		/* parameter table */
	P_process(0),			/* process that made data */

	P_param(PRATE), P_sfc,		/* variable and level */
	P_date(yyyy*10000+mmdd),	/* initial date yyyymmdd */
	P_hour(0),

	P_ave_dy(0,5),			/* averaged from month 0 to 1 */

	P_dec_scale(7),			/* scale numbers by 10**7 */
	P_end);				/* end of arguments */

    /* generate a GDS */

    gds = new_LatLon_GDS(pds,144,72,1.25,-88.75,-1.25,88.75,2.5,2.5);


    /* loop through all the data */

    for (;;) {
        if (fread(&(data[0]), sizeof(float), NXNY, input) != NXNY) break;
        if (fseek(input, NXNY*sizeof(float), SEEK_CUR)) break;
        if (fseek(input, NXNY*sizeof(float), SEEK_CUR)) break;
        if (fseek(input, NXNY*sizeof(float), SEEK_CUR)) break;

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
	if (++pentad == 73) {
	    pentad = 0;
	    yyyy++;
	}
	mmdd = pentad_day1[pentad];
	printf("next date %d\n",yyyy*10000+mmdd);
	pds = PDStool(pds, P_date(yyyy*10000+mmdd), P_end);
	pds = PDStool(pds, P_ave_dy(0,5), P_end);

	if (mmdd == 225 && yyyy % 4 == 0) {
	    pds = PDStool(pds, P_ave_dy(0,6), P_end);
	}

	count++;
    }
    free(pds);
    free(gds);

    printf("%d records converted\n", count);

    fclose(input);
    fclose(output);
}
