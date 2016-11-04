#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <math.h>
#include <float.h>
#include <ctype.h>

#define UNDEFINED	9.999e20
#define UNDEFINED_LOW	9.9989e20
#define UNDEFINED_HIGH	9.9991e20
#define UNDEF_ERR	0.00001

#define UNDEFINED_VAL(x) ((x) >= UNDEFINED_LOW && (x) <= UNDEFINED_HIGH)

unsigned char *rd_GDS(unsigned char *pds, char *filename, int grid_type);

unsigned int size_24section(unsigned char *section);
int wrt_24section(unsigned char *section, char *filename);
int append_24section(unsigned char *section, FILE *output);
unsigned char *rd_24section(char *filename);
unsigned char *cpGRIBsec(unsigned char *section);
void wrt_grib_msg(FILE *output, unsigned char *pds, unsigned char *gds, unsigned 
     char *bms, unsigned char *bds);

void set_ParameterTable(unsigned char *pds, int table);
int get_ParameterTable(unsigned char *pds);

void set_Center(unsigned char *pds, int center);
int get_Center(unsigned char *pds);

void set_ProcessID(unsigned char *pds, int id);
int get_ProcessID(unsigned char *pds);

void set_Parameter(unsigned char *pds, int parameter);
int get_Parameter(unsigned char *pds);

void set_SubCenter(unsigned char *pds, int subcenter);
int get_SubCenter(unsigned char *pds);


void set_DecScale(unsigned char *pds, int dec_scale);
int get_DecScale(unsigned char *pds);
void set_PDSGridType(unsigned char *pds, int n);
int get_PDSGridType(unsigned char *pds);
void set_HasGDS(unsigned char *pds);
void clr_HasGDS(unsigned char *pds);
int get_HasGDS(unsigned char *pds);
void set_HasBMS(unsigned char *pds);
void clr_HasBMS(unsigned char *pds);
int get_HasBMS(unsigned char *pds);

void set_InitTime(unsigned char *pds, unsigned int time);
unsigned int get_InitTime(unsigned char *pds);
void set_InitDate(unsigned char *pds, int year, int month, int day,
	int hour, int minute);
void get_InitDate(unsigned char *pds, int *year, int *month, int *day,
	int *hour, int *minute);
unsigned int get_NextYear(unsigned int time);
unsigned int get_NextMonth(unsigned int time);
unsigned int get_NextDay(unsigned int time);
#define get_Hour(time)		((time) % 100)
#define get_Day(time)		(((time)/100) % 100)
#define get_Month(time)		(((time)/10000) % 100)
#define get_Year(time)		((time)/1000000)

void set_NextYear(unsigned char *pds);
void set_NextMonth(unsigned char *pds);
void set_NextDay(unsigned char *pds);

void set_BDSMaxBits(int n);
void set_BDSMinBits(int n);
int get_BDSMaxBits();
int get_BDSMinBits();


void list2bitstream(int *list, unsigned char *bitstream, int ndata, int nbits);
void flist2bitstream(float *list, unsigned char *bitstream, int ndata, int nbits);

void set_NMCparm(unsigned char *pds, char *name);
char *get_NMCparm(unsigned char *pds);
void set_NMCnparm(unsigned char *pds, int parm);
int get_NMCnparm(unsigned char *pds);

unsigned char *seek_grib(FILE *file, long *pos, long *len_grib, 
        unsigned char *buffer, unsigned int buf_len);

int read_grib(FILE *file, long pos, long len_grib, unsigned char *buffer);

double int_power(double x, int y);
int flt2ibm(float x, unsigned char *ibm);
double ibm2flt(unsigned char *ibm);

/* gio.c */
float *rd_f77data(int *n, FILE *in);
void rd_n_f77data(float *a, int n, FILE *in);
void wrt_n_f77data(float *a, int n, FILE *out);

/* f77_io.c */
int f77skip(FILE *stream);
void *f77read(FILE *stream, int *nbytes);

unsigned char *mk_void_BMS(unsigned char *pds);
unsigned char *mk_BMS(unsigned char *pds, float *bindata, int *n, float undef_low, float undef_hi);
unsigned char *mk_BDS(unsigned char *pds, float *bindata, int n);


int get_nxny(unsigned char *pds, unsigned char *gds, unsigned char *bms, unsigned char *bds);

