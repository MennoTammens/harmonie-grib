c
c	program uses w3lib routines to write std_ncep_gds.c
c
c	uses w3fi71.f and w3fi74.f
c
c	std_ncep_gds.c is basically defines a table of the 
c	standard (ie not user defined) GDSs.  The index to
c	the table is PDS grid type.
c
c	v1.0 7/96 wesley ebisuzaki

        integer igrid, ierr
        integer igds(91)
        character*1 gds(200)

	write(*,5000)
5000	format('#include <stdio.h>',/,'#include <stddef.h>')
	write(*,5010)
5010	format('#include "gribw.h"',/,/)


	do 200 igrid = 0, 255
            call w3fi71(igrid, igds, ierr)
	    if (ierr.eq.0) then
		icomp = 0
		call w3fi74(igds, icomp, gds, lengds, npts, ierr)
            endif
	    if (ierr.eq.0) then
		if (igrid.le.9) then
		    write(*,7010) igrid, lengds
		else if (igrid.le.99) then
		    write(*,7020) igrid, lengds
		else 
		    write(*,7030) igrid, lengds
		endif
7010		format('static unsigned char gds',i1,'[',i3,'] = {')
7020		format('static unsigned char gds',i2,'[',i3,'] = {')
7030		format('static unsigned char gds',i3,'[',i3,'] = {')
		write(*,7000) (ichar(gds(i)),i=1,lengds)
7000		format(10(i3,', '))
		write(*,*) '};'
	    endif
200	continue

	write(*,*) ' '
	write(*,*) 'unsigned char *std_ncep_gds[256] = {'
	do 100 igrid = 0, 255
            call w3fi71(igrid, igds, ierr)
	    if (ierr.eq.0) then
		icomp = 0
		call w3fi74(igds, icomp, gds, lengds, npts, ierr)
            endif
	    if (ierr.eq.0) then
		if (igrid.le.9) then
		    write(*,7050) igrid
		else if (igrid.le.99) then
		    write(*,7060) igrid
		else
		    write(*,7070) igrid
		endif
7050		format('  gds',i1,',')
7060		format('  gds',i2,',')
7070		format('  gds',i3,',')
	    else
		write(*,7080) igrid
7080		format('  NULL, /* ', i3, ' */')
	    endif
100	continue
	write(*,*) '};'

	write(*,8000)
8000	format('unsigned char *NCEP_GDS(unsigned char *pds, ',
     1		'int grid_type) {')
	write(*,8010)
8010	format('    if (std_ncep_gds[grid_type] == NULL) {')
        write(*,8020)
8020	format('        set_PDSGridType(pds, grid_type);')
        write(*,8030)
8030	format('        clr_HasGDS(pds);')
        write(*,8040)
8040	format('        return NULL;')
        write(*,8050)
8050	format('    }')
	write(*,8060)
8060	format('    set_PDSGridType(pds, grid_type);')
	write(*,8070)
8070	format('    set_HasGDS(pds);')
	write(*,8090)
8090	format('    return (cpGRIBsec(std_ncep_gds[grid_type]));')
	write(*,8100)
8100	format('}')

	stop
	end


      SUBROUTINE W3FI71 (IGRID, IGDS, IERR)
