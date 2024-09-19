#include <stdlib.h>		// atoi() e abort()
#include <getopt.h>		// getopt_long()
#include <stdio.h>		// fprintf(), fopen() e fclose()

#include "formatter.h"
#include "file_io.h"

#include "multiplex_exec.h"
#include "uniplex_exec.h"

int file_exists(char *filen);

void print_usage(char *a0);

int main(int argc, char **argv) {
	/**
	 * Codici di uscita:
	 * 	0: esecuzione andata a buon fine
	 * 	1: errore di sinossi del comando o colonne troppo corte
	 * 	2: errore di I/O
	 */
	char *file_in = NULL, *file_out = NULL;
	size_t num_col = 0, col_len = 0, col_lines = 0, col_space = 0;
	static int ow_flag = 0, h_flag = 0, p_flag = 0;

	int c;											// Opzione trovata
	while(1) {
		static struct option options[] = {						// Opzioni da riga di comando
			{"in", required_argument, NULL, 'i'},
			{"out", required_argument, NULL, 'o'},
			{"num-col", required_argument, NULL, 'c'},
			{"col-len", required_argument, NULL, 'w'},
			{"col-lines", required_argument, NULL, 'l'},
			{"col-space", required_argument, NULL, 's'},
			{"overwrite", no_argument, &ow_flag, 'f'},
			{"help", no_argument, &h_flag, 'h'},
			{"multiplex", no_argument, &p_flag, 'p'},
			{0, 0, 0, 0}
		};
		int i = 0;									// Indice opzione analizzata
		c = getopt_long(argc, argv, "i:o:c:w:l:s:fhp", options, &i);

		if(c == -1) break;								// c == -1 indica fine opzioni
		switch(c) {
			case 0: break;								// Opzioni lunghe che settano un flag
			case 'f':
				ow_flag = 1;
				break;
			case 'h':
				h_flag = 1;
				break;
			case 'p':
				p_flag = 1;
				break;
			case 'i':
				file_in = optarg;
				break;
			case 'o':
				file_out = optarg;
				break;
			case 'c':
				num_col = atoi(optarg);
				break;
			case 'w':
				col_len = atoi(optarg);
				break;
			case 'l':
				col_lines = atoi(optarg);
				break;
			case 's':
				col_space = atoi(optarg);
				break;
			case '?':
				// Errore di sinossi
				perror("Errore nel processare le opzioni");
				print_usage(argv[0]);
				return 1;
			default:
				// Errore sconosciuto
				abort();
		}
	}

	if(h_flag) {											// Se ho passato l'opzione "--help"
		print_usage(argv[0]);									// Stampo lo usage ed esco con successo
		return 0;
	}

	if(num_col == 0 || col_len == 0 || col_lines == 0 ||						// Rileva opzioni non valide
		col_space == 0 || file_in == NULL || file_out == NULL) {
		print_usage(argv[0]);
		return 1;
	}

	if(!ow_flag && file_exists(file_out)) {
		fprintf(stderr, "Errore: file già esistente, passare\nl'opzione --overwrite per ignorare\n");
		return 2;
	}

	if(p_flag) return multiplex_exec(file_in, file_out, num_col, col_len, col_lines, col_space);	// Se è stato passata l'opzione -p seguo il programma in modalità multiprocesso
	else return uniplex_exec(file_in, file_out, num_col, col_len, col_lines, col_space);			// Altrimenti eseguo tutto il lavoro in un unico processo
}

void print_usage(char *a0) {
	/**
	 * Stampa lo usage del programma
	 */
	fprintf(stderr, "USAGE:\n\t%s [OPZIONI]\n", a0);
	fprintf(stderr, "Opzioni:\n");
	fprintf(stderr, "\t-i, --in FILENAME:  path del file di testo da formattare\n");
	fprintf(stderr, "\t-o, --out FILENAME: path del file in cui scrivere il testo formattato\n");
	fprintf(stderr, "\t-c, --num-col N:    numero di colonne per pagina\n");
	fprintf(stderr, "\t-w, --col-len N:    lunghezza colonne\n");
	fprintf(stderr, "\t-l, --col-lines N:  linee per colonna\n");
	fprintf(stderr, "\t-s, --col-space N:  numero di spazi tra le colonne\n");
	fprintf(stderr, "\t-f, --overwrite:    non uscire se file_out esiste, ma sovrascrivilo\n");
	fprintf(stderr, "\t-p, --multiplex:    esecuzione in modalità multiprocesso\n");
	fprintf(stderr, "\t-h, --help:         mostra questo messaggio di aiuto ed esce\n");
}

int file_exists(char *filen) {
	/**
	 * Controlla se esiste un file nel
	 * path filen
	 * Ritorna 1 se esiste, 0 altrimenti
	 */
	FILE *test = fopen(filen, "r");
	if(test == NULL) return 0;			// Se non riesco ad aprirlo vuol dire
							//   che non esiste/non è accessibile
	fclose(test);
	return 1;
}
