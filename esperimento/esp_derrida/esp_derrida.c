/**Esperimento di Derrida su reti genetiche per modelli geni-proteine.
 * Può ricevere in input struttura e condizioni iniziali della rete.
 * Genera (o legge) le condizioni iniziali, le perturba. Fa fare un passo al motore,
 * sia sulle CI imperturbate che su quelle perturbate; dopodichè calcola la 
 * distanza di Hamming tra le imperturbate e le perturbate.
 * Effettua questo prcedimento per ogni dimensione della perturbazione.
 * Come risultato si otterrà una curva.
 * Tale esperimento considera una memoria costante del sistema. 
 * Per introdurre memoria variabile si faccia riferimento all'esp_fig7.

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
#include "../operazioni_comuni_derrida.h"

//struttura della rete e struttura del motore.
sistema s;
sistema_motore sm;

//eseguibili dei programmi da richiamare
const char INVOCA_GS[]="/usr/bin/reti_gpbn/generatore_struttura";
const char INVOCA_GCI[]="/usr/bin/reti_gpbn/generatore_condizioni";
const char INVOCA_M[]="/usr/bin/reti_gpbn/motore";

//nomi corrispondenti ai file di lettura e scrittura per la memorizzazione del programma
const char INPUT_GS[]="input_generatore.txt";
const char INPUT_FUN[]="input_fun_bool.txt";
const char INPUT_GCI[]="input_generatore_condiz.txt";
const char INPUT_M[]="input_motore.txt";
const char INPUT_ESP[]="input_derrida.txt";

const char OUTPUT_GS[]="output_generatore.txt";
const char OUTPUT_GCI[]="output_generatore_condiz.txt";
const char OUTPUT_M[]="output_motore.txt";
const char OUTPUT_ESP[]="output_esperimento.txt";

const char CIperturbate[]="condiz_perturbate.txt";
const char OUTPUT_MsuCI[]="output_motore_suCI.txt";
const char OUTPUT_MsuCIpert[]="output_motore_suCIpertur.txt";
const char tuttinumeri[]="output_tuttinumero.txt";


/**Struttura che rappresenta i parametri in input all'esperimento*/
typedef struct parametri parametri;
/**I parametri per l'esperimento sono:
 * - nome_serie: nome per il riconoscimento univoco del lancio. Verrà usato il 
				nome_serie per il nome della cartella e i prefissi ai nomi dei file.
 * - n_reti: indica il numero di reti necessarie per il lancio
 * - max_p_pert: indica il numero massimo di perturbazioni da effettuare: quindi
				da 1 a max_p_pert;
 * - mod_s: indica se la struttura è passata in input al programma o se invece 
			è da generare nuova.
 * - mod_ci: indica se le CI sono passate in input al programma o se bisogna generarle.
 * - passo: indica se bisogna effettuare un passo sulle CI imperturbate per 
			riportare la condizione in uno stato appartenente al sistema.
 * - n_passi: passi iniziali da effettuare prima di andare a perturbare.
 */
struct parametri{
	char nome_serie[255];	
	int n_reti;
	int max_p_pert;
	bool mod_s;		//true:rete passata in input, false viceversa
	bool mod_ci;	//true:condizioni iniziali passati in input, false altrimenti
	bool passo;		//true:fai fare un passo alle CI prima di perturbarle, false
					//altrimenti.
	int n_passi;
};


//dichiarazione di funzioni
bool leggi_parametri(const char *in, parametri *);
bool perturbaCI(int id);
bool crea_file_media(const char* output_media_parz, const char* output_media, 
					parametri p);
static void deallocazione();

