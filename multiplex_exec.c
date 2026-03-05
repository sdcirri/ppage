#include <sys/wait.h>		// waitpid()
#include <string.h>		// strlen()
#include <unistd.h>		// fork(), pipe(), read() e write()
#include <stdlib.h>		// malloc(), realloc() e free()
#include <stdio.h>		// printf(), fopen() e fclose()

#include "formatter.h"
#include "file_io.h"
#include "common.h"

#define IO_CHUNKSIZE 512	// Dimensione dei chunk di dati inviati sulla pipeline tra processi

void process_chunk_lines(filematr_t *minfo, char *linebuf, size_t *linelen, size_t *wc, int *in_par) {
	char *start = linebuf, *nl;
	while ((nl = memchr(start, '\n', (linebuf + *linelen) - start)) != NULL) {
		size_t len = (size_t)(nl - start + 1);						// Include '\n'

		char saved = start[len];							// Copio riga in un buffer separato
		start[len] = 0;
		parse_line(minfo, start, wc, in_par);
		start[len] = saved;

		start = nl + 1;
	}

	size_t remain = (size_t)((linebuf + *linelen) - start);					// Sposto l'eventuale pezzo incompleto all'inizio
	memmove(linebuf, start, remain);
	*linelen = remain;
	linebuf[*linelen] = 0;
}

int multiplex_exec(char *file_in, char *file_out, size_t num_col,
		 size_t col_len, size_t col_lines, size_t col_space) {
	pid_t rpid, wpid;
	int status = -1, rpipe[2], wpipe[2];							// Exit code dei figli e pipe per la comunicazione

	pipe(rpipe);
	rpid = fork();										// Sottoprocesso per lettura
	if(rpid == -1) {
		perror("Fork fallita");
		return 2;
	}
	else if(rpid == 0) {									// Processo figlio che legge
		close(rpipe[0]);
		FILE *ip = (strcmp(file_in, "-") == 0) ? stdin : fopen(file_in, "r");		// Apro il file ("-" = stdin)
		if(ip == NULL) return 2;
		char rbuf[IO_CHUNKSIZE];
		while(fgets(rbuf, IO_CHUNKSIZE, ip))						// Leggo i dati a chunk di IO_CHUNKSIZE bytes e li invio al padre
			write(rpipe[1], rbuf, strlen(rbuf));
		if(ip != stdin) fclose(ip);
		return 0;									// Il figlio ha letto il file con successo e può uscire
	}
	else {											// Processo padre
		close(rpipe[1]);								// Devo solo leggere da rpid

		pipe(wpipe);
		wpid = fork();									// Processo figlio per la scrittura
		if(wpid == -1) {
			perror("Fork fallita");
			return 2;
		}
		else if(wpid == 0) {
			FILE *op = (strcmp(file_out, "-") == 0) ? stdout : fopen(file_out, "w+");
			if(op == NULL) return 2;
			char *wbuf = malloc(IO_CHUNKSIZE);
			close(wpipe[1]);

			ssize_t n;
			while((n = read(wpipe[0], wbuf, IO_CHUNKSIZE)) > 0)
				if(fwrite(wbuf, 1, (size_t)n, op) < (size_t)n) return 2;	// Se non scrivo tutto il chunk esco con errore di I/O
			if(op != stdout) fclose(op);
			free(wbuf);
			return 0;
		}
		else {
			filematr_t *minfo = malloc(sizeof(filematr_t));				// Preparo struttura dati per i paragrafi
			minfo->ptr = malloc(sizeof(char**));
			minfo->ptr[0] = NULL;
			minfo->npars = 1;
			minfo->maxword = 0;
			minfo->nwords = malloc(sizeof(size_t));
			minfo->nwords[0] = 0;

			char *rbuf = malloc(IO_CHUNKSIZE + 1), *linebuf = malloc(1);		// Buffer di lettura
			linebuf[0] = 0;
			size_t wc = 0;								// Contatore per le parole dei paragrafi
			int in_par = 1;								// Flag che indica se sono all'interno di un paragrafo
			ssize_t n;
			size_t linelen = 0;

			while((n = read(rpipe[0], rbuf, IO_CHUNKSIZE)) > 0) {			// Leggo la pipe a chunk di IO_CHUNKSIZE bytes
				char *tmp = realloc(linebuf, linelen + (size_t)n + 1);
	 				if (tmp == NULL) {
					perror("Errore nell'allocazione della memoria");
					free(linebuf);
					close(rpipe[0]);
					close(rpipe[1]);
					return 2;
				}
	 			linebuf = tmp;							// Aggiungo il chunk al buffer di riga
				memcpy(linebuf + linelen, rbuf, (size_t)n);
				linelen += (size_t)n;
				linebuf[linelen] = 0;

				process_chunk_lines(minfo, linebuf, &linelen, &wc, &in_par);
			}
			if(linelen > 0) parse_line(minfo, linebuf, &wc, &in_par);

			waitpid(rpid, &status, 0);						// Attendo che il figlio lettore esca
			if (WIFEXITED(status) && WEXITSTATUS(status) == 2) {			// Se il figlio restituisce 2 non ha potuto leggere il file
				perror("Non è stato possibile leggere il file");
				return 2;
			}
			minfo->nwords[minfo->npars - 1] = wc;					// Salvo le info sull'ultimo paragrafo (non necessariamente
												//   aggiornate in parse_line)
			close(rpipe[0]);							// Ho finito di leggere
			free(rbuf);								// I buffer non servono più
			free(linebuf);

			char *output = format(minfo, num_col, col_len, col_lines, col_space);	// Formatto la pagina
			if(output == NULL) {							// Errore di allocazione
				perror("Errore nell'allocazione della memoria");
				return 2;
			}
			free_fileinfo(minfo);
			free(minfo);

			size_t bytes = strlen(output);
			for(size_t i = 0; i < bytes; i += IO_CHUNKSIZE) {			// Invio output al figlio in chunk da IO_CHUNKSIZE bytes
				size_t chunk = (bytes - i < IO_CHUNKSIZE) ? (bytes - i) : IO_CHUNKSIZE;
				write(wpipe[1], output + i, chunk);
			}
			close(wpipe[1]);

			waitpid(wpid, &status, 0);						// Aspetto che il figlio esca

			free(output);
			if(strcmp(file_out, "-") != 0) fprintf(stderr, "Risultati scritti in %s\n", file_out);
			return 0;
		}
	}
}

