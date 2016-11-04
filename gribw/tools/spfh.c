#include <stdio.h>
#include <math.h>

/*
       convert temp, relhum -> specific humidity

       temp (kelvin)
       relhum (0..100)
       press (mb)
       specific humidity (kg/kg)
 */

#define RLRV	(2.5E6 / 4.6150E2)


double spfh(float temp, float relhum, float press) {

	double cons, es, eofp, w;

	if (relhum <= 0.0) return 0.0;

        cons = 6.1078 * exp(RLRV / 273.16);

/*
C       ES, EOFP, PRESS MUST BE IN THE SAME UNITS
C       ES = SATURATION VAPOR PRESSURE
C       EOFP = VAPOR PRESSURE
C       W = MIXING RATIO
 */

        es = cons / exp(RLRV/temp);
        eofp=relhum*es/100.0;
        w = 0.622 * eofp / (press - 0.378 * eofp);
        
	/* for kg/kg */
        return (w / (1.0 + w));
}
        

