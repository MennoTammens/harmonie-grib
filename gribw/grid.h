/* v0.9 grid.h     w. ebisuzaki
 *
 * for decoding grid information
 */

enum {LatLon, Gaussian, Polar, Lambert, Mercator, SpaceView, SpHarmonic,
      undecoded} GridType;

int GDS_info(int pds_grid, unsigned char *gds, int *nx, int *ny, int *nxny);


