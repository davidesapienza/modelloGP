/**Esperimento  su reti genetiche per modelli geni-proteine.
 * Variante dell'esperimento di Derrida compost.
 * Per ogni MDT, lancia l'esperimento di Derrida con perturbazione sui geni.
 *
 *
 * In questo esperimento viene chiamato l'esperimento di Derrida sui geni semplice, 
 * andando a modificare ogni volta la memoria del sistema (MDT). Non ci sono limiti 
 * nè per numero reti generate, o condizioni, nè per MDT variabili.

author: Davide Sapienza.

*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include "../../struttura_dati.h"
#include "../../motore/struttura_motore.h"
#include "../operazioni_file.h"
#include "../funzionalit.h"

//file che non vengono più modificati dopo che l'utente gli ha settati
const char INPUT_FUN[]="input_fun_bool.txt";
const char INPUT_GCI[]="input_generatore_condiz.txt";
const char INPUT_M[]="input_motore.txt";
const char INPUT_ESP[]="input_esp_derrida_geni_composto.txt";

//file che verranno modificati durante l'esecuzione
const char INPUT_GS[]="input_generatore.txt";
const char OUTPUT_DERRIDA_GENI[]="output_esperimento.txt";

//comandi per le chiamate di system
const char invocaDerridaGeni[]="/usr/bin/reti_gpbn/esp_derrida_geni";


typedef struct parametri parametri;

/**Struttura per la memorizzazione dei parametri passati in input al programma.
 * Parametri:
 * - nome_serie: indica il nome della simulazione (usato per la creazione di files
				univoci - prefisso ai nomi dei file)
 * - n_MDT: dimensione della lista degli MDT passati in input.
 * - l_MDT: lista degli MDT per cui fare la simulazione (derrida per ognuno di essi).
 * - tempo_fisso: variabile booleana che indica se le proteine devono avere un 
				tempo di decadimento tutte uguali al MDT corrispondente (true), o 
				se invece devono avere un tdec preso con probabilità uniforme tra 
				1 ed MDT.
 */
struct parametri{
	char nome_serie[100];
	int n_MDT;
	int *l_MDT;
	bool tempo_fisso;
};

//dichiarazione di funzioni
bool leggi_input(const char *in, parametri *p);

/**Funzione principale. Operazioni:
 * -creazione cartella dell'esperimento
 * -copia dei file non modificati dal programma dentro la cartella
 * -per ogni MDT:	
 * 		-crea la sotto cartella
 * 		-modifica l'input del generatore con il nuovo MDT (può essere variabile o fisso)
 * 		-invoca Derrida e copia i risultati
*/
int main( int argc, char **argv){
	int i;
	//creaDir mantiene il comando per la creazione della nuova cartella
	char creaDir[100]="mkdir ";
	//creaDircorr mantiene il comando per la creazione della sottocartella
	char creaDircorr[100]="";
	//percorso dalla cartella corrente alla sottocartella
	char nome_serie_aus[100]="";
	parametri p;
	//nome ausiliario dove andare a memorizzare l'input del generatore prima delle	
	//modifiche da parte del programma (verrà poi ricopiato al termine
	char nome_input_aus[]="input_aus.txt";
	if(!copia_file(INPUT_GS, nome_input_aus)){
		printf("errore in copia %s per copia aus\n",INPUT_GS);
		return -1;}

	if(!leggi_input(INPUT_ESP, &p)){
		printf("errore in lettura %s\n",INPUT_ESP);
		return 1;}
	
	//crea la cartella
	strcat(creaDir,p.nome_serie);
	system(creaDir);

	//copia i file non modificati.
	if(!copia_file_loc(INPUT_FUN, p.nome_serie,1, p.nome_serie, 0)){
		printf("errore in copia_file_loc %s\n",INPUT_FUN);
		return -1;}
	if(!copia_file_loc(INPUT_GCI, p.nome_serie,1, p.nome_serie, 0)){
		printf("errore in copia_file_loc %s\n",INPUT_GCI);
		return -1;}
	if(!copia_file_loc(INPUT_M, p.nome_serie,1, p.nome_serie, 0)){
		printf("errore in copia_file_loc %s\n",INPUT_M);
		return -1;}
	if(!copia_file_loc(INPUT_ESP, p.nome_serie,1, p.nome_serie, 0)){
		printf("errore in copia_file_loc %s\n",INPUT_ESP);
		return -1;}

	//per ogni MDT specificato in input
	for(i=0;i<p.n_MDT;i++){
		//genera la sotto cartella con il tempo di decadimento
		sprintf(creaDircorr,"%s/%d",creaDir,p.l_MDT[i]);
		system(creaDircorr);
		sprintf(nome_serie_aus,"%s/%d",p.nome_serie,p.l_MDT[i]);
		//modifica l'imput del generatore andando a modificare il MDT
		modifica_input_generatore(INPUT_GS, p.tempo_fisso, p.l_MDT[i]);
		//copia il file in input_generatore appena modificato nella sottocartella
		if(!copia_file_loc(INPUT_GS, nome_serie_aus,1, p.nome_serie, p.l_MDT[i])){
			printf("errore in copia_file_loc %s\n",INPUT_GS);
			return -1;}
		//lancia Derrida
		if(system(invocaDerridaGeni)!=0){
			printf("Errore!! esecuzione fallita di %s\n",invocaDerridaGeni);
			return -1;}
		//copia solo il risultato
		if(!copia_file_loc(OUTPUT_DERRIDA_GENI, nome_serie_aus,1, p.nome_serie, p.l_MDT[i])){
			printf("errore in copia_file_loc %s\n",OUTPUT_DERRIDA_GENI);
			return -1;}
		}
	if(!copia_file(nome_input_aus, INPUT_GS)){
		printf("errore in copia %s per copia aus\n",nome_input_aus);
		return -1;}
	free(p.l_MDT);
	return 0;
}

/**Funzione che legge il file di input e memorizza le informazioni nella 
 * struttura passata come parametro
 */
bool leggi_input(const char *in, parametri *p){
	FILE *pfile;
	char c;
	int i;
	pfile=fopen(in,"r");
	if(pfile==NULL){
		printf("errore apertura in lettura file!!! error is %d\n",errno);	
		return false;
		}
	fscanf(pfile,"nome serie:%s\n",p->nome_serie);
	fscanf(pfile,"numero di MDT della lista:%d\n",&p->n_MDT);
	fscanf(pfile,"lista di MDT per cui runnare:");
	p->l_MDT=malloc(p->n_MDT*sizeof(int));
	for(i=0;i<p->n_MDT;i++)
		fscanf(pfile,"%d ",&p->l_MDT[i]);
	fscanf(pfile,"\ntempo di decadimento di tutte proteine fisso a MDT:%c\n",&c);
	//se i t_dec di tutte proteine devono essere fissi al MDT 
	if(c=='s')
		p->tempo_fisso=true;
	else if(c=='n')
		p->tempo_fisso=false;
	else{
		printf("errore in lettura modalità sui tdec!\n");
		return false;}

	if(fclose(pfile)!=0){
		printf("errore in chiusura file!!! error is %d\n",errno);		
		return false;
		}
	return true;
}
 