int rd_grib_msg(FILE *input, long int pos, unsigned char **pds,
    unsigned char **gds, unsigned char **bms, unsigned char **bds);

float *get_unpk_bds(unsigned char *pds, unsigned char *gds, unsigned char *bms,
        unsigned char *bds);
int unpk_bds(float *array, unsigned char *pds, unsigned char *gds, 
        unsigned char *bms, unsigned char *bds, int array_size);

int *get_bitmap(unsigned char *pds, unsigned char *gds, unsigned char *bms, 
      unsigned char *bds);
void unpk_bms(int *bitmap, unsigned char *bms, int nxny);

unsigned char *gds_grid2();
unsigned char *NCEP_GDS(unsigned char *pds, int grid_type);

/* private */
void set_int3(unsigned char *string, int n);
void set_int2(unsigned char *string, int n);
void set_PDSlevel(unsigned char *pds, int type, int value);
void set_TimeRange(unsigned char *pds, int time_range, int p1, int p2, \
	int units, int nave, int nmissing);
void get_TimeRange(unsigned char *pds, int *time_range, int *p1, int *p2, \
	int *units, int *nave, int *nmissing);

/* read the wgrib inventory file for pds/gds */
unsigned char *inv_pds(const char *inventory);
unsigned char *inv_gds(const char *inventory);

void wrt_grib_rec(unsigned char *pds, unsigned char *gds, float *data,
            int ndata,  FILE *output);

#define _LEN24(pds)	((int) ((pds[0]<<16)+(pds[1]<<8)+pds[2]))
#define __LEN24(pds)	((pds) == NULL ? 0 : (int) ((pds[0]<<16)+(pds[1]<<8)+pds[2]))


/* version 3.4 of grib headers  w. ebisuzaki */
/* this version is incomplete */

#ifndef INT2
#define INT2(a,b)   ((1-(int) ((unsigned) (a & 0x80) >> 6)) * (int) (((a & 0x7f) << 8) + b))
#endif

#define PDS_Len1(pds)		(pds[0])
#define PDS_Len2(pds)		(pds[1])
#define PDS_Len3(pds)		(pds[2])
#define PDS_LEN(pds)		((int) ((pds[0]<<16)+(pds[1]<<8)+pds[2]))
#define PDS_Vsn(pds)		(pds[3])
#define PDS_Center(pds)		(pds[4])
#define PDS_Model(pds)		(pds[5])
#define PDS_Grid(pds)		(pds[6])
#define PDS_HAS_GDS(pds)	((pds[7] & 128) != 0)
#define PDS_HAS_BMS(pds)	((pds[7] & 64) != 0)
#define PDS_PARAM(pds)		(pds[8])
#define PDS_L_TYPE(pds)		(pds[9])
#define PDS_LEVEL1(pds)		(pds[10])
#define PDS_LEVEL2(pds)		(pds[11])

#define PDS_KPDS5(pds)		(pds[8])
#define PDS_KPDS6(pds)		(pds[9])
#define PDS_KPDS7(pds)		((int) ((pds[10]<<8) + pds[11]))

/* this requires a 32-bit default integer machine */
#define PDS_Field(pds)		((pds[8]<<24)+(pds[9]<<16)+(pds[10]<<8)+pds[11])

#define PDS_Year(pds)		(pds[12])
#define PDS_Month(pds)		(pds[13])
#define PDS_Day(pds)		(pds[14])
#define PDS_Hour(pds)		(pds[15])
#define PDS_Minute(pds)		(pds[16])
#define PDS_ForecastTimeUnit(pds)	(pds[17])
#define PDS_P1(pds)		(pds[18])
#define PDS_P2(pds)		(pds[19])
#define PDS_TimeRange(pds)	(pds[20])
#define PDS_NumAve(pds)		((int) ((pds[21]<<8)+pds[22]))
#define PDS_NumMissing(pds)	(pds[23])
#define PDS_Century(pds)	(pds[24])
#define PDS_Subcenter(pds)	(pds[25])
#define PDS_DecimalScale(pds)	INT2(pds[26],pds[27])
#define PDS_Year4(pds)          (pds[12] + 100*(pds[24] - (pds[12] != 0)))

/* various centers */
#define NMC			7
#define ECMWF			98

/* ECMWF Extensions */

