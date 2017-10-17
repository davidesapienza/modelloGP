/**Esperimento sulla robustezza delle reti genetiche per modelli geni-proteine.
 * Vengono ricercati gli attrattori della rete per perturbare stati stabili della rete, 
 * si verificano poi gli effetti su altrettanti attrattori (se l'attrattore è
 * lo stesso, oppure se si converge ad attrattori differenti). 
 * Procedimento:
 * - creazione della rete.
 * - creazione delle condizioni iniziali.
 * - evoluzione fino agli attrattori (per ogni CI).
 * - ricerca del tracciato di ogni attrattore.
 * - ogni stato usato come condizione iniziale.
 * - perturbare (perturbazioni casuali) queste nuove condizioni iniziali.
 * - recercare i nuovi attrattori.
 * - stabilire se la rete è stabile o no per ogni attrattore di convergenza.

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
const char INVOCA_RIPORTA_CICLO[]="/usr/bin/reti_gpbn/esp_riporta_ciclo";

//nomi corrispondenti ai file di lettura e scrittura per la memorizzazione del programma
const char INPUT_GS[]="input_generatore.txt";
const char INPUT_FUN[]="input_fun_bool.txt";
const char INPUT_GCI[]="input_generatore_condiz.txt";
const char INPUT_M[]="input_motore.txt";
const char INPUT_ESP[]="input_attrattori.txt";

const char OUTPUT_GS[]="output_generatore.txt";
const char OUTPUT_GCI[]="output_generatore_condiz.txt";
const char OUTPUT_M[]="output_motore.txt";
const char OUTPUT_ESP[]="output_esperimento.txt";
const char OUTPUT_ESP_COMP[]="output_esperimento_completo.txt";

const char CIperturbate[]="condiz_perturbate.txt";
const char OUTPUT_MsuCIpert[]="output_motore_suCIpertur.txt";

const char OUTPUT_M_ATTR[]="output_motore_attrattori.txt";
const char OUTPUT_RIP_CICLO[]="output_esp_riporta_ciclo.txt";
const char OUTPUT_GCI_INIZIALI[]="output_ci_iniziali.txt";
const char OUTPUT_GCI_IMPERT[]="output_ci_imperturbate.txt";

/**Struttura che rappresenta i parametri in input all'esperimento*/
typedef struct parametri parametri;
/**I parametri per l'esperimento sono:
 * - nome_serie: nome per il riconoscimento univoco del lancio. Verrà usato il 
				nome_serie per il nome della cartella e i prefissi ai nomi dei file.
 * - n_reti: indica il numero di reti necessarie per il lancio
 * - n_pert: indica il numero di perturbazioni della lista. numero di perturbazioni
 			che bisogna simulare.
 * - l_pert: lista delle perturbazioni da effettuare.
 */
struct parametri{
	char nome_serie[255];	
	int n_reti;
	int n_pert;
	int *l_pert;
};

typedef struct attrattori attrattori;
/**Struttura che memorizza gli attrattori.
 * I parametri sono:
 * - nome: matrice che contiene tutti i nomi degli attrattori.
 * - num: numero delle righe della matrice. numero degli attrattori trovati.
 * - lung: array delle lunghezze dei cicli degli attrattori trovati.
 * Questa struttura viene inizializzata ad ogni perturbazione; in quanto vengono
 * trovati potenzialmente attrattori differenti.
 */
struct attrattori{
	int **nome;
	int num;
	int *lung;
	int n_ci;
};

typedef struct risultati risultati;
/**Struttura che memorizza i risultati.
 * I parametri sono:
 * - n_si: numero di volte in cui si ritorna allo stesso attrattore;
 * - n_no: numero di volte in cui non si ritorna allo stesso attrattore;
 * - n_tot: numero totale delle CI;
 * - n_incerti: numero di volte in cui non si può dire se ritorna o no allo stesso
  				attrattore. Tipicamente non trova l'attrattore.
*/
struct risultati{
	int *n_si;
	int *n_no;
	int *n_tot;
	int *n_incerti;
};

//dichiarazione di funzioni
bool leggi_parametri(const char *in, parametri *p);
bool perturbaCI(int id);
bool converti_traiettorie_in_condizioni(const char* traie, const char* out_gci, attrattori* a);
bool scrivi_file_output(const char* out_m_cip, const char* out_esp, attrattori a, risultati* r, int id);
bool crea_file_unico(const char* out_comple, risultati r, parametri p);
static void deallocazione();

