#include <sys/wait.h>		// waitpid()
#include <string.h>		// strlen()
#include <unistd.h>		// fork(), pipe(), read() e write()
#include <stdlib.h>		// malloc(), realloc() e free()
#include <stdio.h>		// printf(), fopen() e fclose()

#include "formatter.h"
#include "file_io.h"
#include "common.h"

#define IO_CHUNKSIZE 512	// Dimensione dei chunk di dati inviati sulla pipeline tra processi

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
		FILE *ip = fopen(file_in, "r");							// Apro il file
		if(ip == NULL) return 2;
		char rbuf[IO_CHUNKSIZE];
		while(fgets(rbuf, IO_CHUNKSIZE, ip))						// Leggo i dati a chunk di IO_CHUNKSIZE bytes e li invio al padre
			write(rpipe[1], rbuf, IO_CHUNKSIZE);
		fclose(ip);
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
			FILE *op = fopen(file_out, "w+");
			if(op == NULL) return 2;
			char *wbuf = malloc(IO_CHUNKSIZE);
			close(wpipe[1]);

			while(read(wpipe[0], wbuf, IO_CHUNKSIZE))
				if(fwrite(wbuf, 1, strlen(wbuf), op) < strlen(wbuf)) return 2;	// Se non scrivo tutto il chunk esco con errore di I/O
			fclose(op);
			free(wbuf);
			return 0;
		}
		else {
			filematr_t *minfo = malloc(sizeof(filematr_t));				// Preparo struttura dati per i paragrafi
			minfo->ptr = malloc(sizeof(char**));
			minfo->npars = 1;
			minfo->maxword = 0;
			minfo->nwords = malloc(sizeof(size_t));
			char *rbuf = malloc(IO_CHUNKSIZE + 1), *linebuf = malloc(1);		// Buffer di lettura
			linebuf[0] = 0;
			size_t wc = 0;								// Contatore per le parole dei paragrafi
			int in_par = 1;								// Flag che indica se sono all'interno di un paragrafo

			if(status == 2) {							// Se il figlio restituisce 2 (non ha potuto leggere il file)
				perror("Non è stato possibile leggere il file");
				return 2;
			}


			while(read(rpipe[0], rbuf, IO_CHUNKSIZE)) {				// Leggo la pipe a chunk di IO_CHUNKSIZE bytes
				rbuf[IO_CHUNKSIZE] = 0;						// Termino con NULL il chunk
				linebuf = realloc(linebuf, strlen(linebuf) + strlen(rbuf) + 1);	// Aggiungo il chunk al buffer di riga
				strncat(linebuf, rbuf, strlen(rbuf));
				if(linebuf[strlen(linebuf) - 1] == '\n') {			// Se la riga in linebuf è completa
					parse_line(minfo, linebuf, &wc, &in_par);		// La processo e aggiorno le info in minfo
					linebuf[0] = 0;						// "Azzero" la riga (basta il primo byte)
				}
			}

			waitpid(rpid, &status, 0);						// Attendo che il figlio lettore esca
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

			size_t bytes = strlen(output + 1);
			for(size_t i = 0; i < bytes; i += IO_CHUNKSIZE)				// Invio output al figlio in chunk da IO_CHUNKSIZE bytes
				write(wpipe[1], output + i, IO_CHUNKSIZE);
			close(wpipe[1]);

			waitpid(wpid, &status, 0);						// Aspetto che il figlio esca

			free(output);
			printf("Risultati scritti in %s\n", file_out);
			return 0;
		}
	}
}

