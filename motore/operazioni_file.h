#include "../struttura_dati.h"
#include "struttura_motore.h"
bool lettura_file(const char *file_struct_in, const char *file_condiz, const char *file_imposta_in);
bool scrittura_file(const char *file_out_motore, bool* m_stati, int *t_dec, int id_cond);
bool copia_file(const char copia[], const char dest[]);
