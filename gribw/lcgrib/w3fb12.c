/*C SUBPROGRAM:  W3FB12        LAMBERT(I,J) TO LAT/LON FOR GRIB
C   PRGMMR: STACKPOLE        ORG: NMC42       DATE:88-11-28
C
C ABSTRACT: CONVERTS THE COORDINATES OF A LOCATION ON EARTH GIVEN IN A
C   GRID COORDINATE SYSTEM OVERLAID ON A LAMBERT CONFORMAL TANGENT
C   CONE PROJECTION TRUE AT A GIVEN N OR S LATITUDE TO THE
C   NATURAL COORDINATE SYSTEM OF LATITUDE/LONGITUDE
C   W3FB12 IS THE REVERSE OF W3FB11.
C   USES GRIB SPECIFICATION OF THE LOCATION OF THE GRID
C
C PROGRAM HISTORY LOG:
C   88-11-25  ORIGINAL AUTHOR:  STACKPOLE, W/NMC42
C   90-04-12  R.E.JONES   CONVERT TO CFT77 FORTRAN
C   94-04-28  R.E.JONES   ADD SAVE STATEMENT
*/

#include "stdio.h"
#include "math.h"

#define RERTH 6.3712E+6
#define PI 3.14159

void w3fb12(double xi, double xj, double alat1, double elon1, double dx, 
    double elonv, double alatan, double *alat, double *elon, int *ierr)
{
	static int newmap=0;
	double h, piby2, radpd, degprd, rebydx, alatn1, an, cosltn;
	double elon1l, elonvr, ala1, rmll, elo1, arg;
	double xx, yy, r2, theta, beta;

	double static polei, polej, aninv, aninv2, thing, oldrml = -9.99e20;

        h = (alatan > 0) ? 1.0 : -1.0;

         piby2  = PI / 2.0;
         radpd  = PI  / 180.0;
         degprd = 1.0 / radpd;
         rebydx = RERTH / dx;
         alatn1 = alatan * radpd;
         an     = h * sin(alatn1);
         cosltn = cos(alatn1);

/*
C        MAKE SURE THAT INPUT LONGITUDE DOES NOT PASS THROUGH
C        THE CUT ZONE (FORBIDDEN TERRITORY) OF THE FLAT MAP
C        AS MEASURED FROM THE VERTICAL (REFERENCE) LONGITUDE
*/
         elon1l = elon1;
         if ((elon1 - elonv) > 180.)     elon1l -= 360.;
         if ((elon1 - elonv) < -180.)    elon1l += 360.;

         elonvr = elonv * radpd;
/*
C        RADIUS TO LOWER LEFT HAND (LL) CORNER
*/
         ala1 =  alat1 * radpd;
         rmll = rebydx * pow(cosltn, (1.-an))*pow((1.+an), an) *
                pow( cos(ala1)/(1.+h*sin(ala1)), an)/an;
/*
C        USE RMLL TO TEST IF MAP AND GRID UNCHANGED FROM PREVIOUS
C        CALL TO THIS CODE.  THUS AVOID UNNEEDED RECOMPUTATIONS.
*/
         if (rmll == oldrml)
           newmap = 1;
         else
         {
           newmap = 0;
           oldrml = rmll;
/*
C          USE LL POINT INFO TO LOCATE POLE POINT
*/
           elo1 = elon1l * radpd;
           arg = an * (elo1-elonvr);
           polei = 1. - h * rmll * sin(arg);
           polej = 1. + rmll * cos(arg);
	 }
/*
C        RADIUS TO THE I,J POINT (IN GRID UNITS)
C              YY REVERSED SO POSITIVE IS DOWN
*/
         xx = xi - polei;
         yy = polej - xj;
         r2 = xx*xx + yy*yy;
/*
C        CHECK THAT THE REQUESTED I,J IS NOT IN THE FORBIDDEN ZONE
C           YY MUST BE POSITIVE UP FOR THIS TEST
*/
         theta = PI*(1.-an);
         beta = fabs(atan2(xx,-yy));
         *ierr = 0;
         if (beta <= theta)
	 {
           *ierr = 1;
           *alat = 999.;
           *elon = 999.;
           return;
	  }
/*
C        NOW THE MAGIC FORMULAE
*/
         if (r2 == 0) 
         {
           *alat = h * 90.0;
           *elon = elonv;
	  }
         else
         {
/*
C          FIRST THE LONGITUDE
*/
           *elon = elonv + degprd * atan2(h*xx,yy)/an;
           *elon = fmod(*elon+360., 360.);
/*
C          NOW THE LATITUDE
C          RECALCULATE THE THING ONLY IF MAP IS NEW SINCE LAST TIME
*/
           if (newmap == 0)
           {
             aninv = 1./an;
             aninv2 = aninv/2.;
             thing = pow(an/rebydx, aninv)/
	        (pow(cosltn, (1.-an)*aninv)*(1.+ an));
	    }
             *alat = h*(piby2 - 2.*atan(thing*pow(r2, aninv2)))*degprd;
           }

         return;
}

#ifdef MAIN_NEEDED
main()
{ 
    double *alat, *elon, *oldrml;
    double alat1=0., elon1=0.;
    double dx=70000.;
    double elonv=0.;
    double alatan=45.;
/*    double xi=20., xj=20.;
    double xi=86.90, xj=140.67;
*/
    double xi=74.9579, xj=120.596;
    double al=140, el=120., oldrm=99999.;
    int *ierr;

    alat = &al;
    elon = &el;
    oldrml = &oldrm;

    printf("In main before w3fb12 \n");
    printf("xi=%f, xj=%f\n",xi, xj);

    w3fb12(xi,xj,alat1,elon1, dx, elonv, alatan, alat, elon, *ierr, oldrml);

    printf("In main after w3fb12 \n");
    printf("alat=%f, elon=%f\n",*alat,*elon);

}
#endif
