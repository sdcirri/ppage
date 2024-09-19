#include "formatter.h"
#include "file_io.h"

#include <string.h>		// strlen(), strncat() e memset()
#include <stdlib.h>		// malloc(), realloc() e free()

static char *strcatmatr(char *dest, char **matr, size_t n) {
	/**
	 * Concatena la rappresentazione di matr (n righe separate da \n) a dest
	 * Restituisce il puntatore a dest (potrebbe cambiare a causa di realloc()
	 */
	for(size_t i = 0; i < n; i++) {
		size_t len = strlen(matr[i]);
		dest = realloc(dest, strlen(dest) + len + 2);									// Alloco spazio per la nuova riga, per \n e il terminatore
		if(dest == NULL) return NULL;
		strncat(dest, matr[i], len);
		strncat(dest, "\n", 2);
	}
	return dest;
}

char *format(filematr_t *fileinfo, size_t num_col, size_t col_len, size_t col_lines, size_t col_space) {
	/**
	 * Impagina i paragrafi descritti nella matrice file
	 * fileinfo secondo i parametri forniti:
	 * 	- num_col:   numero di colonne per pagina
	 * 	- col_len:   lunghezza delle colonne
	 * 	- col_lines: numero di linee per colonna
	 * 	- col_space: spazio tra le colonne
	 * Restituisce il puntatore alla stringa contenente l'output formattato, NULL in caso di errori di allocazione della memoria
	 */

	if(fileinfo->maxword > col_len) return NULL;										// Se le colonne sono troppo corte per le parole

	char **pagebuf = malloc(sizeof(char*)), *dest = malloc(1);
	pagebuf[0] = NULL, dest[0] = 0;

	size_t word = 0, par, lcontent, nwords;
	size_t page = 0, col = 0, line = 0, curr_line;
	size_t *words;														// Buffer per indici di parole

	for(par = 0; par < fileinfo->npars; par++) {										// Scansione paragrafi
		while(word < fileinfo->nwords[par]) {										// Scansione parole del paragrafo
			lcontent = 0, nwords = 0;
			words = NULL;
			curr_line = line + page * (col_lines + 3);								// Linea corrente nella matrice
			while(lcontent + nwords + strlen_ascii(fileinfo->ptr[par][word]) - 1 < col_len) {			// Posso ancora inserire parole nella riga?
				++nwords;
				words = realloc(words, nwords * sizeof(size_t));						// Inserisco nuova parola
				if(words == NULL) return NULL;
				words[nwords - 1] = word;
				lcontent += strlen_ascii(fileinfo->ptr[par][word]);						// Calcolo lunghezza complessiva delle parole nella riga
																//   per controllare se posso inserire nuove parole e
																//   successivamente il numero di spazi tra le parole
				++word;												// Analizzo la parola successiva del paragrafo
				if(word >= fileinfo->nwords[par]) break;
			}
			size_t comm_spaces = nwords > 1 ? (col_len - lcontent) / (nwords - 1) : 0,				// Spazi tra le parole di una stessa riga
			       rem = col_len - lcontent - comm_spaces * (nwords - 1);						// Spazi di riempimento (posti tra la penultima e l'ultima parola)

			for(size_t i = 0; i < nwords; i++) {
				size_t len = pagebuf[curr_line] == NULL ? 0 : strlen(pagebuf[curr_line]);
				pagebuf[curr_line] = realloc(pagebuf[curr_line],						// Alloco spazio per la nuova parola
						len + strlen(fileinfo->ptr[par][words[i]]) + 1
				);
				pagebuf[curr_line][len] = 0;
				strncat(											// Concateno la prossima parola
						pagebuf[curr_line],
						fileinfo->ptr[par][words[i]],
						strlen(fileinfo->ptr[par][words[i]])
				);
				size_t spaces;
				if(word >= fileinfo->nwords[par]) {								// L'ultima riga di un paragrafo non va giustificata
					if(i == nwords - 1) spaces = col_len - nwords - lcontent + 1;				// Se è anche l'ulima parola della riga aggiungo spazi di riempimento
					else spaces = 1;
				}
																// Per tutte le altre righe:
				else if(i == nwords - 1) spaces = nwords > 1 ? 0 : col_len - lcontent;				// Dopo l'ultima parola non lascio spazi aggiuntivi, a meno che non
																//   sia l'unica, in tal caso devo aggiungere degli spazi di riempimento
				else if(i == nwords - 2) spaces = comm_spaces + rem;						// Tra l'ultima e la penultima aggiungo anche gli spazi di riempimento
				else spaces = comm_spaces;									// Altrimenti il numero calcolato di spazi

				if(i == nwords - 1 && col < num_col - 1)							// Se sono nell'ultima parola ma non nell'ultima colonna
					spaces += col_space;									//   aggiungo anche gli spazi inter-colonna
				len = strlen(pagebuf[curr_line]);
				pagebuf[curr_line] = realloc(pagebuf[curr_line], len + spaces + 1);				// Alloco spazio per gli spazi
				memset(pagebuf[curr_line] + len, ' ', spaces);							// Aggiungo gli spazi a fine riga
				pagebuf[curr_line][len + spaces] = 0;								// Aggiungo NULL terminator a fine riga
			}
			free(words);												// Libero per prossima iterazione

			if(++line >= col_lines)											// Se ho riempito tutte le righe della colonna
				line = 0, ++col;										//   passo alla colonna successiva
			else {
				++curr_line;
				if(col == 0 && !(par >= fileinfo->npars - 1 && word >= fileinfo->nwords[par] - 1)) {		// Altrimenti alloco spazio per la nuova riga (se non sono arrivato alla fine)
					pagebuf = realloc(pagebuf, (curr_line + 1) * sizeof(char*));
					if(pagebuf == NULL) return NULL;
					pagebuf[curr_line] = NULL;
				}
			}

			if(col >= num_col && par < fileinfo->npars && word < fileinfo->nwords[par]) {				// Se a fine pagina ho ancora parole da scrivere
				dest = strcatmatr(dest, pagebuf, curr_line + 1);						// Converto la pagina in una stringa e la concateno all'output
				if(dest == NULL) return NULL;
				size_t dlen = strlen(dest);
				dest = realloc(dest, dlen + 9);									// Alloco spazio per il separatore di pagina (3 \n, 3 % e 2 spazi) + terminatore
				if(dest == NULL) return NULL;
				strncat(dest, "\n \%\%\% \n\n", 9);
				for(size_t i = 0; i < curr_line; i++)								// Libero la pagina
					free(pagebuf[i]);
				free(pagebuf);
				pagebuf = malloc(sizeof(char*));								// Alloco spazio per nuova pagina
				pagebuf[0] = NULL;
				if(pagebuf == NULL) return NULL;
				col = 0, line = 0, curr_line = 0;
			}
		}
		word = 0;
		if(line < col_lines - 1 && line != 0 && par < fileinfo->npars - 1) {						// Lascio riga vuota a fine paragrafo solo se non è l'ultimo
			size_t len = pagebuf[curr_line] == NULL ? 0 : strlen(pagebuf[curr_line]),
			       spaces = col_len + col_space;
			pagebuf[curr_line] = realloc(pagebuf[curr_line], len + spaces + 1);
			if(pagebuf == NULL) return NULL;
			memset(pagebuf[curr_line] + len, ' ', spaces);
			pagebuf[curr_line][len + spaces] = 0;

			if(col == 0) {
				pagebuf = realloc(pagebuf, (curr_line + 2) * sizeof(char*));
				if(pagebuf == NULL) return NULL;
				pagebuf[curr_line + 1] = NULL;
			}
			++line;
		}
	}
	dest = strcatmatr(dest, pagebuf, (col == 0 ? curr_line : col_lines));							// Concateno l'ultima pagina
	for(size_t i = 0; i < curr_line; i++)											// Libero la memoria occupata dalla pagina
		free(pagebuf[i]);
	free(pagebuf);
	return dest;
}