#define PDS_EcLocalId(pds)	(PDS_LEN(pds) >= 41 ? (pds[40]) : 0)
#define PDS_EcClass(pds)	(PDS_LEN(pds) >= 42 ? (pds[41]) : 0)
#define PDS_EcType(pds)		(PDS_LEN(pds) >= 43 ? (pds[42]) : 0)
#define PDS_EcStream(pds)	(PDS_LEN(pds) >= 45 ? (INT2(pds[43], pds[44])) : 0)



/* gribw0
 *
 * gribw: 
 *  v0.1  8/97 Wesley Ebisuzaki
 *
 * wgrib inverse
 *
 */

#define VERSION "gribw v0.1.2.a (1-98) Wesley Ebisuzaki"
#define TEXT_HEADER_LEN	2000

void main(int argc, char **argv) {
    float *array;
    int nrec, nbytes, file_arg, i, nxny;
    unsigned char *pds, *gds;
    char inv[TEXT_HEADER_LEN];
    int prec = 0, recl = 0;

    FILE *bin_in, *bin_out;
    char *filename_bin_in, *filename_bin_out;

    if (argc == 1) {
        fprintf(stderr, "%s\n", VERSION);
	fprintf(stderr,"usage: %s [options] [bin]\n", argv[0]);
	fprintf(stderr,"  -o [output grib]\n");
	fprintf(stderr,"  -i [input binary]\n");
	fprintf(stderr,"  -nh [recl in words]\n");
	fprintf(stderr,"  -prec [bits]\n");
	exit(8);
    }

    filename_bin_in = NULL;
    filename_bin_out = "dump.grb";

    file_arg = 0;
    for (i = 1; i < argc; i++) {
	if (strcmp(argv[i],"-o") == 0) {
	    filename_bin_out = argv[i+1];
	    i++;
	    continue;
	}
	else if (strcmp(argv[i],"-i") == 0) {
	    filename_bin_in = argv[i+1];
	    i++;
	    continue;
	}
	else if (strcmp(argv[i],"-nh") == 0) {
	    recl = sizeof(float) * atoi(argv[i+1]);
	    i++;
	    continue;
	}
	else if (strcmp(argv[i],"-prec") == 0) {
	    prec = atoi(argv[i+1]);
	    if (prec <= 0) prec = 0;
	    if (prec > 24) prec = 24;
	    i++;
	    continue;
	}
	else if (argv[i][0] == '-') {
	    fprintf(stderr,"argument: %s ????\n", argv[i]);
	}
	else {
	    if (file_arg++ < 1) {
		filename_bin_in = argv[i];
	    }
	    else {
	       fprintf(stderr,"argument: %s ????\n", argv[i]);
	    }
	}
    }

    /* open files */
    if (filename_bin_in == NULL) {
	fprintf(stderr, "missing input binary data file\n");
        exit(8);
    }
    if ((bin_in = fopen(filename_bin_in, "rb")) == NULL) {
	fprintf(stderr, "Input binary data file?\n");
        exit(8);
    }
    if ((bin_out = fopen(filename_bin_out, "wb")) == NULL) {
	fprintf(stderr, "Onput binary data file?\n");
        exit(8);
    }

    nrec = 0;
    inv[TEXT_HEADER_LEN-1] = '\0';

    if (prec == 0) {
	/* NCEP style .. decimal scaling only */
        set_BDSMaxBits(16);
    } else {
	/* ECMWF style .. use binary scaling */
        set_BDSMaxBits(prec);
        set_BDSMinBits(prec);
    }

    /* loop for every input record */
    for (;;) {

	/* read header with PDS/GDS */
	if (fgets(inv,TEXT_HEADER_LEN-1, stdin) == NULL) break;
	/* parse PDS/GDS */
	pds = inv_pds(inv);
	gds = inv_gds(inv);
	if (pds == NULL) fprintf(stderr,"pds is missing\n");
	if (gds == NULL) fprintf(stderr,"gds is missing\n");
	if (pds == NULL) break;
	
	/* read binary data */
	nbytes = recl > 0 ? recl : 0;
	
	if ((array = (float *) f77read(bin_in, &nbytes)) == NULL) break;
	if (nbytes % sizeof(float) == 0) {
	    nxny = nbytes / sizeof(float);
	}
	else {
	    fprintf(stderr,"illegal f77-style record\n");
	    exit(8);
	}

	/* generate grib record */
	wrt_grib_rec(pds, gds, array, nxny, bin_out);
	if (gds) free(gds);
	free(pds);
	free(array);
	nrec++;
    }
    printf("wrote %d record(s)\n", nrec);
}

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


