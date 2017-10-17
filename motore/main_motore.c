/** Motore per reti genetiche per modelli geni-proteine.
 * Richiede come parametri 4 nomi di file.
 	- il nome del file di uscita del generatore (contiene la stuttura del grafo)
	- il nome del file con le impostazioni per il motore
	- il nome del file di uscita dove, in base alla modalità settata nel file
		precedente, riporta o tutta la traiettoria, o solo lo stato finale, o gli
		attrattori trovati. (il tutto per ogni condizione iniziale)
	- il nome del file di uscita per le statistiche (nel caso la modalità scelta 
		fosse la 3) dove memorizza gli attrattori con tutta una serie di informazioni
		utili.
 * Il programma è suddiviso in 3 file differenti: il main, il motore vero e proprio
 * e le operazioni con i file.
 *
 * Questo programma, preso in ingresso una rete di geni e proteine, fa evolvere
 * la rete fino alla soglia prestabilita per ogni modalità possibile.
 * Supporta la possibilità di passare in input diverse condizioni iniziali, e per 
 * ognuna di queste svolge n passi. Ad ogni passo, per ogni nodo, controlla la sua
 * funzione di uscita e gli ingressi, e produce il nuovo stato.

author: Davide Sapienza.

*/
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <signal.h> /* signal() */
#include "../struttura_dati.h"
#include "struttura_motore.h"
#include "operazioni_file.h"
#include "motore.h"


/* creazione della variabile sistema e sistema motore
*/
sistema s;
sistema_motore sm;

/* definizione dei nomi dei file utilizzati
*/
const char INPUT_CI[]="output_generatore_condiz.txt";
const char INPUT_STRUTTURA[]="output_generatore.txt";
const char INPUT_M[]="input_motore.txt";
const char OUTPUT_M[]="output_motore.txt";

//dichiarazione di funzioni
static void inizializzazione();
static void deallocazione();

/** Funzione principale. passi:
 - legge i file in input
 - setta le variabili
 - fa evolvere il sistema secondo la modalità scelta
 - scrive su file i risultati
*/
int main( int argc, char **argv){
	//matr_passi è una matrice di dimensione variabile (dipende dalla modalità),
	//che mantiene i passi per una certa condizione iniziale.
	//ad ogni condizione iniziale, riutilizza questa matrice
	bool *matr_passi;
	//t_dec è un matrice di interi che contiene i tempi di decadimento delle proteine.
	//viene utilizzato solo nella modalità 3 (ricerca degli attrattori)
	int *t_dec;
	int i,j,k;	//j e k servono per il debug
	//calcolo il tempo iniziale
	time_t t=time(NULL);
	if(t==-1)
		printf("errore in chiamata time\n");
	
	//vuole INPUT_STRUTTURA, INPUT_CI, INPUT_M
	if(!lettura_file(INPUT_STRUTTURA, INPUT_CI, INPUT_M)){
		printf("errore in caricamento file\n");
		return 1;
		}
	D2(printf("sm.PMAX=%d\nsm.FINMAX=%d\nsm.modalita=%d\n",sm.PMAX,sm.FINMAX,
			sm.modalita));
	
	inizializzazione();
	
	/* logica del programma */
	/*In base alla modalità letta da file mi alloco la memoria adeguata (dim 
	diverse), dopodichè per ogni condizione iniziale, mi calcolo tutti i passi 
	specificati dalla modalità, scrivo su file e riparto con una nuova condizione 
	iniziale. riutilizzo così la memoria allocata, la sovrascrivo e evito di 
	allocarne troppa.*/

	//in base ala modalità mi alloco quello di cui ho bisogno e con le giuste dimensioni
	switch(sm.modalita){
		case 1: /*per la modalità uno devo scrivere su file tutte le traiettorie
				per ogni condizione iniziale, cioè devo stampare tutti i passi per 
				tutte le condizioni iniziali.
				avrò quindi bisogno di una matrice PMAX*n.geni, e un array dei nomi
				di PMAX nomi.*/
				matr_passi=malloc(sm.PMAX*s.gr.n*sizeof(bool));
				t_dec=malloc(sm.PMAX*s.gr.n*sizeof(int));
				break;

		case 2: /*per la modalità due devo scrivere su file solo lo stato finale 
				per ogni condizione.
				avrò quindi bisogno di una matrice (in realtà array) di un solo 
				passo per il numero dei geni.
				memorizzerò anche un solo nome(solo dello stato finale)*/
				matr_passi=malloc(s.gr.n*sizeof(bool));
				t_dec=malloc(s.gr.n*sizeof(int));
				break;
		case 3: /*per la modalità tre devo scrivere su file l'attrattore trovato
				per ogni condizione iniziale. l'attrattore può essere al massimo
				di lunghezza pari a FINMAX(specificata da file).
				avrò quindi bisogno di una matrice di dimensione FINAMX*n.geni e 
				un array dei nomi di FINMAX nomi.
				In più avrò bisogno di una matrice dei tempi di decadimento di 
				FINMAX*n.geni tempi, in quanto l'attrattore lo riconosco 
				confrontando sia lo stato del gene, che il tempo di decadimento 
				della proteina*/
				matr_passi=malloc(sm.FINMAX*s.gr.n*sizeof(bool));
				t_dec=malloc(sm.FINMAX*s.gr.n*sizeof(int));
				break;
		//nel caso di modalità errata, segnalalo ed esci
		default:printf("errore modalita errata\n");
				return -1;
	}
	//per ogni condizione iniziale
	for(i=0;i<s.n_cond_ini;i++){
		//chiamo il motore
		motore(matr_passi, i, t_dec);
		D2(printf("motore ok\n"));
		//debug per la modalità 1
		if(sm.modalita==1){
			D2(printf("stampo tutti i passi di tutte le condizioni iniziali\n"));
			D2(for(j=0;j<sm.PMAX;j++){
				for(k=0;k<s.gr.n;k++)
					printf("%d ",matr_passi[j*s.gr.n+k]);
				printf("\n");
				});
			}
		//debug per la modalità 3
		if(sm.modalita==3){
			D2(printf("stampo tutti i passi di tutte le condizioni iniziali\n");
			for(j=0;j<sm.FINMAX;j++){
				for(k=0;k<s.gr.n;k++)
					printf("%d ",matr_passi[j*s.gr.n+k]);
				printf("\n");
				});
			}
		D2(printf("vado a scrivere su file\n"));
		//scrivo su file la soluzione parziale, corrispondente a questa condizione 
		//iniziale
		if(!scrittura_file(OUTPUT_M,matr_passi, t_dec, i)){
			printf("errore in scrittura file\n");
			return 1;
			}
		D2(printf("scrittura file effettuata con successo\n"));
		}
	
	deallocazione();
	free(matr_passi);
	free(t_dec);
	D1(printf("time is %f seconds\n",(float)(time(NULL)-t)));
	return 0;
}

