#include "operazioni_file.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

extern sistema s;
extern sistema_motore sm;

/** Funzione che legge da file.
 * Prende in ingresso due parametri che rappresentano i file di input:
	- file_struct_in corrisponde all'output del generatore
	- file_imposta_in contiene le informazioni dui passi e le info per il motore
 * Ritorna un booleano, true se la lettura è andata a buon fine, false altrimenti 
 */
bool lettura_file(const char *file_struct_in, const char *file_condiz, const char *file_imposta_in){
	
	FILE *pfile;
	int i,j,aus;
	char c='a';
	//booleano che mi dice se devo leggere e memorizzarmi o meno il carattere letto
	bool leggi;
	//intero che mi indica quale occorrenze devo ancora leggere (le occorenze sono 3)
	int occorrenze=0;
	//char *ausnome;

	pfile=fopen(file_struct_in,"r");
    if(pfile==NULL){
		printf("errore apertura in lettura file!!! error is %d\n",errno);	
		return false;
		}
	D2(printf("il puntatore:%d\n",pfile);
	printf("il puntatore:%d\n",*pfile);
	printf("il size puntatore:%d\n",sizeof(*pfile)););

	/* memorizzo prima le info sul grafo poi le condizioni iniziali*/
	
	
	fscanf(pfile,"n:%d\ne:%d\nkin_min:%d\nkin_max:%d\n",&s.gr.n,&s.gr.e,
								&s.gr.kin_min,&s.gr.kin_max);
	fscanf(pfile,"tmin:%d\ntmax:%d\n",&s.gr.tmin,&s.gr.tmax);	
	D2(printf("n:%d\ne:%d\nkin_min:%d\nkin_max:%d\n",s.gr.n,s.gr.e,s.gr.kin_min,
															s.gr.kin_max););

	
	
	s.gr.l_geni=malloc(s.gr.n*sizeof(gene));
	s.gr.l_proteine=malloc(s.gr.n*sizeof(proteina));
	//prima memorizzo tutti i geni
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

	//memorizzo le proteine
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
		return false;
		}

	pfile=fopen(file_condiz,"r");
    if(pfile==NULL){
		printf("errore apertura in lettura file!!! error is %d\n",errno);	
		return false;
		}
	fscanf(pfile,"num. condizioni iniziali:%d\n",&s.n_cond_ini);
	D2(printf("le condizioni iniziali sono:%d\n",s.n_cond_ini));

	//espressione condizionale. spiegazione con esempio:
	//se s.gr.n=20-->servono solo 5 char per il nome + '\0'
	//se s.gr.n=18-->servono 18/4(=4) +1 char per il nome +'\0'
	//s.dim_id=(s.gr.n%4==0)?s.gr.n/4+1 : s.gr.n/4+2;

	
	s.stato=malloc((s.n_cond_ini*s.gr.n)*sizeof(bool));
	for(i=0;i<s.n_cond_ini;i++){
		for(j=0;j<s.gr.n;j++)
			s.stato[i*s.gr.n+j]=0;
		}
	s.count_tdec=malloc((s.n_cond_ini*s.gr.n)*sizeof(int));
	fscanf(pfile,"contatori ai tempi di decadimento:\n");
	for(i=0;i<s.n_cond_ini;i++){
		for(j=0;j<s.gr.n;j++){	
			fscanf(pfile,"%d ",&s.count_tdec[i*s.gr.n+j]);}
		fscanf(pfile,"\n");
		}

	if(fclose(pfile)!=0){
		printf("errore in chiusura file!!! error is %d\n",errno);	
		perror("chiusura");	
		return false;
		}

	/*adesso file_imposta_in*/
	pfile=fopen(file_imposta_in,"r");
    if(pfile==NULL){
		printf("errore apertura in lettura file!!! error is %d\n",errno);	
		return false;
		}

	leggi=false;
	while(fscanf(pfile,"%c",&c)!=EOF){
		//se è un ':' allora mi aspetto di leggere un numero
		if(c==':'){
			leggi=true;
			D2(printf("leggi si\n"));
			}	
		//posso leggere
		if(leggi==true){
			switch(occorrenze){
				case 0: fscanf(pfile,"%d\n",&sm.PMAX);
						break;
				case 1: fscanf(pfile,"%d\n",&sm.FINMAX);
						break;
				case 2: fscanf(pfile,"%d",&sm.modalita);
						break;
				default:printf("errore! numero occorrenze errato\n");
				}
			occorrenze++;
			leggi=false;
			}		
		D2(printf("%c\n",c));
		}
	
    if(fclose(pfile)!=0){
		printf("errore in chiusura file!!! error is %d\n",errno);		
		return false;
		}
	D2(printf("PMAX=%d, FINMAX=%d, modalita=%d\n",sm.PMAX,sm.FINMAX,sm.modalita));
	return true;
}