/**Funzione principale del programma.
*/
int main( int argc, char **argv){
	parametri p;
	//creaDir mantiene il comando per la creazione del nuovo esperimento
	char creaDir[50]="mkdir ";
	//creaDircorr mantiene il comando per la creazione della sotto-cartella
	char creaDircorr[50]="";
	char aus_esiste[255]="";
	//nome ausiliario usato per nominare i file in modo univoco
	char nome_serie_aus[255];
	int i,k;

	//legge il file dei parametri
	if(!leggi_parametri(INPUT_ESP, &p)){
		printf("errore in lettura parametri\n");
		return -1;}

	//creazione della cartella
	strcat(creaDir,p.nome_serie);
	system(creaDir);

	//copia i file di input del G, funzioni booleane, GCI e M.
	if(!copia_file_loc(INPUT_GS, p.nome_serie,1, p.nome_serie, 0)){
		printf("errore in copia_file_loc %s\n",INPUT_GS);
		return -1;}
	if(!copia_file_loc(INPUT_FUN, p.nome_serie, 1, p.nome_serie, 0)){
		printf("errore in copia_file_loc %s\n",INPUT_FUN);
		return -1;}
	if(!copia_file_loc(INPUT_GCI, p.nome_serie, 1, p.nome_serie, 0)){
		printf("errore in copia_file_loc %s\n",INPUT_GCI);
		return -1;}
	if(!copia_file_loc(INPUT_M, p.nome_serie, 1, p.nome_serie, 0)){
		printf("errore in copia_file_loc %s\n",INPUT_M);
		return -1;}
	if(!copia_file_loc(INPUT_ESP, p.nome_serie, 1, p.nome_serie, 0)){
		printf("errore in copia_file_loc %s\n",INPUT_ESP);
		return -1;}
	
	srand(time(NULL));

	//per ogni rete
	for(k=0;k<p.n_reti;k++){	
		//genera la sotto cartella con il numero della serie
		sprintf(creaDircorr,"%s/%d",creaDir,k+1);
		system(creaDircorr);
		sprintf(nome_serie_aus,"%s/%d",p.nome_serie,k+1);
		//se la rete non è passata in input
		if(!p.mod_s){
			//adesso può lanciare il G 
			if(system(INVOCA_GS)!=0){
				printf("Errore!! esecuzione fallita di %s\n",INVOCA_GS);
				return -1;}
			}
		//se la rete è passata in input
		//IL NOME SARÀ output_generatore%d.txt
		else{
			//copia allora il file output_generatore%d.txt in output_generatore.txt
			sprintf(aus_esiste,"%s",OUTPUT_GS);
			aus_esiste[strlen(aus_esiste)-4]='\0';
			sprintf(aus_esiste,"%s%d.txt",aus_esiste,k+1);
			if(!copia_file(aus_esiste,OUTPUT_GS)){
				printf("errore in copia_file %s\n",aus_esiste);
				return -1;}
			}	
		//copia il file nella cartella
		if(!copia_file_loc(OUTPUT_GS, nome_serie_aus, 1, p.nome_serie, k+1)){
			printf("errore in copia_file_loc %s\n",OUTPUT_GS);
			return -1;}
		//legge la struttura della rete e la memorizza
		lettura_struttura(OUTPUT_GS);
		//il numero delle perturbazioni non può essere superiore al numero dei nodi
		if(p.max_p_pert>s.gr.n){
			printf("ATTENZIONE NUMERO PROTEINE PERTURBATE PIÙ ALTO DI QUELLE ESISTENTI\n");
			p.max_p_pert=s.gr.n;}

		//per ogni perturbazione
		for(i=0;i<p.max_p_pert;i++){			
			//se non sono specificate le condizioni iniziali in input
			if(!p.mod_ci){
				//crea le condizioni iniziali
				if(system(INVOCA_GCI)!=0){
					printf("Errore!! esecuzione fallita di %s\n",INVOCA_GCI);
					return -1;}

				//verifica se deve fare un passo aggiuntivo sulle CI prima di 
				//perturbarle
				if(p.passo){
					//modifica l'input del motore (modalità 1, così da recuperare le 
					//fasi del passo precedente - quindi ricava la traiettoria).
					if(!modifica_input_motore(INPUT_M, p.n_passi, 500,1)){
						printf("errore in modifica %s\n",INPUT_M);
						return -1;}
					if(system(INVOCA_M)!=0){
						printf("Errore!! esecuzione fallita di %s (passo iniziale)\n",INVOCA_M);
						return -1;}
					if(!converti_out_motore_in_condizioni(OUTPUT_M, OUTPUT_GCI,p.n_passi)){
						printf("errore nella conversione dell'output da motore a condizioni\n");
						return -1;}
					//modifica nuovamente l'input del motore per il passo di Derrida.
					if(!modifica_input_motore(INPUT_M, 1, 500,2)){
						printf("errore in modifica %s\n",INPUT_M);
						return -1;}
					}
				}
			//se le condizioni iniziali sono specificate in input				
			//IL NOME SARÀ output_generatore_condiz%d-%d.txt con numero della rete
			//e numero del file con le condizioni
			else{
				//altrimenti copi il file output_generatore_condiz%d.txt in output_generatore_condiz.txt
				sprintf(aus_esiste,"%s",OUTPUT_GCI);
				aus_esiste[strlen(aus_esiste)-4]='\0';
				sprintf(aus_esiste,"%s%d-%d.txt",aus_esiste,k+1,i+1);
				if(!copia_file(aus_esiste,OUTPUT_GCI)){
					printf("errore in copia_file %s\n",aus_esiste);
					return -1;}
				}		
			//copia le condizioni nella cartella
			if(!copia_file_loc(OUTPUT_GCI, nome_serie_aus, i+1, p.nome_serie, k+1)){
				printf("errore in copia_file_loc %s\n",OUTPUT_GCI);
				return -1;}			
			//invoca il motore sulle CI imperturbate
			if(system(INVOCA_M)!=0){
				printf("Errore!! esecuzione fallita di %s (su CI imperturbate)\n",INVOCA_M);
				return -1;}
			//copia l'output nel file delle condizioni imperturbate
			if(!copia_file(OUTPUT_M,OUTPUT_MsuCI)){
				printf("errore in copia_file %s\n",OUTPUT_M);
				return -1;}
			//copia poi il file dentro alla cartella
			if(!copia_file_loc(OUTPUT_MsuCI, nome_serie_aus, i+1, p.nome_serie, k+1)){
				printf("errore in copia_file_loc %s\n",OUTPUT_MsuCI);
				return -1;}
			//perturba le CI
			if(!perturbaCI(i+1)){
				printf("errore in perturbaCI()\n");
				return -1;}
			//copia il file delle CI perturbate nella cartella
			if(!copia_file_loc(CIperturbate, nome_serie_aus, i+1, p.nome_serie, k+1)){
				printf("errore in copia_file_loc %s\n",CIperturbate);
				return -1;}
			//per lanciare il motore, copia le CI perturbate dentro al file 
			//output_generatore_condiz.txt
			if(!copia_file(CIperturbate,OUTPUT_GCI)){
				printf("errore in copia_file %s\n",CIperturbate);
				return -1;}
			//invoca il motore sulle condizioni perturbate
			if(system(INVOCA_M)!=0){
				printf("Errore!! esecuzione fallita di %s (su CI perturbate)\n",INVOCA_M);
				return -1;}
			//copia l'output nel file delle condizioni perturbate
			if(!copia_file(OUTPUT_M,OUTPUT_MsuCIpert)){
				printf("errore in copia_file %s\n",OUTPUT_M);
				return -1;}
			//copia poi il file dentro la cartella
			if(!copia_file_loc(OUTPUT_MsuCIpert, nome_serie_aus, i+1, p.nome_serie, k+1)){
				printf("errore in copia_file_loc %s\n",OUTPUT_MsuCIpert);
				return -1;}
			//calcola la media per tale perturbazione
			if(!calcola_mediaP(OUTPUT_MsuCI, OUTPUT_MsuCIpert, tuttinumeri, i+1)){
				printf("Errore!! calcola_mediaP ha fallito\n");
				return -1;}
			}
		//solo alla fine, copia il file dei risultati nella cartella
		if(!copia_file_loc(tuttinumeri, nome_serie_aus, 1, p.nome_serie, k+1)){
			printf("errore in copia_file_loc %s\n",tuttinumeri);
			return -1;}
		}
	//genera il file comune con tutte le medie.
	if(!crea_file_media(tuttinumeri,OUTPUT_ESP,p)){
		printf("errore in crea_file_media\n");
		return -1;}
	//copia il file nella cartella
	if(!copia_file_loc(OUTPUT_ESP, p.nome_serie, 1, p.nome_serie, 0)){
		printf("errore in copia_file_loc %s\n",OUTPUT_ESP);
		return -1;}
	deallocazione();	
	return 0;
}

