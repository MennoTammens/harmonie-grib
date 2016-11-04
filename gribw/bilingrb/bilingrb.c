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
 * bilingrb -- A GRIB Interpolator
 *
 * Shrinks & interpolates lat-lon GRIB-records to the desired geographic 
 * subdomain using bilinear interpolation. Handles grids periodic in longitude.
 * 
 * Do not use on directional data. Be careful with interpolation of vector
 * components near the poles.
 *
 * If taint is left out, a node needs 4 wet neighbours in old grid to be defined as wet.
 * This can be set with taint to numbers from 1 to 4. If taint is set to 4, all points 
 * in new grid are defined as wet (no undefs exist), although they will certainly be 
 * land points.
 *
 * Compile: make -f bilingrb.make
 * 
 * Usage: type bilingrb without arguments
 * 
 * Requires: libgribw, angles.h
 *
 * Oyvind.Breivik@met.no
 *
 * 2006-05-24, v1.4: GDS bug fix.


 This file is part of bilingrb and is distributed under terms of the GNU General Public License
 For details see, Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 Boston, MA  02110-1301  USA

24-6-2016 Oyvind.Breivik

 */

/* Constants */
#define USAGE "Usage: %s in.grb out.grb westlon southlat dlon dlat nlon nlat [taint]\n"
#define VERSION "Version: %s v1.4, 2006-05-24, Oyvind.Breivik@met.no\n"
#define HELP "\nShrinks & interpolates lat-lon GRIB-records to the desired geographic\n\
 subdomain using bilinear interpolation and subsampling.\n\
\n\
 Handles grids periodic in longitude.\n\
\n\
 westlon, southlat: lower-left corner of new grid\n\
 dlon, dlat [deg]:  desired grid resolution\n\
 nx, ny:            grid size\n\
 [taint]:           accepted number of dry neighbours [default 0]\n\
\n\
 If [taint] is left out, a node needs 4 wet neighbours in old grid to be defined\n\
 as wet.\n\
 taint can range from 0 (all neighbours must be wet) to 4 (all can be dry).\n\
\n\
 Warning: Do not use on directional data.\n\
 Warning: Be careful with interpolation of vector components near the poles.\n\
\n\
 If new grid matches or is an integer multiple of old grid, a subsample\n\
 without interpolation is performed.\n"
#define MDEG 1000.0
#define N 100000
#define TOL 0.001

