#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <math.h>
#include <float.h>

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

void set_Center(unsigned char *pds, int center);
int get_Center(unsigned char *pds);

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


/* v0.1 w. ebisuzaki */
/* need to put in all values */

/* used by TimeRange */
#define MINUTE	0
#define HOUR	1
#define DAY	2
#define MONTH	3
#define YEAR	4
#define DECADE	5
#define NORMAL	6
#define CENTURY	7
#define SECOND	254

#define set_lev_sfc(pds)		set_PDSlevel(pds,1,0)
#define set_lev_toa(pds)		set_PDSlevel(pds,8,0)

#define set_lev_mb(pds,lev)		set_PDSlevel(pds,100,lev)
#define set_lev_hp(pds,lev)		set_PDSlevel(pds,100,lev)
#define set_lev_mb2mb(pds,top,bottom)	set_PDSlevel(pds,101,((int) top/10)*256+ (int) bottom/10)
#define set_lev_hp2hp(pds,top,bottom)	set_PDSlevel(pds,101,((int) top/10)*256+ (int) bottom/10)
#define set_lev_msl			set_PDSlevel(pds,102,0)
#define set_lev_m_MSL(pds,meters)	set_PDSlevel(pds,103,meters)
#define set_lev_m2m_MSL(pds,top,bot)	set_PDSlevel(pds,104,((int) top/10)*256+(int) bot/10)
#define set_lev_m_gnd(pds,meters)	set_PDSlevel(pds,105,meters)
#define set_lev_m2m_gnd(pds,top,bot)	set_PDSlevel(pds,106,((int) top/10)*256+(int) bot/10)
#define set_lev_sigma(pds,sigma)	set_PDSlevel(pds,107,(int) (sigma*10000))
#define set_lev_cm_ugnd(pds,cm)         set_PDSlevel(pds,111,(int) cm)
#define set_lev_cm2cm_ugnd(pds,top,bot) set_PDSlevel(pds,112,(int) top * 256 + (int) bot)
#define set_lev_K(pds,pot_tmp)		set_PDSlevel(pds,113, pot_tmp)
#define set_lev_clm(pds)		set_PDSlevel(pds,200,0)
#define set_lev_ocean(pds)		set_PDSlevel(pds,201,0)

/* tr p1 p2 unit nave nmissing */
#define set_Analysis(pds)		set_TimeRange(pds,0,0,0,1,0,0)
#define set_UninitAnalysis(pds)		set_TimeRange(pds,0,0,0,1,0,0)
#define set_InitAnalysis(pds)		set_TimeRange(pds,0,0,0,1,0,0)
#define set_Average(pds,p1,p2,unit,nave,nmissing)	set_TimeRange(pds,3,p1,p2,unit,nave,nmissing)
#define set_Accumulation(pds,p1,p2,unit,nave,nmissing)	set_TimeRange(pds,4,p1,p2,unit,nave,nmissing)
#define set_Forecast(pds,time,unit)	set_TimeRange(pds,10,time>>8,time&255,unit,0,0)

#define set_Climo(pds,dt,unit,nyear)	set_TimeRange(pds,51,0,dt,unit,nyear,0)
#define set_DiurnalClimo(pds,dt,unit,nyear)	set_TimeRange(pds,51,1,dt,unit,nyear,0)

#define set_AveForecast(pds,p1,p2,unit,nave,nmissing)	set_TimeRange(pds,113,p1,p2,unit,nave,nmissing)
#define set_AveAnalysis(pds,p2,unit,nave,nmissing)	set_TimeRange(pds,113,0,p2,unit,nave,nmissing)
#define set_AccumFcst(pds,p1,p2,unit,nave,nmissing)	set_TimeRange(pds,113,p1,p2,unit,nave,nmissing)
#define set_AccumAnalysis(pds,p2,unit,nave,nmissing)	set_TimeRange(pds,113,0,p2,unit,nave,nmissing)

#define set_Variance(pds,p1,dt,nave,nmissing)	set_TimeRange(pds,118,p1,dt,unit,nave,nmissing)



/*
 * Takes a gds-less file and adds a gds
 *   -- only for NCEP files --
 *
 *                              Wesley Ebisuzaki
 *
 */

int main(int argc, char **argv) {

    long int len_grib, pos = 0;
    unsigned char *pds, *gds, *bms, *bds, *gds0;
    FILE *input, *output;
    int nrec, ngds;

    /* preliminaries .. open up all files */

    if (argc != 3) {
	fprintf(stderr, "%s [in gribfile] [out gribfile]\n", argv[0]);
	fprintf(stderr, "adds GDS to NCEP files\n");
	exit(8);
    }
    if ((input = fopen(argv[1],"rb")) == NULL) {
        fprintf(stderr,"could not open file: %s\n", argv[1]);
        exit(7);
    }
    if ((output = fopen(argv[2],"wb")) == NULL) {
        fprintf(stderr,"could not open file: %s\n", argv[2]);
        exit(7);
    }

    nrec = ngds = pos = 0;
    for(;;) {				/* loop over all records */

        len_grib = rd_grib_msg(input, pos, &pds, &gds, &bms, &bds);
        if (len_grib <= 0) break;

	nrec++;
        if (PDS_HAS_GDS(pds) || PDS_Center(pds) != NMC) {
            wrt_grib_msg(output, pds, gds, bms, bds);
	}
	else {
	    gds0 = NCEP_GDS(pds, PDS_Grid(pds));
            wrt_grib_msg(output, pds, gds0, bms, bds);
	    if (gds0) free(gds0);
	    ngds++;
	}
	pos += len_grib;
    }
    fclose(output);
    fclose(input);
    printf("%d GDS sections added out of %d records\n", ngds, nrec);
    return 0;
}



/*
 * rd_grib_msg.c
 *
 *                              Wesley Ebisuzaki
 *
 * returns pointers to various sections of a grib file
 * NOTE: must save contents of pds .... bds before calling
 * rd_grib_msg again.
 */

#define BUFF_ALLOC0	40000
#define MSEEK 1024

static unsigned char *buffer = NULL;
static int buffer_size = 0;