/**Funzione principale del programma.
*/
int main( int argc, char **argv){
	parametri p;
	//creaDir mantiene il comando per la creazione del nuovo esperimento
	char creaDir[50]="mkdir ";
	//creaDircorr mantiene il comando per la creazione della sotto-cartella
	char creaDircorr[50]="";
	//nome ausiliario usato per nominare i file in modo univoco
	char nome_serie_aus[255];
	int i,k;
	attrattori attr;	
	risultati ris;
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

	ris.n_si=malloc(p.n_pert*sizeof(int));
	ris.n_no=malloc(p.n_pert*sizeof(int));
	ris.n_tot=malloc(p.n_pert*sizeof(int));
	ris.n_incerti=malloc(p.n_pert*sizeof(int));
	for(i=0;i<p.n_pert;i++){
		ris.n_si[i]=0;
		ris.n_no[i]=0;
		ris.n_tot[i]=0;
		ris.n_incerti[i]=0;
		}
	//per ogni rete
	for(k=0;k<p.n_reti;k++){	
		//genera la sotto cartella con il numero della serie
		sprintf(creaDircorr,"%s/%d",creaDir,k+1);
		system(creaDircorr);
		sprintf(nome_serie_aus,"%s/%d",p.nome_serie,k+1);
		//crea una rete
		if(system(INVOCA_GS)!=0){
			printf("Errore!! esecuzione fallita di %s\n",INVOCA_GS);
			return -1;}
		//copia il file nella cartella
		if(!copia_file_loc(OUTPUT_GS, nome_serie_aus, 1, p.nome_serie, k+1)){
			printf("errore in copia_file_loc %s\n",OUTPUT_GS);
			return -1;}
		//legge la struttura della rete e la memorizza
		lettura_struttura(OUTPUT_GS);

		//crea le condizioni iniziali			
		if(system(INVOCA_GCI)!=0){
			printf("Errore!! esecuzione fallita di %s\n",INVOCA_GCI);
			return -1;}
		if(!copia_file(OUTPUT_GCI,OUTPUT_GCI_INIZIALI)){
			printf("errore in copia_file %s\n",OUTPUT_GCI);
			return -1;}
		if(!copia_file_loc(OUTPUT_GCI_INIZIALI, nome_serie_aus, 1, p.nome_serie, k+1)){
			printf("errore in copia_file_loc %s\n",OUTPUT_GCI_INIZIALI);
			return -1;}	
		if(system(INVOCA_M)!=0){
			printf("Errore!! esecuzione fallita di %s (passo iniziale)\n",INVOCA_M);
			return -1;}
		//ricava le traiettorie degli attrattori
		if(system(INVOCA_RIPORTA_CICLO)!=0){
			printf("Errore!! esecuzione fallita di %s\n",INVOCA_RIPORTA_CICLO);
			return -1;}
		if(!copia_file_loc(OUTPUT_M_ATTR, nome_serie_aus, 1, p.nome_serie, k+1)){
			printf("errore in copia_file_loc %s\n",OUTPUT_M_ATTR);
			return -1;}	
		if(!copia_file_loc(OUTPUT_RIP_CICLO, nome_serie_aus, 1, p.nome_serie, k+1)){
			printf("errore in copia_file_loc %s\n",OUTPUT_RIP_CICLO);
			return -1;}
		//converti le traiettorie in condizioni
		if(!converti_traiettorie_in_condizioni(OUTPUT_RIP_CICLO, OUTPUT_GCI, &attr)){
			printf("errore nella conversione dell'output da motore a condizioni\n");
			return -1;}
		if(!copia_file(OUTPUT_GCI,OUTPUT_GCI_IMPERT)){
			printf("errore in copia_file %s\n",OUTPUT_GCI);
			return -1;}
		if(!copia_file_loc(OUTPUT_GCI_IMPERT, nome_serie_aus, 1, p.nome_serie, k+1)){
			printf("errore in copia_file_loc %s\n",OUTPUT_GCI_IMPERT);
			return -1;}
		//se non ha trovato attrattori, allora rigenera altra rete
		if(attr.num==0){
			printf("Attenzione: non è stato trovato nessun attrattore!\n"
					"Parametri PMAX e FINMAX forse non sufficienti!\n");
			for(i=0;i<attr.num;i++)
				free(attr.nome[i]);
			free(attr.nome);
			free(attr.lung);
			k--;
			continue;}
		//per ogni perturbazione		
		for(i=0;i<p.n_pert;i++){
			//printf("sono dentro la perturbazione vale: %d\n",p.l_pert[i]);
			//il numero delle perturbazioni non può essere superiore al numero dei nodi
			if(p.l_pert[i]>s.gr.n){
				printf("ATTENZIONE NUMERO PROTEINE PERTURBATE PIÙ ALTO DI QUELLE ESISTENTI\n");
				printf("i nodi sono %d e si sta cercando di perturbarne %d\n",s.gr.n,p.l_pert[i]);
				return -1;}			
			if(!copia_file(OUTPUT_GCI_IMPERT,OUTPUT_GCI)){
				printf("errore in copia_file %s\n",OUTPUT_GCI_IMPERT);
				return -1;}	
			//perturba le CI
			if(!perturbaCI(p.l_pert[i])){
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
			if(!scrivi_file_output(OUTPUT_MsuCIpert, OUTPUT_ESP, attr, &ris, i)){
				printf("errore in scrivi file output attrattori\n");
				return -1;}
			//copia poi il file dentro la cartella
			if(!copia_file_loc(OUTPUT_ESP, nome_serie_aus, i+1, p.nome_serie, k+1)){
				printf("errore in copia_file_loc %s\n",OUTPUT_ESP);
				return -1;}
			
			}
		//dealloca gli attrattori (saranno diversi per la prossima rete)		
		for(i=0;i<attr.num;i++)
			free(attr.nome[i]);
		free(attr.nome);
		free(attr.lung);
		}
	if(!crea_file_unico(OUTPUT_ESP_COMP, ris, p)){
		printf("errore in scrivi file output attrattori unico\n");
		return -1;}
	//copia i file di input del G, funzioni booleane, GCI e M.
	if(!copia_file_loc(OUTPUT_ESP_COMP, p.nome_serie,1, p.nome_serie, 0)){
		printf("errore in copia_file_loc %s\n",OUTPUT_ESP_COMP);
		return -1;}
	
	deallocazione();	
	free(p.l_pert);
	free(ris.n_si);
	free(ris.n_no);
	free(ris.n_incerti);
	free(ris.n_tot);
	return 0;
}

/**Funzione che legge i parametri passati in input a Derrida.
 * I parametri sono il file di input all'esperimento e il riferimento alla 
 * struttura (dove memorizzare i parametri).
 * Ritorna true se va a buon fine, false altrimenti.
*/
bool leggi_parametri(const char *in, parametri *p){
	FILE *pfile;
	int i;
	pfile=fopen(in,"r");
	if(pfile==NULL){
		printf("errore apertura in lettura file!!! error is %d\n",errno);	
		return false;
		}
	fscanf(pfile,"nome serie:%s\n",p->nome_serie);
	fscanf(pfile,"numero reti:%d\n",&p->n_reti);
	fscanf(pfile,"numero perturbazioni:%d\n",&p->n_pert);
	fscanf(pfile,"lista perturbazioni:");
	p->l_pert=malloc(p->n_pert*sizeof(int));
	for(i=0;i<p->n_pert;i++)
		fscanf(pfile,"%d ",&p->l_pert[i]);
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

/**Funzione che converte il file di output di esp_riporta_ciclo in un file 
 * consono per le condizioni iniziali; dove ogni riga delle CI corrisponde ad uno
 * stato della traiettorie. Le traiettorie vengono scritte contigue.
 * I parametri in ingresso sono:
 * - traie: file delle traiettorie.
 * - out_gci: file su cui scrivere le nuove CI (ogni stato della traiettoria)
 * - a:struttura attrattore da inizializzare.
 * Ritorna true se la conversione va a buon fine, false altrimenti.
 */
bool converti_traiettorie_in_condizioni(const char * traie, const char* out_gci, attrattori* a){
	FILE *pfileR, *pfileW;
	char c;
	int i, j, aus;
	int l_attr;
	int n_ci=0;
    int n_a=0;
	int **condizioni;
	pfileR=fopen(traie,"r");
	if(pfileR==NULL){
		printf("errore apertura in lettura file!!! error is %d\n",errno);	
		return false;}
	condizioni=malloc(0*sizeof(int*));
	a->nome=malloc(0*sizeof(int*));
	a->lung=malloc(0*sizeof(int));
	do{
		//se arriva alla fine del file
		if(fscanf(pfileR,"%c",&c)==EOF)	{
			break;}
		//altrimenti torna indietro di uno
		fseek(pfileR, -1, SEEK_CUR);
		//altrimenti deve leggere:
		fscanf(pfileR,"attrattore: ");
		for(i=0;i<s.gr.n;i++)
			fscanf(pfileR,"%d ",&aus);
		fscanf(pfileR,"\ntempi di decadimento: ");
		n_a++;
		a->nome=realloc(a->nome,n_a*sizeof(int*));
		a->nome[n_a-1]=malloc(s.gr.n*sizeof(int));
		a->lung=realloc(a->lung,n_a*sizeof(int));
		for(i=0;i<s.gr.n;i++){
			fscanf(pfileR,"%d ",&aus);
			a->nome[n_a-1][i]=aus;
			}
		fscanf(pfileR,"\nlunghezza:%d\n",&l_attr);
		a->lung[n_a-1]=l_attr;
		//alloca l_attr posizioni in più
		condizioni=realloc(condizioni,(n_ci+l_attr)*sizeof(int*));		
		fscanf(pfileR,"ciclo:\n");
		//per ogni stato della traiettoria
		for(i=0;i<l_attr;i++){
			//scarta lo stato dei geni
			for(j=0;j<s.gr.n;j++)
				fscanf(pfileR,"%d ",&aus);
			condizioni[n_ci+i]=malloc(s.gr.n*sizeof(int));
			fscanf(pfileR,"\t");
			//memorizza solo le fasi
			for(j=0;j<s.gr.n;j++){
				fscanf(pfileR,"%d ",&aus);
				condizioni[n_ci+i][j]=aus;
				}
			fscanf(pfileR,"\n");
			}
		//finiti l_attr righe, rimane l'ultima corrispondente all'ultimo stato ripetuto.
		for(j=0;j<s.gr.n;j++)
			fscanf(pfileR,"%d ",&aus);
		fscanf(pfileR,"\t");
		for(j=0;j<s.gr.n;j++)
			fscanf(pfileR,"%d ",&aus);
		fscanf(pfileR,"\n\n");
		//solo adesso aggiorna n_ci
		n_ci+=l_attr;
	}while(true);
	a->num=n_a;
	a->n_ci=n_ci;
	if(fclose(pfileR)!=0){
		printf("errore in chiusura file in lettura!!! error is %d\n",errno);		
		return false;}
	pfileW=fopen(out_gci,"w");
	if(pfileW==NULL){
		printf("errore apertura in scrittura file!!! error is %d\n",errno);	
		return false;}
	fprintf(pfileW,"num. condizioni iniziali:%d\n",n_ci);
	fprintf(pfileW,"contatori ai tempi di decadimento:\n");
	for(i=0;i<n_ci;i++){
		for(j=0;j<s.gr.n;j++)
			fprintf(pfileW,"%d ",condizioni[i][j]);
		fprintf(pfileW,"\n");
		}
	if(fclose(pfileW)!=0){
		printf("errore in chiusura file in scrittura!!! error is %d\n",errno);		
		return false;}
	for(i=0;i<n_ci;i++)
		free(condizioni[i]);
	free(condizioni);	
	return true;
}

/**Funzione che scrive il file di output. Se le CI perturbate conducono allo stesso
 * attrattore, allora verrà scritto sì, altrimenti verrà scritto no.
 * La funzione prende in ingresso:
 * - il file di output del motore sulle CI perturbate.
 * - il file su cui andare a scrivere.
 * - la struttura attrattori, che contengono le informazioni per il controllo.
 * - il contatore dei sì per tale perturbazione.
 * - il contatore dei no per tale perturbazione.
 * Ritorna true se la scrittura va a buon fine, false altrimenti.
 */
bool scrivi_file_output(const char* out_m_cip, const char* out_esp, attrattori a, risultati* r, int id){
	FILE *pfileR, *pfileW;
	//n_ci mantiene il numero delle CI lette dal file del motore
	int n_ci, aus;
	//idx mantiene l'indice dell'attrattore corrente da controllare.
	int i,j, idx=0;
	//count mantiene il conteggio degli stati di un'attrattore (il suo ciclo)
	int count=0;
	//stesso indica se l'attrattore è lo stesso (true), oppure no
	bool stesso;
	//nessun mi indica se il motore non ha ritrovato nessun attrattore
	bool nessun;

	pfileR=fopen(out_m_cip,"r");
	if(pfileR==NULL){
		printf("errore apertura in lettura file!!! error is %d\n",errno);	
		return false;}
	pfileW=fopen(out_esp,"w");
	if(pfileW==NULL){
		printf("errore apertura in scrittura file!!! error is %d\n",errno);	
		return false;}
	fprintf(pfileW,"risultati di confronto per %d condizioni:\n",a.n_ci);
	fscanf(pfileR,"il numero dei nodi e':%d\n",&aus);
	fscanf(pfileR,"le condizioni iniziali sono:%d\n",&n_ci);
	r->n_tot[id]+=a.n_ci;	
	//per ogni condizione bisogna suddividere in gruppi i risultati.
	//infatti i primi a.lung[id] stati delle CI faranno parte della traiettoria dell'
	//attrattore id-esimo, e dovranno essere confrontati con il nome dell' att id-esimo.	
	for(i=0;i<n_ci;i++,count++){	
		stesso=true;
		//se tale CI non fa più parte dell'attrattore i-esimo (se eccedo la dim del suo ciclo)
		if(count==a.lung[idx]){
			//si resetta count
			count=0;
			if(idx==a.num-1){
				printf("Errore grave! id risulta essere più grande di num\n");
				return false;}
			idx++;
			}
		nessun=false;
		//deve controllare che l'attrattore della CI sia lo stesso dell id-esimo attrattore
		//scarta gli stati
		for(j=0;j<s.gr.n;j++){
			fscanf(pfileR,"%d ",&aus);
			if(aus==-1)
				nessun=true;
			}
		//solo se ha trovato l'attrattore
		if(!nessun){
			fscanf(pfileR,"\t");
			//controlla le fasi
			for(j=0;j<s.gr.n;j++){
				fscanf(pfileR,"%d ",&aus);
				if(aus!=a.nome[idx][j])
					stesso=false;
				}
			}
		//se non ha trovato l'attrattore, incremento gli incerti
		if(nessun)
			r->n_incerti[id]+=1;
		//legge la lunghezza
		fscanf(pfileR,"\t%d",&aus);

		//solo se ha trovato l'attrattore
		if(!nessun){
			//effettua un controllo in più: se le fasi combaciano ma le lunghezze no, 
			//allora non è lo stesso attrattore.
			if(stesso && a.lung[idx]!=aus)
				fprintf(pfileW,"lunghezze differenti!\n");
			else{
				//solo se è lo stesso attrattore
				if( stesso){
					fprintf(pfileW,"sì");
					r->n_si[id]+=1;
					}
				//se non è lo stesso
				else{
					fprintf(pfileW,"no");
					r->n_no[id]+=1;
					}
				}
			}
		//legge il transiente
		fscanf(pfileR,"\t%d\n",&aus);
		//scrive il transiente
		fprintf(pfileW,"\tt:%d\n",aus);
		}
	if(fclose(pfileR)!=0){
		printf("errore in chiusura file in lettura!!! error is %d\n",errno);		
		return false;}
	if(fclose(pfileW)!=0){
		printf("errore in chiusura file in scrittura!!! error is %d\n",errno);		
		return false;}
	return true;
}

/**Funzione che scrive un file complessivo di tutti i sì e no per ogni perturbazione.
 * Scrive un file con i numeri delle volte in cui si ritorna all'attrattore e il 
 * numero delle volte in cui non ci si ritorna.
 * Parametri:
 * out_comple: file di output con tutti i numeri completi
 * si: array che contiene il numero di volte che si ritorna all'attrattore
  		corrispondente.
 * no: array che contiene il numero di volte che non si ritorna all'attrattore
		corrispondente.
 * p: struttura con i parametri del programma.
 * Ritorna true se la creazione va a buon fine, false altrimenti.*/
bool crea_file_unico(const char* out_comple, risultati r, parametri p){
	FILE *pfile;
	int i;	
	pfile=fopen(out_comple,"w");
	if(pfile==NULL){
		printf("errore apertura in scrittura file!!! error is %d\n",errno);	
		return false;}
	fprintf(pfile,"risultati di confronto:\n");
	for(i=0;i<p.n_pert;i++)
		fprintf(pfile,"si:%d\tno:%d\tincerti:%d\tsu:%d CI tot\n",r.n_si[i],r.n_no[i],r.n_incerti[i],r.n_tot[i]);
	if(fclose(pfile)!=0){
		printf("errore in chiusura file in scrittura!!! error is %d\n",errno);		
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

