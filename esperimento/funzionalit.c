#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include "operazioni_file.h"
#include "funzionalit.h"

sistema s;
sistema_motore sm;



static bool scrittura_file_get_info(const char *out, bool mod);
bool get_numero_attr();
bool get_periodi_attr();

static bool scrittura_file_get_transienti(const char *out);
bool get_transienti_attr();

int presenza_attrattori(const char *in);

int get_numero_punti_fissi();

/**Funzione che scrive in output le informazioni richieste analizzando i dati letti
 * mod = true --> stampa numero attrattori trovati
 * mod = false --> stampa periodo attrattori
*/
static bool scrittura_file_get_info(const char *out, bool mod){
	FILE *pfile;
	int i,j,k;
	//tempi_attr contiene i nomi degli attrattori
	int *tempi_attr;
	//id indica il numero degli attrattori attrattori:
	//mi serve sia per indirizzare correttamente tempi_attr che per tenermi il conto
	//degli attrattori trovati
	int id=0;

	pfile=fopen(out,"w");
		if(pfile==NULL){
			printf("errore apertura in scrittura file!!! error is %d\n",errno);	
			return false;
			}
	//printf("------\n%d, %d\n-------\n",s.n_cond_ini,s.gr.n);
	//scrivo il numero delle condizioni iniziali
	if(mod)
		fprintf(pfile,"numero di attrattori trovati:");
	else
		fprintf(pfile,"periodi degli attrattori trovati:\n");
	//per ogni condizione iniziale
	for(i=0;i<s.n_cond_ini;i++){
		//se lung = -1 --> l'attrattore non c'è per questa condizione iniziale, 
		//e quindi passo al successivo. Se è diversa invece..		
		if(sm.l_attr[i].lung!=-1){
			//ho trovato un attrattore
			//il primo quindi lo alloco
			if(id==0){
				//devo allocare i tempi
				tempi_attr=malloc(s.gr.n*sizeof(int));
				}
			//altrimenti devo allocarlo ancora
			else{
				//se tempi diversi --> è un nuovo attrattore
				//se tempi uguali --> questa istanza l'ho già contata (saltala)
				for(j=0;j<id;j++){
					for(k=0;k<s.gr.n;k++){
						//se trovo uno tempo diverso, allora vuol dire che è un attrattore diverso
						//esci
//DOPO SCOMMENTA		
						if(tempi_attr[j*s.gr.n+k]!=sm.l_attr[i].tdec[k])
/*VARIANTE SULLA RICERCA DEGLI STATI DEI GENI E NON DELLE FASI*/
//						if(tempi_attr[j*s.gr.n+k]!=sm.l_attr[i].stato[k])
							break;
						}
					//se k==s.gr.n vuol dire che tutti i tempi erano uguali, saltala
					if(k==s.gr.n)
						break;
					}					
				//se j!=id allora sono uscito prima e quindi vuol dire che lo 
				//stesso attrattore l'ho già scritto
				if(j!=id){
					D2(printf("ho rincontrato un vecchio nome\n"););
					continue;
					}
				//qui vuol dire che sono ad un nuovo attrattore, quindi lo alloco
				tempi_attr=realloc(tempi_attr,(id+1)*(s.gr.n)*sizeof(int));
				}
			//se arrivo qui vuol dire che ho un nuovo attrattore, e copio i tempi
			for(j=0;j<s.gr.n;j++){
//DOPO SCOMMENTA	
				tempi_attr[id*s.gr.n+j]=sm.l_attr[i].tdec[j];
//				tempi_attr[id*s.gr.n+j]=sm.l_attr[i].stato[j];			
			}
			if(!mod){					
				//stampo la lunghezza del periodo
				fprintf(pfile,"%d\n",sm.l_attr[i].lung);
				}
			//incremento il numero degli attrattori trovati
			id++;
			}
		}
		//printf("fine ciclo con successo\n");
	//alla fine scrivo il numero totale degli attrattori e il numero complessivo
	//delle condizioni iniziali coperte.
	if(mod)
		fprintf(pfile,"%d\n",id);
	//printf("scritto file con successo\n");
	if(fclose(pfile)!=0){
		printf("errore in chiusura file!!! error is %d\n",errno);		
		return false;
		}
	//printf("chiuso file con successo\n");
	D2(for(i=0;i<id;i++){
		for(j=0;j<s.gr.n;j++)
			printf("%d ",tempi_attr[i*s.gr.n+j]);
		printf("\n");
		});
	if(id!=0){
		D2(printf("ultimo elem:%d\n",tempi_attr[(id-1)*s.gr.n+(s.gr.n-1)]);
		printf("l'indirizzo è: id-1(%d)*s.gr.n(%d)+s.gr.n-1(%d) = %d\n",id-1,s.gr.n,s.gr.n-1,(id-1)*s.gr.n+(s.gr.n-1));
		);
		free(tempi_attr);
		}
	return true;
}
/**Funzione che legge l'output del motore in modalità ricerca attrattori e scrive un 
 * file con il numero di attrattori trovati
 */
bool get_numero_attr(){
	const char OUTPUT_M[]="output_motore.txt";
	const char OUTPUT_OUT[]="output_numero_attrattori.txt";
	//printf("entro get\n");
	if(!lettura_file(OUTPUT_M)){
		printf("errore in caricamento file\n");
		return false;
		}
	//printf("lettura ok\n");
	//printf("lettura n. attr fatta ok\n");
	//ho letto con successo, conto gli attrattori e scrivo su file
	if(!scrittura_file_get_info(OUTPUT_OUT,true)){
		printf("errore in scrittura\n");
		return false;
		}	
	//printf("scrittura ok\n");
	//printf("scrittura n. attr fatta ok\n");
	return true;
}
/**Funzione che legge l'output del motore in modalità ricerca attrattore e scrive un
 * file con i periodi degli attrattori della rete
 */
