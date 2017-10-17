#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include "operazioni_file.h"

extern sistema s;
extern sistema_motore sm;

bool lettura_file(const char *file_out_motore);

bool lettura_struttura(const char *output_gen);

bool converti_out_motore_in_condizioni(const char* m, const char* gci, int np);

bool copia_file_loc(const char file_da_copiare[], char percorso[], int id, 
					char nome_serie[], int n);

bool modifica_input_motore(const char *in_m, int p, int f, int m);

bool modifica_input_generatore(const char *file_in_grafo, bool tempo_fisso, int new_t);

/**Funzione che legge l'output del motore. Prende in ingresso il file con i risultati.
*/
bool lettura_file(const char *file_out_motore){
	FILE *pfile;
	int i,j,k;
	int aus;

	pfile=fopen(file_out_motore,"r");
	if(pfile==NULL){
		printf("errore apertura in scrittura file!!! error is %d\n",errno);	
		return false;
		}

	fscanf(pfile,"il numero dei nodi e':%d\n\n",&s.gr.n);
	D2(printf("ho letto n=%d\n",s.gr.n));

	//leggo le informazioni utili
					
	//SCRIVO LA MATRICE
	fscanf(pfile,"le condizioni iniziali sono:%d\n\n",&s.n_cond_ini);
	D2(printf("ho letto il n. delle cond_ini=%d\n",s.n_cond_ini));
	//alloco la lista degli attrattori	
	sm.l_attr=malloc(s.n_cond_ini*sizeof(attrattore));

	for(i=0;i<s.n_cond_ini;i++){
		fscanf(pfile,"%d ",&aus);
		if(aus!=-1){
			//se l'attrattore è stato trovato, allora alloco lo stato e i tdec
			sm.l_attr[i].stato=malloc(s.gr.n*sizeof(bool));
			sm.l_attr[i].tdec=malloc(s.gr.n*sizeof(int));
			//ho però gia letto il primo stato, quindi lo metto in stato
			sm.l_attr[i].stato[0]=(aus==1)? 1 : 0;
			//scrivo i rimanenti stati
			for(k=1;k<s.gr.n;k++){
				fscanf(pfile,"%d ",&aus);
				sm.l_attr[i].stato[k]=(aus==1)? 1 : 0;
				}
			//prima di andare a capo leggo i tempi di decadimento
			fscanf(pfile,"\t");
			for(k=0;k<s.gr.n;k++)
				fscanf(pfile,"%d ",&sm.l_attr[i].tdec[k]);
			//prima di andare a capo scrivo la lunghezza e la lunghezza del transiente
			fscanf(pfile,"\t%d\t%d\n",&sm.l_attr[i].lung,&sm.l_attr[i].l_trans);

			}
		//altrimenti leggi -1 -1 -1...
		else{
			sm.l_attr[i].lung=-1;
			sm.l_attr[i].stato=NULL;
			//leggo tutti i -1 tranne uno che l'ho già letto
			for(k=1;k<s.gr.n;k++)
				fscanf(pfile,"%d ",&aus);
			fscanf(pfile,"\t%d\t%d\n",&aus,&sm.l_attr[i].l_trans);
			}
	}
	if(fclose(pfile)!=0){
		printf("errore in chiusura file!!! error is %d\n",errno);		
		return false;
		}	
	return true;
}

/** Funzione che legge da file la struttura della rete.
 * Ritorna un booleano, true se la lettura è andata a buon fine, false altrimenti. 
 */
