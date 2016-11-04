#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "gribw.h"

/* gribw0
 *
 * gribw: 
 *  v0.1  8/97 Wesley Ebisuzaki
 *
 * wgrib inverse
 *
 */

#define VERSION "gribw v0.1.2.a (1-98) Wesley Ebisuzaki"
#define TEXT_HEADER_LEN	2000

void main(int argc, char **argv) {
    float *array;
    int nrec, nbytes, file_arg, i, nxny;
    unsigned char *pds, *gds;
    char inv[TEXT_HEADER_LEN];
    int prec = 0, recl = 0;

    FILE *bin_in, *bin_out;
    char *filename_bin_in, *filename_bin_out;

    if (argc == 1) {
        fprintf(stderr, "%s\n", VERSION);
	fprintf(stderr,"usage: %s [options] [bin]\n", argv[0]);
	fprintf(stderr,"  -o [output grib]\n");
	fprintf(stderr,"  -i [input binary]\n");
	fprintf(stderr,"  -nh [recl in words]\n");
	fprintf(stderr,"  -prec [bits]\n");
	exit(8);
    }

    filename_bin_in = NULL;
    filename_bin_out = "dump.grb";

    file_arg = 0;
    for (i = 1; i < argc; i++) {
	if (strcmp(argv[i],"-o") == 0) {
	    filename_bin_out = argv[i+1];
	    i++;
	    continue;
	}
	else if (strcmp(argv[i],"-i") == 0) {
	    filename_bin_in = argv[i+1];
	    i++;
	    continue;
	}
	else if (strcmp(argv[i],"-nh") == 0) {
	    recl = sizeof(float) * atoi(argv[i+1]);
	    i++;
	    continue;
	}
	else if (strcmp(argv[i],"-prec") == 0) {
	    prec = atoi(argv[i+1]);
	    if (prec <= 0) prec = 0;
	    if (prec > 24) prec = 24;
	    i++;
	    continue;
	}
	else if (argv[i][0] == '-') {
	    fprintf(stderr,"argument: %s ????\n", argv[i]);
	}
	else {
	    if (file_arg++ < 1) {
		filename_bin_in = argv[i];
	    }
	    else {
	       fprintf(stderr,"argument: %s ????\n", argv[i]);
	    }
	}
    }

    /* open files */
    if (filename_bin_in == NULL) {
	fprintf(stderr, "missing input binary data file\n");
        exit(8);
    }
    if ((bin_in = fopen(filename_bin_in, "rb")) == NULL) {
	fprintf(stderr, "Input binary data file?\n");
        exit(8);
    }
    if ((bin_out = fopen(filename_bin_out, "wb")) == NULL) {
	fprintf(stderr, "Onput binary data file?\n");
        exit(8);
    }

    nrec = 0;
    inv[TEXT_HEADER_LEN-1] = '\0';

    if (prec == 0) {
	/* NCEP style .. decimal scaling only */
        set_BDSMaxBits(16);
    } else {
	/* ECMWF style .. use binary scaling */
        set_BDSMaxBits(prec);
        set_BDSMinBits(prec);
    }

    /* loop for every input record */
    for (;;) {

	/* read header with PDS/GDS */
	if (fgets(inv,TEXT_HEADER_LEN-1, stdin) == NULL) break;
	/* parse PDS/GDS */
	pds = inv_pds(inv);
	gds = inv_gds(inv);
	if (pds == NULL) fprintf(stderr,"pds is missing\n");
	if (gds == NULL) fprintf(stderr,"gds is missing\n");
	if (pds == NULL) break;
	
	/* read binary data */
	nbytes = recl > 0 ? recl : 0;
	
	if ((array = (float *) f77read(bin_in, &nbytes)) == NULL) break;
	if (nbytes % sizeof(float) == 0) {
	    nxny = nbytes / sizeof(float);
	}
	else {
	    fprintf(stderr,"illegal f77-style record\n");
	    exit(8);
	}

	/* generate grib record */
	wrt_grib_rec(pds, gds, array, nxny, bin_out);
	if (gds) free(gds);
	free(pds);
	free(array);
	nrec++;
    }
    printf("wrote %d record(s)\n", nrec);
}
