#include "../struttura_dati.h"
#include "../motore/struttura_motore.h"
#include "../generatore_struttura/struttura_generatore.h"
bool lettura_file(const char *file_out_motore);
bool lettura_struttura(const char *output_gen);
bool copia_file(const char sorg[], const char dest[]);
const char* crea_path_vero(char *path_rel, const char *nome_file);
bool converti_out_motore_in_condizioni(const char* m, const char* gci, int np);
bool copia_file_loc(const char file_da_copiare[], char percorso[], int id, 
					char nome_serie[], int n);
bool modifica_input_motore(const char *in_m, int p, int f, int m);
bool modifica_input_generatore(const char *file_in_grafo, bool tempo_fisso, int new_t);
