#include <stdio.h>
#include <stdlib.h>

/*
 * 1996 Wesley Ebisuzaki
 *
 * size_24section
 * returns the size of a 24bit section
 * (section must start with 3 byte size)
 *
 */

unsigned int size_24section(unsigned char *section) {

    if (section == NULL) return 0;
    return (section[0] << 16) + (section[1] << 8) + section[2];

}

