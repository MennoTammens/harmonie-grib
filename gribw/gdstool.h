#define _GDSTOOL_

/*
 * 3/29/2001 fix in new_polar_GDS by Gerry Wiener
 * 6/3/2001 change roundoff, suggested by Steven Danz
 * 10/29/2002 Gerry Wiener: lambert-conformal and C++ compatibility
 */


/* round vs trunctation */

#include <math.h>
#ifndef RINT
#define RINT(a)	((int) floor((a)+0.5))
#endif

#ifdef __cplusplus
 extern "C" {
#endif


unsigned char *GDStool(unsigned char *pds, unsigned char *gds, ...);

enum g_tool {g_end, g_init, g_byte, g_2bytes, g_s2bytes,
   g_3bytes, g_s3bytes, g_bit, g_and, g_or};           

/* choose one set */

/*
   for fortran grid: real data(nlon,nlat)

   nlat/nlon = number of lat/lon rows/columns
   lat1/lon1 = lat/lon of data(1,1)
   lat2/lon2 = lat/lon of data(nlon,nlat)

   for S or W, use negative value

   dlon, dlat = degrees of lon/lat of grid interval

 */

#define new_LatLon_GDS(pds,nlon,nlat,lon1,lat1,lon2,lat2,dlon,dlat) \
	GDStool(pds,NULL,g_init,32,0,g_2bytes,6,nlon,g_2bytes,8,nlat, \
	g_s3bytes,10,RINT(1000.0*(lat1)),g_s3bytes,13,RINT(1000.0*(lon1)), \
	g_s3bytes,17,RINT(1000.0*(lat2)),g_s3bytes,20,RINT(1000.0*(lon2)), \
	g_s2bytes,23,RINT(1000.0*(dlon)),g_s2bytes,25,RINT(1000.0*(dlat)), \
	g_or, 27, ((lat1) < (lat2)) * 64, \
	g_byte,16,128,g_byte,4,255,g_end)

/*
   For gaussian grid(nlon,nlat)

   nlon/nlat = number of lon/lat points
   lon1,lat1 = coordinates of grid(1,1)

  */

#define new_Gaussian_GDS(pds,nlon,nlat,lon1,lat1) \
	GDStool(pds,NULL,g_init,32,0,g_2bytes,6,nlon,g_2bytes,8,nlat, \
	g_s3bytes,10,RINT(1000.0*(lat1)), \
	g_s3bytes,13,RINT(1000.0*(lon1)), \
	g_s3bytes,17,RINT(-1000.0*(lat1)), \
        g_s3bytes,20,RINT(1000.0*((lon1)+360.0-360.0/(nlon))), \
	g_s2bytes,23,RINT(1000.0*360.0 / nlon),g_s2bytes,25, nlat / 2, \
	g_or, 27, ((lat1) < 0.0) * 64, \
	g_byte,16,128,g_byte,4,255,g_byte,5,4,g_end)
	
/*
 * make a polar stereographic GDS
 *
 * (nx,ny) = grid dimensions
 * (lon1,lat1) = lon/lat of first grid point (deg)
 * lov = the orientation of the grids
 * (dx,dy) = grid length (km) at 60N or 60S
 * south_pole = 1 for south polar stereographics, 0 for nps
 * scan_s2n = 1 if grid is written from south to north else 0
 */

#define new_polar_GDS(pds,nx,ny,lon1,lat1,lov,dx,dy,south_pole,scan_s2n) \
        GDStool(pds,NULL,g_init,32,0,g_byte,5,5,g_byte,4,255,\
	g_2bytes,6,nx,g_2bytes,8,ny,g_s3bytes,10,(int) (1000.0*(lat1)),\
	g_s3bytes,13,RINT(1000.0*(lon1)),g_byte,16,128,\
	g_s3bytes,17,RINT(1000.0*(lov)),g_s3bytes,20,(int) (1000.0*(dx)),\
	g_s3bytes,23,RINT(1000.0*(dy)),g_byte,26,south_pole*128,\
	g_byte,27,scan_s2n*64,g_end)


/*
 * make a Lambert GDS
 *
 * (nx,ny) = grid dimensions
 * (lat1,lon1) = lat/lon of first grid point (deg)
 * lov = the orientation of the grid, i.e., the east longitude value of the
 *   meridian which is parallel to the y-axis (or columns of the grid) along
 *   which latitude increases as the y-coordinate increases. (Note: The
 *   orientation longitude may, or may not, appear within a particular grid.)J
 *   (deg)
 * (dx,dy) = Grid lengths are in (km), at the intersection latitude
 *   circle nearest to the pole in the projection plane.
 * proj_center =
 *   0 North pole on projection plane and one projection center
 *   128 South pole on projection plane and one projection center
 *   64 North pole on projection plane and projection is bipolar and symmetric
 *   192 South pole on projection plane and projection is bipolar and symmetric
 * scan_s2n = 1 if grid is written from south to north else 0
 *   Grid is assumed to be written from left to right along x axis
 * latin1 = The first latitude from the pole at which the secant cone cuts
 *   the spherical earth. (deg)
 * latin2 = The second latitude from the pole at which the secant cone cuts
 *   the spherical earth. (deg)
 * splat = Latitude of southern pole (deg)
 * splon = Longitude of southern pole (deg)
 */


#define new_lambert_GDS(pds,nx,ny,lat1,lon1,lov,dx,dy,proj_center,scan_s2n,latin1, \
        latin2, splat, splon) \
        GDStool(pds,NULL,g_init,42,0,g_byte,5,3,g_byte,4,255,\
        g_2bytes,6,nx,g_2bytes,8,ny,g_s3bytes,10,RINT(1000.0*(lat1)),\
        g_s3bytes,13,RINT(1000.0*(lon1)),g_byte,16,128,\
        g_s3bytes,17,RINT(1000.0*(lov)),g_s3bytes,20,RINT(1000.0*(dx)),\
        g_s3bytes,23,RINT(1000.0*(dy)),g_byte,26,proj_center,\
        g_byte,27,scan_s2n*64, g_s3bytes, 28, RINT(1000.0*(latin1)), \
        g_s3bytes, 31, RINT(1000.0*(latin2)), \
        g_s3bytes, 34, RINT(1000.0*(splat)), \
        g_s3bytes, 37, RINT(1000.0*(splon)), g_end)

#ifdef __cplusplus
}
#endif

