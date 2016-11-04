#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "gribw.h"
#include "mk_pds.h"

/* necessary */
#define P_PARAM_TABLE(x)	p_param_table,x
#define P_CENTER(cntr,subcntr)	p_center,cntr,p_subcenter,subcntr
#define P_PROCESS(process)	p_process,process

/* optional */
#define P_GRID(grid)		p_grid,grid
#define P_GDS(has_gds)		p_gds,has_gds
#define P_BMS(has_bms)		p_bms,has_bms


/* either or */
#define P_PARAM(var)		p_param,var
#define P_KPDS5(var)		p_param,var

/* Either A, B or C */

/* A */
#define P_L_TYPE(x)		p_l_type,x
#define P_LEVEL(level)		p_level,level

/* B */
#define P_KPDS6(x)		p_l_type,x
#define P_KPDS7(x)		p_level,x

/* C */
#define P_MB(prs_level)		p_l_type,100,p_level,prs_level
#define P_SURFACE()		p_l_type,1,p_level,0
#define P_TROPOPAUSE()		p_l_type,7,p_level,0
#define P_TOA()			p_l_type,8,p_level,0
#define P_MSL()			p_l_type,102,p_level,0
#define P_COLUMN()		p_l_type,200,p_level,0

/* this */
#define P_DATE(yymmddhh)	p_date,yymmddhh

/* or this */
#define P_YEAR(year)		p_year,year
#define P_MONTH(month)		p_month,month
#define P_DAY(day)		p_day,day
#define P_HOUR(hour)		p_hour,hour
#define P_MINUTE(minute)	p_minute,minute

/* time units */

#define MINUTE			0
#define HOUR			1
#define DAY			2
#define MONTH			3
#define YEAR			4
#define DECADE			5
#define NORMAL			6
#define CENTURY			7
#define SECOND			254


/* this */
#define P_ANALYSIS()		p_t_range,0,p_t_p1,0,p_t_p2,0,p_t_unit,HOUR,p_n_ave,0,p_missing,0
#define P_FORECAST(n,unit)	p_t_range,10,p_t_p1,((n)/256),p_t_p2,((n)%256),p_t_unit,unit
#define P_AVERAGE(from,to,unit)
#define P_ACCUM(from,to,unit)


#define P_END()			p_end

enum f_pds {p_end, p_param_table, p_center, p_process, p_grid, p_GDS, p_BMS,
     p_param, p_l_type, p_level, 
     p_year, p_month, p_day, p_hour, p_minute, p_date, 
     p_t_unit, p_t_p1, p_t_p2, p_t_range,
     p_n_ave, p_missing,
     p_subcenter,p_bytes};


