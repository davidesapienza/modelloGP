/**Esperimento  su reti genetiche per modelli geni-proteine.
 * Riproduzione del grafico di fig.2 del paper "Dynamical Properties of a Boolean
 * Model of Gene Regulatory Network with Memory".
 
 * Dal file: "Struttura della rete: 
 * 100 G-P pairs, kin=2 (fisso) , random kout, bias=0.5.
 * Si considera MDT=1,...,10. Per ogni MDT si creano 100 reti. Per ogni rete 100
 * C.I. Si runna il motore in modalità ricerca attrattore (si ci ferma o quando si 
 * trova l'attrattore o quando il periodo eccede i 500 passi."
 
 * Esperimento che trova il numero medio di attrattori differenti all'aumentare
 * del MDT. Vengono lanciate diverse reti per ongi MDT, e si effettua la media
 * sul numero di attrattori trovati.

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

//variabile sistema per memorizzare la scruttura
sistema s;
sistema_motore sm;

//file che non vengono più modificati dopo che l'utente gli ha settati
const char INPUT_FUN[]="input_fun_bool.txt";
const char INPUT_GCI[]="input_generatore_condiz.txt";
const char INPUT_M[]="input_motore.txt";
const char INPUT_ESP[]="input_espfig2.txt";

//file che verranno modificati durante l'esecuzione
const char OUTPUT_ESP[]="output_espfig2.txt";
const char INPUT_GS[]="input_generatore.txt";
const char OUTPUT_GS[]="output_generatore.txt";
const char OUTPUT_GCI[]="output_generatore_condiz.txt";
const char OUTPUT_M[]="output_motore.txt";
const char OUTPUT_N_ATTR[]="output_numero_attrattori.txt";
const char OUTPUT_TOT[]="output_tutti_numeri.txt";
const char INSIEME_RETI[]="dati_reti.txt";

//parametri per le chiamate di system
const char INVOCA_LANCIO[]="/usr/bin/reti_gpbn/lancio";

typedef struct parametri parametri;
typedef struct reti reti;

/**Struttura che memorizza i parametri necessari all'esperimento.
 * nome_serie: nome per il riconoscimento univoco dei file dell'esperimento corrente.
 * n_reti: numero di reti da generare per ogni MDT
 * n_MDT: numero di MDT della lista.
 * l_MDT: lista contenente gli MDT specificati dall'utente e per i quali il programma 
 		 deve essere simulato.
 * tempo_fisso: variabile booleana che indica se il tempo di decadimento di ogni
 			    proteina è fisso a MDT o può variare tra 1 e MDT.
 */
struct parametri{
	char nome_serie[100];
	int n_reti;
	int n_MDT;
	int *l_MDT;
	bool tempo_fisso;
};

/**Struttura per la memorizzazione delle reti suddivise in tre liste in base al 
 * numero di attrattori trovati in base alle CI.
 * Parametri:
 * - reti_no:reti nelle quali per ogni C.I. non viene trovato nessun attrattore.
 * - reti_mist:reti nelle quali almeno una C.I. non conduce ad un attrattore.
 * - reti_si:reti nelle quali trova sempre l'attrattore per ongi C.I. (reti 
			effettivamente usati per il conto della media).
 * -i_no, i_mist, i_si: indici degli array sopra.
 */
struct reti{	
	int *reti_no, *reti_mist, *reti_si;
	int i_no, i_mist, i_si;
};


//dichiarazioni delle funzioni
bool leggi_input(const char *in, parametri *p);
bool crea_file_unico(const char *in, const char *out, int id);
bool calcola_medie(const char *in, const char *out, int id, int nreti);
bool scrivi_file_reti(const char *out,reti r);

/**Funzione principale. Operazoni:
 * -Creazione della cartella esperimento
 * -copia dei file non modificati dal programma dentro la cartella
 * -per ogni MDT:	
 * 		-crea la sotto cartella
 * 		-modifica l'input del generatore con il nuovo MDT
 * 		-per ogni rete:
 *			-invoca lancio (GS, GCI, M)
 * 			-ricava il numero degli attrattori 
 * 			-copia input_generatore, otuput_generatore_condiz, output_motore e
 			 il numero di attrattori nella sottocartella.
 *			-crea un file unico per tutte le reti con stesso MDT con i numeri delgi
			 attrattori
 * 		-calcola le medie sui numeri.
 * -memorizza il file delle medie nella cartella dell'esperimento
 */