bool lettura_struttura(const char *output_gen){
	FILE *pfile;
	int i,j,aus;

	pfile=fopen(output_gen,"r");
    if(pfile==NULL){
		printf("errore apertura in lettura file!!! error is %d\n",errno);	
		return false;}
	fscanf(pfile,"n:%d\ne:%d\nkin_min:%d\nkin_max:%d\n",&s.gr.n,&s.gr.e,
								&s.gr.kin_min,&s.gr.kin_max);
	fscanf(pfile,"tmin:%d\ntmax:%d\n",&s.gr.tmin,&s.gr.tmax);		
	s.gr.l_geni=malloc(s.gr.n*sizeof(gene));
	s.gr.l_proteine=malloc(s.gr.n*sizeof(proteina));
	//prima memorizza tutti i geni
	fscanf(pfile,"l_geni:\n");
	for(i=0;i<s.gr.n;i++){
		//id
		fscanf(pfile,"id:%d\n",&s.gr.l_geni[i].id);
		//kin	
		fscanf(pfile,"kin:%d\n",&s.gr.l_geni[i].kin);
		//*lp_in
		fscanf(pfile,"lp_in:");
		s.gr.l_geni[i].lp_in=malloc(s.gr.l_geni[i].kin*sizeof(int));
		for(j=0;j<s.gr.l_geni[i].kin;j++)
			fscanf(pfile,"%d ",&s.gr.l_geni[i].lp_in[j]);
		//*f_out
		fscanf(pfile,"f_out:");
		s.gr.l_geni[i].f_out=malloc((1<<s.gr.l_geni[i].kin) *sizeof(bool));
		for(j=0;j<(1<<(s.gr.l_geni[i].kin));j++){
			fscanf(pfile,"%d ",&aus);
			s.gr.l_geni[i].f_out[j]=(aus==1)? 1 : 0;}			
		fscanf(pfile,"\n");
		}	
	D2(printf("geni scritti\n"));

	//memorizza le proteine
	fscanf(pfile,"l_proteine:\n");
	for(i=0;i<s.gr.n;i++){	
		//id
		fscanf(pfile,"id:%d\n",&s.gr.l_proteine[i].id);
		//t_decadimento	
		fscanf(pfile,"t_decadimento:%d\n",&s.gr.l_proteine[i].t_decadimento);	
		}	
	D2(printf("proteine scritte\n"));
    if(fclose(pfile)!=0){
		printf("errore in chiusura file!!! error is %d\n",errno);	
		perror("chiusura");	
		return false;}
	return true;
}

/**Funzione di copia del file sorgente nel file di destinazione.
 */