int f77skip(FILE *stream) {
	long int position;
	unsigned int recl, tmp;

	position = ftell(stream);
	if (fread(&recl, sizeof (int), 1, stream) != 1) return 0;
	if (fseek(stream, recl, SEEK_CUR) != position + recl + 
              sizeof(int)) return 0;
	if (fread(&tmp, sizeof (int), 1, stream) != 1) return 0;
	return (tmp == recl);
}

/*
 * stream  .. input file
 * nbyptes .. 0 -> f77 header, otherwise size of record
 *
 * returns NULL/data
 *
 */

void *f77read(FILE *stream, int *nbytes) {

	unsigned int recl, tmp;
	char *data;

	if (*nbytes > 0) {
		recl = *nbytes;
		if ((data = (char *) malloc(recl)) == NULL) {
			return NULL;
		}
		if (fread(data, 1, recl, stream) != recl) {
			fprintf(stderr,"f77read: bad file\n");
			free(data);
			return NULL;
		}
		return (void *) data;
	}

	*nbytes = -1;
	if (fread(&recl, sizeof (int), 1, stream) != 1) {
		return NULL;
	}
	if ((data = (char *) malloc(recl)) == NULL) {
		return NULL;
	}
	if (fread(data, 1, recl, stream) != recl) {
		fprintf(stderr,"Not a f77-style file\n");
		free(data);
		return NULL;
	}
	if (fread(&tmp, sizeof (int), 1, stream) != 1) {
		fprintf(stderr,"Not a f77-style file\n");
		free(data);
		return NULL;
	}
	if (tmp != recl) {
		fprintf(stderr,"Not a f77-style file\n");
		free(data);
		return NULL;
	}
	*nbytes = recl;
	return (void *) data;
}
/* v1.0 w. ebisuzaki
 *
 * takes the sections (pds, gds, bms and bds) and writes a grib message
 */


static unsigned char header[8] = {'G', 'R', 'I', 'B', ' ', ' ', ' ', '\1'},
	trailer[4] = {'7', '7', '7', '7'};

void wrt_grib_msg(FILE *output, unsigned char *pds, unsigned char *gds, 
        unsigned char *bms, unsigned char *bds) {

    unsigned int size;


    /* header */

    size = __LEN24(pds) + __LEN24(gds) + __LEN24(bms) + __LEN24(bds) + 12;
    header[4] = (size >> 16) & 255;
    header[5] = (size >>  8) & 255;
    header[6] = (size      ) & 255;

    if (fwrite(header, sizeof (unsigned char), 8, output) != 8) {
	fprintf(stderr,"error writing output\n");
	exit(8);
    }
    append_24section(pds, output);
    append_24section(gds, output);
    append_24section(bms, output);
    append_24section(bds, output);
    if (fwrite(trailer, sizeof (unsigned char), 4, output) != 4) {
	fprintf(stderr,"error writing output\n");
	exit(8);
    }
}

/*
 * 1996 Wesley Ebisuzaki
 *
 * wrt_24section
 * writes a section (bds, gds, bms, etc)
 * (section must start with 3 byte size)
 *
 */

int append_24section(unsigned char *section, FILE *output) {

    int i;

    i = __LEN24(section);
    if (fwrite(section, sizeof (unsigned char), i, output) != i) {
	fprintf(stderr,"error writing output\n");
	return -1;
    }
    return 0;
}
/* v0.3 w. ebisuzaki
 *
 * routines to make bds and bms
 *
 * v0.2 handle binary scaling
 * v0.3 fix bug in binary scaling
 */

#define TEST
#define SHIFT

unsigned char *mk_void_BMS(unsigned char *pds) {
    clr_HasBMS(pds);
    return NULL;
}

/*
 * min_bits == 0 .. NCEP style
 * min_bits >  0 .. ECMWF style
 */

static int max_bits = 16, min_bits = 0;


void set_BDSMaxBits(int n) {
   if (n > 0) max_bits = n;
   if (n > 31) max_bits = 31;
}
void set_BDSMinBits(int n) {
   if (n >= 0) min_bits = n;
   if (n > 31) min_bits = 31;
}
int get_BDSMaxBits() {
   return max_bits;
}
int get_BDSMinBits() {
   return min_bits;
}