int main( int argc, char **argv){
	int i, j;
	//creaDir mantiene il comando per la creazione della cartella
	char creaDir[100]="mkdir ";
	//creaDircorr mantiene il comando per la creazione della sottocartella
	char creaDircorr[100]="";
	char nome_serie_aus[100]="";
	char nome_rete[100]="";
	//moda indica in quale modalità è la rete i-esima: sempre, mai, a volte attrattori
	int moda;
	parametri p;
	reti r;
	//nome ausiliario dove andare a memorizzare l'input del generatore prima delle	
	//modifiche da parte del programma (verrà poi ricopiato al termine
	char nome_input_aus[]="input_aus.txt";
	if(!copia_file(INPUT_GS, nome_input_aus)){
		printf("errore in copia %s per copia aus\n",INPUT_GS);
		return -1;}

	if(!leggi_input(INPUT_ESP, &p)){
		printf("errore in lettura %s\n",INPUT_ESP);
		return 1;}
	
	//alloca gli array delle reti: sempre attrattori, mai, a volte.
	r.reti_no=malloc(p.n_reti*sizeof(int));
	r.reti_mist=malloc(p.n_reti*sizeof(int));
	r.reti_si=malloc(p.n_reti*sizeof(int));

	//creazione della cartella
	strcat(creaDir,p.nome_serie);
	system(creaDir);
	
	//copia nella cartella i file che non verranno modificati
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
	//per ogni tempo di decadimento specificato in input
	for(i=0;i<p.n_MDT;i++){
		//inizializza a zero gli array delle reti
		for(j=0;j<p.n_reti;j++){
			r.reti_no[j]=0;
			r.reti_mist[j]=0;
			r.reti_si[j]=0;
			}
		//e gli indici
		r.i_no=0;
		r.i_mist=0;
		r.i_si=0;

		//deve generare la sotto cartella con il tempo di decadimento
		sprintf(creaDircorr,"%s/%d",creaDir,p.l_MDT[i]);
		system(creaDircorr);
		sprintf(nome_serie_aus,"%s/%d",p.nome_serie,p.l_MDT[i]);

		//modifica l'input impostando un MDT = p.l_MDT[i]
		modifica_input_generatore(INPUT_GS, p.tempo_fisso, p.l_MDT[i]);
		//copia nella sottocartella il file input_generatore.txt appena modificato
		if(!copia_file_loc(INPUT_GS, nome_serie_aus,1, p.nome_serie, i+1)){
			printf("errore in copia_file_loc %s\n",INPUT_GS);
			return -1;}
		//una volta modificati i tempi genera le reti
		for(j=0;j<p.n_reti;j++){
			sprintf(nome_rete,"%s-%d",p.nome_serie,p.l_MDT[i]);
			//lancio invoca il generatoreS, il genertoreCI e il motore
			if(system(INVOCA_LANCIO)!=0){
				printf("Errore!! esecuzione fallita di %s\n",INVOCA_LANCIO);
				return -1;}
			//controlla se nell'output del motore ad ogni riga corrisponde ad un 
			//attrattore, se non ne trova mai, o se ne trova solo a volte.
			moda=presenza_attrattori(OUTPUT_M);
			//in base a moda, assegna la rete
			if(moda==2){
				r.reti_no[r.i_no]=j;
				r.i_no++;}
			else if(moda==1){
				r.reti_mist[r.i_mist]=j;
				r.i_mist++;}
			//solo se trova sempre gli attrattori per ogni condizione iniziale
			//allora svolge altre operazioni
			else if(moda==0){
				r.reti_si[r.i_si]=j;
				r.i_si++;
				//trova il numero di attrattori differenti
				if(!get_numero_attr()){
					printf("Errore!! get_numero_attr ha fallito\n");
					return -1;}
				//copia il file con i numeri nella sottocartella
				if(!copia_file_loc(OUTPUT_N_ATTR, nome_serie_aus,1, nome_rete, j+1)){
					printf("errore in copia_file_loc %s\n",OUTPUT_N_ATTR);
					return -1;}
				//deve accodare le medie tutte nello stesso file
				if(!crea_file_unico(OUTPUT_N_ATTR,OUTPUT_TOT,j)){
					printf("errore in crea file medie all'iterazione %d\n",j);
					return -1;}
				if(!copia_file_loc(OUTPUT_TOT, nome_serie_aus,1, p.nome_serie, p.l_MDT[i])){
					printf("errore in copia_file_loc %s\n",OUTPUT_TOT);
					return -1;}
				}
			//deve salvare tutti i file di prima : outputG outputM outputGCI, 
			//che erano stati modificati
			if(!copia_file_loc(OUTPUT_GS, nome_serie_aus,1, nome_rete, j+1)){
				printf("errore in copia_file_loc %s\n",OUTPUT_GS);
				return -1;}
			if(!copia_file_loc(OUTPUT_GCI, nome_serie_aus,1, nome_rete, j+1)){
				printf("errore in copia_file_loc %s\n",OUTPUT_GCI);
				return -1;}
			if(!copia_file_loc(OUTPUT_M, nome_serie_aus,1, nome_rete, j+1)){
				printf("errore in copia_file_loc %s\n",OUTPUT_M);
				return -1;}
			}
		//calcola tutte le medie del numero di attrattori trovati per tutte 
		//le reti i_si con stesso MDT
		if(!calcola_medie(OUTPUT_TOT,OUTPUT_ESP,i, r.i_si)){
			printf("errore in calcola_medie per l'iterazione %d\n",i);
			return -1;}
		//scrive il file con gli insiemi delle reti
		if(!scrivi_file_reti(INSIEME_RETI,r)){
			printf("errore in scrivi_file_reti\n");
			return -1;}
		if(!copia_file_loc(INSIEME_RETI, nome_serie_aus,1, p.nome_serie, p.l_MDT[i])){
			printf("errore in copia_file_loc %s\n",INSIEME_RETI);
			return -1;}
		}
	//copia il file delle medie: è un file unico che contiene tutte le medie per
	//tutti gli MDT
	if(!copia_file_loc(OUTPUT_ESP, p.nome_serie,1, p.nome_serie, 0)){
		printf("errore in copia_file_loc %s\n",OUTPUT_ESP);
		return -1;}
	//riassegno il vecchio valore dell'input del generatore
	if(!copia_file(nome_input_aus, INPUT_GS)){
		printf("errore in copia %s per copia aus\n",nome_input_aus);
		return -1;}
	//dealloca la memoria delle tre liste
	free(r.reti_no);
	free(r.reti_mist); 
	free(r.reti_si);
	free(p.l_MDT);
	return 0;
}

