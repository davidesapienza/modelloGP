#include "operazioni_file.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

//dichiarazione delle variabili sistema e sistema_generatore
extern sistema s;
extern sistema_generatore sg;

/**Funzione di scrittura del file.
 * Ritorna true se andato a buon fine e false altrimenti.
 * Parametri in ingresso: il nome del file su cui andare a memorizzare il grafo
 * in formato testo.   
 */
bool scrittura_file(const char *file_out_grafo){
	FILE *pfile;
	int i,j;
	pfile=fopen(file_out_grafo,"w");
	if(pfile==NULL){
		printf("errore apertura in scrittura file!!! error is %d\n",errno);	
		return false;
		}
	D2(printf("apro il file\n"));
	fprintf(pfile,"n:%d\ne:%d\nkin_min:%d\nkin_max:%d\n",s.gr.n,s.gr.e,s.gr.kin_min,s.gr.kin_max);
	fprintf(pfile,"tmin:%d\ntmax:%d\n",s.gr.tmin,s.gr.tmax);
	
	//prima memorizzo tutti i geni
	fprintf(pfile,"l_geni:\n");
	for(i=0;i<s.gr.n;i++){
		//id
		fprintf(pfile,"id:%d\n",s.gr.l_geni[i].id);
		//kin	
		fprintf(pfile,"kin:%d\n",s.gr.l_geni[i].kin);
		//*lp_in
		fprintf(pfile,"lp_in:");
		for(j=0;j<s.gr.l_geni[i].kin;j++)
			fprintf(pfile,"%d ",s.gr.l_geni[i].lp_in[j]);
		//*f_out
		fprintf(pfile,"\nf_out:");
		for(j=0;j<(1<<(s.gr.l_geni[i].kin));j++)
			fprintf(pfile,"%d ",s.gr.l_geni[i].f_out[j]);
		fprintf(pfile,"\n");
		}
	D2(printf("geni scritti\n"));
	//memorizzo le proteine
	fprintf(pfile,"l_proteine:\n");
	for(i=0;i<s.gr.n;i++){	
		//id
		fprintf(pfile,"id:%d\n",s.gr.l_proteine[i].id);
		//t_decadimento	
		fprintf(pfile,"t_decadimento:%d\n",s.gr.l_proteine[i].t_decadimento);
		}
	D2(printf("proteine scritte\n"));
	D2(printf("n:%d\ne:%d\nkin_min%d\nkin_max%d\n",s.gr.n,s.gr.e,s.gr.kin_min,s.gr.kin_max);
	
	//prima memorizzo tutti i geni
	printf("\nl_geni:\n");
	for(i=0;i<s.gr.n;i++){
		//id
		printf("id:%d\n",s.gr.l_geni[i].id);
		//kin	
		printf("kin:%d\n",s.gr.l_geni[i].kin);
		//*lp_in
		printf("lp_in:");
		for(j=0;j<s.gr.l_geni[i].kin;j++)
			printf("%d ",s.gr.l_geni[i].lp_in[j]);
		//*f_out
		printf("f_out:");
		for(j=0;j<(1<<(s.gr.l_geni[i].kin));j++)
			printf("%d ",s.gr.l_geni[i].f_out[j]);
		printf("\n");
		}
	//memorizzo le proteine
	printf("\nl_proteine:\n");
	for(i=0;i<s.gr.n;i++){	
		//id
		printf("id:%d\n",s.gr.l_proteine[i].id);
		//t_decadimento	
		printf("t_decadimento:%d\n",s.gr.l_proteine[i].t_decadimento);	
		});

	if(fclose(pfile)!=0){
		printf("errore in chiusura file!!! error is %d\n",errno);		
		return false;
		}
	D2(printf("file chiuso con successo\n"));
	return true;
}

/**Funzione di lettura file.
 * Ritorna true se andato a buon fine, false altrimenti.
 * Parametri in ingresso: il nome dei due file (uno per le info grafo e l'altro 
 * per le info sulle funzioni booleane).
 */
