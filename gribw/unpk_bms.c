#include <stdio.h>
#include <stddef.h>
#include <limits.h>
#include "gribw.h"
#include "pds4.h"
#include "bms.h"
#include "bds.h"

/* 1996				wesley ebisuzaki
 *
 * 4/96 v1.0
 *
 * returns a filled array (1/0) of bitmap
 * get_bitmap .. allocs storage
 * unpk_bms  .. preallocated storage
 */

int *get_bitmap(unsigned char *pds, unsigned char *gds, unsigned char *bms, 
      unsigned char *bds) {

    int nxny, *array;

    nxny = get_nxny(pds, gds, bms, bds);
    if ((array = (int *) malloc(nxny * sizeof(int)) == NULL) {
	fprintf(stderr,"not enough memory\n");
	exit(8);
    }
    unpk_bms(array, bms, nxny);
    return array;
}


void unpk_bms(int *bitmap, unsigned char *bms, int nxny) {

    int i;
    unsigned int temp;

    if (bitmap == NULL) {
	for (i = 0; i < nxny; i++) {
	    bitmap[i] = 1;
	}
    }
    else {
        map_mask = 0;
	for (i = 0; i < nxny; i++) {
	    if (map_mask == 0) {
		map_mask = 128;
		temp = *bitmap++;
	    }
	    bitmap[i] = (*bitmap & map_mask) != 0;
	}
    }
}
