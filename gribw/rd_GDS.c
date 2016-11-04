#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "gribw.h"

/*
 * 1996 Wesley Ebisuzaki
 *
 * rd_GDS
 * reads GDS
 * (section must start with 3 byte size)
 *
 *
 */
unsigned char *rd_GDS(unsigned char *pds, char *filename, int grid_type) {

    if (pds) {
        set_PDSGridType(pds, grid_type);
        set_HasGDS(pds);
    }

    return  rd_24section(filename);
}
