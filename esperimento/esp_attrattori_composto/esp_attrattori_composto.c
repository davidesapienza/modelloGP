/**Esperimento che studia la robustezza delle reti genetiche gpbn.
 * introduce la variazione di memoria nello studio, facendo riferimento all'
 * esp_attrattori.
 *  
 *

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
const char INPUT_ESP[]="input_attrattori_composto.txt";
const char INPUT_ESP_SEMPLICE[]="input_attrattori.txt";

//file che verranno modificati durante l'esecuzione
const char OUTPUT_ESP[]="output_attrattori_composto.txt";
const char INPUT_GS[]="input_generatore.txt";
const char OUTPUT_GS[]="output_generatore.txt";
const char OUTPUT_GCI[]="output_generatore_condiz.txt";
const char OUTPUT_M[]="output_motore.txt";
	//const char OUTPUT_EspNatt[]="output_numero_attrattori.txt";
	//const char OUTPUT_TOT[]="output_tutti_numeri.txt";
const char OUTPUT_ESP_COMP[]="output_esperimento_completo.txt";

//comandi per le chiamate di system
const char invocaDerridaAtt[]="/usr/bin/reti_gpbn/esp_attrattori";


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
 * - mem: indica se bisogna memorizzare tutte le informazioni riguardanti l'esp
	 	  sugli attrattori con memoria costante
 */
struct parametri{
	char nome_serie[100];
	int n_MDT;
	int *l_MDT;
	bool tempo_fisso;
	bool mem;
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
    char cartella_da_cop[100];
	char comando[200];
	FILE *pfile;
	parametri p;
	//nome ausiliario dove andare a memorizzare l'input del generatore prima delle	
	//modifiche da parte del programma (verrà poi ricopiato al termine
	char input_GS_aus[]="input_aus.txt";
	if(!copia_file(INPUT_GS, input_GS_aus)){
		printf("errore in copia %s per copia aus\n",INPUT_GS);
		return -1;}
	
	if(!leggi_input(INPUT_ESP, &p)){
		printf("errore in lettura file input %s\n",INPUT_ESP);
		return 1;}

	//legge il nome della serie per l'esp sugli attrattori con memoria costante
	pfile=fopen(INPUT_ESP_SEMPLICE, "r");
	if(pfile==NULL){
		printf("errore apertura in lettura file!!! error is %d\n",errno);	
		return false;
		}
	fscanf(pfile,"nome serie:%s\n",cartella_da_cop);	
	if(fclose(pfile)!=0){
		printf("errore in chiusura file!!! error is %d\n",errno);		
		return false;
		}
	
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
		if(system(invocaDerridaAtt)!=0){
			printf("Errore!! esecuzione fallita di %s\n",invocaDerridaAtt);
			return -1;}
		//copia tutti i dati se richiesto
		if(p.mem){
			//copia la cartella dentro alla sottocartella corrispondente
			sprintf(comando,"cp -r %s %s",cartella_da_cop,nome_serie_aus);
			if(system(comando)!=0){
				printf("Errore!! esecuzione fallita di %s\n",comando);
				return -1;}
			}
		//copia solo il risultato
		if(!copia_file_loc(OUTPUT_ESP_COMP, nome_serie_aus,1, p.nome_serie, p.l_MDT[i])){
			printf("errore in copia_file_loc %s\n",OUTPUT_ESP_COMP);
			return -1;}
		}
	if(!copia_file(input_GS_aus, INPUT_GS)){
		printf("errore in copia %s per copia aus\n",input_GS_aus);
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
	
	fscanf(pfile,"memorizza informazioni complete:%c\n",&c);
	//se deve memorizzare tutti i dati dell'esp attrattori semplice 
	if(c=='s')
		p->mem=true;
	else if(c=='n')
		p->mem=false;
	else{
		printf("errore in lettura modalità sulla memorizzazione!\n");
		return false;}

	if(fclose(pfile)!=0){
		printf("errore in chiusura file!!! error is %d\n",errno);		
		return false;
		}
	return true;
}
 