/**Funzione che legge il file di input e memorizza le informazioni nella 
 * struttura passata come parametro.
 * Parametri:
 * - file di input con i settaggi.
 * - riferimento alla struttura parametri.
 * Ritorna true se la lettura dei parametri va a buon fine, false altrimenti.
 */
bool leggi_input(const char *in, parametri *p){
	FILE *pfile;
	char c;
	int i;
	pfile=fopen(in,"r");
	if(pfile==NULL){
		printf("errore apertura in lettura file!!! error is %d\n",errno);	
		return false;}
	fscanf(pfile,"nome serie:%s\n",p->nome_serie);
	fscanf(pfile,"numero reti:%d\n",&p->n_reti);
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
		return false;}
	return true;
}

/**Funzione che dato il file di output dell'esp_numero_attrattori, produce un file
 * unico con tutti i numeri per tutte le reti di un dato tempo di decadimento massimo.
 * Riceve in ingresso i due file (lettura e scrittura) e l'id dell'iterazione,
 * il quale indica se bisogna sovrascrivere il file (prima volta) oppure appendere.
 * Ritorna true se la creazione avviene con successo, false altrimenti.
 */
bool crea_file_unico(const char *in, const char *out, int id){
	FILE *pfile;
	int n;

	pfile=fopen(in,"r");
	if(pfile==NULL){
		printf("errore apertura in lettura file!!! error is %d\n",errno);
		return false;}
	fscanf(pfile,"numero di attrattori trovati:%d\n",&n);
	if(fclose(pfile)!=0){
		printf("errore in chiusura file!!! error is %d\n",errno);	
		return false;}

	if(id==0){
		pfile=fopen(out,"w");
		if(pfile==NULL){
			printf("errore apertura in scrittura file!!! error is %d\n",errno);
			return false;}
		fprintf(pfile,"numero di attrattori trovati:\n");
		}
	else{
		pfile=fopen(out,"a");
		if(pfile==NULL){
			printf("errore apertura in scrittura file!!! error is %d\n",errno);
			return false;}
		}
	fprintf(pfile,"%d\n",n);
	
	if(fclose(pfile)!=0){
		printf("errore in chiusura file!!! error is %d\n",errno);	
		return false;}
	return true;
}

