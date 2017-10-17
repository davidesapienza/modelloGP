#ifndef STRUTTURA_MOTORE
#define STRUTTURA_MOTORE

/* introduco nella struttura dati la libreria stdbool così non devo inserirla
 in tutti i file
*/
#include <stdbool.h>

typedef struct attrattore attrattore;
typedef struct sistema_motore sistema_motore;

/* struttura che rappresenta un attrattore.
  -lung indica la lunghezza del ciclo attrattore
  -stato indica lo stato (binario) dell'attrattore.
  -tdec sono i contatori ai tempi di decadimento dell'attrattore
  -l_trans è la lunghezza del transiente ( numero degli stati attraversati prima
   di entrare nell'attrattore)
*/
struct attrattore{
	int lung;
	bool *stato;
	int *tdec;
	int l_trans;
};

/* struttura che rappresenta l'intero sistema per il motore.
 - PMAX indica il numero di passi massimo
 - FINMAX
 - modalita indica la scelta effettuata dall'utente sul comportamento della rete.
			-1 Eseguire P passi e salvare le traiettorie 
			-2 Eseguire P passi e salvare il solo stato finale
			-3 Cercare l’attrattore cui conduce ogni condizione iniziale; 
 - l_attr rappresenta la lista dei possibili attrattori trovati per una stessa 
	rete. più condizioni iniziale possono portare a più attrattori.
*/
struct sistema_motore{
	int PMAX;
	int FINMAX;
	int modalita;
	attrattore *l_attr;	//alloca cond_iniz posizioni
};

#endif
