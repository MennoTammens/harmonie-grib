#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

static unsigned char gds2_data[32] = {
	0000, 0000, 0040, 0000, 0377, 0000, 0000, 0220,
        0000, 0111, 0001, 0137, 0220, 0000, 0000, 0000,
        0200, 0201, 0137, 0220, 0200, 0011, 0304, 0011,
        0304, 0011, 0304, 0000, 0000, 0000, 0000, 0000};

unsigned char *gds_grid2() {
	unsigned char *temp;
	int i;

	if ((temp = (unsigned char *) malloc(32)) == NULL) {
		fprintf(stderr,"memory problem\n");
		exit(8);
	}
	for (i=0; i < 32; i++) {
	    temp[i] = gds2_data[i];
	}
	return temp;
}
