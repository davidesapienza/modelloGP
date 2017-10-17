/**Esperimento di Derrida su reti genetiche per modelli geni-proteine.
 * Questo esperimento viene effettuato sui geni. Perturbo e calcolo la distanza 
 * di Hamming sui geni (e continuo anche a calcolare la distanza di Hamming sulle P).
 * Genera le condizioni iniziali, se richiesto fa uno o più passi e poi le perturba. 
 * Fa fare un passo al motore, sia sulle CI imperturbate che su quelle perturbate; 
 * dopodichè calcola la distanza di Hamming sugli stati tra le imperturbate e le perturbate.
 * Effettua questo prcedimento per ogni dimensione della perturbazione.
 * Come risultato si otterrà una curva.
 * Tale esperimento considera una memoria costante del sistema. 
 * Per introdurre memoria variabile si faccia riferimento all'esp_derrida_geni_composto.

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
const char INPUT_ESP[]="input_derrida_geni.txt";

const char OUTPUT_GS[]="output_generatore.txt";
const char OUTPUT_GCI[]="output_generatore_condiz.txt";
const char OUTPUT_M[]="output_motore.txt";
const char OUTPUT_ESP[]="output_esperimento.txt";

const char CIperturbate[]="condiz_perturbate.txt";
const char OUTPUT_MsuCI[]="output_motore_suCI.txt";
const char OUTPUT_MsuCIpert[]="output_motore_suCIpertur.txt";
const char tuttinumeri[]="output_tuttinumero.txt";

//file di verifica
const char PRIMA_CI[]="prima_condizione.txt";


/**Struttura che rappresenta i parametri in input all'esperimento*/
typedef struct parametri parametri;
/**I parametri per l'esperimento sono:
 * - nome_serie: nome per il riconoscimento univoco del lancio. Verrà usato il 
				nome_serie per il nome della cartella e i prefissi ai nomi dei file.
 * - n_reti: indica il numero di reti necessarie per il lancio
 * - max_p_pert: indica il numero massimo di perturbazioni da effettuare: quindi
				da 1 a max_p_pert;
 * - passo: indica se il sistema deve o meno riportare la condizione iniziale ad 
 			uno stato appartenente al sistema.
 * - n_passi: passi iniziali da effettuare prima di andare a perturbare.
 */
struct parametri{
	char nome_serie[255];	
	int n_reti;
	int max_p_pert;
	bool passo;
	int n_passi;
};

//typedef aumenta la leggibilità del codice
typedef struct stato_precedente stato_precedente;

/**Struttura di appoggio per memorizzarmi fasi e stati dei geni al passo precedente
 * a quello fatto fare alle CI imperturbate prima di perturbarle.
 * Memorizzo le fasi, in modo tale poi da eventualmente utilizzarle (se gene spento).
 * Memorizzo lo stato dei geni, in modo da sapere qual'era lo stato di partenza e 
 * il successivo.
 * Campi:
 * - fasi: array che mi mantiene le fasi della CI imperturbata di partenza.
 * - stato_geni: array che mi rappresenta i geni della condizione iniziale, poi 
 	 			da perturbare.
 */
struct stato_precedente{
	int **fasi;
    int **stato_geni;
};

//dichiarazione di funzioni
bool leggi_parametri(const char *in, parametri *p);
bool perturbaCI(int id, stato_precedente prec);
bool crea_file_media(const char* output_media_parz, const char* output_media, 
					parametri p);
bool memorizza_passo_init(const char* out_m, stato_precedente *prec, parametri p);
bool memorizza_nopasso_init(const char* out_gci, stato_precedente *prec);
static void deallocazione();


