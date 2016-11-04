float ang180(float ang) {
/* Maps arbitrary angle [deg] to [-180,180) [deg].

   2001-04-30, Oyvind.Breivik@dnmi.no.

This file is part of bilingrb and is distributed under terms of the GNU General Public License
 For details see, Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 Boston, MA  02110-1301  USA

24-6-2016 Oyvind.Breivik

*/
    for ( ; ang < -180.0 || ang >= 180.0; ) {
       ang = ang >= 180.0 ? ang-360.0 : ang;
       ang = ang < -180.0 ? ang+360.0 : ang;
    }
    return ang;

} /* End function ang180 */

float ang360(float ang) {
/* Maps arbitrary angle [deg] to [0, 360) [deg].

   2001-04-30, Oyvind.Breivik@dnmi.no.
*/

    for ( ; ang < 0.0 || ang >= 360.0; ) {
       ang = ang >= 360.0 ? ang-360.0 : ang;
       ang = ang <    0.0 ? ang+360.0 : ang;
    }
    return ang;

} /* End function ang360 */