/** Funzione di inizializzazione/allocazione degli attrattori (unica parte mancante,
 * gli altri già allocati in lettura_file).
 * Setta a zero tutti quei campi che non vengono utilizzati
 */
static void inizializzazione(){
	int i=0;
	//PMAX+1 perchè il primo è la condizione iniziale
	sm.PMAX++;
	//alloco la lista degli attrattori	
	sm.l_attr=malloc(s.n_cond_ini*sizeof(attrattore));

	//alloco anche il nome e azzero la lunghezza
	for(i=0;i<s.n_cond_ini;i++){
		sm.l_attr[i].stato=malloc(s.gr.n*sizeof(bool));
		sm.l_attr[i].tdec=malloc(s.gr.n*sizeof(int));
		sm.l_attr[i].lung=-1;
		sm.l_attr[i].l_trans=0;
		}
}

/** Funzione di deallocazione della memoria utilizzata.
 * La funzione viene chiamata dopo la scrittura(quindi memorizzazione) su file.
 */
static void deallocazione(){
	int i=0;
	//dealloco prima tutte le liste di geni e proteine
	for(i=0; i<s.gr.n; i++){
		free(s.gr.l_geni[i].lp_in);	
		free(s.gr.l_geni[i].f_out);
		}
	D2(printf("dealloco con successo 1\n"));
	//dealloco le liste del grafo
	free(s.gr.l_geni);
	free(s.gr.l_proteine);
	D2(printf("dealloco con successo 2\n"));
	//dealloco le liste dell'attrattore	
	for(i=0;i<s.n_cond_ini;i++){
		free(sm.l_attr[i].stato);
		free(sm.l_attr[i].tdec);
		}
	D2(printf("dealloco con successo 3\n"));
	//dealloco le liste dei due sistemi
	free(sm.l_attr);
	free(s.stato);
	free(s.count_tdec);
	D2(printf("dealloco con successo 4\n"));
}