unsigned char *mk_BMS(unsigned char *pds, float *bindata, int *n, 
	float undef_low, float undef_hi) {
    int bms_size, i, nn, start;
    unsigned int imask, c;
    unsigned char *bms, *cbits;

    if (undef_low > undef_hi) {
        clr_HasBMS(pds);
        return NULL;
    }

    nn = *n;
    bms_size = 6 + (nn + 7) / 8;

    if (bms_size & 1) bms_size++;
    if ((bms = (unsigned char *) malloc(bms_size * sizeof(char))) == NULL) {
	fprintf(stderr,"*** not enough memory: mk_bms ***\n");
	exit(8);
    }

    set_int3(bms, bms_size);
    bms[3] = bms_size*8 - 6*8 - nn;
    bms[4] = bms[5] = bms[bms_size-1] = 0;

    cbits = bms + 6;
    c = start = 0;
    imask = 128;
    for (i = 0; i < nn; i++) {
	if (bindata[i] < undef_low || bindata[i] > undef_hi) {
	    c += imask;
	    bindata[start++] = bindata[i];
	}
	if ((imask >>= 1) == 0) {
	    *cbits++ = c;
	    c = 0;
	    imask = 128;
	}
    }
    if (imask != 128) *cbits = c;
    if (nn == start) {
	free(bms);
        clr_HasBMS(pds);
        return NULL;
    }
    set_HasBMS(pds);
    *n = start;
    return bms;
}

/* assumed that all the undefined values have been eliminated
 * from bindata
 *
 * get scaling from pds .. no binary scaling
 *
 * note bindata is destroyed
 *
 * v0.2
 */

unsigned char *mk_BDS(unsigned char *pds, float *bindata, int n) {

    float min_value, max_value;
    double frange, dec_scale, scale, fmin;
    int i, nbits, bds_size, unused_bits, bin_scale;
    unsigned char *bds;
    unsigned int uval;

    /* remove decimal scaling if any */
    if (get_DecScale(pds)) {
        dec_scale = int_power(10.0, get_DecScale(pds));
        for (i = 0; i < n; i++) {
	    bindata[i] = bindata[i]*dec_scale;
        }
    }

    /* find min and max values */
    for (max_value = min_value = bindata[0], i = 1; i < n; i++) {
	if (min_value > bindata[i]) min_value = bindata[i];
	if (max_value < bindata[i]) max_value = bindata[i];
    }

    /* NCEP style .. no binary scale .. round numbers to nearest int */
    if (min_bits == 0) {
        fmin = floor(min_value + 0.5);
        frange = floor(max_value - fmin + 0.5);
        frexp(frange, &nbits);
	if (nbits <= max_bits) {
	    bin_scale = 0;
	}
	else {
	    bin_scale = nbits - max_bits;
	    nbits = max_bits;
	}
    }
    /* ECMWF style floating point like values */
    else {
        fmin = min_value;
        frange = max_value - fmin;
	if (frange != 0.0) {
            frexp(frange, &i);
	    bin_scale = i - max_bits;
	    nbits = max_bits;
            scale = ldexp(1.0, -bin_scale);
            frange = floor((max_value - fmin)*scale + 0.5);
	    frexp(frange, &i);
	    if (i != nbits) {
		bin_scale++;
	     }
	}
	else {
	    bin_scale = 0;
            nbits = 0;
	}
    }
    scale = 1;
    if (bin_scale) {
	scale = ldexp(1.0, -bin_scale);
	for (i = 0; i < n; i++) {
	    bindata[i] = (bindata[i] - fmin)*scale + 0.5;
	}
    }
    else {
	for (i = 0; i < n; i++) {
	    bindata[i] = bindata[i] - fmin + 0.5;
 	}
    }


#ifdef TEST
    frange = floor((max_value - fmin)*scale+0.5);
    frexp(frange, &i);
    if (i != nbits) {
	fprintf(stderr,"nbits %d %d error\n",nbits, i);
    }
#endif

    bds_size = 11 + (nbits*n + 7)/8;
    if (bds_size % 2) bds_size++;
    unused_bits = bds_size*8 - 11*8 - nbits*n;
    if (unused_bits > 15 || unused_bits < 0) {
        fprintf(stderr,"programming error! unused bits\n");
        exit(8);
    }
 
    if ((bds = (unsigned char *) malloc(bds_size)) == NULL) { 
	fprintf(stderr,"*** not enough memory ***\n");
	exit(8);
    }

    /* now to fill in the bds */

    set_int3(bds, bds_size);
    bds[3] = 0;			/* flags */

    uval = (bin_scale >= 0) ? (unsigned int) bin_scale :
        (unsigned int) (-bin_scale) + (1U << 15);
    bds[4] = uval >> 8;
    bds[5] = uval & 255;

    flt2ibm(fmin, bds+6);	/* reference value */
    bds[10] = nbits;
    bds[3] = bds[3] | (unused_bits & 15);
    bds[bds_size-1] = 0;

    /* fill in the data */
    flist2bitstream(bindata, bds+11, n, nbits);
    return bds;
}

