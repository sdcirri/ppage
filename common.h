#ifndef __COMMON_H
#define __COMMON_H

#include "file_io.h"

/*
 * Collezione di funzioni comuni all'esecuzione uniplex e
 * a quella multiplex
 */

void free_fileinfo(filematr_t *minfo);

void free_matrix(char **m, size_t nlines);

#endif
