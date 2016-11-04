#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "gribw.h"

/*
 * 1996 Wesley Ebisuzaki
 *
 * wrt_24section
 * writes a section (bds, gds, bms, etc)
 * (section must start with 3 byte size)
 *
 */

int wrt_24section(unsigned char *section, char *file_name) {

    unsigned int size;
    FILE *output;

    if (section == NULL) {
	remove(file_name);
        return 0;
    }

    /* size = size_24section(section); */
    size = _LEN(section);
    if ((output = fopen(file_name,"wb")) == NULL) {
	fprintf(stderr,"could not open %s for output\n", file_name);
	exit(8);
    }

    if (fwrite(section, sizeof (unsigned char), size, output) != size) {
	fprintf(stderr,"error writing %s\n", file_name);
	fclose(output);
	exit(8);
    }
    fclose(output);
    return 0;
}