/**Funzione che legge i parametri passati in input a Derrida.
 * I parametri sono il file di input all'esperimento e il riferimento alla 
 * struttura (dove memorizzare i parametri).
 * Ritorna true se va a buon fine, false altrimenti.
*/
bool leggi_parametri(const char *in, parametri *p){
	FILE *pfile;
	char c;
	pfile=fopen(in,"r");
	if(pfile==NULL){
		printf("errore apertura in lettura file!!! error is %d\n",errno);	
		return false;
		}
	fscanf(pfile,"nome serie:%s\n",p->nome_serie);
	fscanf(pfile,"numero reti:%d\n",&p->n_reti);
	fscanf(pfile,"max proteine perturbate:%d\n",&p->max_p_pert);
	fscanf(pfile,"struttura da file:%c\n",&c);
	//se la struttura è passata in input 
	if(c=='s')
		p->mod_s=true;
	else if(c=='n')
		p->mod_s=false;
	else{
		printf("errore in lettura modalita per struttura!\n");
		return false;}

	fscanf(pfile,"condizioni iniziali da file:%c\n",&c);
	//se le condizioni iniziali sono specificate in input
	if(c=='s')
		p->mod_ci=true;
	else if(c=='n')
		p->mod_ci=false;
	else{
		printf("errore in lettura modalita condizioni iniziali!\n");
		return false;}
	//parametro per verificare se bisogna fare un passo sulle CI
	fscanf(pfile,"passo aggiuntivo:%c\n",&c);
	//se bisogna fare un passo sulle CI
	if(c=='s')
		p->passo=true;
	else if(c=='n')
		p->passo=false;
	else{
		printf("errore in lettura modalita passo aggiuntivo iniziale!\n");
		return false;}
	fscanf(pfile,"numero di passi iniziali:%d\n",&p->n_passi);
	if(fclose(pfile)){
		printf("errore in chiusura file!!! error is %d\n",errno);
		return false;}
	return true;
}