/**Funzione principale del programma.
*/
int main( int argc, char **argv){
	parametri p;
	stato_precedente prec;
	//creaDir mantiene il comando per la creazione del nuovo esperimento
	char creaDir[50]="mkdir ";
	//creaDircorr mantiene il comando per la creazione della sotto-cartella
	char creaDircorr[50]="";
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
		//adesso può lanciare il G 
		if(system(INVOCA_GS)!=0){
			printf("Errore!! esecuzione fallita di %s\n",INVOCA_GS);
			return -1;}
				
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
			//crea le condizioni iniziali
			if(system(INVOCA_GCI)!=0){
				printf("Errore!! esecuzione fallita di %s\n",INVOCA_GCI);
				return -1;}
	//parte di prova
			if(!copia_file(OUTPUT_GCI,PRIMA_CI)){
				printf("errore in copia_file %s\n",OUTPUT_GCI);
				return -1;}
			if(!copia_file_loc(PRIMA_CI, nome_serie_aus, i+1, p.nome_serie, k+1)){
				printf("errore in copia_file_loc %s\n",PRIMA_CI);
				return -1;}		
	//
			if(p.passo){
				//modifica l'input del motore (modalità 1, così da recuperare le 
				//fasi del passo precedente - quindi ricava la traiettoria).
				if(!modifica_input_motore(INPUT_M, p.n_passi, 500,1)){
					printf("errore in modifica %s\n",INPUT_M);
					return -1;}
				//effettua un passo o più sulle condizioni iniziali
				if(system(INVOCA_M)!=0){
					printf("Errore!! esecuzione fallita di %s (passo iniziale)\n",INVOCA_M);
					return -1;}
				//memorizza gli stati di questo passo, e le fasi di quello prima
				if(!memorizza_passo_init(OUTPUT_M, &prec, p)){
					printf("errore nella memorizzazione dello stato precedente\n");
					return -1;}
				if(!converti_out_motore_in_condizioni(OUTPUT_M, OUTPUT_GCI, p.n_passi)){
					printf("errore nella conversione dell'output\n");
					return -1;}
				//modifica nuovamente l'input del motore per il passo di Derrida.
				if(!modifica_input_motore(INPUT_M, 1, 500,2)){
					printf("errore in modifica %s\n",INPUT_M);
					return -1;}
				}
			//parte nuova. non lancia il motore per portare le CI in uno stato
			//appartenente al sistema, ma da quelle ottenute, ricava lo stato dei geni
			else{
				//operazioni svolte al posto di memorizza_passo_init
				//memorizza gli stati di questo passo, e le fasi di quello prima
				if(!memorizza_nopasso_init(OUTPUT_GCI, &prec)){
					printf("errore nella memorizzazione dello stato precedente\n");
					return -1;}	
				}
			//copia le condizioni nella cartella
			if(!copia_file_loc(OUTPUT_GCI, nome_serie_aus, i+1, p.nome_serie, k+1)){
				printf("errore in copia_file_loc %s\n",OUTPUT_GCI);
				return -1;}			
			//invoca il motore sulle CI imperturbate
			if(system(INVOCA_M)!=0){
				printf("Errore!! esecuzione fallita di %s (CI imperturbate)\n",INVOCA_M);
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
			if(!perturbaCI(i+1, prec)){
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
				printf("Errore!! esecuzione fallita di %s (CI perturbate)\n",INVOCA_M);
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
	free(prec.fasi);
	free(prec.stato_geni);	
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
	//parametro per verificare se bisogna fare un passo sulle CI
	fscanf(pfile,"passo aggiuntivo:%c\n",&c);
	//se bisogna fare un passo sulle CI
	if(c=='s')
		p->passo=true;
	else if(c=='n')
		p->passo=false;
	else{
		printf("errore in lettura modalita passo aggiuntivo!\n");
		return false;}
	fscanf(pfile,"numero di passi iniziali:%d\n",&p->n_passi);
	if(fclose(pfile)){
		printf("errore in chiusura file!! error is %d\n",errno);
		return false;}
	return true;
}

/**Funzione che perturba il file delle CI in base ai geni.
 * Prende il parametro id rappresentante il numero di nodi da perturbare.
 * Prende lo stato al passo precedente, per imostare le corrette fasi post-perturbazione.
 * Ritorna true se è andato a buon fine, false altrimenti.
 */
bool perturbaCI(int id, stato_precedente prec){
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
		//regole di perturbazione:
		//perturbazione effettuata sui geni (controllo su prec.stato_geni)
		//se il gene j-esimo da 0 lo perturbo a 1, in ogni caso la sua P (fase 
		//della sua P) la setto al massimo t_dec.
		//se il gene j-esimo da 1 lo perturbo a 0, riprendo il vecchio valore della
		//sua P (la fase), la decremento di uno, e setto quella.

		//per ogni j<id perturba il nodo di indice v_indici[j].
		for(j=0;j<id;j++){
			if(prec.stato_geni[i][v_indici[j]]==0){
				//bisognerebbe settare lo stato del gene a 1 (ma non ho questa info)
				//e settare la fase della sua P al suo massimo tempo di decadim.
				//indipendentemente dal valore che adesso assumeva
				s.count_tdec[v_indici[j]]=s.gr.l_proteine[v_indici[j]].t_decadimento;
				}
			else{
				//bisognerebbe settare lo stato del gene a 0.
				//in questo caso bisogna recuperare il vecchio valore della fase
				//della sua proteina, decrementarlo di uno (non sarebbe dovuta essere
				//prodotta), e settare tale fase.
				s.count_tdec[v_indici[j]]=prec.fasi[i][v_indici[j]]-1;
				//controlla che non scenda sotto lo zero (min =0)
				if(s.count_tdec[v_indici[j]]<0)
					s.count_tdec[v_indici[j]]=0;
				}
			}
		//dopodichè copia nel file una volta sola		
		for(j=0;j<s.gr.n;j++)
			fprintf(pfileW,"%d ",s.count_tdec[j]);
		fprintf(pfileW,"\n");
		}

	if(fclose(pfileR)!=0){
		printf("errore in chiusura file lettura!!! error is %d\n",errno);		
		return false;}	
	if(fclose(pfileW)!=0){
		printf("errore in chiusura file scrittura!!! error is %d\n",errno);		
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
		printf("errore in chiusura file dei risultati!!! error is %d\n",errno);		
		return false;}
	return true;
}

/**Funzione che memorizza dentro la struttura prec lo stato iniziale su cui effettuare
 * la perturbazione.
 * Parametri: 
 * - output del motore dopo X passi (contiene tutte le traiettorie).
	 Lo stato di interesse, è il penultimo per le fasi, e l'ultimo per i geni.
 * Ritorna true se la memorizzazione va a buon fine, false altrimenti.
 */
bool memorizza_passo_init(const char* out_m, stato_precedente *prec, parametri p){
	FILE *pfile;
	int n_ci, aus;
	int i,j,k;
	//apre il file di output del motore contenente le traiettorie
	pfile=fopen(out_m,"r");
	if(pfile==NULL){
		printf("errore apertura in lettura file! error is %d\n",errno);	
		return false;}
	fscanf(pfile,"il numero dei nodi e':%d\n",&aus);
	fscanf(pfile,"le condizioni iniziali sono:%d\n",&n_ci);
	//alloca la matrice delle fasi	
	prec->fasi=malloc(n_ci*sizeof(int*));
	//alloca la matrice degli stati	
	prec->stato_geni=malloc(n_ci*sizeof(int*));
	//copia tutti gli stati (primo for annidato)
	//e scarta le fasi (seconfo for annidato)
	for(i=0;i<n_ci;i++){
		prec->fasi[i]=malloc(s.gr.n*sizeof(int));
		prec->stato_geni[i]=malloc(s.gr.n*sizeof(int));
		//n_passi+1 in quanto il motore aggiunge uno sempre al numero dei passi
		for(j=0;j<p.n_passi+1;j++){
			//se non è nè il penultimo, nè l'ultimo stato, allora lo scarta
			if(j<p.n_passi-1){			
				for(k=0;k<s.gr.n;k++)
					fscanf(pfile,"%d ",&aus);
				fscanf(pfile,"\t");
				for(k=0;k<s.gr.n;k++)
					fscanf(pfile,"%d ",&aus);
				fscanf(pfile,"\n");
				}
			//se invece è il penultimo passo, allora memorizza solo le fasi
			else if(j==p.n_passi-1){
				for(k=0;k<s.gr.n;k++)
					fscanf(pfile,"%d ",&aus);
				fscanf(pfile,"\t");
				for(k=0;k<s.gr.n;k++){
					fscanf(pfile,"%d ",&aus);
					prec->fasi[i][k]=aus;
					}
				fscanf(pfile,"\n");
				}
			//altrimenti (è l'ultimo) e memmorizza solo gli stati dei geni
			else{
				for(k=0;k<s.gr.n;k++){
					fscanf(pfile,"%d ",&aus);
					prec->stato_geni[i][k]=aus;
					}
				fscanf(pfile,"\t");
				for(k=0;k<s.gr.n;k++)
					fscanf(pfile,"%d ",&aus);
				fscanf(pfile,"\n");
				}
			}
		}
	if(fclose(pfile)!=0){
		printf("errore in chiusura file!!! error is %d\n",errno);		
		return false;}
	return true;
}


/**Funzione che memorizza dentro la struttura prec lo stato iniziale di partenza,
 * prima che di effettuare il passo aggiuntivo.
 * Parametri: 
 * - output del generatore delle condizioni iniziali (contiene le prime fasi).
	 In base a queste, vengono ricavati gli stati dei geni.
 * - la struttura nella quale andare a memorizzare geni e fasi.
 * Ritorna true se la memorizzazione va a buon fine, false altrimenti.
 */
bool memorizza_nopasso_init(const char* out_gci, stato_precedente *prec){
	FILE *pfile;
	int n_ci, aus;
	int i,j;
	//parte con le proteine
	pfile=fopen(out_gci,"r");
	if(pfile==NULL){
		printf("errore apertura in lettura file! error is %d\n",errno);	
		return false;}
	fscanf(pfile,"num. condizioni iniziali:%d\n",&n_ci);
	fscanf(pfile,"contatori ai tempi di decadimento:\n");
	//alloca la matrice delle fasi	
	prec->fasi=malloc(n_ci*sizeof(int*));
	//copia tutte le fasi
	for(i=0;i<n_ci;i++){
		prec->fasi[i]=malloc(s.gr.n*sizeof(int));
		for(j=0;j<s.gr.n;j++){
			fscanf(pfile,"%d ",&aus);
			prec->fasi[i][j]=aus;
			}
		fscanf(pfile,"\n");
		}
	if(fclose(pfile)!=0){
		printf("errore in chiusura file!!! error is %d\n",errno);		
		return false;}
	//ora i geni
	
	//alloca la matrice degli stati	
	prec->stato_geni=malloc(n_ci*sizeof(int*));
	//copia tutti gli stati (primo for annidato)
	//e scarta le fasi (seconfo for annidato)
	for(i=0;i<n_ci;i++){
		prec->stato_geni[i]=malloc(s.gr.n*sizeof(int));
		for(j=0;j<s.gr.n;j++){
			//lo stato del gene dipende dal valore del contatore delle proteine
			//nel file delle CI
			if(prec->fasi[i][j]<s.gr.l_proteine[j].t_decadimento)
				prec->stato_geni[i][j]=0;
			else
				prec->stato_geni[i][j]=1;
			}
		}
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
