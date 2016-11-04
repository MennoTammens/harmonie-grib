/*
 * translation of tojul
 * VMS-fortran compatible
 *
 * converts month, day, year to days after
 * January 0, 1945
 */

#include <stdio.h>


static int monthjday[12] = {
	0,31,59,90,120,151,181,212,243,273,304,334};

int tojul(int month, int day, int year) {
	int jday, year1;

	jday = monthjday[month-1] + (year-1945)*365 + (year-1945)/4 + day;
	if (month > 2 && (year % 4) == 0) jday++;
	return jday;
}


/*
 * converts from the # of days after Jan 0, 1945
 * day, month, year
 */

#define  YEARS4  (4*365+1)
#define  YEAR	 365
#define  FEB29	 (31+29)

void unjul(int julday, int *month, int *day, int *years) {

	int absday, year, i;

	absday = julday - 1;	/* absday=0 -> Jan 1, 1945 */

	year = 1945 + 4*(absday / YEARS4);
	/* year is now correct to within 3 years */
	absday = (absday % YEARS4);

	i = absday / YEAR;
	/* i is the year increment 0..3 */
	/* i can be for if dec 31 of a leap year ! */
	if (i == 4) i = 3;

	year += i;
	absday = absday - (YEAR*i) + 1;
	/* absday is # of day in the year starting from 1 */
	if ((year % 4) == 0 && absday == FEB29) {
		*month = 2;
		*day = 29;
	}
	else {
		if ((year % 4) == 0 && absday > FEB29) absday--;
		for (i=11; monthjday[i] >= absday; --i);
		*month = i+1;
		*day = absday - monthjday[i];
	}
	*years = year;
}
