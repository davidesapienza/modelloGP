/**Esperimento su reti genetiche per modelli geni-proteine.
 * L'esperimento consiste nel ricavare i cicli attrattori, dato l'output del motore
 * in modalità ricerca attrattore.

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

sistema s;
sistema_motore sm;

//eseguibile del programma da chiamare
const char INVOCA_M[]="/usr/bin/reti_gpbn/motore";


//nomi corrispondenti ai file di lettura e scrittura per la memorizzazione del programma
const char INPUT_M[]="input_motore.txt";
const char OUTPUT_GS[]="output_generatore.txt";
const char OUTPUT_GCI[]="output_generatore_condiz.txt";
const char OUTPUT_M[]="output_motore.txt";
const char OUTPUT_M_ATTR[]="output_motore_attrattori.txt";


const char OUTPUT_ESP[]="output_esp_riporta_ciclo.txt";

//dichiarazioni di funzioni
bool scrittura_file(const char*file_out, const char*new_output_motore, int id_attr, int attr_corr);
bool scrittura_attr_in_ci(const char* file_ci, int id);
//bool crea_file_traiettorie(const char *out_m, const char *out_traiettorie, int id);


/**Funzione principale che parte dall'output del motore e per ogni attrattore trovato, 
 * imposta il file delle C.I. con lo stato e t_dec dell'attrattore, imposta PMAX
 * alla lunghezza dell'attrattore nel file di impostazione del M, e lancia il motore
 * su quella condizione/attrattore. il file di output del M viene poi copiato in 
 * un altro file, e il procedimento si ripete.
*/
int main( int argc, char **argv){
	int i,j,k;
	int *attr_incontrati;
	int n_attr_incon=0;
	FILE *pfileaus;
	char input_M_aus[]="input_ausmotore.txt";
	
	if(!lettura_file(OUTPUT_M)){
		printf("errore in lettura file %s\n",OUTPUT_M);
		return -1;}
	if(!copia_file(OUTPUT_M,OUTPUT_M_ATTR)){
		printf("errore in copia file %s\n",OUTPUT_M);
		return -1;}
	if(!copia_file(INPUT_M,input_M_aus)){
		printf("errore in copia file %s\n",INPUT_M);
		return -1;}

	/*qui, per ogni attrattore, setto i file di inpostazione, appendo al file di output il ciclo.*/

	attr_incontrati=malloc(0*sizeof(int));
	//per ogni condizione iniziale
	for(i=0;i<s.n_cond_ini;i++){
		//se l'attrattore non esiste, passa al successivo passo
		if(sm.l_attr[i].lung==-1)
			continue;
		//controlla che questo attrattore non l'abbia già incontrato
		for(j=0;j<n_attr_incon;j++){
			for(k=0;k<s.gr.n;k++){
				//se trova uno stato diverso, allora è un attrattore diverso,
				//se invece arriva ad avere k=s.gr.n, allora vuol dire che è lo stesso
				//attrattore
				if(sm.l_attr[i].tdec[k]!=attr_incontrati[j*s.gr.n+k])
					break;
				}
			if(k==s.gr.n)
				break;
			}
		//se j!=n_attr_incon vuol dire che l'attrattore corrente l'ha già trovato
		if(j!=n_attr_incon)
			continue;
		//se arriva qui vuol dire che l'attrattore non l'ha trovato.
		//devo riallocare attr_incontrati e allocare una nuova istanza (la n_attr_incon)
		attr_incontrati=realloc(attr_incontrati,(n_attr_incon+1)*s.gr.n*sizeof(int));
		//è un nuovo attrattore, quindi lo memorizza
		for(j=0;j<s.gr.n;j++)
			attr_incontrati[n_attr_incon*s.gr.n+j]=sm.l_attr[i].tdec[j];
		//scrittura dell'attrattore nuovo come condizione iniziale (singola CI)
		if(!scrittura_attr_in_ci(OUTPUT_GCI ,i)){
			printf("errore in scrittura file di scrittura_attr_in_ci\n");
			return -1;}
		//modifica dell'input del motore per espandere l'attrattore
		if(!modifica_input_motore(INPUT_M,sm.l_attr[i].lung,1,1)){
			printf("errore in modifica input motore\n");
			return -1;}

		if(system(INVOCA_M)!=0){
			printf("Errore!! esecuzione fallita di %s\n",INVOCA_M);
			return -1;}
		//copio l'output del motore in nuovo_outputM
		/*if(!crea_file_traiettorie(OUTPUT_M,OUTPUT_TRAIETTORIE, n_attr_incon)){
			printf("errore in crea file triettorie\n");
			return -1;}*/
		//scrittura del file di output dell'esperimento
		if(!scrittura_file(OUTPUT_ESP, OUTPUT_M ,i,n_attr_incon)){
			printf("errore in scrittura file %s\n",OUTPUT_ESP);
			return -1;}
		n_attr_incon++;
		}
	//dato che questo programma viene chiamato da altri, bisogna effettuare un 
	//controllo sul numero di attrattori trovati. Se il numero è 0, allora il 
	//programma non ha mai aperto e scritto il file. Per fare in modo che sia 
	//portabile la soluzione, si apre il file in scrittura e lo di schiude subito dopo.
	if(n_attr_incon == 0){
		pfileaus=fopen(OUTPUT_ESP,"w");
		if(pfileaus==NULL){
			printf("errore apertura in scrittura file!!! error is %d\n",errno);	
			return false;}
		if(fclose(pfileaus)!=0){
			printf("errore in chiusura file!!! error is %d\n",errno);		
			return false;}
		}	
	if(!copia_file(input_M_aus,INPUT_M)){
		printf("errore in copia file %s\n",input_M_aus);
		return -1;}
	//deallocazione
	free(attr_incontrati);
	return 0;
}

