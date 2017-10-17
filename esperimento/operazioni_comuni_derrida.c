#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include "operazioni_comuni_derrida.h"

extern sistema s;
extern sistema_motore sm;

/**Funzione che calcola la media delle differenze per parità di nodi perturbati.
 * Funzione che di fatto calcola la distanza di Hamming.
 * I parametri sono:
 * - out_m_ci: file di output del motore sulle condizioni imperturbate.
 * - out_m_cip: file di output del motore sulle condizioni perturbate.
 * - out_ris: file su cui memorizzare le distanze di Hamming
 * - id rappresenta l'iterazione di copia. Se id==0 allora è la prima volta che viene
	 effettuta la copia, quindi apri il file in scrittura (sovreascrivi- scrivi all'
	 inizio). Se invece id!=0 allora appende al file, perchè la copia è già iniziata.
 * Ritorna true se va a buon fine, false altrimenti.
 */
bool calcola_mediaP(const char* out_m_ci, const char* out_m_cip, const char* out_ris, int id){
	int n_cond;
	int i,j;
	int aus;
	int aus1,aus2;
	float mediastato, mediatempi;
	FILE *pfileR1, *pfileR2, *pfileW;
	pfileR1=fopen(out_m_ci,"r");
	if(pfileR1==NULL){
		printf("errore apertura in lettura file %s!!! error is %d\n",out_m_ci,errno);	
		return false;}
	pfileR2=fopen(out_m_cip,"r");
	if(pfileR2==NULL){
		printf("errore apertura in lettura file %s!!! error is %d\n",out_m_cip,errno);	
		return false;}
	//legge due volte perchè deve leggere tutti e due i file di output del M
	fscanf(pfileR1,"il numero dei nodi e':%d\n",&aus);
	fscanf(pfileR2,"il numero dei nodi e':%d\n",&aus);
	fscanf(pfileR1,"le condizioni iniziali sono:%d\n",&n_cond);	
	fscanf(pfileR2,"le condizioni iniziali sono:%d\n",&n_cond);
	//azzera le somme perchè è un nuovo numero di perturbazioni	
	mediastato=0;
	mediatempi=0;	
	//per ogni condizione iniziale
	for(i=0;i<n_cond;i++){
		//legge gli stati
		for(j=0;j<s.gr.n;j++){
			fscanf(pfileR1,"%d ",&aus1);
			fscanf(pfileR2,"%d ",&aus2);
			mediastato+=(abs(aus1-aus2));
			}		
		fscanf(pfileR1,"\t");
		fscanf(pfileR2,"\t");
		//legge i tempi di decadimento
		for(j=0;j<s.gr.n;j++){
			fscanf(pfileR1,"%d ",&aus1);
			fscanf(pfileR2,"%d ",&aus2);
			/*//conto le differenze: se sono diversi allora 1; se uguali --> 0.
			aus1=(aus1!=aus2)? 1 : 0;*/
			//VARIANTE!!!! SUGLI STATI DELLE PROTEINE
			if((aus1>0 && aus2>0) || (aus1==0 && aus2==0))
				aus1=0;
			else 
				aus1=1;
			mediatempi+=aus1;
			}
		fscanf(pfileR1,"\n");
		fscanf(pfileR2,"\n");
		}
	
	if(fclose(pfileR1)!=0){
		printf("errore in chiusura file %s!!! error is %d\n",out_m_ci,errno);		
		return false;}
	if(fclose(pfileR2)!=0){
		printf("errore in chiusura file %s!!! error is %d\n",out_m_cip,errno);		
		return false;}
	//valuta se deve sovrascrivere o appendere 
	if(id==1)
		pfileW=fopen(out_ris,"w");
	else
		pfileW=fopen(out_ris,"a");	
	if(pfileW==NULL){
		printf("errore apertura in scrittura file!!! error is %d\n",errno);	
		return false;}

	fprintf(pfileW,"%d\t%f\t%f\n",id,mediastato/n_cond,mediatempi/n_cond);

	if(fclose(pfileW)!=0){
		printf("errore in chiusura file scrittura!!! error is %d\n",errno);		
		return false;}
	return true;
}


