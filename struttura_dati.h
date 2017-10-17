#ifndef STRUTTURA_DATI
#define STRUTTURA_DATI

/* introduco nella struttura dati la libreria stdbool così non devo inserirla
 in tutti i file
*/
#include <stdbool.h>

//#define DEBUG_MODE 1
#ifdef DEBUG_MODE
const unsigned int MASCHERA = 3;

#define ALTERNATIVE

#define  DBG(A, B) {if((A) & MASCHERA) { B; } }

#else
#define DBG(A, B)

#endif

#define D1(a) DBG(1, a)
#define D2(a) DBG(2, a)



/* tramite typedef rendo più leggibile il programma, e permetto a gene e proteina 
  di chiamarsi a vicenda
*/
typedef struct gene gene;
typedef struct proteina proteina;
typedef struct grafo grafo;
typedef struct sistema sistema;

/* struttura rappresentante il gene:
 - una lista delle proteine in ingresso (quelle che mi attivano).
 - un vettore di funzione di uscita in risposta alle combinazioni in ingresso.
	la posizione i-esima corrisponde al valore intero dei suoi input.
	es: input:010 --> (int)=2 --> f_out[2]=comportamento corrispondente.
 - identificatore intero incrementale, da 0 a n.geni-1
 - numero di connessioni in ingresso.
*/
struct gene {
	int *lp_in;		//lista delle proteine che lo attivano
	bool *f_out;	// funzione di uscita
	int id;			//identificatore intero
	int kin;
};

/* struttura rappresentante la proteina:
 - un intero per il tempo di decadimento che rappresenta il numero di step nei 
	quali sarà presente la proteina.
 - identificatore intero incrementale, da 0 a n. proteine-1
*/
struct proteina{
	int t_decadimento;	// numero passi in cui la proteina è attiva
	int id;				// identificatore intero	
};


/* struttura del grafo.
 - n--> numero nodi;
 - e--> numero archi;
 - kin_min e kin_max --> rappresentano le due soglie tra cui scegliere kin.
	se uguali, allora tutti i geni hanno kin=kin_min
 - tmin e tmax --> rappresentano le due sogli tra cui scegliere t_decadimento.
	se uguali, allora tutte le proteine hanno t_decadimento uguale a tmin.
 - l_geni--> lista dei geni del grafo
 - l_proteine--> lista delle proteine del grafo
*/
struct grafo{
	int n;
	int e;
	int kin_min;
	int kin_max;
	int tmin;
	int tmax;
	gene *l_geni;
	proteina *l_proteine;
};


/* struttura che mi rappresenta l'intero sistema.
 - un grafo
 - n_cond_ini --> numero condizioni iniziali che il generatore deve produrre in 
	output
 - b --> bias sulla scelta delle condizioni iniziali
 - stato --> insieme degli stati della rete
 - count_tdec --> contatori ai tempi di decadimento
*/
struct sistema{
	grafo gr;
	int n_cond_ini;
	float b;
	bool *stato;
	int *count_tdec;
};
#endif

