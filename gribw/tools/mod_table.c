#include <stdio.h>
#include "gribwlib.h"

/*
 * A simple program to change the table and subcenter of a GRIB file.
 *   Isn't this program too easy?
 *
 * 2/98                             Wesley Ebisuzaki
 */

#define SUBCENTER	1
#define TABLE_VSN	2

void main(int argc, char **argv) {

    long int pos = 0;
    int len_grib;
    unsigned char *pds, *gds, *bms, *bds;
    FILE *input, *output;

    /* preliminaries .. open up all files */

    if (argc != 3) {
	fprintf(stderr, "%s [in gribfile] [out gribfile]\n", argv[0]);
	fprintf(stderr, " modifies grib file\n");
	exit(6);
    }
    if ((input = fopen(argv[1],"rb")) == NULL) {
        fprintf(stderr,"could not open file: %s\n", argv[1]);
        exit(7);
    }
    if ((output = fopen(argv[2],"wb")) == NULL) {
        fprintf(stderr,"could not open file: %s\n", argv[2]);
        exit(8);
    }

    /* main loop */

    while ((len_grib = rd_grib_msg(input, pos, &pds, &gds, &bms, &bds)) > 0) {
	pds[3] = TABLE_VSN;
	pds[25] = SUBCENTER;
	wrt_grib_msg(output, pds, gds, bms, bds);
	pos += len_grib;
    }
    exit(0);
}