int main(int argc, char **argv) {

   /* Function prototype */
   int gribindex(int, int, int, int, int);
   float wneighbour(float a, float weight, int *undefs);

   /* Vars */
   unsigned char *pds, *gds, *bms, *bds, *gdsnew;
   float *arr, *brr; /* tmp work arrays */

   int nrec=0;     /* record counter */
   int nxny;       /* grid dimension */

   long int len_grib, pos=0; /* GRIB file position counters */

   int nx, ny, nx2, ny2;     /* grid dimensions, old and new */
   int i, j, k, l, m, n;     /* indices, 1D and 2D */
   int idir, jdir, iadj, undefs, cyclic; /* logical vars */
   int taint=0; /* number of dry neighbours allowed */

   float dlon, dlat;                               /* from old GDS */
   float dlon2, dlat2;                             /* new grid resolution */
   float wlon, elon, slat, nlat;                   /* old grid boundaries */
   float wlon2, elon2, slat2, nlat2;               /* new grid boundaries */
   float firstlon2, lastlon2, firstlat2, lastlat2; /* for new GDS */
   float lon, lat, lonSW, latSW;

   float pi;      /* yes, good old PI */
   float u, v, w; /* bilinear weights */

   unsigned char mode; /* resolution and mode flags, GDS 17 */
   int scan;           /* GRIB record scan direction, GDS 28 */

   FILE *input, *output;

   /* Initialize */
   pi = 4*atan(1.0);

   /* Optional: number of dry neighbours allowed */
   if (argc == 10) {
      taint = atof(argv[9]);
   }

   /* Help line */
   if (argc == 1) {
      fprintf(stderr, USAGE, argv[0]);
      fprintf(stderr, VERSION, argv[0]);
      fprintf(stderr, HELP);
      exit(8);
   }

   /* Open files */
   if (argc < 9) {
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

   /* Preallocate BIG arrays for speed */
   arr = (float *) malloc(N * sizeof(float));
   brr = (float *) malloc(N * sizeof(float));
   
   /* Loop over input records */
   while ((len_grib = rd_grib_msg(input, pos, &pds, &gds, &bms, &bds)) > 0) {

      nrec++; /* count records */

      /* New grid dims reset for every record from user input */
      wlon2 = (float) atof(argv[3]);   /* western boundary of new grid */
      slat2 = (float) atof(argv[4]);   /* southern boundary */
      dlon2 = (float) atof(argv[5]);   /* east dir grid resolution */
      dlat2 = (float) atof(argv[6]);   /* north res */
      nx2 = atoi(argv[7]);             /* east dir grid size */
      ny2 = atoi(argv[8]);             /* north size */

      /* Derived grid parameters */
      elon2 = ang360(wlon2+(nx2-1)*dlon2); /* eastern boundary */
      nlat2 = slat2+(ny2-1)*dlat2;         /* northern boundary */

      /* Reallocate arrays if necessary */
      nxny = get_nxny(pds, gds, bms, bds);
      if (nxny > N) {
         free(arr);
         arr =  (float *) malloc((nxny) * sizeof(float));
         if (arr == NULL) {
            fprintf(stderr,"Memory allocation failed\n");
         }
      } /* end if nxny */

      if (nx2*ny2 > N) {
         free(brr);
         brr = (float *) malloc((nx2*ny2) * sizeof(float));
         if (brr == NULL) {
            fprintf(stderr,"Memory allocation failed\n");
         }
      } /* end if nxny */

      /* Unpack data */
      unpk_bds(arr, pds, gds, bms, bds, nxny);

      /* Is grid in geographic coordinates? If not, bitch and quit */
      if (!GDS_LatLon(gds)) {
         fprintf(stderr,\
         "Error: GRIB rec no %4d not in geographic coordinates (lat-lon)\n",\
          nrec);
         exit(6);
      } 

      /* Old grid parameters */
      scan = GDS_LatLon_scan(gds);
      nx = GDS_LatLon_nx(gds);
      ny = GDS_LatLon_ny(gds);
      dlon = GDS_LatLon_dx(gds)/MDEG;
      dlat = GDS_LatLon_dy(gds)/MDEG;

      /* Grid scan direction */
      idir = ((scan&128)==128);
      idir = 1-idir;          /* West to east is "true" */
      jdir = ((scan&64)==64); /* South to north is "true" */
      iadj = ((scan&32)==32);
      iadj = 1-iadj;          /* Zonal points adjacent is "true" */

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

      /* Is old grid cyclic zonally? */
      cyclic = (abs(ang180(elon+dlon-wlon)) < TOL);

      /* Crop the new grid to the confines of the old grid, but warn */
      if (!cyclic) {
         /* Western lon outside grid? */
         if (ang360(wlon2-wlon) > ang360(elon-wlon)) {
            wlon2=wlon; /* Set new western bound to old grid bound and warn */
            fprintf(stderr,"Error: western boundary exceeds original grid "
            "domain in record no %d\n",nrec);
            exit(5);
         }
         /* Eastern lon outside grid? */
         if (ang360(elon2-wlon) > ang360(elon-wlon)) {
            elon2=elon;   /* Set new eastern bound to old grid bound and warn */
            fprintf(stderr,"Error: eastern boundary exceeds original grid "
            "domain in record no %d\n",nrec);
            exit(5);
         }
         /* Eastern and western lons swapped round? */
         if (ang360(wlon2-wlon) > ang360(elon2-wlon)) {
            elon2 = elon; /* Retain eastern part of grid and bitch */
            fprintf(stderr,"Error: boundary crossed in non-periodic grid "
            "record no %d ... retaining eastern part of grid\n",nrec);
            exit(5);
         }
      } /* end if cyclic */

      if (slat2 < slat) {
         slat2 = slat; /* Set new southern bound to old grid bound and warn */
         fprintf(stderr,"Error: southern boundary exceeds original grid "
         "domain in record no %d\n",nrec);
         exit(5);
      }
      if (nlat2 > nlat) {
         nlat2 = nlat; /* Set new northern bound to old grid bound and warn */
         fprintf(stderr,"Error: northern boundary exceeds original grid "
         "domain in record no %d\n",nrec);
         exit(5);
      }

      /* Loop over new grid */
      for (m=1; m<=nx2; m++) {    /* lons */
         for (n=1; n<=ny2; n++) { /* lats */

            /* 1D index in new grid */
            l = gribindex(m,n,nx2,ny2,scan);

            /* Lon & lat of node in new grid */
            lon = ang360(wlon2+(m-1)*dlon2);
            lat = slat2 + (n-1)*dlat2;

            /* Indices of SW nearest node in old grid */
            i = (int) (ang360((lon-wlon))/dlon) + 1;
            j = (int) ((lat-slat)/dlat) + 1;

            /* Lon & lat of SW neighbour in old grid */
            lonSW = ang360(wlon + (i-1)*dlon);
            latSW = slat + (j-1)*dlat;

            /* Bilinear weights, corrected for latitude */
            u = cos(lat*pi/180.0)*ang360(lon-lonSW)/dlon;
            v = (lat-latSW)/dlat;

            /* Bilinear interpolation, undefined nodes accounted for */
            undefs = 4;
            brr[l] = 0.0;
            
            k = gribindex(i,j,nx,ny,scan);   /* SW neighbour in old grid */
            w = (1.0-u)*(1.0-v);
            brr[l] += wneighbour(arr[k],w,&undefs);

            k = gribindex(i,j+1,nx,ny,scan); /* North-west */
            w = (1.0-u)*v;
            brr[l] += wneighbour(arr[k],w,&undefs);

            /* Eastern neighbours, use modulo in case of cyclic grid */
            i = i%nx+1;

            k = gribindex(i,j,nx,ny,scan);   /* South-east */
            w = u*(1.0-v);
            brr[l] += wneighbour(arr[k],w,&undefs);

            k = gribindex(i,j+1,nx,ny,scan); /* North-east */
            w = u*v;
            brr[l] += wneighbour(arr[k],w,&undefs);

            /* One undef neighbour in old grid is enough to taint new grid node 
             * and make it undefined if taint is not set on command line */
            if ( undefs > taint ) {
               brr[l] = UNDEFINED;
            }

         } /* end for n, north index */
      } /* end for m, east index */

      /* Cosmetics: make sure longitudes lie in [-180,180) */
      wlon2 = ang180(wlon2);
      elon2 = ang180(elon2);

      /* Cosmetics: change e.g. range 150:-150 deg to 150:210 deg */
      if ( (ang360(wlon2)<180.0) && (ang360(elon2)>=180.0) ) {
        elon2 = ang360(elon2);
      }

      /* Cosmetics: last longitude should not be -180 */
      if (elon2 == -180.0) {
         elon2 = 180.0;
      }

      /* Compute new grid boundaries in lat-lon */
      if (idir) {
         firstlon2 = wlon2;  /* grid starts on western rim (default) */
         lastlon2 = elon2;
      }
      else {
         firstlon2 = elon2;
         lastlon2 = wlon2;
      }
         
      if (jdir) {
         firstlat2 = slat2;
         lastlat2 = nlat2;
      }
      else {
         firstlat2 = nlat2;  /* grid starts on northern rim (default) */
         lastlat2 = slat2;
      }

      /* Remake grid definition section, GDS */
      mode = gds[16];
      if ((gdsnew = malloc(GDS_LEN(gds))) == NULL) {
         fprintf(stderr,"malloc failure gdsnew\n");
         exit(8);
      }

      /* Copy old GDS */
      for (i=0; i<GDS_LEN(gds); i++) gdsnew[i] = gds[i];
      /* Change some */
      gds = new_LatLon_GDS(pds,nx2,ny2,firstlon2,firstlat2,lastlon2,
                           lastlat2,dlon2,dlat2);
      /* And copy again */
      for (i=6; i<GDS_LEN(gds); i++) gdsnew[i] = gds[i];
      gdsnew[16] = mode; /* Restore the mode flags */

      /* Remake bitmap section, BMS */
      nxny = nx2*ny2;
      bms = mk_BMS(pds, brr, &nxny, UNDEFINED_LOW, UNDEFINED_HIGH);

      /* Remake binary data section, BDS */
      if (BDS_BinScale(bds)) {               /* ECMWF style? */
         set_BDSMinBits(BDS_NumBits(bds));
         set_BDSMaxBits(BDS_NumBits(bds));
      }
      bds = mk_BDS(pds, brr, nxny);

      /* Write new GRIB record */
      wrt_grib_msg(output, pds, gdsnew, bms, bds);

      /* Clean up */
      if (gds != NULL) {
        free(gds);
        gds = NULL;
      }
      if (gdsnew != NULL) {
        free(gdsnew);
        gdsnew = NULL;
      }
      if (bms != NULL) {
        free(bms);
        bms = NULL;
      }
      if (bds != NULL) {
        free(bds);
        bds = NULL;
      }

      pos += len_grib; /* file position */
   } /* end for nrec */

   /* Close files */
   fclose(input);
   fclose(output);

   /* Clean up */
   if (arr != NULL) {
     free(arr);
     arr = NULL;
   }
   if (brr != NULL) {
     free(brr);
     brr = NULL;
   }

   /* Output */
   if (nrec > 1) {
      printf("Wrote %d records\n", nrec);
   }
   else if (nrec == 1) {
      printf("Wrote one record\n");
   }
   else {
      fprintf(stderr,"ERROR: no records written\n");
      exit(4);
   } /* end if */

   return 0;
} /* end main */


int gribindex(int i, int j, int nx, int ny, int scan)
{
   /* Compute 1D GRIB record index from two independent grid indices (east and
    * north indices) */

   int k;
   int ii, jj, idir, jdir, iadj;


   /* Grid orientation, GDS 28 */
   idir = ((scan&128)==128);
   idir = 1-idir;          /* west to east */
   jdir = ((scan&64)==64); /* south to north */
   iadj = ((scan&32)==32);
   iadj = 1-iadj;          /* zonal points adjacent */

   /* Positive direction index i? */
   ii = i;
   if (!idir) {
      ii = nx-i+1;     /* not default */
   }

   /* Positive direction index j? */
   jj = j;
   if (!jdir) {
      jj = ny-j+1;     /* default */
   }

   /* Index i adjacent (grid running east-west)? */
   if (iadj) {
      jj = (jj-1)*nx;  /* default */
   }
   else {
      ii = (ii-1)*ny;  /* index j adjacent, running north-south */
   }

   k = ii+jj-1;

   return k;
} /* end function gribindex */

float wneighbour(float a, float weight, int *undefs)
{
   /* Return weighted neighbour */
   float b=0.0;

   /* Exclude undef'd neighbours from weighting */
   if (a < UNDEFINED_LOW) {
      b = a*weight; /* neighbour in old grid */
   }

   /* Do not taint node if undef'd neighbour's weight is negligible */
   if ( (a < UNDEFINED_LOW) || (fabs(weight) < TOL) ) (*undefs)--;

   return b;
} /* end function wneighbour */
