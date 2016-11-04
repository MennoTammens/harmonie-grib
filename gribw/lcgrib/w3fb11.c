/*C SUBPROGRAM:  W3FB11        LAT/LON TO LAMBERT(I,J) FOR GRIB
C   PRGMMR: STACKPOLE        ORG: NMC42       DATE:88-11-28
C
C ABSTRACT: CONVERTS THE COORDINATES OF A LOCATION ON EARTH GIVEN IN
C   THE NATURAL COORDINATE SYSTEM OF LATITUDE/LONGITUDE TO A GRID
C   COORDINATE SYSTEM OVERLAID ON A LAMBERT CONFORMAL TANGENT CONE
C   PROJECTION TRUE AT A GIVEN N OR S LATITUDE. W3FB11 IS THE REVERSE
C   OF W3FB12. USES GRIB SPECIFICATION OF THE LOCATION OF THE GRID
C*/

#include <stdio.h>
#include <math.h>

#define RERTH 6.3712E+6
#define PI 3.14159

void w3fb11(double alat, double elon, double alat1, double elon1, 
    double dx, double elonv, double alatan, double *xi, double *xj)
{
	double radpd, rebydx, alatn1, an, cosltn, elon1l;
	double ala1, rmll, elo1, arg, polei, polej, ala, rm, elo;
	double elonl, elonvr;

	double h;

	h = (alatan > 0) ? 1 : -1;

	radpd = PI / 180.0;
	rebydx = RERTH / dx;
	alatn1 = alatan * radpd;
	an     = h * sin(alatn1);
	cosltn = cos(alatn1);
 
/*       MAKE SURE THAT INPUT LONGITUDES DO NOT PASS THROUGH
         THE CUT ZONE (FORBIDDEN TERRITORY) OF THE FLAT MAP
         AS MEASURED FROM THE VERTICAL (REFERENCE) LONGITUDE.
*/
	 elon1l = elon1;
	 if ((elon1 - elonv) > 180.)     elon1l = elon1 - 360.;
	 if ((elon1 - elonv) < -180.)    elon1l = elon1 + 360.;
 
	 elonl = elon;
	 if ((elon  - elonv) > 180.)     elonl = elon - 360.;
	 if ((elon - elonv) < -180.)     elonl = elon + 360.;
 
	 elonvr = elonv * radpd;
/*
C        RADIUS TO LOWER LEFT HAND (LL) CORNER
*/
	 ala1 =  alat1 * radpd;
	 rmll = rebydx * pow(cosltn, 1.-an) * pow(1.+an,an) *
		pow(cos(ala1)/(1.+h*sin(ala1)), an) / an;
/*
C        USE LL POINT INFO TO LOCATE POLE POINT
*/
	 elo1 = elon1l * radpd;
	 arg = an * (elo1-elonvr);
       
	 polei = 1. - h * rmll * sin(arg);
	 polej = 1. + rmll * cos(arg);
/*
C        RADIUS TO DESIRED POINT AND THE I J TOO
*/
	 ala =  alat * radpd;
	 rm = rebydx * pow(cosltn, (1.-an))*pow((1.+an), an) *
	      pow(cos(ala)/(1.+h*sin(ala)), an)/an;
 
	 elo = elonl * radpd;
	 arg = an*(elo-elonvr);
	 *xi = polei + h * rm * sin(arg);
	 *xj = polej - rm * cos(arg);

      return;
}