unsigned char *pds_tool(char *old_pds, ...) {

	unsigned char pds[MAX_LEN_PDS], *pds2;
	enum {undefined, defined, default_val, copied}  pds_def[MAX_LEN_PDS];


	va_list ap;
        enum f_pds type;
	int i, pds_len = 28, debug = 1;

	va_start(ap, old_pds);

	if (old_pds == NULL) {
	    for (i = 0 ; i < MAX_LEN_PDS; i++) {
		pds[i] = 0;
		pds_def[i] = undefined;
	    }
	    pds[0] = pds[1] = 0; pds[2] = pds_len;
	    pds_def[0] = pds_def[1] = pds_def[2] = default_val;
	}
	else {
	    pds_len = (pds[0] << 16) + (pds[1] << 8) + pds[2];
	    for (i = 0 ; i < pds_len; i++) {
		pds[i] = old_pds[i];
		pds_def[i] = copied;
	    }
	}
        

	while ((type = va_arg(ap, enum f_pds)) != p_end) {
	    i = va_arg(ap, int);

	    switch(type) {

		case p_param_table:
			if (debug) printf("param table %d\n", i);
			pds[3] = i;
			pds_def[3] = defined;
			break;

		case p_center:
			if (debug) printf("center %d\n", i);
			pds[4] = i;
			pds_def[4] = defined;
			break;

		case p_process:
			if (debug) printf("process %d\n", i);
			pds[5] = i;
			pds_def[5] = defined;
			break;

		case p_grid:
			if (debug) printf("grid %d\n", i);
			pds[6] = i;
			pds_def[6] = defined;
			break;

		case p_GDS:
			if (debug) printf("gds %d\n", i);
			pds[7] = (pds[7] & 127) | (i ? 128 : 0);
			break;

		case p_BMS:
			if (debug) printf("gds %d\n", i);
			pds[7] = (pds[7] & 0xbf) | (i ? 64 : 0);
			break;

		case p_param:
			if (debug) printf("param %d\n", i);
			pds[8] = i;
			pds_def[8] = defined;
			break;

		case p_l_type:
			if (debug) printf("l_type %d\n", i);
			pds[9] = i;
			pds_def[9] = defined;
			break;

		case p_level:
			if (debug) printf("level %d\n", i);
			pds[10] = i / 256;
			pds[11] = i % 256;
			pds_def[10] = defined;
			pds_def[11] = defined;
			break;

		case p_year: 
			if (debug) printf("year %d\n", i);
			pds[12] = i % 100;
			pds_def[12] = defined;
			pds[24] = (i - 1) / 100 + 1;
			pds_def[24] = defined;
			break;

		case p_month: 
			if (debug) printf("month %d\n", i);
			pds[13] = i;
			pds_def[13] = defined;
			break;

		case p_day: 
			if (debug) printf("day %d\n", i);
			pds[14] = i;
			pds_def[14] = defined;
			break;

		case p_hour: 
			if (debug) printf("hour %d\n", i);
			pds[15] = i;
			pds_def[15] = defined;
			break;

		case p_minute: 
			if (debug) printf("hour %d\n", i);
			pds[16] = i;
			pds_def[16] = defined;
			break;

		case p_date:
			if (debug) printf("date %d\n", i);
			/* minute */
			pds[16] = 0;
			pds_def[16] = defined;
			/* hour */
			pds[15] = i % 100;
			pds_def[15] = defined;
			i = i / 100;
			/* day */
			pds[14] = i % 100;
			pds_def[14] = defined;
			/* month */
			pds[13] = i % 100;
			pds_def[13] = defined;
			i = i / 100;
			/* year */
			pds[12] = i % 100;
			pds_def[12] = defined;
			pds[24] = (i - 1) / 100 + 1;
			pds_def[24] = defined;
			break;

		case p_t_unit:
			if (debug) printf("forecast time units %d\n", i);
			pds[17] = i;
			pds_def[17] = defined;
			break;

		case p_t_p1:
			if (debug) printf("forecast p1 %d\n", i);
			pds[18] = i;
			pds_def[18] = defined;
			break;

		case p_t_p2:
			if (debug) printf("forecast p2 %d\n", i);
			pds[19] = i;
			pds_def[19] = defined;
			break;

		case p_t_range:
			if (debug) printf("forecast time range %d\n", i);
			pds[20] = i;
			pds_def[20] = defined;
			break;

		case p_n_ave:
			if (debug) printf("n ave %d\n", i);
			pds[21] = i / 256;
			pds[22] = i % 256;
			pds_def[21] = defined;
			pds_def[22] = defined;
			break;

		case p_missing:
			if (debug) printf("missing %d\n", i);
			pds[23] = i;
			pds_def[23] = defined;
			break;

		case p_subcenter:
			if (debug) printf("subcenter %d\n", i);
			pds[25] = i;
			pds_def[25] = defined;
			break;

		default:
			printf("undefined argument:mk_pds\n");
	    		printf("%d %d\n", type, i);
			break;
	    }
	}
	va_end(ap);

	/* set default values */

	/* parameter table */
	if (pds_def[3] == undefined) {
	    pds[3] = DEFAULT_PARAMETER_TABLE;
	    pds_def[3] = default_val;
	}
	/* center */
	if (pds_def[4] == undefined) {
	    pds[4] = DEFAULT_CENTER;
	    pds_def[4] = default_val;
	}
	/* subcenter */
	if (pds_def[25] == undefined) {
	    pds[25] = DEFAULT_SUBCENTER;
	    pds_def[25] = default_val;
	}
	/* process */
	if (pds_def[5] == undefined) {
	    pds[5] = DEFAULT_PROCESS;
	    pds_def[5] = default_val;
	}
	/* kpds5, kpds6, kpds7 */
	if (pds_def[8] == undefined) {
	    pds[8] = DEFAULT_PARAM;
	    pds_def[8] = default_val;
	}
	if (pds_def[9] == undefined) {
	    pds[9] = DEFAULT_L_TYPE;
	    pds_def[9] = default_val;
	}
	if (pds_def[10] == undefined) {
	    pds[10] = DEFAULT_LEVEL1;
	    pds_def[10] = default_val;
	}
	if (pds_def[11] == undefined) {
	    pds[11] = DEFAULT_LEVEL2;
	    pds_def[11] = default_val;
	}

	/* time range etc */
	if (pds_def[17] == undefined) {
	    pds[17] = DEFAULT_TIME_UNIT;
	    pds_def[17] = default_val;
	}
	if (pds_def[18] == undefined) {
	    pds[18] = DEFAULT_TIME_P1;
	    pds_def[18] = default_val;
	}
	if (pds_def[19] == undefined) {
	    pds[19] = DEFAULT_TIME_P2;
	    pds_def[19] = default_val;
	}
	if (pds_def[20] == undefined) {
	    pds[20] = DEFAULT_TIME_RANGE;
	    pds_def[20] = default_val;
	}

	if (debug) {
	    for (i = 3; i < pds_len; i++) {
		if (i != 7 && i != 22 && i != 26 && i != 27) {
		    if (pds_def[i] == undefined) {
		        fprintf(stderr,"PDS octet %d is undefined\n", i+1);
		    }
		    if (pds_def[i] == default_val) {
		        fprintf(stderr,"PDS octet %d has default value\n", i+1);
		    }
		}
	    }
	}

	if (old_pds != NULL) {
		pds2 = old_pds;
	}
	else {
		if ((pds2 = (unsigned char *) malloc(pds_len)) == NULL) {
	    		fprintf(stderr,"memory allocation error: mk_pds\n");
	    		exit(9);
		}
	}
	for (i = 0; i < pds_len; i++) {
	    pds2[i] = pds[i];
	}
	return pds2;
}

void main(void) {
    unsigned char *pds, *gds, *bms, *bds;
    int i, nxny;
    float array[144*73];
    FILE *output;

    if ((output = fopen("output.grb","wb")) == NULL) {
        fprintf(stderr,"could not open file: output.grb\n");
        exit(7);
    }

    for (i = 0; i < 144*73; i++) {
	array[i] = i;
    }

    pds = pds_tool(NULL,P_DATE(1996123106),P_CENTER(7,0),P_PARAM(7),
      P_PROCESS(62),P_MB(500), P_ANALYSIS(), p_end);

    gds = NCEP_GDS(pds,2);

    nxny = 144*73;
    bms = mk_BMS(pds, array, &nxny, 1.0, -1.0);
    bds = mk_BDS(pds, array, nxny);
    wrt_grib_msg(output, pds, gds, bms, bds);
    free(bds);
    if (bms) free(bms);
    free(gds);
    free(pds);
}
