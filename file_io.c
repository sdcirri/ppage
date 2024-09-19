#include "file_io.h"

#define _POSIX_C_SOURCE 200809L // Per getline()
#include <stdlib.h>		// malloc(), realloc() e free()
#include <string.h>		// strcmp(), strtok() e strncpy()
#include <stdio.h>		// fopen(), fclose(), fprintf() e getline()

size_t strlen_ascii(char *s) {
	/**
	 * Conta i caratteri della stringa s escludendo i
	 * caratteri extra introdotti da UTF-8 (0b10xxxxxx)
	 */
	size_t i, n = 0;
	for(i = 0; s[i] != '\0'; i++)
		if((int)(s[i] & 0b11000000) == 0b10000000)									// Seleziono i 2 MSBs del byte e controllo che siano esattamente 10
			++n;
	return i - n;
}

void parse_line(filematr_t *fileinfo, char *line, size_t *wc, int *in_par) {
	/**
	 * Processa la riga di un file per la suddivisione in paragrafi
	 * Parametri:
	 * 	- fileinfo: puntatore alla struttura in cui salvare le info
	 * 	- line:     riga letta
	 * 	- wc:       puntatore al contatore di parole del paragrafo
	 * 	- in_par:   puntatore al flag che indica se sono in un paragrafo
	 */
	line = strtok(line, "\n\r");
	if(line == NULL) {
		if(*in_par) {
			fileinfo->nwords[fileinfo->npars - 1] = *wc;
			*in_par = 0, *wc = 0;
		}
	}
	else {
		if(!*in_par) {
			++fileinfo->npars;
			fileinfo->ptr = realloc(fileinfo->ptr, fileinfo->npars * sizeof(char*));
			fileinfo->nwords = realloc(fileinfo->nwords, fileinfo->npars * sizeof(size_t));
			fileinfo->ptr[fileinfo->npars - 1] = NULL;
			*in_par = 1;
		}
		char *word = strtok(line, " \t");
		for(; word != NULL; (*wc)++) {
			size_t clen = strlen_ascii(word), real_len = strlen(word);
			fileinfo->ptr[fileinfo->npars - 1] = realloc(fileinfo->ptr[fileinfo->npars - 1], (*wc + 1) * sizeof(char*));	// Alloco spazio per una nuova parola
			fileinfo->ptr[fileinfo->npars - 1][*wc] = malloc(real_len + 1);							// Alloco spazio per la mia parola
			memset(fileinfo->ptr[fileinfo->npars - 1][*wc], 0, real_len + 1);
			fileinfo->ptr[fileinfo->npars - 1][*wc] = strncpy(fileinfo->ptr[fileinfo->npars - 1][*wc], word, real_len);	// Copio la parola nella matrice
			if(clen > fileinfo->maxword) fileinfo->maxword = clen;								// Ricerca parola massima
			word = strtok(NULL, " \t");											// Prossima parola
		}
		free(word);
	}
}

filematr_t file2matr(char *filen) {
	/**
	 * Legge le righe del file al path filen e le memorizza in
	 * modo efficiente dividendo il testo per parole e per paragrafi
	 * Ritorna il puntatore alla matrice, il numero di paragrafi e
	 * la lunghezza della parola più lunga (per controllo errori)
	 * in una struttura dati apposita
	 * In caso di errore il puntatore ritornato è NULL
	 */

	FILE *fp = fopen(filen, "r");												// Apro il file
	if(fp == NULL) {													// Se si verifica un errore nell'apertura
		filematr_t badret;												//   ritorno una struct con ptr NULL
		badret.ptr = NULL;
		return badret;
	}
	
	filematr_t ret = {													// struct da ritornare:
		malloc(sizeof(char**)),												//   pointer alla lista di paragrafi
		0,														//   lunghezza parola più lunga (per controllo errori)
		1,														//   numero di paragrafi
		malloc(sizeof(size_t))												//   lunghezza dei diversi paragrafi
	};
	size_t linestot, n = 0, k = 0;
	char *buf;
	int par = 1;														// Nuovo paragrafo
	for(linestot = 0; getline(&buf, &n, fp) != -1; linestot++)								// Leggo la prossima riga finché non incontro EOF
		parse_line(&ret, buf, &k, &par);
	ret.nwords[ret.npars - 1] = k;												// Conteggio parole ultimo paragrafo
	free(buf);
	fclose(fp);														// Chiudo il file
	return ret;
}

