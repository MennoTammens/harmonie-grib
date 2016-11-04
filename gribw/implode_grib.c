#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <math.h>
#include <float.h>

#include "gribw.h"


/*
 * implode_grib v1.0
 *
 *                              Wesley Ebisuzaki
 *
 */


int main(int argc, char **argv) {

    unsigned char *pds, *gds, *bms, *bds;
    float *bindata;
    int i, n;
    FILE *output, *data;

    output = fopen("output.grb", "wb");
    data = fopen("ieee.data.12", "rb");

    pds = rd_24section("pds");
    set_NMCparm(pds,"TMP");
    set_InitTime(pds,1999010203);
    set_lev_mb(pds,200);
    set_Forecast(pds,12,HOUR);
    set_DecScale(pds,get_DecScale(pds)+1);
    gds = rd_GDS(pds,"gds.98",98);

    bindata = rd_f77data(&n, data);
    bms = mk_BMS(pds, bindata, &n, 9.9989e20, 9.9991e20);
    bds = mk_BDS(pds, bindata, n);

    wrt_grib_msg(output, pds, gds, bms, bds);

    free(bds);
    free(bindata);
    if (bms) free(bms);

    fclose(output);
    return 0;
}