/**Funzione che perturba il file delle CI.
 * Prende il parametro id rappresentante il numero di nodi da perturbare.
 * Ritorna true se è andato a buon fine, false altrimenti.
 */
bool perturbaCI(int id){
	//idx è l'indice casuale che mescola l'array di indici
	int i,j, idx;
	int aus;	
	//vettore che contiene gli indici dei nodi da perturbare
	int v_indici[s.gr.n];
	FILE *pfileR, *pfileW;
	//crea il vettore degli indici
	for(i=0;i<s.gr.n;i++)
		v_indici[i]=i;

	//apre in lettura il file outputG_CI
	pfileR=fopen(OUTPUT_GCI,"r");
	if(pfileR==NULL){
		printf("errore apertura in lettura file!!! error is %d\n",errno);	
		return false;}
	//apro in scrittura il file di CIperturbate
	pfileW=fopen(CIperturbate,"w");
	if(pfileW==NULL){
		printf("errore apertura in scrittura file!!! error is %d\n",errno);	
		return false;}

	s.stato=malloc(s.gr.n*sizeof(bool));
	fscanf(pfileR,"num. condizioni iniziali:%d\n",&s.n_cond_ini);
	fprintf(pfileW,"num. condizioni iniziali:%d\n",s.n_cond_ini);
	
	//alloca la matrice dei t_dec
	s.count_tdec=malloc(s.gr.n*sizeof(int));	

	fscanf(pfileR,"contatori ai tempi di decadimento:\n");
	fprintf(pfileW,"contatori ai tempi di decadimento:\n");

	for(i=0;i<s.n_cond_ini;i++){
		for(j=0;j<s.gr.n;j++)
			fscanf(pfileR,"%d ",&s.count_tdec[j]);
		fscanf(pfileR,"\n");
	
		//perturbazione vera e propria. Mescolo l'array per 75% degli elementi
		for(j=0;j<(3*s.gr.n/4);j++){
			idx=rand()%s.gr.n;
			aus=v_indici[j];
			v_indici[j]=v_indici[idx];
			v_indici[idx]=aus;
			}
		//per ogni j<id perturba il nodo di indice v_indici[j].
		for(j=0;j<id;j++)
			s.count_tdec[v_indici[j]]=(s.count_tdec[v_indici[j]]>0)? 
				0 : rand()%(s.gr.l_proteine[v_indici[j]].t_decadimento)+1;
		//dopodichè copia nel file una volta sola		
		for(j=0;j<s.gr.n;j++)
			fprintf(pfileW,"%d ",s.count_tdec[j]);
		fprintf(pfileW,"\n");
		}

	if(fclose(pfileR)!=0){
		printf("errore in chiusura file in lettura!!! error is %d\n",errno);		
		return false;}	
	if(fclose(pfileW)!=0){
		printf("errore in chiusura file in scrittura!!! error is %d\n",errno);		
		return false;}
	return true;
}

