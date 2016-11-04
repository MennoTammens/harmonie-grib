#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* interactive pds maker
 * v1.0 w. ebisuzaki
 */

void main() {

	unsigned char pds[28];
	char line[200];

	printf("PDS maker\n\n");

	/* pds length */
	pds[0] = pds[1] = 0; pds[2] = 28;

	/* parameter table version # */
	pds[3] = 2;

	/* Center */
	do {
		printf("Centers: (incomplete list)\n");
		printf("  7 = NMC or NCEP\n");
		printf(" 34 = JMA\n");
		printf(" 58 = FNOC\n");
		printf(" 85 = French Weather Service\n");
		printf(" 98 = ECMWF\n");
		printf("\nEnter Center: ");
		gets(line);
		pds[4] = atoi(line);
	}
	while (pds[4] == 0);

	printf("Sub-Centers: (incomplete list)\n");
	printf("  7 = NMC or NCEP\n");
	printf(" 34 = JMA\n");
	printf(" 58 = FNOC\n");
	printf(" 85 = French Weather Service\n");
	printf(" 98 = ECMWF\n");
	printf("\nEnter Center: ");
	gets(line);
	pds[4] = atoi(line);

	do {
		printf("\nEnter Process: ");
		gets(line);
		pds[5] = atoi(line);
}
