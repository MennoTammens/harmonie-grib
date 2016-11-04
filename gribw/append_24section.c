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

int append_24section(unsigned char *section, FILE *output) {

    int i;

    i = __LEN24(section);

    if (fwrite(section, sizeof (unsigned char), i, output) != i) {
	fprintf(stderr,"error writing output\n");
	return -1;
    }
    return 0;
}
