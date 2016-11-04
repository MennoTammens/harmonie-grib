/* v1.0 w. ebisuzaki
 *
 * takes the sections (pds, gds, bms and bds) and writes a grib message
 */

#include <stdio.h>
#include <stdlib.h>
#include "gribw.h"

static unsigned char header[8] = {'G', 'R', 'I', 'B', ' ', ' ', ' ', '\1'},
	trailer[4] = {'7', '7', '7', '7'};

void wrt_grib_msg(FILE *output, unsigned char *pds, unsigned char *gds, 
        unsigned char *bms, unsigned char *bds) {

    unsigned int size;


    /* header */

    size = __LEN24(pds) + __LEN24(gds) + __LEN24(bms) + __LEN24(bds) + 12;
    header[4] = (size >> 16) & 255;
    header[5] = (size >>  8) & 255;
    header[6] = (size      ) & 255;

    if (fwrite(header, sizeof (unsigned char), 8, output) != 8) {
	fprintf(stderr,"error writing output\n");
	exit(8);
    }
    append_24section(pds, output);
    append_24section(gds, output);
    append_24section(bms, output);
    append_24section(bds, output);
    if (fwrite(trailer, sizeof (unsigned char), 4, output) != 4) {
	fprintf(stderr,"error writing output\n");
	exit(8);
    }
}