/** Funzione di scrittura del file.
 * Prende in ingresso 4 parametri:
	- file_out_motore nel quale memorizza le traiettorie o gli attrattori trovati
		(dipende dalla modalità scelta)
	- m_stati rappresenta la matrice degli stati
	- nomi rappresenta l'array dei nomi degli stati
	- t_dec rappresenta la matrice dei tempi di decadimento
	- id_cond il numero della condizione iniziale corrente
 * Ritorna un booleano, true se la scrittura è andata a buon fine, false altrimenti.
 *
 * Questa funzione viene chiamata per ogni condizione iniziale, accodando le stampe
 * su file.
 * m_stati, nomi e t_dec sono di dimensione variabile: dipende sempre dalla scelta della
 * modalità.
 */
bool scrittura_file(const char *file_out_motore, bool *m_stati, int *t_dec, int id_cond){
	FILE *pfile;
	int j,k,dimciclo;

	/*la dimensione del ciclo dipende dalla modalità scelta:
	per modalita==1 devo stampare tutti i passi
	per modalita==2 devo stampare solo un passo
	per modalita==3 devo stampare solo l'attrattore(se esiste)*/
	switch(sm.modalita){
		case 1: dimciclo=sm.PMAX;
				break;
		case 2: dimciclo=1;
				break;
		case 3: //la dimensione del ciclo è solo uno(esiste un solo attrattore)
				//avevo usato sm.FINMAX per controllare l'effettivo ciclo nell' 
				//array circolare di dimensione appunto FINMAX
				dimciclo=1;//sm.FINMAX;
				break;
		default:printf("errore!! modalità errata!\n");
				return false;
	}
	//se è la prima condizione iniziale, non scrivere in coda ma sovrascrivi il 
	//file nel caso sia già esistente
	if(id_cond==0){
		pfile=fopen(file_out_motore,"w");
		if(pfile==NULL){
			printf("errore apertura in scrittura file!!! error is %d\n",errno);	
			return false;
			}
		}
	//negli altri casi accoda al file
	else{
		pfile=fopen(file_out_motore,"a");
		if(pfile==NULL){
			printf("errore apertura in scrittura file!!! error is %d\n",errno);	
			return false;
			}
		}
	//solo la prima volta scrivi l'intestazione
	if(id_cond==0){
		fprintf(pfile,"il numero dei nodi e':%d\n",s.gr.n);		
		fprintf(pfile,"le condizioni iniziali sono:%d\n",s.n_cond_ini);	
		}	
	//SCRIVO LA MATRICE
	//fprintf(pfile,"condizione iniziale %d:\n",id_cond+1);	
	//so sono nella modalità 1 o 2
	if(sm.modalita!=3){
		//per ogni passo
		for(j=0;j<dimciclo;j++){
			//scrivo lo stato del passo
			for(k=0;k<s.gr.n;k++)
				fprintf(pfile,"%d ",m_stati[(j*s.gr.n)+k]);
			fprintf(pfile,"\t");
			for(k=0;k<s.gr.n;k++)
				fprintf(pfile,"%d ",t_dec[(j*s.gr.n)+k]);
			fprintf(pfile,"\n");
			}
		}
	//se sono nella modalità 3
	else{
		//se l'attrattore esiste scrivilo
		if(sm.l_attr[id_cond].lung!=-1){
			//per ogni passo
			for(j=0;j<dimciclo;j++){
				//scrivo lo stato del passo
				for(k=0;k<s.gr.n;k++)
					fprintf(pfile,"%d ",sm.l_attr[id_cond].stato[(j*s.gr.n)+k]);
				fprintf(pfile,"\t");
				for(k=0;k<s.gr.n;k++)
					fprintf(pfile,"%d ",sm.l_attr[id_cond].tdec[(j*s.gr.n)+k]);
				fprintf(pfile,"\t%d\t%d\n",sm.l_attr[id_cond].lung,sm.l_attr[id_cond].l_trans);
				}
			}
		//altrimenti scrivi -1 -1 -1...
		else{
			//per ogni passo
			//for(j=0;j<dimciclo;j++){
				//scrivo lo stato del passo
				for(k=0;k<s.gr.n;k++)
					fprintf(pfile,"%d ",-1);
				fprintf(pfile,"\t%d\t%d\n",0,sm.PMAX);
				//}
			}
		}
	if(fclose(pfile)!=0){
		printf("errore in chiusura file!!! error is %d\n",errno);		
		return false;
		}
	return true;
}
/**Funzione di copia del file di output del generatore e del file di output del 
 * gen_condiz_ini nei file del motore.
 */
bool copia_file(const char copia[], const char dest[]){
	FILE *pfileW, *pfileR;
	char buf[200];
	pfileR=fopen(copia,"r");
	if(pfileR==NULL){
		printf("errore apertura in lettura file in copia_file del motore!!! error is %d\n",errno);	
		return false;
		}
	pfileW=fopen(dest,"w");
	if(pfileW==NULL){
		printf("errore apertura in scrittura file in copia_file del motore!!! error is %d\n",errno);	
		return false;
		}
	while(1) {
	    fgets(buf, 200, pfileR);
		if( feof(pfileR) )
			break;
    	fputs(buf, pfileW);
		}

	if(fclose(pfileR)!=0){
		printf("errore in chiusura file in copia_file del motore!!! error is %d\n",errno);		
		return false;
		}
	if(fclose(pfileW)!=0){
		printf("errore in chiusura file in copia_file del motore!!! error is %d\n",errno);		
		return false;
		}
	return true;
}

