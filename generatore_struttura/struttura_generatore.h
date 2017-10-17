#ifndef STRUTTURA_MOTORE
#define STRUTTURA_MOTORE

/*introduco nella struttura dati la libreria stdbool così non devo inserirla
 in tutti i file
*/
#include <stdbool.h>

//typedef per aumentare la leggibilità del codice
typedef struct funzioni funzioni;
typedef struct sistema_generatore sistema_generatore;

/**Struttura per memorizzare le funzioni booleane lette da file.
 - kin -->identifica tutte quelle fun che hanno stesso num di input.
 - n_fun -->quante funzioni ci sono per stesso kin.
		per kin=2 ci sono 10 fun. so che da file ne devo leggere 10.
 - f -->array di funzioni booleane per numero kin
	(se kin=3 --> allora in f ci saranno tutte quelle fun con 3 input)
 - p --> array di interi, rappresenta la probabilità della funzione corrispondente
	(se kin=3 --> allora prob[i] sarà la probabilità dell'i-esima fun con 3 input) 
 - bias --> corrisponde al bias specificato per kin
*/
struct funzioni{
	int kin;
	int n_fun;
	bool *f;
	float *p;
	float bias;
	
};


/**Struttura che mi contiene alcune informazioni specifiche per il generatore.
 - l_fun --> lista delle funzioni che saranno passate in input
 - n_k --> indica gli elem di l_fun.
		n_k+kin_min mi da il massimo k per cui ho definito le fun_booleane 
 - bias --> bias utilizzato per quei k in cui non è specificata la funzione, o non
		è specificato altro bias.
 - la modalità di scelta della creazione dei collegamenti.
	1 utilizza kin e kmax
	2 utilizza e ed assegna i link con prob uniforme
	3 utilizza un misto: utilizza kin per avere la connessione minima per ongi gene,
		i restanti link assegnali con prob uniforme.
 - prob_kin -->
 - kmax --> massimo numero di collegamenti in ingresso (serve in modalità 2 e 3,
		per evitare di avere un 2^kin troppo grande)
*/
struct sistema_generatore{
	funzioni *l_fun;
	int n_k;
	float bias;	
	int modalita;
	float *prob_kin;
	int kmax;
};


#endif