C$$$  SUBPROGRAM DOCUMENTATION BLOCK
C                .      .    .                                       .
C SUBPROGRAM:    W3FI71      MAKE ARRAY USED BY GRIB PACKER FOR GDS
C   PRGMMR: R.E.JONES        ORG: W/NMC42    DATE: 93-03-26
C
C ABSTRACT: W3FI71 MAKES A 18, 37, 55, 64, OR 91 WORD INTEGER ARRAY
C     USED BY W3FI72 GRIB PACKER TO MAKE THE GRID DESCRIPTION SECTION
C     (GDS) - SECTION 2.
C
C PROGRAM HISTORY LOG:
C   92-02-21  R.E.JONES
C   92-07-01  M. FARLEY    ADDED REMARKS FOR 'IGDS' ARRAY ELEMENTS.
C                          ADDED LAMBERT CONFORMAL GRIDS AND ENLARGED
C                          IDGS ARRAY FROM 14 TO 18 WORDS.
C   92-10-03  R.E.JONES    ADDED CORRECTIONS TO AWIPS GRIB TABLES
C   92-10-16  R.E.JONES    ADD GAUSSIAN GRID 126 TO TABLES
C   92-10-18  R.E.JONES    CORRECTIONS TO LAMBERT CONFORMAL TABLES
C                          AND OTHER TABLES
C   92-10-19  R.E.JONES    ADD GAUSSIAN GRID  98 TO TABLES
C   93-01-25  R.E.JONES    ADD ON84 GRIDS 87, 106, 107 TO TABLES
C   93-03-10  R.E.JONES    ADD ON84 GRIDS 1, 55, 56 TO TABLES
C   93-03-26  R.E.JONES    ADD GRIB GRIDS 2, 3 TO TABLES
C   93-03-29  R.E.JONES    ADD SAVE STATEMENT
C   93-06-15  R.E.JONES    ADD GRIB GRIDS 37 TO 44 TO TABLES
C   93-09-29  R.E.JONES    GAUSSIAN GRID DOCUMENT NOT CORRECT,
C                          W3FI74 WILL BE CHANGED TO AGREE WITH
C                          IT. GAUSSIAN GRID 98 TABLE HAS WRONG
C                          VALUE.
C   93-10-12  R.E.JONES    CHANGES FOR ON388 REV. OCT 8,1993 FOR
C                          GRID 204, 208.
C   93-10-13  R.E.JONES    CORRECTION FOR GRIDS 37-44, BYTES 7-8,
C                          24-25 SET TO ALL BITS 1 FOR MISSING.
C   93-11-23  R.E.JONES    ADD GRIDS 90-93 FOR ETA MODEL
C                          ADD GRID 4 FOR 720*361 .5 DEG. GRID
C   94-04-12  R.E.JONES    CORRECTION FOR GRID 28
C   94-06-01  R.E.JONES    ADD GRID 45, 288*145 1.25 DEG. GRID
C   94-06-22  R.E.JONES    ADD GRIDS 94, 95 FOR ETA MODEL
C   95-04-11  R.E.JONES    ADD GRIDS 96, 97 FOR ETA MODEL
C   95-05-19  R.E.JONES    ADD FROM 20 KM ETA MODEL AWIPS GRID 215
C   95-10-19  R.E.JONES    ADD FROM 20 KM ETA MODEL ALASKA GRID 216
C   95-10-31  IREDELL      REMOVED SAVES AND PRINTS
C   96-05-08  IREDELL      CORRECT FIRST LATITUDE FOR GRIDS 27 AND 28
C   96-07-02  R.E.JONES    ADD FROM 10 KM ETA MODEL OLYMPIC GRID 218
C   96-07-02  R.E.JONES    ADD 196 FOR ETA MODEL
C   96-08-15  R.E.JONES    ADD O.N. 84 GRID 8 AND 53 AS GRIB GRID 8
C                          AND 53
C   96-11-29  R.E.JONES    CORRECTION TO TABLES FOR GRID 21-26, 61-64
C   97-01-31  IREDELL      CORRECT FIRST LATITUDE FOR GRID 30
C   97-10-20  IREDELL      CORRECT LAST LONGITUDE FOR GRID 98
C   98-07-07  Gilbert      Add grids 217 and 219 through 235
C   98-09-21  BALDWIN      ADD GRIDS 190, 192 FOR ETA MODEL
C   99-01-20  BALDWIN      ADD GRIDS 236, 237
C   99-08-18  IREDELL      ADD GRID 170
C
C USAGE:    CALL W3FI71 (IGRID, IGDS, IERR)
C   INPUT ARGUMENT LIST:
C     IGRID       - GRIB GRID NUMBER, OR OFFICE NOTE 84 GRID NUMBER
C
C   OUTPUT ARGUMENT LIST:
C     IGDS      - 18, 37, 55, 64, OR 91 WORD INTEGER ARRAY WITH
C                 INFORMATION TO MAKE A GRIB GRID DESCRIPTION SECTION.
C     IERR       - 0  CORRECT EXIT
C                  1  GRID TYPE IN IGRID IS NOT IN TABLE
C
C REMARKS:
C    1) OFFICE NOTE GRID TYPE 26 IS 6 IN GRIB, 26 IS AN
C       INTERNATIONAL EXCHANGE GRID.
C
C    2) VALUES RETURNED IN 18, 37, 55, 64, OR 91 WORD INTEGER ARRAY
C        IGDS VARY DEPENDING ON GRID REPRESENTATION TYPE.
C
C       LAT/LON GRID:
C           IGDS( 1) = NUMBER OF VERTICAL COORDINATES
C           IGDS( 2) = PV, PL OR 255
C           IGDS( 3) = DATA REPRESENTATION TYPE (CODE TABLE 6)
C           IGDS( 4) = NO. OF POINTS ALONG A LATITUDE
C           IGDS( 5) = NO. OF POINTS ALONG A LONGITUDE MERIDIAN
C           IGDS( 6) = LATITUDE OF ORIGIN (SOUTH - IVE)
C           IGDS( 7) = LONGITUDE OF ORIGIN (WEST -IVE)
C           IGDS( 8) = RESOLUTION FLAG (CODE TABLE 7)
C           IGDS( 9) = LATITUDE OF EXTREME POINT (SOUTH - IVE)
C           IGDS(10) = LONGITUDE OF EXTREME POINT (WEST - IVE)
C           IGDS(11) = LATITUDE INCREMENT
C           IGDS(12) = LONGITUDE INCREMENT
C           IGDS(13) = SCANNING MODE FLAGS (CODE TABLE 8)
C           IGDS(14) = ... THROUGH ...
C           IGDS(18) =   ... NOT USED FOR THIS GRID
C           IGDS(19) - IGDS(91) FOR GRIDS 37-44, NUMBER OF POINTS
C                      IN EACH OF 73 ROWS.
C
C       GAUSSIAN GRID:
C           IGDS( 1) = ... THROUGH ...
C           IGDS(10) =   ... SAME AS LAT/LON GRID
C           IGDS(11) = NUMBER OF LATITUDE LINES BETWEEN A POLE
C                      AND THE EQUATOR
C           IGDS(12) = LONGITUDE INCREMENT
C           IGDS(13) = SCANNING MODE FLAGS (CODE TABLE 8)
C           IGDS(14) = ... THROUGH ...
C           IGDS(18) =   ... NOT USED FOR THIS GRID
C
C       SPHERICAL HARMONICS:
C           IGDS( 1) = NUMBER OF VERTICAL COORDINATES
C           IGDS( 2) = PV, PL OR 255
C           IGDS( 3) = DATA REPRESENTATION TYPE (CODE TABLE 6)
C           IGDS( 4) = J - PENTAGONAL RESOLUTION PARAMETER
C           IGDS( 5) = K - PENTAGONAL RESOLUTION PARAMETER
C           IGDS( 6) = M - PENTAGONAL RESOLUTION PARAMETER
C           IGDS( 7) = REPRESENTATION TYPE (CODE TABLE 9)
C           IGDS( 8) = REPRESENTATION MODE (CODE TABLE 10)
C           IGDS( 9) = ... THROUGH ...
C           IGDS(18) =   ... NOT USED FOR THIS GRID
C
C       POLAR STEREOGRAPHIC:
C           IGDS( 1) = NUMBER OF VERTICAL COORDINATES
C           IGDS( 2) = PV, PL OR 255
C           IGDS( 3) = DATA REPRESENTATION TYPE (CODE TABLE 6)
C           IGDS( 4) = NO. OF POINTS ALONG X-AXIS
C           IGDS( 5) = NO. OF POINTS ALONG Y-AXIS
C           IGDS( 6) = LATITUDE OF ORIGIN (SOUTH -IVE)
C           IGDS( 7) = LONGITUTE OF ORIGIN (WEST -IVE)
C           IGDS( 8) = RESOLUTION FLAG (CODE TABLE 7)
C           IGDS( 9) = LONGITUDE OF MERIDIAN PARALLEL TO Y-AXIS
C           IGDS(10) = X-DIRECTION GRID LENGTH (INCREMENT)
C           IGDS(11) = Y-DIRECTION GRID LENGTH (INCREMENT)
C           IGDS(12) = PROJECTION CENTER FLAG (0=NORTH POLE ON PLANE,
C                                              1=SOUTH POLE ON PLANE,
C           IGDS(13) = SCANNING MODE FLAGS (CODE TABLE 8)
C           IGDS(14) = ... THROUGH ...
C           IGDS(18) =   .. NOT USED FOR THIS GRID
C
C       MERCATOR:
C           IGDS( 1) = ... THROUGH ...
C           IGDS(12) =   ... SAME AS LAT/LON GRID
C           IGDS(13) = LATITUDE AT WHICH PROJECTION CYLINDER
C                        INTERSECTS EARTH
C           IGDS(14) = SCANNING MODE FLAGS
C           IGDS(15) = ... THROUGH ...
C           IGDS(18) =   .. NOT USED FOR THIS GRID
C
C       LAMBERT CONFORMAL:
C           IGDS( 1) = NUMBER OF VERTICAL COORDINATES
C           IGDS( 2) = PV, PL OR 255
C           IGDS( 3) = DATA REPRESENTATION TYPE (CODE TABLE 6)
C           IGDS( 4) = NO. OF POINTS ALONG X-AXIS
C           IGDS( 5) = NO. OF POINTS ALONG Y-AXIS
C           IGDS( 6) = LATITUDE OF ORIGIN (SOUTH -IVE)
C           IGDS( 7) = LONGITUTE OF ORIGIN (WEST -IVE)
C           IGDS( 8) = RESOLUTION FLAG (CODE TABLE 7)
C           IGDS( 9) = LONGITUDE OF MERIDIAN PARALLEL TO Y-AXIS
C           IGDS(10) = X-DIRECTION GRID LENGTH (INCREMENT)
C           IGDS(11) = Y-DIRECTION GRID LENGTH (INCREMENT)
C           IGDS(12) = PROJECTION CENTER FLAG (0=NORTH POLE ON PLANE,
C                                              1=SOUTH POLE ON PLANE,
C           IGDS(13) = SCANNING MODE FLAGS (CODE TABLE 8)
C           IGDS(14) = NOT USED
C           IGDS(15) = FIRST LATITUDE FROM THE POLE AT WHICH THE
C                      SECANT CONE CUTS THE SPERICAL EARTH
C           IGDS(16) = SECOND LATITUDE ...
C           IGDS(17) = LATITUDE OF SOUTH POLE (MILLIDEGREES)
C           IGDS(18) = LONGITUDE OF SOUTH POLE (MILLIDEGREES)
C
C       ARAKAWA SEMI-STAGGERED E-GRID ON ROTATED LAT/LON GRID
C           IGDS( 1) = NUMBER OF VERTICAL COORDINATES
C           IGDS( 2) = PV, PL OR 255
C           IGDS( 3) = DATA REPRESENTATION TYPE (CODE TABLE 6) [201]
C           IGDS( 4) = NI  - TOTAL NUMBER OF ACTUAL DATA POINTS
C                            INCLUDED ON GRID
C           IGDS( 5) = NJ  - DUMMY SECOND DIMENSION; SET=1
C           IGDS( 6) = LA1 - LATITUDE  OF FIRST GRID POINT
C           IGDS( 7) = LO1 - LONGITUDE OF FIRST GRID POINT
C           IGDS( 8) = RESOLUTION AND COMPONENT FLAG (CODE TABLE 7)
C           IGDS( 9) = LA2 - NUMBER OF MASS POINTS ALONG
C                            SOUTHERNMOST ROW OF GRID
C           IGDS(10) = LO2 - NUMBER OF ROWS IN EACH COLUMN
C           IGDS(11) = DI  - LONGITUDINAL DIRECTION INCREMENT
C           IGDS(12) = DJ  - LATITUDINAL  DIRECTION INCREMENT
C           IGDS(13) = SCANNING MODE FLAGS (CODE TABLE 8)
C           IGDS(14) = ... THROUGH ...
C           IGDS(18) = ... NOT USED FOR THIS GRID (SET TO ZERO)
C
C       ARAKAWA FILLED E-GRID ON ROTATED LAT/LON GRID
C           IGDS( 1) = NUMBER OF VERTICAL COORDINATES
C           IGDS( 2) = PV, PL OR 255
C           IGDS( 3) = DATA REPRESENTATION TYPE (CODE TABLE 6) [202]
C           IGDS( 4) = NI  - TOTAL NUMBER OF ACTUAL DATA POINTS
C                            INCLUDED ON GRID
C           IGDS( 5) = NJ  - DUMMY SECOND DIMENTION; SET=1
C           IGDS( 6) = LA1 - LATITUDE LATITUDE OF FIRST GRID POINT
C           IGDS( 7) = LO1 - LONGITUDE OF FIRST GRID POINT
C           IGDS( 8) = RESOLUTION AND COMPONENT FLAG (CODE TABLE 7)
C           IGDS( 9) = LA2 - NUMBER OF (ZONAL) POINTS IN EACH ROW
C           IGDS(10) = LO2 - NUMBER OF (MERIDIONAL) POINTS IN EACH
C                            COLUMN
C           IGDS(11) = DI  - LONGITUDINAL DIRECTION INCREMENT
C           IGDS(12) = DJ  - LATITUDINAL  DIRECTION INCREMENT
C           IGDS(13) = SCANNING MODE FLAGS (CODE TABLE 8)
C           IGDS(14) = ... THROUGH ...
C           IGDS(18) = ... NOT USED FOR THIS GRID
C
C       ARAKAWA STAGGERED E-GRID ON ROTATED LAT/LON GRID
C           IGDS( 1) = NUMBER OF VERTICAL COORDINATES
C           IGDS( 2) = PV, PL OR 255
C           IGDS( 3) = DATA REPRESENTATION TYPE (CODE TABLE 6) [203]
C           IGDS( 4) = NI  - NUMBER OF DATA POINTS IN EACH ROW
C           IGDS( 5) = NJ  - NUMBER OF ROWS
C           IGDS( 6) = LA1 - LATITUDE OF FIRST GRID POINT
C           IGDS( 7) = LO1 - LONGITUDE OF FIRST GRID POINT
C           IGDS( 8) = RESOLUTION AND COMPONENT FLAG (CODE TABLE 7)
C           IGDS( 9) = LA2 - CENTRAL LATITUDE
C           IGDS(10) = LO2 - CENTRAL LONGTITUDE
C           IGDS(11) = DI  - LONGITUDINAL DIRECTION INCREMENT
C           IGDS(12) = DJ  - LATITUDINAL  DIRECTION INCREMENT
C           IGDS(13) = SCANNING MODE FLAGS (CODE TABLE 8)
C           IGDS(14) = ... THROUGH ...
C           IGDS(18) = ... NOT USED FOR THIS GRID
C
C   SUBPROGRAM CAN BE CALLED FROM A MULTIPROCESSING ENVIRONMENT.
CC
C ATTRIBUTES:
C   LANGUAGE: SiliconGraphics 3.5 FORTRAN 77
C   MACHINE:  SiliconGraphics IRIS-4D/25, 35, INDIGO, Indy
C   LANGUAGE: IBM VS FORTRAN, CRAY CFT77 FORTRAN
C   MACHINE:  HDS, CRAY C916-128, Y-MP8/864, CRAY Y-MP EL92/256
C
C$$$
C
      INTEGER       IGRID
      INTEGER       IGDS  (*)
      INTEGER       GRD1  (18)
      INTEGER       GRD2  (18)
      INTEGER       GRD3  (18)
      INTEGER       GRD4  (18)
      INTEGER       GRD5  (18)
      INTEGER       GRD6  (18)
      INTEGER       GRD8  (18)
      INTEGER       GRD21 (55)
      INTEGER       GRD22 (55)
      INTEGER       GRD23 (55)
      INTEGER       GRD24 (55)
      INTEGER       GRD25 (37)
      INTEGER       GRD26 (37)
      INTEGER       GRD27 (18)
      INTEGER       GRD28 (18)
      INTEGER       GRD29 (18)
      INTEGER       GRD30 (18)
      INTEGER       GRD33 (18)
      INTEGER       GRD34 (18)
      INTEGER       GRD37 (91)
      INTEGER       GRD38 (91)
      INTEGER       GRD39 (91)
      INTEGER       GRD40 (91)
      INTEGER       GRD41 (91)
      INTEGER       GRD42 (91)
      INTEGER       GRD43 (91)
      INTEGER       GRD44 (91)
      INTEGER       GRD45 (18)
C     INTEGER       GRD50 (18)
      INTEGER       GRD53 (18)
      INTEGER       GRD55 (18)
      INTEGER       GRD56 (18)
      INTEGER       GRD61 (64)
      INTEGER       GRD62 (64)
      INTEGER       GRD63 (64)
      INTEGER       GRD64 (64)
      INTEGER       GRD85 (18)
      INTEGER       GRD86 (18)
      INTEGER       GRD87 (18)
      INTEGER       GRD90 (18)
      INTEGER       GRD91 (18)
      INTEGER       GRD92 (18)
      INTEGER       GRD93 (18)
      INTEGER       GRD94 (18)
      INTEGER       GRD95 (18)
      INTEGER       GRD96 (18)
      INTEGER       GRD97 (18)
      INTEGER       GRD98 (18)
      INTEGER       GRD100(18)
      INTEGER       GRD101(18)
      INTEGER       GRD103(18)
      INTEGER       GRD104(18)
      INTEGER       GRD105(18)
      INTEGER       GRD106(18)
      INTEGER       GRD107(18)
      INTEGER       GRD126(18)
      INTEGER       GRD170(18)
      INTEGER       GRD190(18)
      INTEGER       GRD192(18)
      INTEGER       GRD196(18)
      INTEGER       GRD201(18)
      INTEGER       GRD202(18)
      INTEGER       GRD203(18)
      INTEGER       GRD204(18)
      INTEGER       GRD205(18)
      INTEGER       GRD206(18)
      INTEGER       GRD207(18)
      INTEGER       GRD208(18)
      INTEGER       GRD209(18)
      INTEGER       GRD210(18)
      INTEGER       GRD211(18)
      INTEGER       GRD212(18)
      INTEGER       GRD213(18)
      INTEGER       GRD214(18)
      INTEGER       GRD215(18)
      INTEGER       GRD216(18)
      INTEGER       GRD217(18)
      INTEGER       GRD218(18)
      INTEGER       GRD219(18)
      INTEGER       GRD220(18)
      INTEGER       GRD221(18)
      INTEGER       GRD222(18)
      INTEGER       GRD223(18)
      INTEGER       GRD224(18)
      INTEGER       GRD225(18)
      INTEGER       GRD226(18)
      INTEGER       GRD227(18)
      INTEGER       GRD228(18)
      INTEGER       GRD229(18)
      INTEGER       GRD230(18)
      INTEGER       GRD231(18)
      INTEGER       GRD232(18)
      INTEGER       GRD233(18)
      INTEGER       GRD234(18)
      INTEGER       GRD235(18)
      INTEGER       GRD236(18)
      INTEGER       GRD237(18)
C
      DATA  GRD1  / 0, 255, 1,  73, 23, -48090,       0, 128,   48090,
     &       0, 513669,513669, 22500, 64, 0, 0, 0, 0/
      DATA  GRD2  / 0, 255, 0, 144, 73,  90000,       0, 128,  -90000,
     &   -2500,   2500, 2500,  0, 0, 0, 0, 0, 0/
      DATA  GRD3  / 0, 255, 0, 360,181,  90000,       0, 128,  -90000,
     &   -1000,   1000, 1000,  0, 0, 0, 0, 0, 0/
      DATA  GRD4  / 0, 255, 0, 720,361,  90000,       0, 128,  -90000,
     &    -500,    500,  500,  0, 0, 0, 0, 0, 0/
      DATA  GRD5  / 0, 255, 5,  53, 57,   7647, -133443,   8, -105000,
     &  190500, 190500, 0, 64, 0, 0, 0, 0, 0/
      DATA  GRD6  / 0, 255, 5,  53, 45,   7647, -133443,   8, -105000,
     &  190500, 190500, 0, 64, 0, 0, 0, 0, 0/
      DATA  GRD8  / 0, 255, 1, 116, 44, -48670,    3104, 128,   61050,
     &       0, 318830, 318830, 22500, 64, 0, 0, 0, 0/
      DATA  GRD21 / 0,  33, 0,65535,37,      0,       0, 128,   90000,
     &  180000,   2500, 5000, 64, 0, 0, 0, 0, 0,
     & 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37,
     & 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37,
     & 37, 37, 37, 37, 37, 37,  1/
      DATA  GRD22 / 0,  33, 0,65535,37,      0, -180000, 128,   90000,
     &       0,   2500, 5000, 64, 0, 0, 0, 0, 0,
     & 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37,
     & 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37,
     & 37, 37, 37, 37, 37, 37,  1/
      DATA  GRD23 / 0,  33, 0,65535, 37, -90000,       0, 128,       0,
     &  180000,   2500, 5000, 64, 0, 0, 0, 0, 0,
     &  1, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37,
     & 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37,
     & 37, 37, 37, 37, 37, 37, 37/
      DATA  GRD24 / 0,  33, 0,65535, 37, -90000, -180000, 128,       0,
     &       0,   2500, 5000, 64, 0, 0, 0, 0, 0,
     &  1, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37,
     & 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37,
     & 37, 37, 37, 37, 37, 37, 37/
      DATA  GRD25 / 0,  33, 0,65535, 19,      0,       0, 128,   90000,
     &  355000,   5000, 5000, 64, 0, 0, 0, 0, 0,
     & 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72,
     & 72, 72, 72,  1/
      DATA  GRD26 / 0,  33, 0,65535, 19, -90000,       0, 128,       0,
     &  355000,   5000, 5000, 64, 0, 0, 0, 0, 0,
     &  1, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72,
     & 72, 72, 72, 72/
      DATA  GRD27 / 0, 255, 5,  65, 65, -20826, -125000,   8,  -80000,
     &  381000, 381000, 0, 64, 0, 0, 0, 0, 0/
      DATA  GRD28 / 0, 255, 5,  65, 65,  20826,  145000,   8,  100000,
     &  381000, 381000,128, 64, 0, 0, 0, 0, 0/
      DATA  GRD29 / 0, 255, 0, 145, 37,      0,       0, 128,   90000,
     &  360000,   2500, 2500, 64, 0, 0, 0, 0, 0/
      DATA  GRD30 / 0, 255, 0, 145, 37,  -90000,      0, 128,       0,
     &  360000,   2500, 2500, 64, 0, 0, 0, 0, 0/
      DATA  GRD33 / 0, 255, 0, 181, 46,      0,       0, 128,   90000,
     &  360000,   2000, 2000, 64, 0, 0, 0, 0, 0/
      DATA  GRD34 / 0, 255, 0, 181, 46, -90000,       0, 128,       0,
     &  360000,   2000, 2000, 64, 0, 0, 0, 0, 0/
      DATA  GRD37 / 0,  33, 0,65535,73,      0,  -30000, 128,   90000,
     &   60000,  1250,65535, 64, 0, 0, 0, 0, 0,
     & 73, 73, 73, 73, 73, 73, 73, 73, 72, 72, 72, 71, 71, 71, 70,
     & 70, 69, 69, 68, 67, 67, 66, 65, 65, 64, 63, 62, 61, 60, 60,
     & 59, 58, 57, 56, 55, 54, 52, 51, 50, 49, 48, 47, 45, 44, 43,
     & 42, 40, 39, 38, 36, 35, 33, 32, 30, 29, 28, 26, 25, 23, 22,
     & 20, 19, 17, 16, 14, 12, 11,  9,  8,  6,  5,  3,  2/
      DATA  GRD38 / 0,  33, 0,65535,73,      0,   60000, 128,   90000,
     &  150000,  1250,65535, 64, 0, 0, 0, 0, 0,
     & 73, 73, 73, 73, 73, 73, 73, 73, 72, 72, 72, 71, 71, 71, 70,
     & 70, 69, 69, 68, 67, 67, 66, 65, 65, 64, 63, 62, 61, 60, 60,
     & 59, 58, 57, 56, 55, 54, 52, 51, 50, 49, 48, 47, 45, 44, 43,
     & 42, 40, 39, 38, 36, 35, 33, 32, 30, 29, 28, 26, 25, 23, 22,
     & 20, 19, 17, 16, 14, 12, 11,  9,  8,  6,  5,  3,  2/
      DATA  GRD39 / 0,  33, 0,65535,73,      0,  150000, 128,   90000,
     & -120000,  1250,65535, 64, 0, 0, 0, 0, 0,
     & 73, 73, 73, 73, 73, 73, 73, 73, 72, 72, 72, 71, 71, 71, 70,
     & 70, 69, 69, 68, 67, 67, 66, 65, 65, 64, 63, 62, 61, 60, 60,
     & 59, 58, 57, 56, 55, 54, 52, 51, 50, 49, 48, 47, 45, 44, 43,
     & 42, 40, 39, 38, 36, 35, 33, 32, 30, 29, 28, 26, 25, 23, 22,
     & 20, 19, 17, 16, 14, 12, 11,  9,  8,  6,  5,  3,  2/
      DATA  GRD40 / 0,  33, 0,65535,73,       0, -120000, 128,   90000,
     &  -30000,  1250,65535, 64, 0, 0, 0, 0, 0,
     & 73, 73, 73, 73, 73, 73, 73, 73, 72, 72, 72, 71, 71, 71, 70,
     & 70, 69, 69, 68, 67, 67, 66, 65, 65, 64, 63, 62, 61, 60, 60,
     & 59, 58, 57, 56, 55, 54, 52, 51, 50, 49, 48, 47, 45, 44, 43,
     & 42, 40, 39, 38, 36, 35, 33, 32, 30, 29, 28, 26, 25, 23, 22,
     & 20, 19, 17, 16, 14, 12, 11,  9,  8,  6,  5,  3,  2/
      DATA  GRD41 / 0,  33, 0,65535,73, -90000,  -30000, 128,       0,
     &   60000,  1250,65535, 64, 0, 0, 0, 0, 0,
     &  2,  3,  5,  6,  8,  9, 11, 12, 14, 16, 17, 19, 20, 22, 23,
     & 25, 26, 28, 29, 30, 32, 33, 35, 36, 38, 39, 40, 42, 43, 44,
     & 45, 47, 48, 49, 50, 51, 52, 54, 55, 56, 57, 58, 59, 60, 60,
     & 61, 62, 63, 64, 65, 65, 66, 67, 67, 68, 69, 69, 70, 70, 71,
     & 71, 71, 72, 72, 72, 73, 73, 73, 73, 73, 73, 73, 73/
      DATA  GRD42 / 0,  33, 0,65535,73, -90000,   60000, 128,       0,
     &  150000,  1250,65535, 64, 0, 0, 0, 0, 0,
     &  2,  3,  5,  6,  8,  9, 11, 12, 14, 16, 17, 19, 20, 22, 23,
     & 25, 26, 28, 29, 30, 32, 33, 35, 36, 38, 39, 40, 42, 43, 44,
     & 45, 47, 48, 49, 50, 51, 52, 54, 55, 56, 57, 58, 59, 60, 60,
     & 61, 62, 63, 64, 65, 65, 66, 67, 67, 68, 69, 69, 70, 70, 71,
     & 71, 71, 72, 72, 72, 73, 73, 73, 73, 73, 73, 73, 73/
      DATA  GRD43 / 0,  33, 0,65535,73, -90000,  150000, 128,       0,
     & -120000,  1250,65535, 64, 0, 0, 0, 0, 0,
     &  2,  3,  5,  6,  8,  9, 11, 12, 14, 16, 17, 19, 20, 22, 23,
     & 25, 26, 28, 29, 30, 32, 33, 35, 36, 38, 39, 40, 42, 43, 44,
     & 45, 47, 48, 49, 50, 51, 52, 54, 55, 56, 57, 58, 59, 60, 60,
     & 61, 62, 63, 64, 65, 65, 66, 67, 67, 68, 69, 69, 70, 70, 71,
     & 71, 71, 72, 72, 72, 73, 73, 73, 73, 73, 73, 73, 73/
      DATA  GRD44 / 0,  33, 0,65535,73, -90000, -120000, 128,       0,
     &  -30000,  1250,65535, 64, 0, 0, 0, 0, 0,
     &  2,  3,  5,  6,  8,  9, 11, 12, 14, 16, 17, 19, 20, 22, 23,
     & 25, 26, 28, 29, 30, 32, 33, 35, 36, 38, 39, 40, 42, 43, 44,
     & 45, 47, 48, 49, 50, 51, 52, 54, 55, 56, 57, 58, 59, 60, 60,
     & 61, 62, 63, 64, 65, 65, 66, 67, 67, 68, 69, 69, 70, 70, 71,
     & 71, 71, 72, 72, 72, 73, 73, 73, 73, 73, 73, 73, 73/
      DATA  GRD45 / 0, 255, 0, 288,145,  90000,       0, 128,  -90000,
     &   -1250,   1250, 1250,  0, 0, 0, 0, 0, 0/
      DATA  GRD53 / 0, 255, 1, 117, 51, -61050,       0, 128,   61050,
     &       0,  318830, 318830, 22500, 64, 0, 0, 0, 0/
      DATA  GRD55 / 0, 255, 5,  87, 71, -10947, -154289,   8, -105000,
     &  254000, 254000, 0, 64, 0, 0, 0, 0, 0/
      DATA  GRD56 / 0, 255, 5,  87, 71,   7647, -133443,   8, -105000,
     &  127000, 127000, 0, 64, 0, 0, 0, 0, 0/
      DATA  GRD61 / 0,  33, 0,65535, 46,      0,       0, 128,   90000,
     &  180000,   2000, 2000, 64, 0, 0, 0, 0, 0,
     & 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91,
     & 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91,
     & 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91,
     &  1/
      DATA  GRD62 / 0,  33, 0,65535, 46,      0, -180000, 128,   90000,
     &       0,   2000, 2000, 64, 0, 0, 0, 0, 0,
     & 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91,
     & 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91,
     & 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91,
     &  1/
      DATA  GRD63 / 0,  33, 0,65535, 46,      0,  -90000, 128,       0,
     &  180000,   2000, 2000, 64, 0, 0, 0, 0, 0,
     &  1, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91,
     & 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91,
     & 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91,
     & 91/
      DATA  GRD64 / 0,  33, 0,65535, 46, -90000, -180000, 128,       0,
     &       0,   2000, 2000, 64, 0, 0, 0, 0, 0,
     &  1, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91,
     & 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91,
     & 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91,
     & 91/
      DATA  GRD85 / 0, 255, 0, 360, 90,    500,     500, 128,   89500,
     &  359500,   1000, 1000, 64, 0, 0, 0, 0, 0/
      DATA  GRD86 / 0, 255, 0, 360, 90, -89500,     500, 128,    -500,
     &  359500,   1000, 1000, 64, 0, 0, 0, 0, 0/
      DATA  GRD87 / 0, 255, 5,  81, 62,  22876, -120491,   8, -105000,
     &   68153,  68153, 0, 64, 0, 0, 0, 0, 0/
      DATA  GRD90 / 0, 255,201,12902,1,    182, -149887, 136,      92,
     &     141,    577,538,64, 0, 0, 0, 0, 0/
      DATA  GRD91 / 0, 255,202,25803,1,    182, -149887, 136,     183,
     &     141,    577,538,64, 0, 0, 0, 0, 0/
      DATA  GRD92 / 0, 255,201,27071,3,    407, -144094, 136,     223,
     &     365,    222,205,64, 0, 0, 0, 0, 0/
      DATA  GRD93 / 0, 255,202,32485,5,    407, -144094, 136,     445,
     &     365,    222,205,64, 0, 0, 0, 0, 0/
      DATA  GRD94 / 0, 255,201,48916,1,   9678, -128826, 136,     181,
     &     271,    194,185,64, 0, 0, 0, 0, 0/
      DATA  GRD95 / 0, 255,202,97831,1,   9678, -128826, 136,     361,
     &     271,    194,185,64, 0, 0, 0, 0, 0/
      DATA  GRD96 / 0, 255,201,41630,1,  -3441, -148799, 136,     160,
     &     261,    333,308,64, 0, 0, 0, 0, 0/
      DATA  GRD97 / 0, 255,202,83259,1,  -3441, -148799, 136,     319,
     &     261,    333,308,64, 0, 0, 0, 0, 0/
      DATA  GRD98 / 0, 255, 4, 192, 94,  88542,       0, 128,  -88542,
     &    -1875, 47,1875, 0, 0, 0, 0, 0, 0/
      DATA  GRD100/ 0, 255, 5,  83, 83,  17108, -129296,   8, -105000,
     &   91452,  91452, 0, 64, 0, 0, 0, 0, 0/
      DATA  GRD101/ 0, 255, 5, 113, 91,  10528, -137146,   8, -105000,
     &   91452,  91452, 0, 64, 0, 0, 0, 0, 0/
      DATA  GRD103/ 0, 255, 5,  65, 56,  22405, -121352,   8, -105000,
     &   91452,  91452, 0, 64, 0, 0, 0, 0, 0/
      DATA  GRD104/ 0, 255, 5, 147,110,   -268, -139475,   8, -105000,
     &   90755,  90755, 0, 64, 0, 0, 0, 0, 0/
      DATA  GRD105/ 0, 255, 5,  83, 83,  17529, -129296,   8, -105000,
     &   90755,  90755, 0, 64, 0, 0, 0, 0, 0/
      DATA  GRD106/ 0, 255, 5, 165,117,  17533, -129296,   8, -105000,
     &   45373,  45373, 0, 64, 0, 0, 0, 0, 0/
      DATA  GRD107/ 0, 255, 5, 120, 92,  23438, -120168,   8, -105000,
     &   45373,  45373, 0, 64, 0, 0, 0, 0, 0/
      DATA  GRD126/ 0, 255, 4, 384,190,  89277,       0, 128,  -89277,
     &    -938,    95, 938, 0, 0, 0, 0, 0, 0/
      DATA  GRD170/ 0, 255, 4, 512,256,  89463,       0, 128,  -89463,
     &    -703,   128, 703, 0, 0, 0, 0, 0, 0/
      DATA  GRD190 / 0, 255,203, 92,141,    182, -149887, 136,   52000,
     & -111000,    577,538,64, 0, 0, 0, 0, 0/
      DATA  GRD192 / 0, 255,203,223,365,    407, -144094, 136,   50000,
     & -107000,    222,205,64, 0, 0, 0, 0, 0/
      DATA  GRD196/ 0, 255,201,45903,1,  23476,  -96745, 136,     151,
     &     305,     67, 66, 64, 0, 0, 0, 0, 0/
      DATA  GRD201/ 0, 255, 5,  65, 65, -20826, -150000,   8, -105000,
     &  381000, 381000, 0, 64, 0, 0, 0, 0, 0/
      DATA  GRD202/ 0, 255, 5,  65, 43,   7838, -141028,   8, -105000,
     &  190500, 190500, 0, 64, 0, 0, 0, 0, 0/
      DATA  GRD203/ 0, 255, 5,  45, 39,  19132, -185837,   8, -150000,
     &  190500, 190500, 0, 64, 0, 0, 0, 0, 0/
      DATA  GRD204/ 0, 255, 1,  93, 68, -25000,  110000, 128,   60644,
     & -109129, 160000, 160000, 20000, 64, 0, 0, 0, 0/
      DATA  GRD205/ 0, 255, 5,  45, 39,    616,  -84904,   8,  -60000,
     &  190500, 190500, 0, 64, 0, 0, 0, 0, 0/
      DATA  GRD206/ 0, 255, 3,  51, 41,  22289, -117991,   8, - 95000,
     &   81271,  81271, 0, 64, 0, 25000, 25000, 0, 0/
      DATA  GRD207/ 0, 255, 5,  49, 35,  42085, -175641,   8, -150000,
     &   95250,  95250, 0, 64, 0, 0, 0, 0, 0/
      DATA  GRD208/ 0, 255, 1,  29, 27,   9343, -167315, 128,   28092,
     & -145878, 80000, 80000, 20000, 64, 0, 0, 0, 0/
      DATA  GRD209/ 0, 255, 3, 101, 81,  22289, -117991,   8,  -95000,
     &   40635,  40635, 0, 64, 0, 25000, 25000, 0, 0/
      DATA  GRD210/ 0, 255, 1,  25, 25,   9000,  -77000, 128,   26422,
     &  -58625, 80000, 80000, 20000, 64, 0, 0, 0, 0/
      DATA  GRD211/ 0, 255, 3,  93, 65,  12190, -133459,   8,  -95000,
     &   81271,  81271, 0, 64, 0, 25000, 25000, 0, 0/
      DATA  GRD212/ 0, 255, 3, 185,129,  12190, -133459,   8,  -95000,
     &   40635,  40635, 0, 64, 0, 25000, 25000, 0, 0/
      DATA  GRD213/ 0, 255, 5, 129, 85,   7838, -141028,   8, -105000,
     &   95250,  95250, 0, 64, 0, 0, 0, 0, 0/
      DATA  GRD214/ 0, 255, 5,  97, 69,  42085, -175641,   8, -150000,
     &   47625,  47625, 0, 64, 0, 0, 0, 0, 0/
      DATA  GRD215/ 0, 255, 3, 369,257,  12190, -133459,   8,  -95000,
     &   20318,  20318, 0, 64, 0, 25000, 25000, 0, 0/
      DATA  GRD216/ 0, 255, 5, 139,107,  30000, -173000,   8, -135000,
     &   45000,  45000, 0, 64, 0, 0, 0, 0, 0/
      DATA  GRD217/ 0, 255, 5, 289,205,  42085, -175641,   8, -150000,
     &   15875,  15875, 0, 64, 0, 0, 0, 0, 0/
      DATA  GRD218/ 0, 255, 3, 737,513,  12190, -133459,   8,  -95000,
     &   10159,  10159, 0, 64, 0, 25000, 25000, 0, 0/
      DATA  GRD219/ 0, 255, 5, 385,465,  25008, -119559,  72,  -80000,
     &   25400,  25400, 0, 64, 0, 0, 0, 0, 0/
      DATA  GRD220/ 0, 255, 5, 345,355, -36889, -220194,  72, -260000,
     &   25400,  25400, 1, 64, 0, 0, 0, 0, 0/
      DATA  GRD221/ 0, 255, 3, 349,277,   1000, -145500,   8, -107000,
     &   32463,  32463, 0, 64, 0, 50000, 50000, 0, 0/
      DATA  GRD222/ 0, 255, 3,  59, 47,   1000, -145500,   8, -107000,
     &  194780, 194780, 0, 64, 0, 50000, 50000, 0, 0/
      DATA  GRD223/ 0, 255, 5, 129,129, -20826, -150000,   8, -105000,
     &  190500, 190500, 0, 64, 0, 0, 0, 0, 0/
      DATA  GRD224/ 0, 255, 5,  65, 65,  20826,  120000,   8, -105000,
     &  381000, 381000, 0, 64, 0, 0, 0, 0, 0/
      DATA  GRD225/ 0, 255, 1, 185,135, -25000, -250000, 128,   60640,
     & -250871, 80000, 80000, 20000, 64, 0, 0, 0, 0/
      DATA  GRD226/ 0, 255, 3, 737,513,  12190, -133459,   8,  -95000,
     &   10159,  10159, 0, 64, 0, 25000, 25000, 0, 0/
      DATA  GRD227/ 0, 255, 3,1473,1025,  12190, -133459,   8,  -95000,
     &    5079,   5079, 0, 64, 0, 25000, 25000, 0, 0/
      DATA  GRD228/ 0, 255, 0, 144, 73,  90000,       0, 128,  -90000,
     &   -2500,   2500, 2500, 64, 0, 0, 0, 0, 0/
      DATA  GRD229/ 0, 255, 0, 360,181,  90000,       0, 128,  -90000,
     &   -1000,   1000, 1000, 64, 0, 0, 0, 0, 0/
      DATA  GRD230/ 0, 255, 0, 720,361,  90000,       0, 128,  -90000,
     &    -500,    500,  500, 64, 0, 0, 0, 0, 0/
      DATA  GRD231/ 0, 255, 0, 720,181,      0,       0, 128,   90000,
     &    -500,    500,  500, 64, 0, 0, 0, 0, 0/
      DATA  GRD232/ 0, 255, 0, 360, 91,      0,       0, 128,   90000,
     &   -1000,   1000, 1000, 64, 0, 0, 0, 0, 0/
      DATA  GRD233/ 0, 255, 0, 288,157,  78000,       0, 128,  -78000,
     &   -1250,   1250, 1000, 64, 0, 0, 0, 0, 0/
      DATA  GRD234/ 0, 255, 0, 133,121,  15000,  -98000, 128,  -45000,
     &  -65000,    250,  250, 64, 0, 0, 0, 0, 0/
      DATA  GRD235/ 0, 255, 0, 720,360,  89750,     250,  72,  -89750,
     &    -250,    250, 1000, 64, 0, 0, 0, 0, 0/
      DATA  GRD236/ 0, 255, 3, 151,113,  16281,  233862,   8,  -95000,
     &   40635,  40635, 0, 64, 0, 25000, 25000, 0, 0/
      DATA  GRD237/ 0, 255, 3,  54, 47,  16201,  285720,   8, -107000,
     &   32463,  32463, 0, 64, 0, 50000, 50000, 0, 0/
C
      IERR = 0
C
        DO 1 I = 1,18
          IGDS(I) = 0
 1      CONTINUE
C
      IF (IGRID.GE.37.AND.IGRID.LE.44) THEN
        DO 2 I = 19,91
          IGDS(I) = 0
 2      CONTINUE
      END IF
C
      IF (IGRID.GE.21.AND.IGRID.LE.24) THEN
        DO I = 19,55
          IGDS(I) = 0
        END DO
      END IF
C
      IF (IGRID.GE.25.AND.IGRID.LE.26) THEN
        DO I = 19,37
          IGDS(I) = 0
        END DO
      END IF
C
      IF (IGRID.GE.61.AND.IGRID.LE.64) THEN
        DO I = 19,64
          IGDS(I) = 0
        END DO
      END IF
C
      IF (IGRID.EQ.1) THEN
        DO 3 I = 1,14
          IGDS(I) = GRD1(I)
  3     CONTINUE
C
      ELSE IF (IGRID.EQ.2) THEN
        DO 4 I = 1,14
          IGDS(I) = GRD2(I)
  4     CONTINUE
C
      ELSE IF (IGRID.EQ.3) THEN
        DO 5 I = 1,14
          IGDS(I) = GRD3(I)
  5     CONTINUE
C
      ELSE IF (IGRID.EQ.4) THEN
        DO 6 I = 1,14
          IGDS(I) = GRD4(I)
  6     CONTINUE
C
      ELSE IF (IGRID.EQ.5) THEN
        DO 10 I = 1,14
          IGDS(I) = GRD5(I)
 10     CONTINUE
C
      ELSE IF (IGRID.EQ.6) THEN
        DO 20 I = 1,14
          IGDS(I) = GRD6(I)
 20     CONTINUE
C
      ELSE IF (IGRID.EQ.8) THEN
        DO I = 1,14
          IGDS(I) = GRD8(I)
        END DO
C
      ELSE IF (IGRID.EQ.21) THEN
        DO 30 I = 1,55
          IGDS(I) = GRD21(I)
 30     CONTINUE
C
      ELSE IF (IGRID.EQ.22) THEN
        DO 40 I = 1,55
          IGDS(I) = GRD22(I)
 40     CONTINUE
C
      ELSE IF (IGRID.EQ.23) THEN
        DO 50 I = 1,55
          IGDS(I) = GRD23(I)
 50     CONTINUE
C
      ELSE IF (IGRID.EQ.24) THEN
        DO 60 I = 1,55
          IGDS(I) = GRD24(I)
 60     CONTINUE
C
      ELSE IF (IGRID.EQ.25) THEN
        DO 70 I = 1,37
          IGDS(I) = GRD25(I)
 70     CONTINUE
C
      ELSE IF (IGRID.EQ.26) THEN
        DO 80 I = 1,37
          IGDS(I) = GRD26(I)
 80     CONTINUE
C
      ELSE IF (IGRID.EQ.27) THEN
        DO 90 I = 1,14
          IGDS(I) = GRD27(I)
 90     CONTINUE
C
      ELSE IF (IGRID.EQ.28) THEN
        DO 100 I = 1,14
          IGDS(I) = GRD28(I)
 100    CONTINUE
C
      ELSE IF (IGRID.EQ.29) THEN
        DO 110 I = 1,14
          IGDS(I) = GRD29(I)
 110    CONTINUE
C
      ELSE IF (IGRID.EQ.30) THEN
        DO 120 I = 1,14
          IGDS(I) = GRD30(I)
 120    CONTINUE
C
      ELSE IF (IGRID.EQ.33) THEN
        DO 130 I = 1,14
          IGDS(I) = GRD33(I)
 130     CONTINUE
C
      ELSE IF (IGRID.EQ.34) THEN
        DO 140 I = 1,14
          IGDS(I) = GRD34(I)
 140    CONTINUE
C
      ELSE IF (IGRID.EQ.37) THEN
        DO 141 I = 1,91
          IGDS(I) = GRD37(I)
 141    CONTINUE
C
      ELSE IF (IGRID.EQ.38) THEN
        DO 142 I = 1,91
          IGDS(I) = GRD38(I)
 142    CONTINUE
C
      ELSE IF (IGRID.EQ.39) THEN
        DO 143 I = 1,91
          IGDS(I) = GRD39(I)
 143    CONTINUE
C
      ELSE IF (IGRID.EQ.40) THEN
        DO 144 I = 1,91
          IGDS(I) = GRD40(I)
 144    CONTINUE
C
      ELSE IF (IGRID.EQ.41) THEN
        DO 145 I = 1,91
          IGDS(I) = GRD41(I)
 145    CONTINUE
C
      ELSE IF (IGRID.EQ.42) THEN
        DO 146 I = 1,91
          IGDS(I) = GRD42(I)
 146    CONTINUE
C
      ELSE IF (IGRID.EQ.43) THEN
        DO 147 I = 1,91
          IGDS(I) = GRD43(I)
 147    CONTINUE
C
      ELSE IF (IGRID.EQ.44) THEN
        DO 148 I = 1,91
          IGDS(I) = GRD44(I)
 148    CONTINUE
C
      ELSE IF (IGRID.EQ.45) THEN
        DO 149 I = 1,14
          IGDS(I) = GRD45(I)
 149    CONTINUE
C
C     ELSE IF (IGRID.EQ.50) THEN
C       DO 150 I = 1,14
C         IGDS(I) = GRD50(I)
C150    CONTINUE
C
      ELSE IF (IGRID.EQ.53) THEN
        DO I = 1,14
          IGDS(I) = GRD53(I)
        END DO
C
      ELSE IF (IGRID.EQ.55) THEN
        DO 152 I = 1,14
          IGDS(I) = GRD55(I)
 152    CONTINUE
C
      ELSE IF (IGRID.EQ.56) THEN
        DO 154 I = 1,14
          IGDS(I) = GRD56(I)
 154    CONTINUE
C
      ELSE IF (IGRID.EQ.61) THEN
        DO 160 I = 1,64
          IGDS(I) = GRD61(I)
 160    CONTINUE
C
      ELSE IF (IGRID.EQ.62) THEN
        DO 170 I = 1,64
          IGDS(I) = GRD62(I)
 170    CONTINUE
C
      ELSE IF (IGRID.EQ.63) THEN
        DO 180 I = 1,64
          IGDS(I) = GRD63(I)
 180    CONTINUE
C
      ELSE IF (IGRID.EQ.64) THEN
        DO 190 I = 1,64
          IGDS(I) = GRD64(I)
 190    CONTINUE
C
      ELSE IF (IGRID.EQ.85) THEN
        DO 192 I = 1,14
          IGDS(I) = GRD85(I)
 192    CONTINUE
C
      ELSE IF (IGRID.EQ.86) THEN
        DO 194 I = 1,14
          IGDS(I) = GRD86(I)
 194    CONTINUE
C
      ELSE IF (IGRID.EQ.87) THEN
        DO 195 I = 1,14
          IGDS(I) = GRD87(I)
 195    CONTINUE
C
      ELSE IF (IGRID.EQ.90) THEN
        DO 196 I = 1,14
          IGDS(I) = GRD90(I)
 196    CONTINUE
C
      ELSE IF (IGRID.EQ.91) THEN
        DO 197 I = 1,14
          IGDS(I) = GRD91(I)
 197    CONTINUE
C
      ELSE IF (IGRID.EQ.92) THEN
        DO 198 I = 1,14
          IGDS(I) = GRD92(I)
 198    CONTINUE
C
      ELSE IF (IGRID.EQ.93) THEN
        DO 199 I = 1,14
          IGDS(I) = GRD93(I)
 199    CONTINUE
C
      ELSE IF (IGRID.EQ.94) THEN
        DO 200 I = 1,14
          IGDS(I) = GRD94(I)
 200    CONTINUE
C
      ELSE IF (IGRID.EQ.95) THEN
        DO 201 I = 1,14
          IGDS(I) = GRD95(I)
 201    CONTINUE
C
      ELSE IF (IGRID.EQ.96) THEN
        DO 202 I = 1,14
          IGDS(I) = GRD96(I)
 202    CONTINUE
C
      ELSE IF (IGRID.EQ.97) THEN
        DO 203 I = 1,14
          IGDS(I) = GRD97(I)
 203    CONTINUE
C
      ELSE IF (IGRID.EQ.98) THEN
        DO 204 I = 1,14
          IGDS(I) = GRD98(I)
 204    CONTINUE
C
      ELSE IF (IGRID.EQ.100) THEN
        DO 205 I = 1,14
          IGDS(I) = GRD100(I)
 205    CONTINUE
C
      ELSE IF (IGRID.EQ.101) THEN
        DO 210 I = 1,14
          IGDS(I) = GRD101(I)
 210    CONTINUE
C
      ELSE IF (IGRID.EQ.103) THEN
        DO 220 I = 1,14
          IGDS(I) = GRD103(I)
 220   CONTINUE
C
      ELSE IF (IGRID.EQ.104) THEN
        DO 230 I = 1,14
          IGDS(I) = GRD104(I)
 230    CONTINUE
C
      ELSE IF (IGRID.EQ.105) THEN
        DO 240 I = 1,14
          IGDS(I) = GRD105(I)
 240    CONTINUE
C
      ELSE IF (IGRID.EQ.106) THEN
        DO 242 I = 1,14
          IGDS(I) = GRD106(I)
 242    CONTINUE
C
      ELSE IF (IGRID.EQ.107) THEN
        DO 244 I = 1,14
          IGDS(I) = GRD107(I)
 244    CONTINUE
C
      ELSE IF (IGRID.EQ.126) THEN
        DO 245 I = 1,14
          IGDS(I) = GRD126(I)
 245    CONTINUE
C
      ELSE IF (IGRID.EQ.170) THEN
        DO I = 1,14
          IGDS(I) = GRD170(I)
        ENDDO
C
      ELSE IF (IGRID.EQ.190) THEN
        DO 2190 I = 1,14
          IGDS(I) = GRD190(I)
 2190   CONTINUE
C
      ELSE IF (IGRID.EQ.192) THEN
        DO 2192 I = 1,14
          IGDS(I) = GRD192(I)
 2192   CONTINUE
C
      ELSE IF (IGRID.EQ.196) THEN
        DO 249 I = 1,14
          IGDS(I) = GRD196(I)
 249    CONTINUE
C
      ELSE IF (IGRID.EQ.201) THEN
        DO 250 I = 1,14
          IGDS(I) = GRD201(I)
 250    CONTINUE
C
      ELSE IF (IGRID.EQ.202) THEN
        DO 260 I = 1,14
          IGDS(I) = GRD202(I)
 260    CONTINUE
C
      ELSE IF (IGRID.EQ.203) THEN
        DO 270 I = 1,14
          IGDS(I) = GRD203(I)
 270    CONTINUE
C
      ELSE IF (IGRID.EQ.204) THEN
        DO 280 I = 1,14
          IGDS(I) = GRD204(I)
 280    CONTINUE
C
      ELSE IF (IGRID.EQ.205) THEN
        DO 290 I = 1,14
          IGDS(I) = GRD205(I)
 290    CONTINUE
C
      ELSE IF (IGRID.EQ.206) THEN
        DO 300 I = 1,18
          IGDS(I) = GRD206(I)
 300    CONTINUE
C
      ELSE IF (IGRID.EQ.207) THEN
        DO 310 I = 1,14
          IGDS(I) = GRD207(I)
 310    CONTINUE
C
      ELSE IF (IGRID.EQ.208) THEN
        DO 320 I = 1,14
          IGDS(I) = GRD208(I)
 320    CONTINUE
C
      ELSE IF (IGRID.EQ.209) THEN
        DO 330 I = 1,18
          IGDS(I) = GRD209(I)
 330    CONTINUE
C
      ELSE IF (IGRID.EQ.210) THEN
        DO 340 I = 1,14
          IGDS(I) = GRD210(I)
 340    CONTINUE
C
      ELSE IF (IGRID.EQ.211) THEN
        DO 350 I = 1,18
          IGDS(I) = GRD211(I)
 350    CONTINUE
C
      ELSE IF (IGRID.EQ.212) THEN
        DO 360 I = 1,18
          IGDS(I) = GRD212(I)
 360    CONTINUE
C
      ELSE IF (IGRID.EQ.213) THEN
        DO 370 I = 1,14
          IGDS(I) = GRD213(I)
 370    CONTINUE
C
      ELSE IF (IGRID.EQ.214) THEN
        DO 380 I = 1,14
          IGDS(I) = GRD214(I)
 380    CONTINUE
C
      ELSE IF (IGRID.EQ.215) THEN
        DO 390 I = 1,18
          IGDS(I) = GRD215(I)
 390    CONTINUE
C
      ELSE IF (IGRID.EQ.216) THEN
        DO 400 I = 1,14
          IGDS(I) = GRD216(I)
 400    CONTINUE
C
      ELSE IF (IGRID.EQ.217) THEN
        DO 401 I = 1,14
          IGDS(I) = GRD217(I)
 401    CONTINUE
C
      ELSE IF (IGRID.EQ.218) THEN
        DO 410 I = 1,18
          IGDS(I) = GRD218(I)
 410    CONTINUE
C
      ELSE IF (IGRID.EQ.219) THEN
        DO 411 I = 1,14
          IGDS(I) = GRD219(I)
 411    CONTINUE
C
      ELSE IF (IGRID.EQ.220) THEN
        DO 412 I = 1,14
          IGDS(I) = GRD220(I)
 412    CONTINUE
C
      ELSE IF (IGRID.EQ.221) THEN
        DO 413 I = 1,18
          IGDS(I) = GRD221(I)
 413    CONTINUE
C
      ELSE IF (IGRID.EQ.222) THEN
        DO 414 I = 1,18
          IGDS(I) = GRD222(I)
 414    CONTINUE
C
      ELSE IF (IGRID.EQ.223) THEN
        DO 415 I = 1,14
          IGDS(I) = GRD223(I)
 415    CONTINUE
C
      ELSE IF (IGRID.EQ.224) THEN
        DO 416 I = 1,14
          IGDS(I) = GRD224(I)
 416    CONTINUE
C
      ELSE IF (IGRID.EQ.225) THEN
        DO 417 I = 1,14
          IGDS(I) = GRD225(I)
 417    CONTINUE
C
      ELSE IF (IGRID.EQ.226) THEN
        DO 418 I = 1,18
          IGDS(I) = GRD226(I)
 418    CONTINUE
C
      ELSE IF (IGRID.EQ.227) THEN
        DO 419 I = 1,18
          IGDS(I) = GRD227(I)
 419    CONTINUE
C
      ELSE IF (IGRID.EQ.228) THEN
        DO 420 I = 1,14
          IGDS(I) = GRD228(I)
 420    CONTINUE
C
      ELSE IF (IGRID.EQ.229) THEN
        DO 421 I = 1,14
          IGDS(I) = GRD229(I)
 421    CONTINUE
C
      ELSE IF (IGRID.EQ.230) THEN
        DO 422 I = 1,14
          IGDS(I) = GRD230(I)
 422    CONTINUE
C
      ELSE IF (IGRID.EQ.231) THEN
        DO 423 I = 1,14
          IGDS(I) = GRD231(I)
 423    CONTINUE
C
      ELSE IF (IGRID.EQ.232) THEN
        DO 424 I = 1,14
          IGDS(I) = GRD232(I)
 424    CONTINUE
C
      ELSE IF (IGRID.EQ.233) THEN
        DO 425 I = 1,14
          IGDS(I) = GRD233(I)
 425    CONTINUE
C
      ELSE IF (IGRID.EQ.234) THEN
        DO 426 I = 1,14
          IGDS(I) = GRD234(I)
 426    CONTINUE
C
      ELSE IF (IGRID.EQ.235) THEN
        DO 427 I = 1,14
          IGDS(I) = GRD235(I)
 427    CONTINUE
C
      ELSE IF (IGRID.EQ.236) THEN
        DO 428 I = 1,18
          IGDS(I) = GRD236(I)
 428    CONTINUE
C
      ELSE IF (IGRID.EQ.237) THEN
        DO 429 I = 1,18
          IGDS(I) = GRD237(I)
 429    CONTINUE
C
      ELSE
        IERR = 1
      ENDIF
C
      RETURN
      END

