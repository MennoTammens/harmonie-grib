#include <stdio.h>
#include <stdlib.h>


int f77skip(FILE *stream) {
	long int position;
	unsigned int recl, tmp;

	position = ftell(stream);
	if (fread(&recl, sizeof (int), 1, stream) != 1) return 0;
	if (fseek(stream, recl, SEEK_CUR) != position + recl + 
              sizeof(int)) return 0;
	if (fread(&tmp, sizeof (int), 1, stream) != 1) return 0;
	return (tmp == recl);
}

/*
 * stream  .. input file
 * nbyptes .. 0 -> f77 header, otherwise size of record
 *
 * returns NULL/data
 *
 */

void *f77read(FILE *stream, int *nbytes) {

	unsigned int recl, tmp;
	char *data;

	if (*nbytes > 0) {
		recl = *nbytes;
		if ((data = (char *) malloc(recl)) == NULL) {
			return NULL;
		}
		if (fread(data, 1, recl, stream) != recl) {
			fprintf(stderr,"f77read: bad file\n");
			free(data);
			return NULL;
		}
		return (void *) data;
	}

	*nbytes = -1;
	if (fread(&recl, sizeof (int), 1, stream) != 1) {
		return NULL;
	}
	if ((data = (char *) malloc(recl)) == NULL) {
		return NULL;
	}
	if (fread(data, 1, recl, stream) != recl) {
		fprintf(stderr,"Not a f77-style file\n");
		free(data);
		return NULL;
	}
	if (fread(&tmp, sizeof (int), 1, stream) != 1) {
		fprintf(stderr,"Not a f77-style file\n");
		free(data);
		return NULL;
	}
	if (tmp != recl) {
		fprintf(stderr,"Not a f77-style file\n");
		free(data);
		return NULL;
	}
	*nbytes = recl;
	return (void *) data;
}
