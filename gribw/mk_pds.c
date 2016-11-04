#define PDSm_end		-1

#define PDSm_ParamTable		1
#define PDSm_Center		2
#define PDSm_Process		3
#define PDSm_Grid		4

/* flags */
#define PDSm_HasGDS		5
#define PDSm_HasBMS		6
#define PDSm_Subcenter		50

/* optional */
#define PDSm_KPDS5		7
#define PDSm_KPDS6		8
#define PDSm_KPDS7		9
#define PDSm_Year		10
#define PDSm_Month		11
#define PDSm_Day		12
#define PDSm_Hour		13
#define PDSm_Minute		14
#define PDSm_FcstTimeUnit	15
#define PDSm_P1			16
#define PDSm_P2			17
#define PDSm_TimeRange		18
#define PDSm_NAve		19
#define PDSm_DecScale		22

/* only for making PDS's greater than 28 bytes */

#define PDSm_Length		23


#include <stdio.h>
#include <stdarg.h>


