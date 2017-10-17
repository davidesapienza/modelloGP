#include "operazioni_file.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
extern sistema s;


/**Funzione di scrittura su file delle condizioni iniziali calcolate.
*/
bool scrittura_file(const char *file_out_condiz){
	FILE *pfile;
	int i,j;


	pfile=fopen(file_out_condiz,"w");
	if(pfile==NULL){
		printf("errore apertura in scrittura file!!! error is %d\n",errno);	
		return false;
		}
	fprintf(pfile,"num. condizioni iniziali:%d\n",s.n_cond_ini);
	fprintf(pfile,"contatori ai tempi di decadimento:\n");
	for(i=0;i<s.n_cond_ini;i++){
		for(j=0;j<s.gr.n;j++){	
			fprintf(pfile,"%d ",s.count_tdec[i*s.gr.n+j]);}
		fprintf(pfile,"\n");
		}
	if(fclose(pfile)!=0){
		printf("errore in chiusura file!!! error is %d\n",errno);		
		return false;
		}
	return true;
}

/**Funzione di lettura del file della struttura e delle impostazioni per il 
 * generatore delle C.I.
 * devo leggermi tutta la struttura della rete perchè mi serve sapere i tdecmin e tdecmax
*/
bool lettura_file(const char *file_out_gen, const char *file_in_condiz){

	FILE *pfile;
	int i,j,aus;

	pfile=fopen(file_out_gen,"r");
    if(pfile==NULL){
		printf("errore apertura in lettura file!!! error is %d\n",errno);	
		return false;
		}
	D2(printf("il puntatore:%d\n",pfile);
	printf("il puntatore:%d\n",*pfile);
	printf("il size puntatore:%d\n",sizeof(*pfile)););

	/* memorizzo prima le condizioni iniziali poi le info sul grafo*/
	
	
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
		fscanf(pfile,"\nf_out:");
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
		D2(printf("t_decadimento:%d\n%d\n",s.gr.l_proteine[i].t_decadimento,i););
		}	
	D2(printf("proteine scritte\n"));

    if(fclose(pfile)!=0){
		printf("errore in chiusura file!!! error is %d\n",errno);	
		perror("chiusura");	
		return false;
		}

	pfile=fopen(file_in_condiz,"r");
	if(pfile==NULL){
		printf("errore apertura in lettura file!!! error is %d\n",errno);
		return false;
		}
	fscanf(pfile,"num. condizioni iniziali:%d\n",&s.n_cond_ini);
	fscanf(pfile,"b:%f",&s.b);
	D2(printf("ho letto s.n_cond_ini=%d\n",s.n_cond_ini);
	printf("ho letto s.b=%f\n",s.b););
	if(fclose(pfile)!=0){
		printf("errore in chiusura file!!! error is %d\n",errno);		
		return false;
		}
	return true;
}	

/**Funzione di copia del file di output del generatore nel file di input del
 * gen_condiz_ini.
 */
bool copia_struct(const char copia[], const char dest[]){
	FILE *pfileW, *pfileR;
	char buf[200];
	pfileR=fopen(copia,"r");
	if(pfileR==NULL){
		printf("errore apertura in lettura file in copia_struct del gen_condiz_ini!!! error is %d\n",errno);	
		return false;
		}
	pfileW=fopen(dest,"w");
	if(pfileW==NULL){
		printf("errore apertura in scrittura file in copia_struct del gen_condiz_ini!!! error is %d\n",errno);	
		return false;
		}
	while(1) {
	    fgets(buf, 200, pfileR);
		if( feof(pfileR) )
			break;
    	fputs(buf, pfileW);
		}

	if(fclose(pfileR)!=0){
		printf("errore in chiusura file in copia_struct del gen_condiz_ini!!! error is %d\n",errno);		
		return false;
		}
	if(fclose(pfileW)!=0){
		printf("errore in chiusura file in copia_struct del gen_condiz_ini!!! error is %d\n",errno);		
		return false;
		}
	return true;
}
