#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <float.h>
#include "bds.h"
#include "gdstool.h"
#include "gds.h"
#include "gribw.h"
#include "angles.h"

/*
 * ggrib -- Shrinks lat-lon GRIB files to selected geographic subdomain
 *
 *  Compile: make -f ggrib.make
 *  Usage: type ggrib without arguments
 *  Requires: libgribw
 *
 *  Oyvind.Breivik@met.no
 *    minor performance mods WNE 3/04
 *    calculate dx dy rather than use values in grib file, fixes a problem
 *     where dx = 8.33333333 km whereas the GDS had rounded to 8.3 km  WNE 12/06

This file is part of ggrib and is distributed under terms of the GNU General Public License
 For details see, Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 Boston, MA  02110-1301  USA

24-6-2016 Oyvind.Breivik

 */

/* Constants */
#define VERSION "ggrib v2.9beta 2006-12-15, Oyvind.Breivik@met.no"
#define USAGE "Usage: %s in.grb out.grb westlon southlat eastlon northlat\n"
#define MDEG 1000.0

/* save memory .. will allocate to needed size later */
#define N 1
#define TOL 0.001
#define IOBUFSIZ	(8 * 8192)

int main(int argc, char **argv) {

   unsigned char *pds, *gds, *bms, *bds;
   float *arr, *arr2; /* tmp work arrays */

   int nrec=0, nskip=0, nwrit; /* record counters */
   int nxny;        /* grid dimensions */
   int nxny2;       /* grid dimensions */
   int arr_size=N; /* default arr dimension */
   int arr_size2=N; /* default arr dimension */

   long int len_grib, pos=0;

   int nx, ny, nx2, ny2;                 /* grid dimensions, old and new */
   int i, j, k, l, m, n, i1, j1, i2, j2, ii, jj, mm, nn, scan; /* indices */
   int idir, jdir, iadj, cyclic, global; /* logical vars */

   float lon1, lon2, lat1, lat2;
   float dlon, dlat;                               /* from orig GDS */
   float calc_dlon, calc_dlat;                     /* calculated from end points */
   float firstlon2, lastlon2, firstlat2, lastlat2; /* for new GDS */
   float wlon, elon, slat, nlat;                   /* orig grid boundaries */
   float wlon2, elon2, slat2, nlat2;               /* new grid boundaries */

   unsigned char mode; /* GDS octet 17 */

   FILE *input, *output;

   /* Help line */
   if (argc == 1) {
      fprintf(stderr, "%s\n", VERSION);
      fprintf(stderr,"Shrinks lon/lat GRIB records to the domain "
      "(westlon southlat eastlon northlat)\n");
      fprintf(stderr, USAGE, argv[0]);
      fprintf(stderr,"  Bounding coords can be left out by typing a dot (.), ");
      fprintf(stderr,"for example:\n\n    ggrib in.grb out.grb -25 . . 78\n\n");
      fprintf(stderr,"  will produce a grid from 25 E and southern rim of orig "
      "grid to eastern rim of\n  orig grid and 78 N.\n");
      exit(8);
   }

   /* Open files */
   if (argc < 7) {
      fprintf(stderr, USAGE, argv[0]);
      exit(8);
   }
   if ((input = fopen(argv[1],"rb")) == NULL) {
      fprintf(stderr,"Could not open file: %s\n", argv[1]); 
      exit(7);                                              
   }                                                         
   if ((output = fopen(argv[2],"wb")) == NULL) {             
      fprintf(stderr,"Could not open file: %s\n", argv[2]); 
      exit(7);                                              
   }                                                         

   /* increase size of i/o buffers for speed WNE */
   setvbuf(input, NULL, _IOFBF, IOBUFSIZ/2);
   setvbuf(output, NULL, _IOFBF, IOBUFSIZ*2);

   /* Preallocate BIG arrays for speed */
   arr = (float *) malloc(N * sizeof(float));
   arr2 = (float *) malloc(N * sizeof(float));
   
   /* Loop over input records */
   while ((len_grib = rd_grib_msg(input, pos, &pds, &gds, &bms, &bds)) > 0) {

      nrec++; /* count records */

      /* Is grid in geographic coordinates? */
      if (!GDS_LatLon(gds)) {
         fprintf(stderr, "ERROR: Grid #%4d not in geographic coordinates (lon,lat), exiting\n", nrec);
         exit(9);
      } 

      /* Original grid parameters */
      scan = GDS_LatLon_scan(gds);
      nx = GDS_LatLon_nx(gds);
      ny = GDS_LatLon_ny(gds);
      dlon = GDS_LatLon_dx(gds)/MDEG;
      dlat = GDS_LatLon_dy(gds)/MDEG;

      /* Grid orientation */
      idir = ((scan & 128) == 0);	/* West to east is "true" */
      jdir = ((scan & 64) == 64);	/* South to north is "true" */
      iadj = ((scan & 32) == 0);	/* Zonal points adjacent is "true" */

      /* Reallocate arrays if nxny > N */
      nxny = get_nxny(pds, gds, bms, bds);
      if (nxny > arr_size) {
         free(arr);
         arr_size = nxny;
         if ((arr =  (float *) malloc(arr_size * sizeof(float))) == NULL) {
            fprintf(stderr,"Memory allocation failed\n");
            exit(8);
         }
      }

      /* Unpack data, get_unpk_bds doesn't seem to work properly */
      unpk_bds(arr, pds, gds, bms, bds, nxny);

      if (idir) {
         wlon = GDS_LatLon_Lo1(gds)/MDEG; /* grid running W to E (default)? */
         elon = GDS_LatLon_Lo2(gds)/MDEG;
      }
      else {
         elon = GDS_LatLon_Lo1(gds)/MDEG; /* or E to W? */
         wlon = GDS_LatLon_Lo2(gds)/MDEG;
      }
         
      if (jdir) {
         slat = GDS_LatLon_La1(gds)/MDEG; /* grid running S to N? */
         nlat = GDS_LatLon_La2(gds)/MDEG;
      }
      else {
         nlat = GDS_LatLon_La1(gds)/MDEG; /* or N to S (default)? */
         slat = GDS_LatLon_La2(gds)/MDEG;
      }

      if (nlat < slat) {
         fprintf(stderr,"Error: scan direction mismatches with south and "
         "north grid boundaries, scan=%d\n",scan);
         exit(5);
      }

      /* high precision dlat/dlon */
      calc_dlat = (nlat - slat) / (ny - 1.0);
      if (dlat < 0) calc_dlat = -calc_dlat;

      calc_dlon = (elon - wlon);
      if (calc_dlon <= 0.0) calc_dlon += 360.0;
      if (fabs(dlon) * (nx-1) > 360.0) calc_dlon += 360.0;
      calc_dlon /= (nx - 1.0);
      if (dlon < 0.0) calc_dlon = -calc_dlon;

      if (abs(dlat - calc_dlat) > 0.0006) {
          fprintf(stderr,"problem with grib file dlat is not consistent %lf %lf\n",dlat, calc_dlat);
      }
      else {
          dlat = calc_dlat;
      }
      if (abs(dlon - calc_dlon) > 0.0006) {
          fprintf(stderr,"problem with grib file dlon is not consistent %lf %lf\n", dlon, calc_dlon);
      }
      else {
          dlon = calc_dlon;
      }

      /* User input */
      if (argv[3][0]=='.') {
         lon1=wlon;
      }
      else {
         lon1 = (float) atof(argv[3]); /* western boundary */
      }

      if (argv[4][0]=='.') {
         lat1=slat;
      }
      else {
         lat1 = (float) atof(argv[4]); /* southern boundary */
      }
      if (argv[5][0]=='.') {
         lon2=elon;
      }
      else {
         lon2 = (float) atof(argv[5]); /* eastern boundary */
      }
      if (argv[6][0]=='.') {
         lat2=nlat;
      }
      else {
         lat2 = (float) atof(argv[6]); /* northern boundary */
      }

      /* Grid around the world in longitude? */
      cyclic = (abs(ang180(elon+dlon-wlon)) < TOL);

      /* Confine boundaries to grid domain */
      if (!cyclic) {
         /* Western lon outside grid? */
         if (ang360(lon1-wlon) > ang360(elon-wlon)) {
            lon1=wlon;
         }
         /* Eastern lon outside grid? */
         if (ang360(lon2-wlon) > ang360(elon-wlon)) {
            lon2=elon;
         }
         /* Eastern and western lons swapped round? */
         if (ang360(lon1-wlon) > ang360(lon2-wlon)) {
            lon2 = elon; /* Retain eastern part of grid and bitch */
            fprintf(stderr,"Warning: boundary crossed in non-periodic grid "
            "record no %d ... retaining eastern part of grid\n", nrec);
         }
      } /* end if cyclic */
      wlon2 = lon1;
      elon2 = lon2;

      slat2 = lat1 < slat ? slat : lat1;
      nlat2 = lat2 > nlat ? nlat : lat2;

      /* Find bounding grid indices */
      i1 = (int) (ang360(wlon2-wlon)/dlon + 1.5); /* round index to nearest */
      i2 = (int) (ang360(elon2-wlon)/dlon + 1.5);
      j1 = (int) ((slat2-slat)/dlat + 1.5);
      j2 = (int) ((nlat2-slat)/dlat + 1.5);

      /* Find new grid dimensions */
      nx2 = (int) (ang360(elon2-wlon2)/dlon + 1.5);
      ny2 = j2-j1+1;

      /* Make new grid global in lon if lon1==lon2 */
      global = ((nx2==1) && cyclic);
      if (global) {
         nx2 = nx;
         i2 = i1>1 ? i1-1 : nx2; /* i2 = i1-1 with wrap-around */
      } 
      else {
         nx2 = i2-i1+1;
         if (nx2 < 1) {
            nx2 = nx+nx2; /* cyclic grids are tricky bastards */
         }
      }

      /* Final check on new grid dimensions */
      if ( (nx2<=0) || (ny2<=0) ) {
         fprintf(stderr, "Warning: New grid undefined (outside old grid), nlon=%d, nlat=%d, skipping record no %d\n", nx2, ny2, nrec);
         pos += len_grib; /* update file position */
         nskip++;
         continue;        /* then skip to next record */
         exit(6);
      }
      nxny2 = nx2*ny2;

      /* allocate output grid */
      if (nxny2 > arr_size2) {
         free(arr2);
         arr_size2 = nxny2;
         if ((arr2 = (float *) malloc(arr_size2 * sizeof(float))) == NULL) {
            fprintf(stderr,"Memory allocation failed\n");
            exit(8);
         }
      }

      /* Adjust boundaries to nearest exact grid points */
      wlon2 = ang360(wlon+((float)(i1-1))*dlon); /* find exact grid longitude */
      elon2 = ang360(wlon+((float)(i2-1))*dlon); 
      slat2 = slat+((float)(j1-1))*dlat;
      nlat2 = slat+((float)(j2-1))*dlat;

      /* Shrink grid */
      for (m=1; m<=nx2; m++) {    /* lons */

         ii = (i1+m-2)%nx + 1;  /* wrap around if grid is periodic in lon*/
         mm = m;
         if (!idir) {
            ii = nx-ii+1;     /* not default */
            mm = nx2-m+1;
         }
         /* Index i adjacent (grid running east-west)? */
         if (!iadj) {
               ii = (ii-1)*ny;  /* index j adjacent, running north-south */
               mm = (mm-1)*ny2;
         }


         for (n=1; n<=ny2; n++) { /* lats */
            jj = j1+n-1;           /* non-periodic in lat */

            /* Positive direction index j? */
            nn = n;
            if (!jdir) {
               jj = ny-jj+1;     /* default */
               nn = ny2-n+1;
            }

            /* Index i adjacent (grid running east-west)? */
            if (iadj) {
               jj = (jj-1)*nx;  /* default */
               nn = (nn-1)*nx2;
            }

            arr2[mm+nn-1] = arr[ii+jj-1];
         } /* end for n */
      } /* end for m */

      /* Make sure longitudes lie in [-180,180) */
      wlon2 = ang180(wlon2);
      elon2 = ang180(elon2);

      /* Cosmetics: change e.g. range 150:-150 deg to read 150:210 deg */
      if ( (ang360(wlon2)<180.0) && (ang360(elon2)>=180.0) ) {
        elon2 = ang360(elon2);
      }

      /* Cosmetics: last longitude should not be -180 */
      if (elon2 == -180.0) {
         elon2 = 180.0;
      }

      /* Compute new grid boundaries in lon/lat */
      if (idir) {
         firstlon2 = wlon2;  /* grid starts on western rim (default) */
         lastlon2 = elon2;
      }
      else {
         firstlon2 = elon2;
         lastlon2 = wlon2;
      }
         
      if (jdir) {
         firstlat2 = slat2;  /* grid starts on southern rim */
         lastlat2 = nlat2;
      }
      else {
         firstlat2 = nlat2;  /* default */
         lastlat2 = slat2;
      }

      /* Remake grid definition section, GDS */
      mode = gds[16];
      gds = new_LatLon_GDS(pds,nx2,ny2,firstlon2,firstlat2,lastlon2,
                           lastlat2,dlon,dlat);
      gds[16] = mode; /* Restore the mode flags */

      /* Remake bitmap section, BMS if necessary */
      if (bms) {
	  bms = mk_BMS(pds, arr2, &nxny2, UNDEFINED_LOW, UNDEFINED_HIGH);
      }

      /* Remake binary data section, BDS */
      if (BDS_BinScale(bds)) { /* ECMWF style? */
         set_BDSMinBits(BDS_NumBits(bds));
         set_BDSMaxBits(BDS_NumBits(bds));
      }
      bds = mk_BDS(pds, arr2, nxny2);

      /* Write new GRIB record */
      wrt_grib_msg(output, pds, gds, bms, bds);

      /* free allocated memory */
      if (bms) free(bms);
      free(bds);
      free(gds);

      pos += len_grib; /* file position */
   } /* end for nrec */

   /* Close files */
   fclose(input);
   fclose(output);

   /* Output */
   nwrit = nrec-nskip;
   if (nwrit > 1) {
      printf("Wrote %d records", nwrit);
   }
   else if (nwrit == 1) {
      printf("Wrote one record");
   }
   else {
      printf("No records written");
   } /* end if */
   printf(", skipped %d\n", nskip);

   return 0;
} /* end main */
