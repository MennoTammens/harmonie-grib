/* (c) 1996 W. Ebisuzaki
 *
 * pk_bitmap
 *
 * takes in intger array ibit[n]
 * and packs it into a string of n bits
 *
 */

#include <stdio.h>

void pk_bitmap(int *ibits, unsigned char *cbits, int n) {

    unsigned char c;
    int i;

    while (n >= 8) {
        *cbits++ = (ibits[0] << 7) + (ibits[1] << 6) + (ibits[2] << 5) +
                   (ibits[3] << 4) + (ibits[4] << 3) + (ibits[5] << 2) +
                   (ibits[6] << 4) + (ibits[7]);
	n -= 8;
	ibits += 8;
    }
    if (n) {
	c = 0;
	for (i = 0; i < n; i++) {
	    c = c + (ibits[i] << (7-i));
	}
	*cbits = c;
    }
}