int rd_grib_msg(FILE *input, long int pos, unsigned char **pds,
    unsigned char **gds, unsigned char **bms, unsigned char **bds) {

    unsigned char *msg, *pointer, *ppds;
    long int len_grib;

    /* setup grib buffer */
    if (buffer == NULL) {
        if ((buffer = (unsigned char *) malloc(BUFF_ALLOC0)) == NULL) {
	    fprintf(stderr,"not enough memory: rd_grib_msg\n");
            buffer_size = BUFF_ALLOC0;
	}
    }

    /* find start and length of message */

    msg = seek_grib(input, &pos, &len_grib, buffer, MSEEK);
    if (msg == NULL) {
	return -1;
    }

    /* read all whole grib record */

    if (len_grib + msg - buffer > buffer_size) {
        buffer_size = len_grib + msg - buffer + 1000;
        buffer = (unsigned char *) realloc((void *) buffer, buffer_size);
        if (buffer == NULL) {
            fprintf(stderr,"ran out of memory\n");
            exit(8);
        }
    }
    read_grib(input, pos, len_grib, buffer);

    /* parse grib message */

    msg = buffer;
    *pds = ppds = (msg + 8);
    pointer = ppds + PDS_LEN(ppds);

    if (PDS_HAS_GDS(ppds)) {
        *gds = pointer;
        pointer += _LEN24(pointer);
    }
    else {
        *gds = NULL;
    }

    if (PDS_HAS_BMS(ppds)) {
        *bms = pointer;
        pointer += _LEN24(pointer);
    }
    else {
        *bms = NULL;
    }

    *bds = pointer;
    pointer += _LEN24(pointer);

    /* end section - "7777" in ascii */
    if (pointer[0] != 0x37 || pointer[1] != 0x37 ||
            pointer[2] != 0x37 || pointer[3] != 0x37) {
        fprintf(stderr,"\n\n    missing end section\n");
        fprintf(stderr, "%2x %2x %2x %2x\n", pointer[0], pointer[1], 
		pointer[2], pointer[3]);
	exit(8);
    }
    return len_grib;
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
 * find next grib header
 *
 * file = what do you think?
 * pos = initial position to start looking at  ( = 0 for 1st call)
 *       returns with position of next grib header (units=bytes)
 * len_grib = length of the grib record (bytes)
 * buffer[buf_len] = buffer for reading/writing
 *
 * returns (char *) to start of GRIB header+PDS
 *         NULL if not found
 *
 * adapted from SKGB (Mark Iredell)
 *
 * v1.1 9/94 Wesley Ebisuzaki
 * v1.2 3/96 Wesley Ebisuzaki handles short records at end of file
 *
 */

#ifndef min
   #define min(a,b)  ((a) < (b) ? (a) : (b))
#endif

#define NTRY 100
/* #define LEN_HEADER_PDS (28+42+100) */
#define LEN_HEADER_PDS (28+8)

unsigned char *seek_grib(FILE *file, long *pos, long *len_grib, 
        unsigned char *buffer, unsigned int buf_len) {

    int i, j, len;

    for (j = 0; j < NTRY; j++) {

        if (fseek(file, *pos, SEEK_SET) == -1) {
            *len_grib = 0;
            return (unsigned char *) NULL;
        }
     
        i = fread(buffer, sizeof (unsigned char), buf_len, file);
     
        len = i - LEN_HEADER_PDS;
     
        for (i = 0; i < len; i++) {
            if (buffer[i] == 'G' && buffer[i+1] == 'R' && buffer[i+2] == 'I'
                && buffer[i+3] == 'B' && buffer[i+7] == 1) {
                    *pos = i + *pos;
                    *len_grib = (buffer[i+4] << 16) + (buffer[i+5] << 8) +
                            buffer[i+6];
                    return (buffer+i);
            }
        }
	*pos = *pos + (buf_len - LEN_HEADER_PDS);
    }

    *len_grib = 0;
    return (unsigned char *) NULL;
}
/*
 * read_grib.c
 *
 * v1.0 9/94 Wesley Ebisuzaki
 *
 */

int read_grib(FILE *file, long pos, long len_grib, unsigned char *buffer) {

    int i;


    if (fseek(file, pos, SEEK_SET) == -1) {
	    return 0;
    }

    i = fread(buffer, sizeof (unsigned char), len_grib, file);
    return (i == len_grib);
}


static unsigned char gds1[ 42] = {
  0,   0,  42,   0, 255,   1,   0,  73,   0,  23, 
128, 187, 218,   0,   0,   0, 128,   0, 187, 218, 
  0,   0,   0,   0,  87, 228,   0,  64,   7, 214, 
133,   7, 214, 133,   0,   0,   0,   0,   0,   0, 
  0,   0, 
 };
static unsigned char gds2[ 32] = {
  0,   0,  32,   0, 255,   0,   0, 144,   0,  73, 
  1,  95, 144,   0,   0,   0, 128, 129,  95, 144, 
128,   9, 196,   9, 196,   9, 196,   0,   0,   0, 
  0,   0, 
 };
static unsigned char gds3[ 32] = {
  0,   0,  32,   0, 255,   0,   1, 104,   0, 181, 
  1,  95, 144,   0,   0,   0, 128, 129,  95, 144, 
128,   3, 232,   3, 232,   3, 232,   0,   0,   0, 
  0,   0, 
 };
static unsigned char gds4[ 32] = {
  0,   0,  32,   0, 255,   0,   2, 208,   1, 105, 
  1,  95, 144,   0,   0,   0, 128, 129,  95, 144, 
128,   1, 244,   1, 244,   1, 244,   0,   0,   0, 
  0,   0, 
 };
static unsigned char gds5[ 32] = {
  0,   0,  32,   0, 255,   5,   0,  53,   0,  57, 
  0,  29, 223, 130,   9,  67,   8, 129, 154,  40, 
  2, 232,  36,   2, 232,  36,   0,  64,   0,   0, 
  0,   0, 
 };
static unsigned char gds6[ 32] = {
  0,   0,  32,   0, 255,   5,   0,  53,   0,  45, 
  0,  29, 223, 130,   9,  67,   8, 129, 154,  40, 
  2, 232,  36,   2, 232,  36,   0,  64,   0,   0, 
  0,   0, 
 };
static unsigned char gds21[ 32] = {
  0,   0,  32,   0, 255,   0,   0,  37,   0,  37, 
  0,   0,   0,   0,   0,   0, 128,   1,  95, 144, 
  2, 191,  32,  19, 136,   9, 196,  64,   0,   0, 
  0,   0, 
 };
static unsigned char gds22[ 32] = {
  0,   0,  32,   0, 255,   0,   0,  37,   0,  37, 
  0,   0,   0, 130, 191,  32, 128,   1,  95, 144, 
  0,   0,   0,  19, 136,   9, 196,  64,   0,   0, 
  0,   0, 
 };
static unsigned char gds23[ 32] = {
  0,   0,  32,   0, 255,   0,   0,  37,   0,  37, 
129,  95, 144,   0,   0,   0, 128,   0,   0,   0, 
  2, 191,  32,  19, 136,   9, 196,  64,   0,   0, 
  0,   0, 
 };
static unsigned char gds24[ 32] = {
  0,   0,  32,   0, 255,   0,   0,  37,   0,  37, 
129,  95, 144, 130, 191,  32, 128,   0,   0,   0, 
  0,   0,   0,  19, 136,   9, 196,  64,   0,   0, 
  0,   0, 
 };
static unsigned char gds25[ 32] = {
  0,   0,  32,   0, 255,   0,   0,  72,   0,  19, 
  0,   0,   0,   0,   0,   0, 128,   1,  95, 144, 
  5, 106, 184,  19, 136,  19, 136,  64,   0,   0, 
  0,   0, 
 };
static unsigned char gds26[ 32] = {
  0,   0,  32,   0, 255,   0,   0,  72,   0,  19, 
129,  95, 144,   0,   0,   0, 128,   0,   0,   0, 
  5, 106, 184,  19, 136,  19, 136,  64,   0,   0, 
  0,   0, 
 };
static unsigned char gds27[ 32] = {
  0,   0,  32,   0, 255,   5,   0,  65,   0,  65, 
128,  79,  62, 129, 232,  72,   8, 129,  56, 128, 
  5, 208,  72,   5, 208,  72,   0,  64,   0,   0, 
  0,   0, 
 };
static unsigned char gds28[ 32] = {
  0,   0,  32,   0, 255,   5,   0,  65,   0,  65, 
  0,  79,  62,   2,  54, 104,   8,   1, 134, 160, 
  5, 208,  72,   5, 208,  72, 128,  64,   0,   0, 
  0,   0, 
 };
static unsigned char gds29[ 32] = {
  0,   0,  32,   0, 255,   0,   0, 145,   0,  37, 
  0,   0,   0,   0,   0,   0, 128,   1,  95, 144, 
  5, 126,  64,   9, 196,   9, 196,  64,   0,   0, 
  0,   0, 
 };
static unsigned char gds30[ 32] = {
  0,   0,  32,   0, 255,   0,   0, 145,   0,  37, 
  0,   0,   0, 129,  95, 144, 128,   0,   0,   0, 
  5, 126,  64,   9, 196,   9, 196,  64,   0,   0, 
  0,   0, 
 };
static unsigned char gds33[ 32] = {
  0,   0,  32,   0, 255,   0,   0, 181,   0,  46, 
  0,   0,   0,   0,   0,   0, 128,   1,  95, 144, 
  5, 126,  64,   7, 208,   7, 208,  64,   0,   0, 
  0,   0, 
 };
static unsigned char gds34[ 32] = {
  0,   0,  32,   0, 255,   0,   0, 181,   0,  46, 
129,  95, 144,   0,   0,   0, 128,   0,   0,   0, 
  5, 126,  64,   7, 208,   7, 208,  64,   0,   0, 
  0,   0, 
 };
static unsigned char gds37[178] = {
  0,   0, 178,   0,  33,   0, 255, 255,   0,  73, 
  0,   0,   0, 128, 117,  48, 128,   1,  95, 144, 
  0, 234,  96, 255, 255,   4, 226,  64,   0,   0, 
  0,   0,   0,  73,   0,  73,   0,  73,   0,  73, 
  0,  73,   0,  73,   0,  73,   0,  73,   0,  72, 
  0,  72,   0,  72,   0,  71,   0,  71,   0,  71, 
  0,  70,   0,  70,   0,  69,   0,  69,   0,  68, 
  0,  67,   0,  67,   0,  66,   0,  65,   0,  65, 
  0,  64,   0,  63,   0,  62,   0,  61,   0,  60, 
  0,  60,   0,  59,   0,  58,   0,  57,   0,  56, 
  0,  55,   0,  54,   0,  52,   0,  51,   0,  50, 
  0,  49,   0,  48,   0,  47,   0,  45,   0,  44, 
  0,  43,   0,  42,   0,  40,   0,  39,   0,  38, 
  0,  36,   0,  35,   0,  33,   0,  32,   0,  30, 
  0,  29,   0,  28,   0,  26,   0,  25,   0,  23, 
  0,  22,   0,  20,   0,  19,   0,  17,   0,  16, 
  0,  14,   0,  12,   0,  11,   0,   9,   0,   8, 
  0,   6,   0,   5,   0,   3,   0,   2, 
 };
static unsigned char gds38[178] = {
  0,   0, 178,   0,  33,   0, 255, 255,   0,  73, 
  0,   0,   0,   0, 234,  96, 128,   1,  95, 144, 
  2,  73, 240, 255, 255,   4, 226,  64,   0,   0, 
  0,   0,   0,  73,   0,  73,   0,  73,   0,  73, 
  0,  73,   0,  73,   0,  73,   0,  73,   0,  72, 
  0,  72,   0,  72,   0,  71,   0,  71,   0,  71, 
  0,  70,   0,  70,   0,  69,   0,  69,   0,  68, 
  0,  67,   0,  67,   0,  66,   0,  65,   0,  65, 
  0,  64,   0,  63,   0,  62,   0,  61,   0,  60, 
  0,  60,   0,  59,   0,  58,   0,  57,   0,  56, 
  0,  55,   0,  54,   0,  52,   0,  51,   0,  50, 
  0,  49,   0,  48,   0,  47,   0,  45,   0,  44, 
  0,  43,   0,  42,   0,  40,   0,  39,   0,  38, 
  0,  36,   0,  35,   0,  33,   0,  32,   0,  30, 
  0,  29,   0,  28,   0,  26,   0,  25,   0,  23, 
  0,  22,   0,  20,   0,  19,   0,  17,   0,  16, 
  0,  14,   0,  12,   0,  11,   0,   9,   0,   8, 
  0,   6,   0,   5,   0,   3,   0,   2, 
 };
static unsigned char gds39[178] = {
  0,   0, 178,   0,  33,   0, 255, 255,   0,  73, 
  0,   0,   0,   2,  73, 240, 128,   1,  95, 144, 
129, 212, 192, 255, 255,   4, 226,  64,   0,   0, 
  0,   0,   0,  73,   0,  73,   0,  73,   0,  73, 
  0,  73,   0,  73,   0,  73,   0,  73,   0,  72, 
  0,  72,   0,  72,   0,  71,   0,  71,   0,  71, 
  0,  70,   0,  70,   0,  69,   0,  69,   0,  68, 
  0,  67,   0,  67,   0,  66,   0,  65,   0,  65, 
  0,  64,   0,  63,   0,  62,   0,  61,   0,  60, 
  0,  60,   0,  59,   0,  58,   0,  57,   0,  56, 
  0,  55,   0,  54,   0,  52,   0,  51,   0,  50, 
  0,  49,   0,  48,   0,  47,   0,  45,   0,  44, 
  0,  43,   0,  42,   0,  40,   0,  39,   0,  38, 
  0,  36,   0,  35,   0,  33,   0,  32,   0,  30, 
  0,  29,   0,  28,   0,  26,   0,  25,   0,  23, 
  0,  22,   0,  20,   0,  19,   0,  17,   0,  16, 
  0,  14,   0,  12,   0,  11,   0,   9,   0,   8, 
  0,   6,   0,   5,   0,   3,   0,   2, 
 };
static unsigned char gds40[178] = {
  0,   0, 178,   0,  33,   0, 255, 255,   0,  73, 
  0,   0,   0, 129, 212, 192, 128,   1,  95, 144, 
128, 117,  48, 255, 255,   4, 226,  64,   0,   0, 
  0,   0,   0,  73,   0,  73,   0,  73,   0,  73, 
  0,  73,   0,  73,   0,  73,   0,  73,   0,  72, 
  0,  72,   0,  72,   0,  71,   0,  71,   0,  71, 
  0,  70,   0,  70,   0,  69,   0,  69,   0,  68, 
  0,  67,   0,  67,   0,  66,   0,  65,   0,  65, 
  0,  64,   0,  63,   0,  62,   0,  61,   0,  60, 
  0,  60,   0,  59,   0,  58,   0,  57,   0,  56, 
  0,  55,   0,  54,   0,  52,   0,  51,   0,  50, 
  0,  49,   0,  48,   0,  47,   0,  45,   0,  44, 
  0,  43,   0,  42,   0,  40,   0,  39,   0,  38, 
  0,  36,   0,  35,   0,  33,   0,  32,   0,  30, 
  0,  29,   0,  28,   0,  26,   0,  25,   0,  23, 
  0,  22,   0,  20,   0,  19,   0,  17,   0,  16, 
  0,  14,   0,  12,   0,  11,   0,   9,   0,   8, 
  0,   6,   0,   5,   0,   3,   0,   2, 
 };
static unsigned char gds41[178] = {
  0,   0, 178,   0,  33,   0, 255, 255,   0,  73, 
129,  95, 144, 128, 117,  48, 128,   0,   0,   0, 
  0, 234,  96, 255, 255,   4, 226,  64,   0,   0, 
  0,   0,   0,   2,   0,   3,   0,   5,   0,   6, 
  0,   8,   0,   9,   0,  11,   0,  12,   0,  14, 
  0,  16,   0,  17,   0,  19,   0,  20,   0,  22, 
  0,  23,   0,  25,   0,  26,   0,  28,   0,  29, 
  0,  30,   0,  32,   0,  33,   0,  35,   0,  36, 
  0,  38,   0,  39,   0,  40,   0,  42,   0,  43, 
  0,  44,   0,  45,   0,  47,   0,  48,   0,  49, 
  0,  50,   0,  51,   0,  52,   0,  54,   0,  55, 
  0,  56,   0,  57,   0,  58,   0,  59,   0,  60, 
  0,  60,   0,  61,   0,  62,   0,  63,   0,  64, 
  0,  65,   0,  65,   0,  66,   0,  67,   0,  67, 
  0,  68,   0,  69,   0,  69,   0,  70,   0,  70, 
  0,  71,   0,  71,   0,  71,   0,  72,   0,  72, 
  0,  72,   0,  73,   0,  73,   0,  73,   0,  73, 
  0,  73,   0,  73,   0,  73,   0,  73, 
 };
static unsigned char gds42[178] = {
  0,   0, 178,   0,  33,   0, 255, 255,   0,  73, 
129,  95, 144,   0, 234,  96, 128,   0,   0,   0, 
  2,  73, 240, 255, 255,   4, 226,  64,   0,   0, 
  0,   0,   0,   2,   0,   3,   0,   5,   0,   6, 
  0,   8,   0,   9,   0,  11,   0,  12,   0,  14, 
  0,  16,   0,  17,   0,  19,   0,  20,   0,  22, 
  0,  23,   0,  25,   0,  26,   0,  28,   0,  29, 
  0,  30,   0,  32,   0,  33,   0,  35,   0,  36, 
  0,  38,   0,  39,   0,  40,   0,  42,   0,  43, 
  0,  44,   0,  45,   0,  47,   0,  48,   0,  49, 
  0,  50,   0,  51,   0,  52,   0,  54,   0,  55, 
  0,  56,   0,  57,   0,  58,   0,  59,   0,  60, 
  0,  60,   0,  61,   0,  62,   0,  63,   0,  64, 
  0,  65,   0,  65,   0,  66,   0,  67,   0,  67, 
  0,  68,   0,  69,   0,  69,   0,  70,   0,  70, 
  0,  71,   0,  71,   0,  71,   0,  72,   0,  72, 
  0,  72,   0,  73,   0,  73,   0,  73,   0,  73, 
  0,  73,   0,  73,   0,  73,   0,  73, 
 };
static unsigned char gds43[178] = {
  0,   0, 178,   0,  33,   0, 255, 255,   0,  73, 
129,  95, 144,   2,  73, 240, 128,   0,   0,   0, 
129, 212, 192, 255, 255,   4, 226,  64,   0,   0, 
  0,   0,   0,   2,   0,   3,   0,   5,   0,   6, 
  0,   8,   0,   9,   0,  11,   0,  12,   0,  14, 
  0,  16,   0,  17,   0,  19,   0,  20,   0,  22, 
  0,  23,   0,  25,   0,  26,   0,  28,   0,  29, 
  0,  30,   0,  32,   0,  33,   0,  35,   0,  36, 
  0,  38,   0,  39,   0,  40,   0,  42,   0,  43, 
  0,  44,   0,  45,   0,  47,   0,  48,   0,  49, 
  0,  50,   0,  51,   0,  52,   0,  54,   0,  55, 
  0,  56,   0,  57,   0,  58,   0,  59,   0,  60, 
  0,  60,   0,  61,   0,  62,   0,  63,   0,  64, 
  0,  65,   0,  65,   0,  66,   0,  67,   0,  67, 
  0,  68,   0,  69,   0,  69,   0,  70,   0,  70, 
  0,  71,   0,  71,   0,  71,   0,  72,   0,  72, 
  0,  72,   0,  73,   0,  73,   0,  73,   0,  73, 
  0,  73,   0,  73,   0,  73,   0,  73, 
 };
static unsigned char gds44[178] = {
  0,   0, 178,   0,  33,   0, 255, 255,   0,  73, 
129,  95, 144, 129, 212, 192, 128,   0,   0,   0, 
128, 117,  48, 255, 255,   4, 226,  64,   0,   0, 
  0,   0,   0,   2,   0,   3,   0,   5,   0,   6, 
  0,   8,   0,   9,   0,  11,   0,  12,   0,  14, 
  0,  16,   0,  17,   0,  19,   0,  20,   0,  22, 
  0,  23,   0,  25,   0,  26,   0,  28,   0,  29, 
  0,  30,   0,  32,   0,  33,   0,  35,   0,  36, 
  0,  38,   0,  39,   0,  40,   0,  42,   0,  43, 
  0,  44,   0,  45,   0,  47,   0,  48,   0,  49, 
  0,  50,   0,  51,   0,  52,   0,  54,   0,  55, 
  0,  56,   0,  57,   0,  58,   0,  59,   0,  60, 
  0,  60,   0,  61,   0,  62,   0,  63,   0,  64, 
  0,  65,   0,  65,   0,  66,   0,  67,   0,  67, 
  0,  68,   0,  69,   0,  69,   0,  70,   0,  70, 
  0,  71,   0,  71,   0,  71,   0,  72,   0,  72, 
  0,  72,   0,  73,   0,  73,   0,  73,   0,  73, 
  0,  73,   0,  73,   0,  73,   0,  73, 
 };
static unsigned char gds45[ 32] = {
  0,   0,  32,   0, 255,   0,   1,  32,   0, 145, 
  1,  95, 144,   0,   0,   0, 128, 129,  95, 144, 
128,   4, 226,   4, 226,   4, 226,   0,   0,   0, 
  0,   0, 
 };
static unsigned char gds55[ 32] = {
  0,   0,  32,   0, 255,   5,   0,  87,   0,  71, 
128,  42, 195, 130,  90, 177,   8, 129, 154,  40, 
  3, 224,  48,   3, 224,  48,   0,  64,   0,   0, 
  0,   0, 
 };
static unsigned char gds56[ 32] = {
  0,   0,  32,   0, 255,   5,   0,  87,   0,  71, 
  0,  29, 223, 130,   9,  67,   8, 129, 154,  40, 
  1, 240,  24,   1, 240,  24,   0,  64,   0,   0, 
  0,   0, 
 };
static unsigned char gds61[ 32] = {
  0,   0,  32,   0, 255,   0,   0,  91,   0,  46, 
  0,   0,   0,   0,   0,   0, 128,   1,  95, 144, 
  2, 191,  32,   7, 208,   7, 208,  64,   0,   0, 
  0,   0, 
 };
static unsigned char gds62[ 32] = {
  0,   0,  32,   0, 255,   0,   0,  91,   0,  46, 
  0,   0,   0, 130, 191,  32, 128,   1,  95, 144, 
  0,   0,   0,   7, 208,   7, 208,  64,   0,   0, 
  0,   0, 
 };
static unsigned char gds63[ 32] = {
  0,   0,  32,   0, 255,   0,   0,  91,   0,  46, 
  0,   0,   0, 129,  95, 144, 128,   0,   0,   0, 
  2, 191,  32,   7, 208,   7, 208,  64,   0,   0, 
  0,   0, 
 };
static unsigned char gds64[ 32] = {
  0,   0,  32,   0, 255,   0,   0,  91,   0,  46, 
129,  95, 144, 130, 191,  32, 128,   0,   0,   0, 
  0,   0,   0,   7, 208,   7, 208,  64,   0,   0, 
  0,   0, 
 };
static unsigned char gds85[ 32] = {
  0,   0,  32,   0, 255,   0,   1, 104,   0,  90, 
  0,   1, 244,   0,   1, 244, 128,   1,  93, 156, 
  5, 124,  76,   3, 232,   3, 232,  64,   0,   0, 
  0,   0, 
 };
static unsigned char gds86[ 32] = {
  0,   0,  32,   0, 255,   0,   1, 104,   0,  90, 
129,  93, 156,   0,   1, 244, 128, 128,   1, 244, 
  5, 124,  76,   3, 232,   3, 232,  64,   0,   0, 
  0,   0, 
 };
static unsigned char gds87[ 32] = {
  0,   0,  32,   0, 255,   5,   0,  81,   0,  62, 
  0,  89,  92, 129, 214, 171,   8, 129, 154,  40, 
  1,  10,  57,   1,  10,  57,   0,  64,   0,   0, 
  0,   0, 
 };
static unsigned char gds90[ 32] = {
  0,   0,  32,   0, 255, 201,  50, 102,   0,   1, 
  0,   0, 182, 130,  73, 127, 136,   0,   0,  92, 
  0,   0, 141,   2,  65,   2,  26,  64,   0,   0, 
  0,   0, 
 };
static unsigned char gds91[ 32] = {
  0,   0,  32,   0, 255, 202, 100, 203,   0,   1, 
  0,   0, 182, 130,  73, 127, 136,   0,   0, 183, 
  0,   0, 141,   2,  65,   2,  26,  64,   0,   0, 
  0,   0, 
 };
static unsigned char gds92[ 32] = {
  0,   0,  32,   0, 255, 201,  94,  98,   0,   1, 
  0,  37, 206, 129, 247,  58, 136,   0,   0, 127, 
  0,   0, 191,   1,  22,   1,   7,  64,   0,   0, 
  0,   0, 
 };
static unsigned char gds93[ 32] = {
  0,   0,  32,   0, 255, 202, 188, 195,   0,   1, 
  0,  37, 206, 129, 247,  58, 136,   0,   0, 253, 
  0,   0, 191,   1,  22,   1,   7,  64,   0,   0, 
  0,   0, 
 };
static unsigned char gds94[ 32] = {
  0,   0,  32,   0, 255, 201, 191,  20,   0,   1, 
  0,  37, 206, 129, 247,  58, 136,   0,   0, 181, 
  0,   1,  15,   0, 194,   0, 185,  64,   0,   0, 
  0,   0, 
 };
static unsigned char gds95[ 32] = {
  0,   0,  32,   0, 255, 202, 126,  39,   0,   1, 
  0,  37, 206, 129, 247,  58, 136,   0,   1, 105, 
  0,   1,  15,   0, 194,   0, 185,  64,   0,   0, 
  0,   0, 
 };
static unsigned char gds96[ 32] = {
  0,   0,  32,   0, 255, 201, 162, 158,   0,   1, 
128,  13, 113, 130,  69,  63, 136,   0,   0, 160, 
  0,   1,   5,   1,  77,   1,  52,  64,   0,   0, 
  0,   0, 
 };
static unsigned char gds97[ 32] = {
  0,   0,  32,   0, 255, 202,  69,  59,   0,   1, 
128,  13, 113, 130,  69,  63, 136,   0,   1,  63, 
  0,   1,   5,   1,  77,   1,  52,  64,   0,   0, 
  0,   0, 
 };
static unsigned char gds98[ 32] = {
  0,   0,  32,   0, 255,   4,   0, 192,   0,  94, 
  1,  89, 222,   0,   0,   0, 128, 129,  89, 222, 
128,   3, 170,   7,  83,   0,  47,   0,   0,   0, 
  0,   0, 
 };
static unsigned char gds100[ 32] = {
  0,   0,  32,   0, 255,   5,   0,  83,   0,  83, 
  0,  66, 212, 129, 249,  16,   8, 129, 154,  40, 
  1, 101,  60,   1, 101,  60,   0,  64,   0,   0, 
  0,   0, 
 };
static unsigned char gds101[ 32] = {
  0,   0,  32,   0, 255,   5,   0, 113,   0,  91, 
  0,  41,  32, 130,  23, 186,   8, 129, 154,  40, 
  1, 101,  60,   1, 101,  60,   0,  64,   0,   0, 
  0,   0, 
 };
static unsigned char gds103[ 32] = {
  0,   0,  32,   0, 255,   5,   0,  65,   0,  56, 
  0,  87, 133, 129, 218,   8,   8, 129, 154,  40, 
  1, 101,  60,   1, 101,  60,   0,  64,   0,   0, 
  0,   0, 
 };
static unsigned char gds104[ 32] = {
  0,   0,  32,   0, 255,   5,   0, 147,   0, 110, 
128,   1,  12, 130,  32, 211,   8, 129, 154,  40, 
  1,  98, 131,   1,  98, 131,   0,  64,   0,   0, 
  0,   0, 
 };
static unsigned char gds105[ 32] = {
  0,   0,  32,   0, 255,   5,   0,  83,   0,  83, 
  0,  68, 121, 129, 249,  16,   8, 129, 154,  40, 
  1,  98, 131,   1,  98, 131,   0,  64,   0,   0, 
  0,   0, 
 };
static unsigned char gds106[ 32] = {
  0,   0,  32,   0, 255,   5,   0, 165,   0, 117, 
  0,  68, 125, 129, 249,  16,   8, 129, 154,  40, 
  0, 177,  61,   0, 177,  61,   0,  64,   0,   0, 
  0,   0, 
 };
static unsigned char gds107[ 32] = {
  0,   0,  32,   0, 255,   5,   0, 120,   0,  92, 
  0,  91, 142, 129, 213, 104,   8, 129, 154,  40, 
  0, 177,  61,   0, 177,  61,   0,  64,   0,   0, 
  0,   0, 
 };
static unsigned char gds126[ 32] = {
  0,   0,  32,   0, 255,   4,   1, 128,   0, 190, 
  1,  92, 189,   0,   0,   0, 128, 129,  92, 189, 
128,   3, 170,   3, 170,   0,  95,   0,   0,   0, 
  0,   0, 
 };
static unsigned char gds192[ 32] = {
  0, 0, 32, 0, 255, 203, 0, 237, 1, 131, 128, 13, 
  113, 130, 69, 63, 136, 0, 195, 80, 129, 177, 152, 
  0, 225, 0, 207, 64, 0, 0, 0, 0
 };
 
static unsigned char gds201[ 32] = {
  0,   0,  32,   0, 255,   5,   0,  65,   0,  65, 
128,  81,  90, 130,  73, 240,   8, 129, 154,  40, 
  5, 208,  72,   5, 208,  72,   0,  64,   0,   0, 
  0,   0, 
 };
static unsigned char gds202[ 32] = {
  0,   0,  32,   0, 255,   5,   0,  65,   0,  43, 
  0,  30, 158, 130,  38, 228,   8, 129, 154,  40, 
  2, 232,  36,   2, 232,  36,   0,  64,   0,   0, 
  0,   0, 
 };
static unsigned char gds203[ 32] = {
  0,   0,  32,   0, 255,   5,   0,  45,   0,  39, 
  0,  74, 188, 130, 213, 237,   8, 130,  73, 240, 
  2, 232,  36,   2, 232,  36,   0,  64,   0,   0, 
  0,   0, 
 };
static unsigned char gds204[ 42] = {
  0,   0,  42,   0, 255,   1,   0,  93,   0,  68, 
128,  97, 168,   1, 173, 176, 128,   0, 236, 228, 
129, 170,  73,   0,  78,  32,   0,  64,   2, 113, 
  0,   2, 113,   0,   0,   0,   0,   0,   0,   0, 
  0,   0, 
 };
static unsigned char gds205[ 32] = {
  0,   0,  32,   0, 255,   5,   0,  45,   0,  39, 
  0,   2, 104, 129,  75, 168,   8, 128, 234,  96, 
  2, 232,  36,   2, 232,  36,   0,  64,   0,   0, 
  0,   0, 
 };
static unsigned char gds206[ 42] = {
  0,   0,  42,   0, 255,   3,   0,  51,   0,  41, 
  0,  87,  17, 129, 204, 231,   8, 129, 115,  24, 
  1,  61, 119,   1,  61, 119,   0,  64,   0,  97, 
168,   0,  97, 168,   0,   0,   0,   0,   0,   0, 
  0,   0, 
 };
static unsigned char gds207[ 32] = {
  0,   0,  32,   0, 255,   5,   0,  49,   0,  35, 
  0, 164, 101, 130, 174,  25,   8, 130,  73, 240, 
  1, 116,  18,   1, 116,  18,   0,  64,   0,   0, 
  0,   0, 
 };
static unsigned char gds208[ 42] = {
  0,   0,  42,   0, 255,   1,   0,  29,   0,  27, 
  0,  36, 127, 130, 141, 147, 128,   0, 109, 188, 
130,  57, 214,   0,  78,  32,   0,  64,   1,  56, 
128,   1,  56, 128,   0,   0,   0,   0,   0,   0, 
  0,   0, 
 };
static unsigned char gds209[ 42] = {
  0,   0,  42,   0, 255,   3,   0, 101,   0,  81, 
  0,  87,  17, 129, 204, 231,   8, 129, 115,  24, 
  0, 158, 187,   0, 158, 187,   0,  64,   0,  97, 
168,   0,  97, 168,   0,   0,   0,   0,   0,   0, 
  0,   0, 
 };
static unsigned char gds210[ 42] = {
  0,   0,  42,   0, 255,   1,   0,  25,   0,  25, 
  0,  35,  40, 129,  44, 200, 128,   0, 103,  54, 
128, 229,   1,   0,  78,  32,   0,  64,   1,  56, 
128,   1,  56, 128,   0,   0,   0,   0,   0,   0, 
  0,   0, 
 };
static unsigned char gds211[ 42] = {
  0,   0,  42,   0, 255,   3,   0,  93,   0,  65, 
  0,  47, 158, 130,   9,  83,   8, 129, 115,  24, 
  1,  61, 119,   1,  61, 119,   0,  64,   0,  97, 
168,   0,  97, 168,   0,   0,   0,   0,   0,   0, 
  0,   0, 
 };
static unsigned char gds212[ 42] = {
  0,   0,  42,   0, 255,   3,   0, 185,   0, 129, 
  0,  47, 158, 130,   9,  83,   8, 129, 115,  24, 
  0, 158, 187,   0, 158, 187,   0,  64,   0,  97, 
168,   0,  97, 168,   0,   0,   0,   0,   0,   0, 
  0,   0, 
 };
static unsigned char gds213[ 32] = {
  0,   0,  32,   0, 255,   5,   0, 129,   0,  85, 
  0,  30, 158, 130,  38, 228,   8, 129, 154,  40, 
  1, 116,  18,   1, 116,  18,   0,  64,   0,   0, 
  0,   0, 
 };
static unsigned char gds214[ 32] = {
  0,   0,  32,   0, 255,   5,   0,  97,   0,  69, 
  0, 164, 101, 130, 174,  25,   8, 130,  73, 240, 
  0, 186,   9,   0, 186,   9,   0,  64,   0,   0, 
  0,   0, 
 };
static unsigned char gds215[ 42] = {
  0,   0,  42,   0, 255,   3,   1, 113,   1,   1, 
  0,  47, 158, 130,   9,  83,   8, 129, 115,  24, 
  0,  79,  94,   0,  79,  94,   0,  64,   0,  97, 
168,   0,  97, 168,   0,   0,   0,   0,   0,   0, 
  0,   0, 
 };
  
static unsigned char *std_ncep_gds[256] = {
  NULL, /*   0 */
  gds1,
  gds2,
  gds3,
  gds4,
  gds5,
  gds6,
  NULL, /*   7 */
  NULL, /*   8 */
  NULL, /*   9 */
  NULL, /*  10 */
  NULL, /*  11 */
  NULL, /*  12 */
  NULL, /*  13 */
  NULL, /*  14 */
  NULL, /*  15 */
  NULL, /*  16 */
  NULL, /*  17 */
  NULL, /*  18 */
  NULL, /*  19 */
  NULL, /*  20 */
  gds21,
  gds22,
  gds23,
  gds24,
  gds25,
  gds26,
  gds27,
  gds28,
  gds29,
  gds30,
  NULL, /*  31 */
  NULL, /*  32 */
  gds33,
  gds34,
  NULL, /*  35 */
  NULL, /*  36 */
  gds37,
  gds38,
  gds39,
  gds40,
  gds41,
  gds42,
  gds43,
  gds44,
  gds45,
  NULL, /*  46 */
  NULL, /*  47 */
  NULL, /*  48 */
  NULL, /*  49 */
  NULL, /*  50 */
  NULL, /*  51 */
  NULL, /*  52 */
  NULL, /*  53 */
  NULL, /*  54 */
  gds55,
  gds56,
  NULL, /*  57 */
  NULL, /*  58 */
  NULL, /*  59 */
  NULL, /*  60 */
  gds61,
  gds62,
  gds63,
  gds64,
  NULL, /*  65 */
  NULL, /*  66 */
  NULL, /*  67 */
  NULL, /*  68 */
  NULL, /*  69 */
  NULL, /*  70 */
  NULL, /*  71 */
  NULL, /*  72 */
  NULL, /*  73 */
  NULL, /*  74 */
  NULL, /*  75 */
  NULL, /*  76 */
  NULL, /*  77 */
  NULL, /*  78 */
  NULL, /*  79 */
  NULL, /*  80 */
  NULL, /*  81 */
  NULL, /*  82 */
  NULL, /*  83 */
  NULL, /*  84 */
  gds85,
  gds86,
  gds87,
  NULL, /*  88 */
  NULL, /*  89 */
  gds90,
  gds91,
  gds92,
  gds93,
  gds94,
  gds95,
  gds96,
  gds97,
  gds98,
  NULL, /*  99 */
  gds100,
  gds101,
  NULL, /* 102 */
  gds103,
  gds104,
  gds105,
  gds106,
  gds107,
  NULL, /* 108 */
  NULL, /* 109 */
  NULL, /* 110 */
  NULL, /* 111 */
  NULL, /* 112 */
  NULL, /* 113 */
  NULL, /* 114 */
  NULL, /* 115 */
  NULL, /* 116 */
  NULL, /* 117 */
  NULL, /* 118 */
  NULL, /* 119 */
  NULL, /* 120 */
  NULL, /* 121 */
  NULL, /* 122 */
  NULL, /* 123 */
  NULL, /* 124 */
  NULL, /* 125 */
  gds126,
  NULL, /* 127 */
  NULL, /* 128 */
  NULL, /* 129 */
  NULL, /* 130 */
  NULL, /* 131 */
  NULL, /* 132 */
  NULL, /* 133 */
  NULL, /* 134 */
  NULL, /* 135 */
  NULL, /* 136 */
  NULL, /* 137 */
  NULL, /* 138 */
  NULL, /* 139 */
  NULL, /* 140 */
  NULL, /* 141 */
  NULL, /* 142 */
  NULL, /* 143 */
  NULL, /* 144 */
  NULL, /* 145 */
  NULL, /* 146 */
  NULL, /* 147 */
  NULL, /* 148 */
  NULL, /* 149 */
  NULL, /* 150 */
  NULL, /* 151 */
  NULL, /* 152 */
  NULL, /* 153 */
  NULL, /* 154 */
  NULL, /* 155 */
  NULL, /* 156 */
  NULL, /* 157 */
  NULL, /* 158 */
  NULL, /* 159 */
  NULL, /* 160 */
  NULL, /* 161 */
  NULL, /* 162 */
  NULL, /* 163 */
  NULL, /* 164 */
  NULL, /* 165 */
  NULL, /* 166 */
  NULL, /* 167 */
  NULL, /* 168 */
  NULL, /* 169 */
  NULL, /* 170 */
  NULL, /* 171 */
  NULL, /* 172 */
  NULL, /* 173 */
  NULL, /* 174 */
  NULL, /* 175 */
  NULL, /* 176 */
  NULL, /* 177 */
  NULL, /* 178 */
  NULL, /* 179 */
  NULL, /* 180 */
  NULL, /* 181 */
  NULL, /* 182 */
  NULL, /* 183 */
  NULL, /* 184 */
  NULL, /* 185 */
  NULL, /* 186 */
  NULL, /* 187 */
  NULL, /* 188 */
  NULL, /* 189 */
  NULL, /* 190 */
  NULL, /* 191 */
  gds192,
  NULL, /* 193 */
  NULL, /* 194 */
  NULL, /* 195 */
  NULL, /* 196 */
  NULL, /* 197 */
  NULL, /* 198 */
  NULL, /* 199 */
  NULL, /* 200 */
  gds201,
  gds202,
  gds203,
  gds204,
  gds205,
  gds206,
  gds207,
  gds208,
  gds209,
  gds210,
  gds211,
  gds212,
  gds213,
  gds214,
  gds215,
  NULL, /* 216 */
  NULL, /* 217 */
  NULL, /* 218 */
  NULL, /* 219 */
  NULL, /* 220 */
  NULL, /* 221 */
  NULL, /* 222 */
  NULL, /* 223 */
  NULL, /* 224 */
  NULL, /* 225 */
  NULL, /* 226 */
  NULL, /* 227 */
  NULL, /* 228 */
  NULL, /* 229 */
  NULL, /* 230 */
  NULL, /* 231 */
  NULL, /* 232 */
  NULL, /* 233 */
  NULL, /* 234 */
  NULL, /* 235 */
  NULL, /* 236 */
  NULL, /* 237 */
  NULL, /* 238 */
  NULL, /* 239 */
  NULL, /* 240 */
  NULL, /* 241 */
  NULL, /* 242 */
  NULL, /* 243 */
  NULL, /* 244 */
  NULL, /* 245 */
  NULL, /* 246 */
  NULL, /* 247 */
  NULL, /* 248 */
  NULL, /* 249 */
  NULL, /* 250 */
  NULL, /* 251 */
  NULL, /* 252 */
  NULL, /* 253 */
  NULL, /* 254 */
  NULL, /* 255 */
 };

unsigned char *NCEP_GDS(unsigned char *pds, int grid_type) {


    if (std_ncep_gds[grid_type] == NULL) {
        set_PDSGridType(pds, grid_type);
        clr_HasGDS(pds);
        return NULL;
    }
    set_PDSGridType(pds, grid_type);
    set_HasGDS(pds);
    return (cpGRIBsec(std_ncep_gds[grid_type]));
}

/* w. ebisuzaki
 *
 * creates a new copy of a 24section
 *
 */

unsigned char *cpGRIBsec(unsigned char *section) {

    int len;
    unsigned char *newsection;

    if (section == NULL) return NULL;

    len = _LEN24(section);
    if ((newsection = (unsigned char *) malloc(len)) == NULL) {
	fprintf(stderr,"ran out of memory in cp_section\n");
	exit(8);
    }

    memcpy(newsection, section, len);
    return newsection;
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

void set_Center(unsigned char *pds, int center) {
    pds[4] = center;
}
int get_Center(unsigned char *pds) {
    return pds[4];
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

void set_PDSGridType(unsigned char *pds, int n) {
    pds[6] = n;
}
int get_PDSGridTYPE (unsigned char *pds) {
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

