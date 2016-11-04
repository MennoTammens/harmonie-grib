#include <stdio.h>
#include <stddef.h>
#include "gribw.h"

/* v0.1 get_intlist.c
 * wesley ebisuzaki
 *
 * unpacks the BDS into an integer list
 * (partial unpacking)
 *
 * will be used by grib compression routines
 *
 */
--------------------------------
int get_intlist(int *flt, unsigned char *bits, unsigned char *bitmap,
	int n_bits, int n, double ref, double scale) {

    int i, j, k;
    unsigned int map_mask, bit_mask;

    map_mask = bit_mask = 128;

    for (i = 0; i < n; i++) {
	if (bitmap) {
	    j = (*bitmap & map_mask);
	    if ((map_mask >>= 1) == 0) {
		map_mask = 128;
		bitmap++;
	    }
	    if (j == 0) {
		*flt++ = UNDEFINED;
		continue;
	    }
	}

	j = 0;
	k = n_bits;
	while (k) {
	    if (k >= 8 && bit_mask == 128) {
		j = 256 * j + *bits;
		bits++;
		k -= 8;
	    }
	    else {
	        j = j + j + ((*bits & bit_mask) != 0);
		if ((bit_mask >>= 1) == 0) {
		    bits++;
		    bit_mask = 128;
		}
		k--;
	    }
	}
	*flt++ = ref + scale*j;
   }
   return;
}