/**Funzione di copia del file di output del M.
 * Prende in ingresso:
 * - il file di output dell'esperimento,
 * - il file di output del motore con la traiettoria.
 * - l'indice della condizione iniziale corrispondente a questo attrattore.
 * - l'indice dell'attrattore incontrato (sarà l'ultimo incontrato, quindi il numero
   	 degli attrattori incontrati)
 * ritorna true se la scrittura e lettura vanno a buon fine, false altrimenti
*/
bool scrittura_file(const char*file_out, const char*new_output_motore, int id_attr, int attr_corr){
	FILE *pfileW, *pfileR;
	char buf[200];
	int ausn, j;

	pfileR=fopen(new_output_motore,"r");
	if(pfileR==NULL){
		printf("errore apertura in lettura file in scrittura_file() di esp_riporta_ciclo!!! error is %d\n",errno);	
		return false;}
	//se è il primo attrattore allora sovrascrivi il file
	if(attr_corr==0){
		pfileW=fopen(file_out,"w");
		if(pfileW==NULL){
			printf("errore apertura in scrittura file di esp_riporta_ciclo!!! error is %d\n",errno);	
			return false;}
		}
	//altrimenti appendi al file
	else{
		pfileW=fopen(file_out,"a");
		if(pfileW==NULL){
			printf("errore apertura in scrittura file di esp_riporta_ciclo!!! error is %d\n",errno);	
			return false;}
		}
	fscanf(pfileR,"il numero dei nodi e':%d\n",&ausn);
	fscanf(pfileR,"le condizioni iniziali sono:%d\n",&ausn);

	//scrittura dell'intestazione
	fprintf(pfileW,"attrattore: ");
	for(j=0;j<s.gr.n;j++)
		fprintf(pfileW,"%d ",sm.l_attr[id_attr].tdec[j]);
	fprintf(pfileW,"\n");
	fprintf(pfileW,"tempi di decadimento: ");
	for(j=0;j<s.gr.n;j++)
		fprintf(pfileW,"%d ",sm.l_attr[id_attr].tdec[j]);
	fprintf(pfileW,"\nlunghezza:%d\n",sm.l_attr[id_attr].lung);
	fprintf(pfileW,"ciclo:\n");
	while(1) {
	    fgets(buf, 200, pfileR);
		if( feof(pfileR) )
			break;
    	fputs(buf, pfileW);
  	
	}
	fprintf(pfileW,"\n");
	if(fclose(pfileR)!=0){
		printf("errore in chiusura file di esp_riporta_ciclo!!! error is %d\n",errno);		
		return false;}
	if(fclose(pfileW)!=0){
		printf("errore in chiusura file di esp_riporta_ciclo!!! error is %d\n",errno);		
		return false;}
	return true;
}

