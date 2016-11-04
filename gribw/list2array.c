/* (c) 1996 Wesley Ebisuzaki
 *
 * grib: convert from a linear list of floats to an array:w
 * using the bitmap
 */

#include <stdio.h>
#include <string.h>

void list2array(float *list, float *array, unsigned char *bitmap, int nxny) {

    unsigned int mask, bitmap0;

    if (bitmap == NULL) {
	memcpy(array, list, nxny * sizeof(float));
	return;
    }

    mask = 128;
    bitmap0 = *bitmap++;
    while (nxny-- > 0) {
	if (mask & bitmap0) {
	    *array++ = *list++;
	}
	else {
	    *array++ = 0.0;
	}
	if ((mask >>= 1) == 0) {
	    mask = 128;
	    bitmap0 = *bitmap++;
	}
    }
}
