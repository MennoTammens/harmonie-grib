#include <stdio.h>
#include <stdlib.h>
#include "gribw.h"

/* read write binary f77 ieee style arrays */


float *rd_f77data(int *n, FILE *in) {
	int i, ii;
	float *a;

	if (fread(&i, sizeof(int), 1, in) != 1) {
		fprintf(stderr,"missing binary data!\n");
		exit(8);
	}

	if (i % sizeof(float) != 0) {
		fprintf(stderr,"not a float array!\n");
		exit(8);
	}

	if (i) {
	    if ((a = (float *) malloc(i)) == NULL) {
		fprintf(stderr,"not enough memory in read_data!\n");
		exit(8);
	    }
	    if (fread(a, 1, i, in) != i) {
		fprintf(stderr,"missing binary data!\n");
		exit(8);
	    }
	}
	else {
	    a = NULL;
	}
	if (fread(&ii, sizeof(int), 1, in) != 1) {
		fprintf(stderr,"missing trailer!\n");
		exit(8);
	}
	if (i != ii) {
		fprintf(stderr,"f77 format?\n");
		exit(8);
	}
	*n = i / sizeof(float);
	return a;
}


void rd_n_f77data(float *a, int n, FILE *in) {
	int i;

	if (fread(&i, sizeof(int), 1, in) != 1) exit(8);
	if (i != n * sizeof(float)) exit(8);
	if (fread(a, sizeof(float), n, in) != n) exit(8);
	if (fread(&i, sizeof(int), 1, in) != 1) exit(8);
	if (i != n * sizeof(float)) exit(8);
}

void wrt_n_f77data(float *a, int n, FILE *out) {
	int i;

	i = n * sizeof(float);
        if (fwrite(&i, sizeof(int), 1, out) != 1) exit(8);
        if (fwrite(a, sizeof(float), n, out) != n) exit(8);
        if (fwrite(&i, sizeof(int), 1, out) != 1) exit(8);
}