/**Funzione che calcola la media degli attrattori trovati.
 * Produce un file con tutte le medie, ciascuna riferita ad un dato MDT.
 * Parametri:
 * - in: file in input contenente tutti il numero di attrattori trovati per ogni 
		 rete in reti_si (reti per le quali si trova sempre l'attrattore per ogni
		 CI)
 * - out: file in qui andare a memoriazzare le medie.
 * - id: indice di iterazione (se 0 allora scrivi nuovo file - è all'inizio; se 
		 diverso da 0, allora accoda al file).
 * - nreti: numero di reti per cui dividere la somma degli attrattori trovati
			(sono le reti di reti_si, per le quali si trova sempre un attrattore).
 * Ritorna true se il calcolo va a buon fine, false altrimenti. 
 */
bool calcola_medie(const char *in, const char *out, int id, int nreti){
	FILE *pfile;
	float media=0;
	int i,val[nreti];
	float ds[nreti], ds_tot=0;

	pfile=fopen(in,"r");
	if(pfile==NULL){
		printf("errore apertura in lettura file!!! error is %d\n",errno);
		return false;}
	fscanf(pfile,"numero di attrattori trovati:\n");
	//calcola la somma degli attrattori trovati
	for(i=0;i<nreti;i++){
		fscanf(pfile,"%d\n",&val[i]);
		media+=(float)val[i];
		}
	if(fclose(pfile)!=0){
		printf("errore in chiusura file!!! error is %d\n",errno);	
		return false;}
	//se è la prima iterazione del programma, allora sovrescrive/scrive
	if(id==0){
		pfile=fopen(out,"w");
		if(pfile==NULL){
			printf("errore apertura in scrittura file!!! error is %d\n",errno);
			return false;}
		fprintf(pfile,"le medie degli attrattori trovati:\n");
		fprintf(pfile,"media\tds\tds/rad(N)\tds*3/rad(N)\n");
		}
	//altrimenti appende
	else{
		pfile=fopen(out,"a");
		if(pfile==NULL){
			printf("errore apertura in scrittura file!!! error is %d\n",errno);
			return false;}
		}
	media=media/nreti;
	fprintf(pfile,"%f\t",media);
	for(i=0;i<nreti;i++){
		ds[i]=((float)val[i])-media;
		ds[i]=(ds[i]*ds[i]);
		ds_tot+=ds[i];
		}
	fprintf(pfile,"%f\t",sqrt(ds_tot/nreti));
	fprintf(pfile,"%f\t",sqrt(ds_tot/nreti)/sqrt(nreti));
	fprintf(pfile,"%f\n",sqrt(ds_tot/nreti)*3/sqrt(nreti));
	
	if(fclose(pfile)!=0){
		printf("errore in chiusura file!!! error is %d\n",errno);	
		return false;}
	return true;
}

/**Funzione che scrive il file con le liste delle reti.
 * Il file è composto da tre liste di reti: resi_no, reti_mist, reti_si.
 * Parametri:
 * - out: file su cui andare a scrivere le liste delle reti.
 * - r: struttura contenente le tre liste.
 * Ritorna true se la scrittura va a buon fine, false altrimenti.
 */
bool scrivi_file_reti(const char *out,reti r){
	FILE *pfile;
	int i;
	pfile=fopen(out,"w");
	if(pfile==NULL){
		printf("errore apertura in lettura file!!! error is %d\n",errno);
		return false;}
	fprintf(pfile,"reti senza nessun attrattore:\n");
	for(i=0;i<r.i_no;i++)
		fprintf(pfile,"%d ",r.reti_no[i]);
	fprintf(pfile,"\nreti con almeno una CI senza attrattore:\n");
	for(i=0;i<r.i_mist;i++)
		fprintf(pfile,"%d ",r.reti_mist[i]);
	fprintf(pfile,"\nreti con attrattore per ogni CI:\n");
	for(i=0;i<r.i_si;i++)
		fprintf(pfile,"%d ",r.reti_si[i]);
	fprintf(pfile,"\n");
	if(fclose(pfile)!=0){
		printf("errore in chiusura file!!! error is %d\n",errno);	
		return false;}
	return true;
}