/**Funzione di scrittura del file delle condizioni iniziali.
 * La funzione scrive una sola CI (l'attrattore i-esimo).
 * I parametri in ingresso sono:
 * - file_ci: file di output delle condizioni iniziali, da sovrascrivere.
 * - id: indice della condizione iniziale in analisi. Per questa condizione è stato
 		 trovato un attrattore nuovo, il nome è da inserire come condizione.
 * Ritorna true se la scrittura va a buon fine, false altrimenti.
 */
bool scrittura_attr_in_ci(const char* file_ci, int id){
	FILE *pfile;
	int i;
	pfile=fopen(file_ci,"w");
	if(pfile==NULL){
		printf("errore apertura in scrittura file!!! error is %d\n",errno);	
		return false;}
	fprintf(pfile,"num. condizioni iniziali:1\n");
	fprintf(pfile,"contatori ai tempi di decadimento:\n");
	//scrive le fasi
	for(i=0;i<s.gr.n;i++)
		fprintf(pfile,"%d ",sm.l_attr[id].tdec[i]);
	fprintf(pfile,"\n");
	if(fclose(pfile)!=0){
		printf("errore in chiusura file!!! error is %d\n",errno);		
		return false;}	
	return true;
}

/**Funzione che crea un file con tutte le traiettorie di ogni attrattorie.
 * Parametri:
 * - out_m: output del motore contenente tutta la traittoria di un attrattore.
 * - out_traiettorie: file in  cui memorizzare tutte le traiettorie.
 * - id: rappresenta l'indice di scrittura. Se vale 0, allora scrive intestazione
		 ed apre il file in scrittura; altrimenti, appendi al file. (id=0 nel caso
		 del primoattrattore trovato).
 * Ritorna true se la scrittura va a buon fine, false altrimenti.
 */
/*bool crea_file_traiettorie(const char *out_m, const char *out_traiettorie, int id){
	FILE *pfileR, *pfileW;
	char buf[200];
	pfileR=fopen(out_m,"r");
	if(pfileR==NULL){
		printf("errore apertura in lettura file!!! error is %d\n",errno);	
		return false;}
	if(id==0){
		pfileW=fopen(out_traiettorie,"w");
		if(pfileW==NULL){
			printf("errore apertura in scrittura file!!! error is %d\n",errno);	
			return false;}
		fprintf(pfileW,"tutte le traiettorie degli attrattori sono:\n");
		}
	else{		
		pfileW=fopen(out_traiettorie,"a");
		if(pfileW==NULL){
			printf("errore apertura in scrittura (appende) file!!! error is %d\n",errno);	
			return false;}
		}	
	while(1) {
	    fgets(buf, 200, pfileR);
		if( feof(pfileR) )
			break;
    	fputs(buf, pfileW);
		}

	if(fclose(pfileR)!=0){
		printf("errore in chiusura file in copia_file!!! error is %d\n",errno);		
		return false;
		}
	if(fclose(pfileW)!=0){
		printf("errore in chiusura file in copia_file!!! error is %d\n",errno);		
		return false;
		}
	return true;
}*/