/**Funzione che crea il file delle medie.
 * Parametri:
 * -output_media_parz: il file che contiene le distanze di Hamming.
 * -output_media: file su cui andare a memorizzare le medie delle distanze.
    (medie delle medie).
 * -p: struttura che contiene i parametri presi in input del programma.
 */
bool crea_file_media(const char* output_media_parz, const char* output_media, parametri p){
	int i,j, aus;
	float v_media_stati[p.max_p_pert], v_media_tempi[p.max_p_pert];
	float aus_stato, aus_tempi;
	FILE *pfile;
	//costruisce il nome del file
	char file[1000]="";

	//per ogni serie
	for(i=0;i<p.max_p_pert;i++){
		v_media_stati[i]=0;
		v_media_tempi[i]=0;
		}
	for(i=0;i<p.n_reti;i++){
		//deve aprire in lettura il file della serie i-esima, che si chiama:
		//nome_serie+i+output_media
		sprintf(file,"%s/%d/%s-%d-%s",p.nome_serie,i+1,p.nome_serie,i+1,output_media_parz);
		pfile=fopen(file,"r");
		if(pfile==NULL){
			printf("errore apertura in lettura file della serie %d-esima!!!"
					" error is %d\n",i+1,errno);	
			return false;}
		//aperto il file, legge tutte le sue n_p righe
		for(j=0;j<p.max_p_pert;j++){
			fscanf(pfile,"%d\t%f\t%f\n",&aus,&aus_stato,&aus_tempi);		
			v_media_stati[j]+=aus_stato;
			v_media_tempi[j]+=aus_tempi;
			}
		if(fclose(pfile)!=0){
			printf("errore in chiusura file della serie %d-esima!!! error is %d\n",
					i+1,errno);		
			return false;}
		}
	//alla fine ha letto tutti i file, scrive su un unico file i risultati
	pfile=fopen(output_media,"w");
	if(pfile==NULL){
		printf("errore apertura in scrittura file dei risultati!!! error is %d\n",
				errno);	
		return false;}
	for(j=0;j<p.max_p_pert;j++)
			fprintf(pfile,"%d\t%f\t%f\n",j+1,v_media_stati[j]/p.n_reti,
				v_media_tempi[j]/p.n_reti);
	if(fclose(pfile)!=0){
		printf("errore in chiusura file!!! error is %d\n",errno);		
		return false;}
	return true;
}

/** Funzione di deallocazione della memoria utilizzata.
 * La funzione viene chiamata dopo la scrittura(quindi memorizzazione) su file.
 */
static void deallocazione(){
	int i=0;
	//dealloca prima tutte le liste di geni e proteine
	for(i=0; i<s.gr.n; i++){
		free(s.gr.l_geni[i].lp_in);	
		free(s.gr.l_geni[i].f_out);
		}
	D2(printf("dealloco con successo 1\n"));
	//dealloca le liste del grafo
	free(s.gr.l_geni);
	free(s.gr.l_proteine);
	D2(printf("dealloco con successo 2\n"));
	D2(printf("dealloco con successo 3\n"));
	//dealloca le liste dei due sistemi
	free(s.stato);
	free(s.count_tdec);
	D2(printf("dealloco con successo 4\n"));
}
