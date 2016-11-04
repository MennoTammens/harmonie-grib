#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "gribwlib.h"
#include "nceptab140.h"
#include "ncepreanl.h"

#define NXNY		144*73
#define GDS_GRID	2
#define LOCAL_UNDEF	10e10

/*
 * gribify afit and gfit
 *
 * uses PDStool(), NCEP_GDS()
 */

float plevs[21] = { 1000,925,850,700,500,400,300,
     250,200,150,100,70,50,30,20,10,7,5,3,2,1};

int param[] = {
	TNadpu, TAadpu, TAAadpu, TGadpu, TGGadpu, TMP,
	ZNadpu, ZAadpu, ZAAadpu, ZGadpu, ZGGadpu, HGT,
	QNadpu, QAadpu, QAAadpu, QGadpu, QGGadpu, SPFH,
	UNadpu, UAadpu, UAAadpu, UGadpu, UGGadpu, UGRD,
	VNadpu, VAadpu, VAAadpu, VGadpu, VGGadpu, VGRD};


void main(int argc, char **argv) {

    unsigned char *pds, *pds_debug, *gds;
    FILE *input, *output;
    int count = 0, yyyymm, i;
    int j, icnt, itype;
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
	NCEP_reanl, 			/* center, subcenter */
        P_param_table(140), 		/* parameter table */
	P_process(0),			/* process that made data */

	P_param(TNadpu), P_top_atmos,	/* variable and level */
	P_date(yyyymm*100+1),		/* initial date yyyymmdd */
	P_hour(0),

	P_ave_mon(0,1),			/* averaged from month 0 to 1 */

	P_dec_scale(1),			/* scale numbers by 10**1 */
	P_end);				/* end of arguments */

    /* generate a GDS */

    /* use NCEP grid number 2 */

    gds = NCEP_GDS(pds,GDS_GRID);	/* use 144x73 grid */

    /* loop through all the data */
   
   for (itype = 18; itype < 24; itype++) {

        switch (itype % 6) {
           case 0:
	       /* number of obs */
               set_BDSMaxBits(16);	/* NCEP style */
               set_BDSMinBits(0);
               pds = PDStool(pds, P_param_table(140), P_process(80), P_end);
	       pds = PDStool(pds, P_dec_scale(0), P_acc_mon(0,1), P_end); break;

           case 1:
           case 2:
           case 3:
           case 4:
               set_BDSMaxBits(12);	/* ECMWF style */
               set_BDSMinBits(12);
               pds = PDStool(pds, P_param_table(140), P_process(80), P_end);
	       pds = PDStool(pds, P_dec_scale(0), P_acc_mon(0,1), P_end); break;
	       
           case 5:
               set_BDSMaxBits(12);	/* ECMWF style */
               set_BDSMinBits(12);
               pds = PDStool(pds, P_param_table(2), P_process(80), P_end);
	       pds = PDStool(pds, P_dec_scale(0), P_ave_mon(0,1), P_end); break;

           default:
	       fprintf(stderr,"prog. error 1\n");
	       exit(8);
        }
        pds = PDStool(pds, P_param(param[itype]), P_end);

        for (i = 0; i < sizeof(plevs)/sizeof(plevs[0]); i++) {
            pds = PDStool(pds,P_mb((int) plevs[i]), P_end);

#ifdef F77FILE
            j=fread(&(icnt), sizeof(int), 1, input);
            if (j == 0) break;
            if (j != 1  || icnt != NXNY * sizeof(float)) {
		fprintf(stderr,"header problem %d %d\n",icnt,j);
		exit(9);
	    }
#endif

            j=fread(&(data[0]), sizeof(float), NXNY, input);
            if (j == 0) break;
            if (j != NXNY) {
		fprintf(stderr,"header problem %d %d\n",icnt,j);
		exit(9);
	    }

#ifdef F77FILE
            j=fread(&(icnt), sizeof(int), 1, input);
            if (j != 1  || icnt != NXNY * sizeof(float)) {
		fprintf(stderr,"trailer problem %d %d\n",icnt,j);
		exit(9);
	    }
#endif
	    for (j = 0; j < NXNY; j++) {
		if (data[j] > LOCAL_UNDEF*0.999 &&
		    data[j] < LOCAL_UNDEF*1.001) data[j] = UNDEFINED;
            }
	    printf("writing grib itype %d lev %f %d\n",itype,plevs[i],i);
	    wrt_grib_rec(pds, gds, data, NXNY, output);
	    count++;
        }
    }
    free(pds);
    free(gds);

    printf("%d records converted\n", count);

    fclose(input);
    fclose(output);
}
