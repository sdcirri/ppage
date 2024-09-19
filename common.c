#include "common.h"

#include <stdlib.h>

#include "file_io.h"

void free_fileinfo(filematr_t *minfo) {
	/**
	 * Libera la memoria occupata dalla matrice in minfo
	 */
	for(size_t i = 0; i < minfo->npars; i++) {
		for(size_t j = 0; j < minfo->nwords[i]; j++)
			free(minfo->ptr[i][j]);
		free(minfo->ptr[i]);
	}
	free(minfo->ptr);
	free(minfo->nwords);
}

void free_matrix(char **m, size_t nlines) {
	/**
	 * Libera la memoria occupata dalla matrice in m
	 * Parametri:
	 * 	- nlines: numero di linee
	 */
	for(size_t i = 0; i < nlines; i++)
		free(m[i]);
	free(m);
}