bool lettura_file(const char *file_in_grafo, const char *file_in_fun){
	FILE *pfile;
	char c;
	//booleano che mi dice se devo leggere e memorizzarmi o meno il carattere letto
	bool leggi;
	//intero che mi indica quale occorrenze devo ancora leggere (le occorenze sono 6)
	int occorrenze=0;
	int i,j,k,aus;
	
	pfile=fopen(file_in_grafo,"r");
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
				case 0: fscanf(pfile,"%d",&s.gr.n);
						break;
				case 1: fscanf(pfile,"%d",&s.gr.e);
						break;
				case 2: fscanf(pfile,"%d %d",&s.gr.kin_min,&s.gr.kin_max);
						break;
				case 3: //devo leggere le probabilità delle kin
						//alloco i float, ne alloco kinmax-kinmin+1 (min e max li ho già letti)
						sg.prob_kin=malloc((s.gr.kin_max-s.gr.kin_min+1)*sizeof(float));
						for(i=0;i<s.gr.kin_max-s.gr.kin_min+1;i++)
							fscanf(pfile,"%f ",&sg.prob_kin[i]);
						D2(for(i=0;i<s.gr.kin_max-s.gr.kin_min+1;i++)
							printf("%f ",sg.prob_kin[i]););
						break;
				case 4: fscanf(pfile,"%d",&sg.modalita);
						break;
				case 5: fscanf(pfile,"%d %d",&s.gr.tmin,&s.gr.tmax);
						break;
				case 6: fscanf(pfile,"%d",&sg.kmax);
						break;
				default:printf("errore! numero occorrenze errato\n");
				}
			occorrenze++;
			leggi=false;
			}
		D2(printf("%c\n",c));
		}
	if(occorrenze!=7){
		printf("errore nel file di input_generatore.txt..\ncampi mancanti\n");
		return false;
		}

	if(fclose(pfile)!=0){
		printf("errore in chiusura file!!! error is %d\n",errno);	
		return false;
		}
	D2(printf("n.nodi letto e' %d\n",s.gr.n);
	printf("kin_min:%d\n",s.gr.kin_min);
	printf("kin_max:%d\n",s.gr.kin_max););

	/* lettura del file delle funzioni booleane */
	pfile=fopen(file_in_fun,"r");
	if(pfile==NULL){
		printf("errore in apertura file!!! error is %d\n",errno);		
		printf("si tratta del file delle funzioni\n");		
		return false;
		}
	fscanf(pfile,"n_k=%d\n",&sg.n_k);
	sg.l_fun=malloc(sg.n_k*sizeof(funzioni));
	D2(printf("il numero dei k per cui c'e' la fun o un bias specificato e':%d\n",sg.n_k));
	for(i=0;i<sg.n_k;i++){
		D2(printf("per i=%d leggo.....\n",i));
		fscanf(pfile,"kin=%d\n",&sg.l_fun[i].kin);
	/*attenzione al procedimento:
		-se leggo un 'n' leggerò poi un "_fun"
		-se leggo un 'b' leggerò un "ias"
	devo differire i due casi.*/
		fscanf(pfile,"%c",&c);
		if(c=='n'){
			fscanf(pfile,"_fun=%d\n",&sg.l_fun[i].n_fun);
			D2(printf("ho letto kin=%d e n_fun=%d\n",sg.l_fun[i].kin,sg.l_fun[i].n_fun));
			sg.l_fun[i].f=malloc((1<<sg.l_fun[i].kin)*sg.l_fun[i].n_fun*sizeof(bool));
			sg.l_fun[i].p=malloc(sg.l_fun[i].n_fun*sizeof(float));
			for(j=0;j<sg.l_fun[i].n_fun;j++){
				D2(printf("per j=%d leggo..\n",j);
				printf("sto leggendo con %d indice\n",j*(1<<sg.l_fun[i].kin)+k);
				printf("perche' 1<<eccc = %d\n",1<<sg.l_fun[i].kin););
				for(k=0;k<(1<<sg.l_fun[i].kin);k++){
					fscanf(pfile,"%d ",&aus);
					sg.l_fun[i].f[j*(1<<sg.l_fun[i].kin)+k]=(aus==1)? 1 : 0;	
					}		
				fscanf(pfile,"%f\n",&sg.l_fun[i].p[j]);
				}
			//qui imposto il bias per questo kin =0 perchè uso le funzioni
			sg.l_fun[i].bias=0;
			}
		else if(c=='b'){
			fscanf(pfile,"ias=%f\n",&sg.l_fun[i].bias);
			//qui imposto le funzioni e i pesi a null (n_fun=0)
			sg.l_fun[i].n_fun=0;
			sg.l_fun[i].f=NULL;
			sg.l_fun[i].p=NULL;
			}
		else
			printf("NON DEVI TROVARTI QUI!! LETTURA FILE FUNZIONI\n");
		}
	fscanf(pfile,"\nbias=%f",&sg.bias);

	D2(for(i=0;i<sg.n_k;i++){
		printf("entro del for i<sg.n_k\n");
		for(j=0;j<sg.l_fun[i].n_fun;j++){
			for(k=0;k<(1<<sg.l_fun[i].kin);k++)
				printf("%d ",sg.l_fun[i].f[j*(1<<sg.l_fun[i].kin)+k]);
				
			printf("\t%f\n",sg.l_fun[i].p[j]);
			}
		});
	if(fclose(pfile)!=0){
		printf("errore in chiusura file!!! error is %d\n",errno);		
		return false;
		}
	return true;
}
