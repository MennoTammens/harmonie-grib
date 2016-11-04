#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "gribw.h"

/* #define DEBUG */

/*
 * routines to read a wgrib inventory
 *
 * inv_pds v1.0 4/97                      wesley ebisuzki
 *
 * input: a character string (line) from wgrib with PDS=...
 * returns: the pds (binary)
 *
 * inv_gds v1.0 4/97                      wesley ebisuzki
 *
 * input: a character string (line) from wgrib with GDS=...
 * returns: the gds (binary)
 *
 */

static int hex_char(const int ch) {
    /* assume a valid hex character */
    if (ch >= '0' && ch <= '9') return ch - '0';
    if (ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
    return ch - 'A' + 10;
}


static unsigned char *get_hex_inv(const char *inventory, const char *id) {

    int i, nchar;
    char *start, *p;
    unsigned char *section;

    start = strstr(inventory,id);
    if (start == NULL) return NULL;
    start += strlen(id);

    /* find length of section */
    p = start;
    while (isxdigit(*p)) {
	p++;
    }
    nchar = (p - start) >> 1;

    if (nchar ==  0) return NULL;

    if ((section = (unsigned char *) malloc(nchar)) == NULL) {
	fprintf(stderr,"\n***get_inv: not enough memory for %s ***\n", id);
        exit(8);
    }
    for (i = 0; i < nchar; i++) {
	section[i] = (hex_char(start[0]) << 4) + hex_char(start[1]);
        start += 2;
    }
    return section;
}

static unsigned char *get_dec_inv(const char *inventory, const char *id) {

    int i, nchar, n;
    char *start, *p;
    unsigned char *section;

#ifdef DEBUG
    fprintf(stderr,">>get_dec_inv: inventory=%s\n",inventory);
    fprintf(stderr,">>get_dec_inv: id=%s\n",id);
#endif
    start = strstr(inventory,id);
    if (start == NULL) return NULL;
    start += strlen(id);
#ifdef DEBUG
    fprintf(stderr,">>get_dec_inv start-inv=%d\n",start-inventory);
#endif

    /* find length of section */
    p = start;
    nchar = 0;
    while (*p != ':' && *p != '\00') {
	if (isdigit(*p) && !isdigit(p[-1])) nchar++;
	p++;
    }

    if (nchar == 0) return NULL;
    if ((section = (unsigned char *) malloc(nchar)) == NULL) {
        fprintf(stderr,"\n***get_inv: not enough memory for %s ***\n", id);
        exit(8);
    }

    p = start;
    for (i = 0; i < nchar; i++) {
	while (!isdigit(*p)) {
	   p++;
	}
	n = 0;
	while (isdigit(*p)) {
	    n = (n * 10) + *p - '0';
	    p++;
	}
        section[i] = n;
    }
    return section;
}
   


unsigned char *inv_pds(const char *inventory) {
    unsigned char *inv;
    inv = get_hex_inv(inventory, "PDS=");
    if (inv != NULL) return inv;
    return get_dec_inv(inventory, "PDS10=");
}

unsigned char *inv_gds(const char *inventory) {
    unsigned char *inv;
    inv = get_hex_inv(inventory, "GDS=");
    if (inv != NULL) return inv;
    return get_dec_inv(inventory, "GDS10=");
}
