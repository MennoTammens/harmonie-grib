/* (c) 1996 Wesley Ebisuzaki
 *
 * grib: convert from an array to a linear list of floats
 * using the bitmap
 */

#include <stdio.h>
#include <string.h>

void array2list(float *array, float *list, unsigned char *bitmap, int nxny, 
	int *ndata) {

    int i;
    unsigned int mask, bitmap0;

    if (bitmap == NULL) {
	memcpy(list, array, nxny * sizeof(float));
	*ndata = nxny;
	return;
    }

    i = 0;
    mask = 128;
    bitmap0 = *bitmap++;
    while (nxny-- > 0) {
	if (mask & bitmap0) {
	    *list++ = *array;
	    i++;
	}
	array++;
	if ((mask >>= 1) == 0) {
	    mask = 128;
	    bitmap0 = *bitmap++;
	}
    }
    *ndata = i;
}
		
