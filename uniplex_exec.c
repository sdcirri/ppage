#include <stdlib.h>		// free()
#include <string.h>		// strlen()
#include <stdio.h>		// printf() e fprintf()

#include "formatter.h"
#include "file_io.h"
#include "common.h"

int uniplex_exec(char *file_in, char *file_out, size_t num_col,
		 size_t col_len, size_t col_lines, size_t col_space) {
	/**
	 * Esecuzione monoprocesso del programma
	 */
	filematr_t minfo = file2matr(file_in);
	if(minfo.ptr == NULL) {
		perror("Non è stato possibile leggere il file");
		return 2;
	}
	if(minfo.maxword > col_len) {								// Non posso formattare il file
		fprintf(stderr, "Errore: le colonne sono troppo corte\n");
		free_fileinfo(&minfo);
		return 1;
	}

	char *output = format(&minfo, num_col, col_len, col_lines, col_space);			// Formatto la pagina
	if(output == NULL) {									// Errore di allocazione
		perror("Errore nell'allocazione della memoria");
		return 2;
	}
	free_fileinfo(&minfo);

	FILE *op = (strcmp(file_out, "-") == 0) ? stdout : fopen(file_out, "w+");		// Apro il file per la scrittura ("-" = stdout)
	if(op == NULL || !fprintf(op, "%s", output)) {						// Errore di apertura o scrittura
		perror("Errore nella scrittura del file");
		return 2;
	}
	if(op != stdout) fclose(op);										// Se la scrittura va a buon fine posso chiudere il file e continuare

	free(output);
	if(strcmp(file_out, "-") != 0) fprintf(stderr, "Risultati scritti in %s\n", file_out);
	return 0;
}