bool copia_file(const char sorg[], const char dest[]){
	FILE *pfileW, *pfileR;
	char buf[200];
	pfileR=fopen(sorg,"r");
	if(pfileR==NULL){
		printf("errore apertura in lettura file in copia_file!!! error is %d\n",errno);	
		return false;
		}
	pfileW=fopen(dest,"w");
	if(pfileW==NULL){
		printf("errore apertura in scrittura file in copia_file!!! error is %d\n",errno);	
		return false;
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
}

/**Funzione che compone il path relativo corretto dipendente dal chiamante e dal
 * file che si vuole accedere
*/
const char* crea_path_vero(char *path_rel, const char *nome_file){
	char *s=malloc((strlen(path_rel)+strlen(nome_file))*sizeof(char));
	sprintf(s,"%s%s",path_rel,nome_file);
	printf("sono dentro crea_path_vero e s=%s\n",s);
	return s;
	
}

/**Funzione che converte l'output del motore nel file dell'output del generatore
 * delle condizioni iniziali. 
 * Prende in ingresso i nomi dei due file, e il numero di passi della traiettoria.
 * Deve saltare i primi passi per ogni CI, fino ad arrivare all'ultimo: questo 
 * sarà la lo stato giusto da convertire in condizione iniziale
 * Ritorna true se la conversione va a buon fine, false altrimenti.
 */
bool converti_out_motore_in_condizioni(const char* m, const char* gci, int np){
	FILE *pfileR, *pfileW;
	int aus, i, j, k, n_ci;
	//apre il lettura il file m
	pfileR=fopen(m,"r");
	if(pfileR==NULL){
		printf("errore apertura in lettura file motore!!! error is %d\n",errno);	
		return false;}
	//apre in scrittura il file di gci
	pfileW=fopen(gci,"w");
	if(pfileW==NULL){
		printf("errore apertura in scrittura file output_gci!!! error is %d\n",errno);	
		return false;}
	fscanf(pfileR,"il numero dei nodi e':%d\n",&aus);
	fscanf(pfileR,"le condizioni iniziali sono:%d\n",&n_ci);
	fprintf(pfileW,"num. condizioni iniziali:%d\n",n_ci);
	fprintf(pfileW,"contatori ai tempi di decadimento:\n");
	//per ogni condizione iniziale
	for(i=0;i<n_ci;i++){
		//i passi effettivi del file di output del motore sono in realtà np+1.
		//Questo perchè il motore aggiunge sempre un passo in più rispetto a quelli 
		//specificati: quello per la condizione iniziale.

		//per np passi quindi scarta le informazioni
		for(j=0;j<np;j++){	
			for(k=0;k<s.gr.n;k++)
				fscanf(pfileR,"%d ",&aus);
			fscanf(pfileR,"\t");
			for(k=0;k<s.gr.n;k++)
				fscanf(pfileR,"%d ",&aus);
			fscanf(pfileR,"\n");
			}
		//arriva all'ultimo stato per la i-esima condizione, e memorizza questo.
		for(k=0;k<s.gr.n;k++)
			fscanf(pfileR,"%d ",&aus);
		fscanf(pfileR,"\t");
		for(k=0;k<s.gr.n;k++){
			fscanf(pfileR,"%d ",&aus);
			fprintf(pfileW,"%d ",aus);
			}
		fscanf(pfileR,"\n");
		fprintf(pfileW,"\n");
		}
		//recupera solo per l'ultimo stato: è la condizione iniziale da utilizzare.
	if(fclose(pfileR)!=0){
		printf("errore in chiusura file di perturbaCI()!!! error is %d\n",errno);		
		return false;}	
	if(fclose(pfileW)!=0){
		printf("errore in chiusura file di perturbaCI()!!! error is %d\n",errno);		
		return false;}
	return true;
}

/**Funzione di copia dei file passati in input all'interno della cartella 
 * specificata. All'interno di quest'ultima il file avrà lo stesso nome.
 * I parametri sono:
 * - il nome del file da copiare nella cartella
 * - il percorso dove memorizzare il file
 * - id rappresenta l'iterazione di copia. Se id==0 allora è la prima volta che viene
	 effettuta la copia, quindi apri il file in scrittura (sovreascrivi- scrivi all'
	 inizio). Se invece id!=0 allora appende al file, perchè la copia è già iniziata.
 * - nome_serie rappresenta il prefisso da aggiungere per identificare il file 
  	 in maniera univoca rispetto ad altri lanci dello stesso esperimento.
 * - n rappresenta il suffisso da aggiungere al nome del file (tipicamente per 
	 identificare diverse reti).
 * Ritorna true se la copia va a buon fine, false altrimenti.
 */
bool copia_file_loc(const char file_da_copiare[], char percorso[], int id, char nome_serie[], int n){
	//buf permette di copiare il contenuto di un file in un altro, 
	//senza sapere come è formattato
	char buf[200];
	FILE *pfileR, *pfileW;
	//nome_copia sarà l'unione del percorso e del nome del file da copiare
	char nome_copia[100]="";
	strcat(nome_copia,percorso);
	strcat(nome_copia,"/");
	sprintf(nome_copia,"%s%s-",nome_copia,nome_serie);
	if(n!=0)
		sprintf(nome_copia,"%s%d-",nome_copia,n);
	strcat(nome_copia,file_da_copiare);
	nome_copia[strlen(nome_copia)]='\0';

	//copio il file
	pfileR=fopen(file_da_copiare,"r");
	if(pfileR==NULL){
		printf("errore apertura in lettura file in copia_file_loc() di esp_derrida!!! error is %d\n",errno);	
		return false;
		}
	//se è la prima volta, apre in scrittura (lo sovrascrive o lo crea se non 
	//esiste)
	if(id==1){
		pfileW=fopen(nome_copia,"w");
		if(pfileW==NULL){
			printf("errore apertura in scrittura file in copia_file_loc() di esp_derrida!!! error is %d\n",errno);	
			return false;
			}	
		}
	//altrimenti appendi a file già esistente
	else{
		pfileW=fopen(nome_copia,"a");
		if(pfileW==NULL){
			printf("errore apertura in scrittura file in copia_file_loc() di esp_derrida!!! error is %d\n",errno);	
			return false;
			}	
		}
	//copia tutto
	while(1) {
	    fgets(buf, 200, pfileR);
		if( feof(pfileR) )
			break;
    	fputs(buf, pfileW);
		}
	if(fclose(pfileR)!=0){
		printf("errore in chiusura file di esp_derrida!!! error is %d\n",errno);		
		return false;
		}
	if(fclose(pfileW)!=0){
		printf("errore in chiusura file di esp_derrida!!! error is %d\n",errno);		
		return false;
		}
	return true;
}

/**Funzione che modifica l'input del motore secondo i parametri della funzione.
 * Parametri:
 * - in_m: file di input del motore su cui andare a scriver.
 * - p: numero di passi da impostare.
 * - f: lunghezza massima dell'attrattore ricercabile, da impostare.
 * - m: modalità di lancio del motore; da impostare.
 * Ritorna true se la scrittura del file avviene con successo, false altrimenti.
 */
bool modifica_input_motore(const char *in_m, int p, int f, int m){
	FILE *pfile;
	pfile=fopen(in_m,"w");
	if(pfile==NULL){
		printf("errore in apertura scrittura %s! error id %d\n",in_m,errno);
		return false;}
	fprintf(pfile,"PMAX:%d\n",p);
	fprintf(pfile,"FINMAX:%d\n",f);
	fprintf(pfile,"modalità:%d\n",m);
	if(fclose(pfile)!=0){
		printf("errore in chiusura file %s! error is %d\n",in_m,errno);		
		return false;}
	return true;
}

/**Funzione che modifica l'input del generatore andando a leggere tutto il file, e 
 * scrivendo solo il tempo di decadimento massimo dell'iterazione i-esima.
 * Parametri:
 * - file_in_grafo: file di input del generatore.
 * - tempo_fisso: valore booleano che indica se il tempo di decadimento di ogni 
				proteina è fisso a MDT oopure variabile tra 1 e MDT.
 * - new_t: nuovo MDT da impostare.
*/
bool modifica_input_generatore(const char *file_in_grafo, bool tempo_fisso, int new_t){
	FILE *pfile;
	char c;
	//booleano che indica se leggere e memorizzare o meno il carattere letto
	bool leggi;
	//intero che indica quale occorrenze bisogna ancora leggere (le occorenze sono 6)
	int occorrenze=0;
	int i,iaus,aus1,aus2;
	float faus;
	int offset;

	pfile=fopen(file_in_grafo,"r+");
	if(pfile==NULL){
		printf("errore apertura in lettura file!!! error is %d\n",errno);
		return false;}
	leggi=false;
	while(fscanf(pfile,"%c",&c)!=EOF){
		//se è un ':' allora si aspetta di leggere un numero
		if(c==':'){
			leggi=true;
			D2(printf("leggi si\n"));
			}	
		//può leggere
		if(leggi==true){
			switch(occorrenze){
				case 0: fscanf(pfile,"%d",&iaus);
						break;
				case 1: fscanf(pfile,"%d",&iaus);
						break;
				case 2: fscanf(pfile,"%d %d",&aus1,&aus2);
						break;
				case 3: //deve leggere le probabilità delle kin
						//alloca i float, ne alloca kinmax-kinmin+1 (min e max già letti)
						for(i=0;i<aus2-aus1+1;i++)
							fscanf(pfile,"%f ",&faus);
						break;
				case 4: fscanf(pfile,"%d",&iaus);
						break;
				case 5: //caso di interesse
						fscanf(pfile,"%d ",&iaus);
						fscanf(pfile,"%d",&iaus);
						offset=(iaus>=10)?2:1;
						offset=(iaus>=100)?3:offset;
						offset=(iaus>=1000)?4:offset;
						//se il tempo di dec delle proteine è fisso	
						if(tempo_fisso){	
							fseek(pfile, -(2*offset+1), SEEK_CUR);	
							fprintf(pfile,"%d %d ",new_t,new_t);
							}
						//se deve prendere con prob uniforme tra 1 ... MDT
						else{		
							fseek(pfile, -offset, SEEK_CUR);
							fprintf(pfile,"%d ",new_t);
							}
						if((iaus<10 && new_t>=10) || (iaus<100 && new_t>=100) || 
														(iaus<1000 && new_t>=1000))
							fprintf(pfile,"\n");
						break;
				case 6: break;
				default:printf("errore! numero occorrenze errato in mod gen\n");
				}
			occorrenze++;
			leggi=false;
			}
		}

	if(fclose(pfile)!=0){
		printf("errore in chiusura file!!! error is %d\n",errno);	
		return false;}
	return true;
}
