#ifndef __FILE_IO_H
#define __FILE_IO_H

#include <stddef.h>

// Struttura per restituire info sul file letto
typedef struct FileMatrixInfo {
	char ***ptr;
	size_t maxword;
	size_t npars;
	size_t *nwords;
} filematr_t;

size_t strlen_ascii(char *s);

void parse_line(filematr_t *fileinfo, char *line, size_t *wc, int *in_par);

filematr_t file2matr(char *filen);

#endif