/*
 * convert a float to an IBM single precision number v1.1
 *
 *                      Wesley Ebisuzaki
 *
 * doesn't handle subnormal numbers
 * v1.1 .. rounding
 */

int flt2ibm(float x, unsigned char *ibm) {

	int sign, exp;
	double mant;
	int imant;

	if (x == 0.0) {
		ibm[0] = ibm[1] = ibm[2] = ibm[3] = 0;
		return 0;
	}

	/* sign bit */
	if (x < 0.0) {
		sign = 128;
		x = -x;
	}
	else sign = 0;

	mant = frexp((double) x, &exp);

	/* round up by adding 2**-24 */
	/* mant = mant + 1.0/16777216.0; */

	if (mant >= 1.0) {
		mant = 0.5;
		exp++;
	}
	while (exp & 3) {
		mant *= 0.5;
		exp++;
	}

        imant = floor(mant * 256.0 * 256.0 * 256.0 + 0.5);
        if (imant >= 256 * 256 * 256) {
            /* imant = 256 * 256 * 256 - 1; */
            imant = floor(mant * 16.0 * 256.0 * 256.0 + 0.5);
            exp -= 4;
	}
	
	exp = exp/4 + 64;

	if (exp < 0) {
		fprintf(stderr,"underflow in flt2ibm\n");
		ibm[0] = ibm[1] = ibm[2] = ibm[3] = 0;
		return 0;
	}
	if (exp > 127) {
		fprintf(stderr,"overflow in flt2ibm\n");
		ibm[0] = sign | 127;
		ibm[1] = ibm[2] = ibm[3] = 255;
		return -1;
	}

	/* normal number */

	ibm[0] = sign | exp;

        ibm[3] = imant & 255;
        ibm[2] = (imant >> 8) & 255;
        ibm[1] = (imant >> 16) & 255;

	return 0;
}


/*
 * w. ebisuzaki
 *
 *  return x**y
 *
 *
 *  input: double x
 *	   int y
 */
double int_power(double x, int y) {

	double value;

	if (y < 0) {
		y = -y;
		x = 1.0 / x;
	}
	value = 1.0;

	while (y) {
		if (y & 1) {
			value *= x;
		}
		x = x * x;
		y >>= 1;
	}
	return value;
}

/*    set/get various parmeters from pds */


void set_ParameterTable(unsigned char *pds, int table) {
    pds[3] = table;
}
int get_ParameterTable(unsigned char *pds) {
    return pds[3];
}

void set_Center(unsigned char *pds, int center) {
    pds[4] = center;
}
int get_Center(unsigned char *pds) {
    return pds[4];
}

void set_ProcessID(unsigned char *pds, int id) {
    pds[5] = id;
}
int get_ProcessID(unsigned char *pds) {
    return pds[5];
}

void set_PDSGridType(unsigned char *pds, int n) {
    pds[6] = n;
}
int get_PDSGridType(unsigned char *pds) {
    return pds[6];
}

void set_HasGDS(unsigned char *pds) {
    pds[7] |= 128;
}
void clr_HasGDS(unsigned char *pds) {
    pds[7] &= 127;
}
int get_HasGDS(unsigned char *pds) {
    return (pds[7] & 128) != 0;
}

void set_HasBMS(unsigned char *pds) {
    pds[7] |= 0x40;
}
void clr_HasBMS(unsigned char *pds) {
    pds[7] &= 0xbf;
}
int get_HasBMS(unsigned char *pds) {
    return (pds[7] & 0x40) != 0;
}

void set_Parameter(unsigned char *pds, int n) {
    pds[8] = n;
}
int get_Parameter(unsigned char *pds) {
    return pds[8];
}


void set_SubCenter(unsigned char *pds, int subcenter) {
    pds[25] = subcenter;
}
int get_SubCenter(unsigned char *pds) {
    return pds[26];
}


