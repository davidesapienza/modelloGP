/** Generatore di condizioni iniziali per reti genetiche per modelli geni-proteine.

author: Davide Sapienza.

*/
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include "../struttura_dati.h"
#include "operazioni_file.h"

sistema s;

/* definizione dei nomi dei file utilizzati
*/
const char INPUT_GCI[]="input_generatore_condiz.txt";
const char INPUT_STRUTTURA[]="output_generatore.txt";
const char OUTPUT_GCI[]="output_generatore_condiz.txt";

static void inizializzazione();
static void deallocazione();

/**Funzione principale a cui bisogna passare tre parametri:
 - il file della struttura
 - il file di input per la generazione delle condizioni iniziali
 - il file di output delle condizioni iniziali
*/
int main( int argc, char **argv){
	
	//lettura dei 2 file di ingresso
	if(!lettura_file(INPUT_STRUTTURA, INPUT_GCI)){
		printf("errore in caricamento file\n");
		return 1;}
	inizializzazione();

	//scrittura in file di uscita
	if(!scrittura_file(OUTPUT_GCI)){
		printf("errore in scrittura file\n");
		return 1;
		}
	deallocazione();
return 0;
}

/**Funzione di inizializzazione delle informazioni necessarie per il generatore
 * delle condizioni iniziali: tempi di dec iniziali delle proteine.
 */
static void inizializzazione(){
	int i,j;
	//variabile ausiliaria che mi rappresenta lo stato della proteina
	int aus;
	D2(printf("entro ini inizializzazione\n"));
	//instanzio i tempi di decadimento (unica info utile qui)
	s.count_tdec=malloc(s.n_cond_ini*s.gr.n*sizeof(int));
	srand(time(NULL)*(unsigned int)getpid());
	for(i=0;i<s.n_cond_ini;i++){
		for(j=0;j<s.gr.n;j++){
			aus=(rand()%100<(int)(s.b*100))? 1 : 0;
			//la fase è un numero tra 1 e t_decadimento moltiplicata per lo stato della proteina i-esima
			//se lo stato = 1 allora moltiplico per uno e il risultato non cambia
			//se lo stato = 0 allora moltiplico per zero, la fase è nulla -->OK
			s.count_tdec[i*s.gr.n+j]=((rand()%s.gr.l_proteine[j].t_decadimento)+1)*aus;
			}
		}
	}

/**Funzione di deallocazione delle variabili allocate nel programma
*/
static void deallocazione(){
	free(s.count_tdec);
}

