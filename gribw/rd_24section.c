#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "gribw.h"

/*
 * 1996 Wesley Ebisuzaki
 *
 * rd_24section
 * reads a section (bds, gds, bms, pds)
 * (section must start with 3 byte size)
 *
 *
 */
unsigned char *rd_24section(char *filename) {

    FILE *input;
    int i, c;
    unsigned char *string;
    unsigned int24;

    if ((input = fopen(filename, "rb")) == NULL) {
	fprintf(stderr,"could not open %s, returning NULL file\n", filename);
	return NULL;
    }

    for (int24 = i = 0; i < 3; i++) {
	c = getc(input);
	if (c == EOF) return NULL;
	int24 = (int24 << 8) + c;
    }

    if ((string = (unsigned char *) malloc((size_t) int24)) == NULL) {
	fprintf(stderr,"not allocate memory .. FATAL ERROR\n");
	exit(8);
    }

    string[0] = (int24 >> 16) & 255;
    string[1] = (int24 >>  8) & 255;
    string[2] = (int24      ) & 255;

    i = fread(string+3, sizeof (unsigned char), int24-3, input);
    fclose(input);
    if (i == int24-3) return string;
    fprintf(stderr,"could not read %s .. FATAL ERROR\n", filename);

    exit(8);
}
