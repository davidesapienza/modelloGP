#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "generatore.h"
#include <time.h>


extern sistema s;
extern sistema_generatore sg;


/**Funzione che controlla se la coppia proteina-->gene è già presente.
 * Se la lista è nulla allora la proteina selezionata va bene, altrimenti 
 * devo scorrere tutta la lista fino alla fine (controllando che non ci sia) 
 * e poi, nel caso, inserirla.
 * La funzione prende in ingresso due interi corrispondenti agli indici della 
 * proteina e del gene.
 * Ritorna true tutto è andato a buon fine, false altrimenti.
 */
static bool gia_presente(int p,int g){
	//lungP rappresenta la lunghezza della lista di adiacenza in infresso di g
	int i, lungP=0;
	//puntatore ausiliare per aumentare la leggibilità del codice
	int *p1;
	p1=s.gr.l_geni[g].lp_in;

	D2(printf("******************************************\t p=%d, g=%d\n",p,g));
	D2(printf("g1 e p1 puntano alla prima posizione dell'array corrispondenti"
	"g1 a %d\n",g1));
	D2(printf("g1 e p1 puntano alla prima posizione dell'array corrispondenti"
	"p1 a %d\n",p1));
	D2(printf("il valore puntato da g1 e' %d\n",*g1));
	D2(printf("il valore puntato da p1 e' %d\n",*p1));

	/* scorro la lista delle proteine lp_in fino alla fine, controllando che non 
		ci sia la proteina selezionata 'p'*/
	for(i=0; *p1 != -1; i++){
		D2(printf("sono nel for con i=%d, e p1 punta a %d\n",i,p1));
		if(s.gr.l_geni[g].lp_in[i] == p)
			return true;
		p1++;
		}
	//lunghezza del vettore escluso il -1
	lungP=i;		//la prima volta è 0
	D2(printf("lungP=%d\n",lungP));
	//realloc per il vettore
	s.gr.l_geni[g].lp_in=realloc(s.gr.l_geni[g].lp_in,(lungP+2)*sizeof(int));

	D2(printf("ho riallocato la memoria con successo\n"));
	//inserisco il nuovo elemento
	D2(printf("sto inserendo i nuovi valori che sono di valore p=%d e g=%d\n",p,g));
	s.gr.l_geni[g].lp_in[lungP]=p;
	D2(printf("ho inserito il nuovo valore con successo\n"));
	//inserisco poi il -1 finale
	s.gr.l_geni[g].lp_in[lungP+1]=-1;

	D2(printf("ho rinserito i -1 con successo\n"));
	
	D2(printf("############################\n"));
	D2(printf("stampo le due liste di geni e proteine\n"));
	D2(for(i=0;i<lungP+2;i++)
		printf("s.gr.l_geni[%d].lp_in[%d]=%d\n",g,i,s.gr.l_geni[g].lp_in[i]););
	
	return false;		
}

/**Funzione generatore. Assegna gli archi in base alla modalità scelta.
 * 1 modalità: per ogni gene, tante volte quanto è il suo kin, scegli una proteina
			 random, effettuando alcuni controlli: 
			- la coppia gene e proteina non può essere i-esima_i-esima 
				(altrimenti c'è auto attivazione)
			- l'arco p-g non deve essere già presente (no link doppi)
 * 2 modalità: per ogni arco, assegnalo in modo casuale (nessun vincolo sul kin).
			   Rimangono i vincoli sui link doppi e gli auto-link.
 * 3 modalità: combianzione delle precedenti (vincolo sul kin minimo)
 */
void generatore(){
	int i,j,k;
	int p,g;		//i due nodi scelti a caso
					//p è la proteina, g è il gene

	/*"passi" conta per quante volte ho già generato un gene casualmente per 
	proteina fissata. se arriva a gr.n+1, vuol dire che la proteina è già 
	molto connessa. quindi dopo gr.n passi cambia proteina (per evitare stallo).
	*/	
	int passi;
	//numero archi già assegnati se si tratta della modalità 3
	//(rappresenta anche la somma di tutti i kin di tutti i geni
	int ne_ass=0;
	
	//dipende dalla modalità scelta
	switch(sg.modalita){
		case 3:	/*caso 3, combinazioni delle 2 seguenti, prima un kin minimo 
				per tutti, poi i rimanenti archi s.gr.e-ne_ass li assegno a caso*/
					
		case 1: //caso 1, in cui si assegnano i link solo in base al kin di ogni nodo
				//per ogni gene 	
				for(i=0;i<s.gr.n;i++){
					//per ogni kin
					for(j=0;j<s.gr.l_geni[i].kin;j++){
						//finchè non è valido			
						do{	
							//scelgo una proteina di ingresso
							p=rand()%s.gr.n;
					//nella condizione sarebbe s.gr.l_geni[i].id che però è == ad i
						}while(p==i || gia_presente(p,i) );
						ne_ass++;
						}
					}			
				if(sg.modalita==1)
					break;
		case 2: //caso 2, in cui si assegnano i link in maniera completamente casuale
				//per ogni arco	
				for(i=0;i<s.gr.e-ne_ass;i++){
					p=rand()%s.gr.n;
					//passi inizializzati a 0
					passi=0;
					do{			
						g=rand()%s.gr.n;
						//printf("g=%d\n",g);
						//se il kin del gene g è = a kmax allora scartalo
						//printf("kin gel gene %d è %d\n",g,s.gr.l_geni[g].kin
						if(s.gr.l_geni[g].kin==sg.kmax){
							//printf("dentro\n");
							g=p;
							continue;}
					//se non è un autolink allora incrementa i passi
						if(p!=g)
							passi++;
					//se arrivo ad un numero di passi=al num dei geni allora esci 
						if(passi==s.gr.n+1)
							break;
					}while(p==g || gia_presente(p,g));
					//se sei uscito a causa dei troppi passi, allora devi cambiare proteina, 
					//quindi ritorna indietro di uno(riseleziona il link corrente) e ripeti 
					//la procedura.
					if(passi==s.gr.n+1){
						i--;
						printf("ATTENZIONE!!! RAGGIUNTO NUM PASSI!!\n");	
						continue;
						}
					//qui ho assegnato la proteina p al gene g
					//devo aggiornare il suo kin
					s.gr.l_geni[g].kin++;
					}
				break;
		default:printf("errore!! modalita' scelta connessioni di input non valida\n");			
		}
}