bool get_periodi_attr(){
	const char OUTPUT_M[]="output_motore.txt";
	const char OUTPUT_OUT[]="output_periodo_attrattori.txt";
	if(!lettura_file(OUTPUT_M)){
		printf("errore in caricamento file\n");
		return false;
		}
	//ho letto con successo, conto gli attrattori e scrivo su file
	if(!scrittura_file_get_info(OUTPUT_OUT,false)){
		printf("errore in scrittura\n");
		return false;
		}	
	return true;
}

/**Funzione che scrive in output le informazioni richieste analizzando i dati letti
*/
static bool scrittura_file_get_transienti(const char *out){
	FILE *pfile;
	int i;

	pfile=fopen(out,"w");
		if(pfile==NULL){
			printf("errore apertura in scrittura file!!! error is %d\n",errno);	
			return false;
			}
	//scrivo il numero delle condizioni iniziali
	fprintf(pfile,"transienti degli attrattori trovati:\n");
	//per ogni condizione iniziale
	for(i=0;i<s.n_cond_ini;i++){
		fprintf(pfile,"%d\n",sm.l_attr[i].l_trans);
		}

	if(fclose(pfile)!=0){
		printf("errore in chiusura file!!! error is %d\n",errno);		
		return false;
		}

	return true;
}
/**Funzione che legge l'output del motore in modalità ricerca attrattore e scrive un
 * file con le lunghezze dei transienti degli attrattori della rete
 */
bool get_transienti_attr(){
	const char OUTPUT_M[]="output_motore.txt";
	const char OUTPUT_OUT[]="output_transiente_attrattori.txt";
	if(!lettura_file(OUTPUT_M)){
		printf("errore in caricamento file\n");
		return false;
		}
	//ho letto con successo, conto gli attrattori e scrivo su file
	if(!scrittura_file_get_transienti(OUTPUT_OUT)){
		printf("errore in scrittura\n");
		return false;
		}	
	return true;
}

/**Funzione che guarda per una data rete se ci sono attrattori.
 * In particolare in ingresso prende l'output del motore (che deve essere in modalità
 * ricerca attrattore) e ritorna:
 *		-0 --> se per ogni C.I. trova un attrattore
 *		-1 --> se esiste una C.I. per cui non trova un attrattore
 * 		-2 --> se per ogni C.I. non trova un attrattore
 */
int presenza_attrattori(const char *in){
	FILE *pfile;
	int n_nodi, n_ci;
	int i,j, aus;
	//presuppongo di non trovare mai nessun attrattore
	int ret=2;	
	pfile=fopen(in,"r");
		if(pfile==NULL){
			printf("errore apertura in scrittura file!!! error is %d\n",errno);	
			return false;
			}
	fscanf(pfile,"il numero dei nodi e':%d\n",&n_nodi);
	fscanf(pfile,"le condizioni iniziali sono:%d\n",&n_ci);
	/*logica: presuppongo di non trovare mai nessun attrattore.
	se i=0 e trovo un attrattore allora entro in ret=1 cioè presuppongo che per qualcuno non lo troverò*/
	for(i=0;i<n_ci;i++){
		fscanf(pfile,"%d ",&aus);
		//se trovo l'attrattore, e c'è una sola CI, allora ritorna 0
		if(aus!=-1 && i==0 && i==n_ci-1){
			ret=0;
			break;
			}
		//se trovo l'attrattore, presupponevo di non trovarne neanche uno e i=0 allora presuppongo che per alcune CI non lo troverò
		else if(aus!=-1 && ret==2 && i==0)
			ret=1;
		//se trovo l'attrattore, presupponevo di non trovarne neanche uno ma i!=0 allora esci! per alcuni lo hai trovato e per altre CI no (ret=1)
		else if(aus!=-1 && ret==2 && i!=0){
			ret=1;
			break;
			}
		//se trovo l'attrattore, presupponevo che per almeno una CI non trovassi l'attrattore, ma sono all'ultima CI, allora ritorna 0 (lo hai trovato per tutte
		else if(aus!=-1 && ret==1 && i==n_ci-1){
			ret=0;
			break;
			}
		//se non trovo l'attrattore ed ero già con ret=1 allora esci per alcune non hai trovato l'attrattore
		else if(aus==-1 && ret==1)
			break;
		//altrimenti hai incontrato -1 ed eri con ret=2 e continua rimanendo in ret=2

		//in questo modo ho letto un numero, ne devo ancora leggere n_nodi-1
		//ne devo leggere due in più per periodo e transiente.
		for(j=0;j<n_nodi-1+2;j++)
			fscanf(pfile,"%d ",&aus);
		fscanf(pfile,"\n");
		}

	if(fclose(pfile)!=0){
		printf("errore in chiusura file!!! error is %d\n",errno);		
		return false;
		}
	return ret;
}

/**Funzione che ritorna il numero dei punti fissi di un output del motore in 
 * modalità ricerca attrattore.
 */
int get_numero_punti_fissi(){
	const char OUTPUT_M[]="output_motore.txt";
	int i;
	//numero di attrattori punti fissi trovati
	int ret=0;
	if(!lettura_file(OUTPUT_M)){
		printf("errore in caricamento file\n");
		return false;
		}
	for(i=0;i<s.n_cond_ini;i++){
		if(sm.l_attr[i].lung==1)
			ret++;
		}
	return ret;
}