void set_DecScale(unsigned char *pds, int dec_scale) {
    if (dec_scale >= 0 && dec_scale < 32768) {
        pds[26] = ((unsigned int) dec_scale) >> 8;
        pds[27] = dec_scale & 255;
    }
    else if (dec_scale < 0 && dec_scale > -32768) {
	dec_scale = - dec_scale;
        pds[26] = (((unsigned int) dec_scale) >> 8) | 128;
        pds[27] = dec_scale & 255;
    }
    else {
	fprintf(stderr,"Bad dec_scale %d\n", dec_scale);
	exit(8);
    }
}


int get_DecScale(unsigned char *pds) {
    return (int) (PDS_DecimalScale(pds));
}


/* Private - Internal Use Only */

void set_int3(unsigned char *string, int n) {
    string[0] = (n >> 16) & 255;
    string[1] = (n >>  8) & 255;
    string[2] =  n        & 255;
}

void set_int2(unsigned char *string, int n) {
    string[0] = (n >>  8) & 255;
    string[1] =  n        & 255;
}

void set_PDSlevel(unsigned char *pds, int type, int value) {
    pds[9] = type;
    pds[10] = (value >> 8) & 255;
    pds[11] =  value & 255;
}

void set_TimeRange(unsigned char *pds, int time_range, int p1, int p2, 
int units, int nave, int nmissing) {
        pds[17] = units;
        pds[18] = p1;
        pds[19] = p2;
        pds[20] = time_range;
	pds[21] = (nave >> 8) & 255;
	pds[22] =  nave       & 255;
	pds[23] = nmissing;

}

void get_TimeRange(unsigned char *pds, int *time_range, int *p1, int *p2, 
int *units, int *nave, int *nmissing) {
        *units = pds[17];
        *p1 = pds[18];
        *p2 = pds[19];
        *time_range = pds[20];
	*nave =  (pds[21] << 8) | pds[22];
	*nmissing = pds[23];
}


/* (c) 1996 Wesley Ebisuzaki
 *
 * grib: convert linear list of ints to a bitstream
 */


static unsigned int mask[] = {0,1,3,7,15,31,63,127,255};

void flist2bitstream(float *list, unsigned char *bitstream, int ndata, int nbits) 
{

    int cbits, jbits;
    unsigned int j, c;

    if (nbits == 0) {
	return;
    }
    if (nbits < 0) {
	fprintf(stderr,"nbits < 0!  nbits = %d\n", nbits);
	exit(0);
    }

    cbits = 8;
    c = 0;
    while (ndata-- > 0) {
	/* note float -> unsigned int .. truncate */
        j = (unsigned int) *list++;
	jbits = nbits;
	while (cbits <= jbits) {
	    if (cbits == 8) {
	        jbits -= 8;
	        *bitstream++ = (j >> jbits) & 255;
	    }
	    else {
	        jbits -= cbits;
	        *bitstream++ = (c << cbits) + ((j >> jbits) & mask[cbits]);
		cbits = 8;
	        c = 0;
	    }
	}
	/* now jbits < cbits */
	if (jbits) {
	    c = (c << jbits) + (j & mask[jbits]);
	    cbits -= jbits;
	}
    }
    if (cbits != 8) *bitstream++ = c << cbits;
}


/*
 * gribw level 0C
 *  v0.1  8/97 Wesley Ebisuzaki
 *
 * simple C interface to write a grib file (level 0)
 *
 * usage int wrt_grib_rec(pds, gds, data, ndata,  output)
 *
 * input:
 *   unsigned char pds[]
 *   unsigned char gds[]
 *   float data[ndata]
 *   FILE *stream
 *
 * returns
 *   nothing
 *
 * undefined values have a special value (see gribw.h)
 *
 * data will be modified
 */

void wrt_grib_rec(unsigned char *pds, unsigned char *gds, float *data, 
	    int ndata,  FILE *output) {

	unsigned char *bms, *bds;

	/* generate grib record */
        bms = mk_BMS(pds, data, &ndata, UNDEFINED_LOW, UNDEFINED_HIGH);

	/* do not generate record with no data */
	if (ndata != 0) {
	    bds = mk_BDS(pds, data, ndata);
	    wrt_grib_msg(output, pds, gds, bms, bds);
	    free(bds);
	}
	if (bms) free(bms);
}
