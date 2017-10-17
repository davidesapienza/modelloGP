/**Generatore di reti genetiche per modelli geni-proteine.
 * Creazione di una rete secondo alcuni parametri specificati nei file di input.
 * Tale Generatore non accetta parametri in ingrasso, ma utilizza nomi fissi per
 * i file di input e di output, i quali sono:
 *
 * - input_generatore.txt; indica i seguenti parametri (in ordine):
 *	- n. nodi:10	->nodi della rete (Geni e Proteine avranno lo stesso numero)
 *	- n. link:20    ->numero dei link (info. utilizzata in modalità 2 e 3)
 *	- connettività:2 5 ->numero di link in ingresso(min max) (info. per modalità 1,3)
 *	- probabilità sui kin:0.5 0.2 0.2 0.1 
					   ->probabilità associata al nemero dei link in ingresso.
						 Tanti numeri (la cui somma =1) quanto è la differenza
						 tra max e min di "connettività" 
 *	- modalità:1	->modalità di connessione della rete:
						1--si scegle il kin per ogni gene (tra min e max) e il 
						   numero dei link dipende esclusivamente dalla somma dei kin
						2--dato il n. link si tira a caso una proteina e un gene
    					   a cui associare tale link
						3--si garantisce un numero di link minimo in ingresso ad 
						   ongi gene(min=max) e i rimanenti n.link-quelli già associati 
						   sono scelti come nella modalità 2. 
 *	- tempo decadimento:1 3 ->tempi di decadimento min e max. Le proteine avranno 
							  un MDT casuale tra il min e il max.
 * 	- massimo kin:5	->numero massimo di collegamenti in ingresso (per evitare eccessiva 
					  memoria per memorizzare le funzioni di output -nell'es. 2^5).
 * 
 * -input_fun_bool.txt; indica i parametri sulle funzioni di uscita: 
 * 	-n_k=2 ->numero di kin per cui è specificato un certo comportamento (funzioni o bias)
 *	-lista di kin per qui è specificato un comportamento (mi aspetto siano 2)
 * 	kin=2 
 *	n_fun=5
 *	0 0 0 1 0.1
 *	1 1 1 0 0.2
 *	0 1 1 1 0.2
 *	1 0 0 1 0.2
 *	1 0 1 0 0.3
		->per kin=2 specifico 5 funzioni e le elenco
 *	kin=3
 *	bias=0.4
 * 		->per kin=3 invece specifico un suo bias particolare
 *
 * 		->necessaria riga vuota di separazione
 *	bias=0.5
 *  	->specifico un bias da utilizzare per tutti i kin non specificati 
		  (comportamento di default).
 *
 * -output_generatore.txt; fornisce le informazioni sulla struttura della rete 
 *  nel seguente modo:
 * 	-n:100		->numero dei nodi
 *	-e:350  	->numero dei link (info. utile solo se la modalità era 2 o 3)
 *	-kin_min:2 	->connettività minima
 *	-kin_max:2 	->connettività massima
 *	-tmin:1 	->tempo minimo di decadimento
 *	-tmax:6 	->tempo massimo di decadimento
 *	-l_geni:		->lista dei geni, per ognuno:
 *		-id:0			->indice del gene
 *		-kin:2  		->numero di link in ingresso
 *		-lp_in:55 23 	->id delle proteine in ingresso
 *		-f_out:1 0 1 0 	->funzione di output sugli ingressi
 *	-l_proteine:	->lista delle proteine, per ognuna:
 *		-id:0			->indice della proteina 
 *		-t_decadimento:4 ->tempo massimo di decadimento
 *
 *
author: Davide Sapienza.

*/
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include "../struttura_dati.h"
#include "struttura_generatore.h"
#include "operazioni_file.h"
#include "generatore.h"

/* creazione delle variabili sistema e sistema generatore
*/
sistema s;
sistema_generatore sg;

/* definizione dei nomi dei file utilizzati
*/
const char INPUT_GS[]="input_generatore.txt";
const char INPUT_FUN[]="input_fun_bool.txt";
const char OUTPUT_GS[]="output_generatore.txt";

/* dichiarazioni funzioni 
*/
static void inizializzazione();
static void deallocazione();
static void assegna_funzione();

/**Funzione principale, passi:
	- lettura file in ingresso ("input_generatore.txt")
	- lettura del file delle funzioni booleane di uscita ("input_fun_bool.txt")
	- creazione vera e propria della rete
	- scrittura del file di uscita ("output_generatore.txt");
*/
int main( int argc, char **argv){
	
	//lettura dei 2 file di ingresso
	if(!lettura_file(INPUT_GS,INPUT_FUN)){
		printf("errore in caricamento file\n");
		return 1;
		}
	D1(printf("lettura effettuata con successo\n"));

	//inizializzazione della rete
	inizializzazione();
	D1(printf("inizializzazione terminata\n"));
	//--logica del programma --
	generatore();
	D1(printf("fase di generazione terminata\n"));
	assegna_funzione();
	D1(printf("fase di assegnazione_funzioni terminata\n"));

	//scrittura in file di uscita
	if(!scrittura_file(OUTPUT_GS)){
		printf("errore in scrittura file\n");
		return 1;
		}
	D1(printf("scrittura file effettuata con successo\n"));
	deallocazione();
	D1(printf("deallocato con successo\n"));
return 0;
}

/**Funzione di inizializzazione del sistema.
 * Prima vengono settate alcune informazioni del grafo, poi le variabili sel sistema.
 * Vengono istanziate le due liste di geni e proteine andando poi ad inizializzare
 * per ogni elemento, rispettivamente:
 * - id, lp_in, kin e f_out per i geni.
 * - id, t_decadimento per le proteine.
 */
static void inizializzazione(){
	int i,j,ausk,aust, aus;
	//variabili utilizzate per l'assegnazione del kin ad ongi gene
	int somma,random;
	
	/*inizializzo la funzione rand utilizzata per determinare 
	il t_decadimento della proteina ed eventualmente il kin*/
	srand(time(NULL)*(unsigned int)getpid());
	
	/* parto dal grafo:
		-n, e, kin_min, kin_max, tmin, tmax sono già state settate perchè lette
			da file
		-l_geni e l_proteine devono essere allocate e ogni loro dato allocato/
			inizializzato
	*/
	//creo le liste di geni e proteine: rispettivamente l_geni e l_proteine
	s.gr.l_geni=malloc(s.gr.n*sizeof(gene));
	D2(printf("dovrei aver riservato per i g%d di mem\n",s.gr.n*sizeof(gene)));
	s.gr.l_proteine=malloc(s.gr.n*sizeof(proteina));
	D2(printf("dovrei aver riservato per le p %d di mem\n",s.gr.n
			*sizeof(proteina)));
	D2(printf("stampo le dimensioni dei due vettori in ogni posizione\n"));
	D2(
	for(i=0;i<s.gr.n;i++){
		printf("al passo %d, ho dim gene=%d, e dim proteina=%d\n",i,
										&s.gr.l_geni[i],&s.gr.l_proteine[i]);
	}	);

	//se il kin max specificato supera quello consentito, risetta il kin max.
	if(s.gr.kin_max>sg.kmax){
		printf("ATTENZIONE: kin_max > kmax. risettato\n");
		aus=s.gr.kin_max;
		s.gr.kin_max=sg.kmax;
		//in questo caso però devo anche risettare le probabilità..
		//l'ultima disponibile deve sommare sè stessa e quelle che sono state scartate
		for(i=0;i<aus-sg.kmax;i++)
			sg.prob_kin[sg.kmax-s.gr.kin_min]+=sg.prob_kin[sg.kmax-s.gr.kin_min+1+i];
		sg.prob_kin=realloc(sg.prob_kin,(s.gr.kin_max-s.gr.kin_min+1)*sizeof(float));
		}

	//setto gli aus per il kin e il t_decadimento
	ausk=s.gr.kin_max-s.gr.kin_min+1;
	aust=s.gr.tmax-s.gr.tmin+1;	
	// per ogni elemento delle liste inserisco alcune informazioni di base
	for(i=0;i<s.gr.n;i++){
		//setto gli id di geni e proteine
		s.gr.l_geni[i].id=i;
		s.gr.l_proteine[i].id=i;

		//setto il kin variabile al nodo i-esimo se la modalità e 1
	/*logica: genero un numero casuale tra 1 e 100. Somma parte dalla probabilità 
	  del kin minimo. Finchè somma non è maggiore del numero random, somma la
	  prossima probabilità. Quando somma è >= random, allora associa quel kin al 
	  gene i. 
	  Somma è la somma delle probabilità dei kin moltiplicata per 100.
	  Quando somma>=random, vuol dire che ho appena sommato la probabilità j-esima, 
      quindi devo utilizzare il kin j-esimo(kinmin+j)*/
		if(sg.modalita==1){
			somma=0;
			random=rand()%100+1;
			D2(printf("random==%d\n",random));
			for(j=0;j<ausk;j++){
				somma+=(sg.prob_kin[j]*100);
				D2(printf("coin j=%d ho somma==%d\n",j,somma));
				if(random<=somma){
					s.gr.l_geni[i].kin=s.gr.kin_min+j;
					break;
					}	
				}
			}
		//se la modalità è la 3 allora setta kin uguale al minimo
		else if(sg.modalita==3)
			s.gr.l_geni[i].kin=s.gr.kin_min;
		//altrimenti settalo a zero (si aggiornerà dopo)
		else
			s.gr.l_geni[i].kin=0;
		
		//assegna un tempo di decadimento alla proteina i-esima:
		//se tmax e tmin sono uguali allora assegna tmin, altrimenti a caso tra i 2	
		s.gr.l_proteine[i].t_decadimento= (s.gr.tmax>s.gr.tmin)? 
							(rand()%aust)+s.gr.tmin : s.gr.tmin;
		//alloco un elemento nella lista di ingresso del gene i-esimo e lo setto a -1
		s.gr.l_geni[i].lp_in=malloc(1*sizeof(int));
		*s.gr.l_geni[i].lp_in=-1;
		}
}

/**Funzione che assegna le funzioni booleane ai geni, in base al kin.
 */
static void assegna_funzione(){	
	int i,j,k;
	//somma e random sono variabili per l'assegnazione della f_out in base alla 
	//probabilità associata alle funzioni per kin.
	int somma,random;
	//per ogni gene
	for(i=0;i<s.gr.n;i++){
		//alloco la funzione : 2^kin
		s.gr.l_geni[i].f_out=malloc((1<<s.gr.l_geni[i].kin) *sizeof(bool));
		//si distinguono i casi in cui le funzioni ci sono e non ci sono
		/* se n_k!=0 (vuol dire che esistono funzioni per almeno un kin)
		   se il kin del gene è compreso tra il primo kin specificato e il primo
			sommato al numero dei kin specificati, allora entra.*/
		if(sg.n_k!=0 && s.gr.l_geni[i].kin>=sg.l_fun[0].kin && 
				s.gr.l_geni[i].kin<sg.l_fun[0].kin+sg.n_k){	
			/*quindi il kin è specificato: ci sono due casi
			-viene specificato solo il bias, o vengono specificate le funzioni*/

			//se è specificato il bias(e non le funzioni)
			if(sg.l_fun[s.gr.l_geni[i].kin-sg.l_fun[0].kin].n_fun==0){
				D2(printf("utilizza il bias del kin\n"));
				//utilizza il bias del kin
				for(j=0;j<(1<<(s.gr.l_geni[i].kin)); j++)                                   
					s.gr.l_geni[i].f_out[j]=(rand()%100<(int)
						(sg.l_fun[s.gr.l_geni[i].kin-sg.l_fun[0].kin].bias*100)) ? 1 : 0;
				}
			//altrimenti esistono le funzioni
			else{
			
			/*Genero un numero casuale, lo moltiplico per 100, e in base al 
			  numero seleziono la funzione.
			  Scorro tutte le probabilità sommandole mano a mano finchè la somma  
		      risulta più piccola del numero random. In questo caso, scarto tale 
			  funzione. Quando arrivo ad avere una somma più grande di random, 
			  vuol dire che la fun j-esima è quella da utilizzare.*/
				somma=0;
				random=rand()%100+1;
				//per ogni funzione
				for(j=0;j<sg.l_fun[s.gr.l_geni[i].kin-sg.l_fun[0].kin].n_fun;j++){
					//guarda la probabilità e calcola la somma
					somma+=(sg.l_fun[s.gr.l_geni[i].kin-sg.l_fun[0].kin].p[j]*100);
					D2(printf("quanto vale sta somma? =%d\n",somma));
					if(random<=somma){
						//allora utilizza la funzione *j
						D2(printf("la soomma è maggiore\ns.gr.l_geni[i].kin=%d\n",
									s.gr.l_geni[i].kin));
						//per la funzione j, copia tutti i singoli stati
						for(k=0;k<(1<<s.gr.l_geni[i].kin);k++)
							s.gr.l_geni[i].f_out[k]=sg.l_fun[s.gr.l_geni[i].kin-
									sg.l_fun[0].kin].f[j*(1<<s.gr.l_geni[i].kin)+k];	
												
						D2(for(k=0;k<(1<<s.gr.l_geni[i].kin);k++){
							printf("%d ",s.gr.l_geni[i].f_out[k]);
							});
						//ho trovato e scritto la funzione, quindi passa al prox gene
						break;				
						}
					
					}
				}
			}
		//se invece n_k==0 allora utilizza il bias
		else{
			D2(printf("utilizza il bias\n"));
			for(j=0;j<(1<<(s.gr.l_geni[i].kin)); j++)                                   
				s.gr.l_geni[i].f_out[j]=(rand()%100<(int)(sg.bias*100)) ? 1 : 0;
			}
		}

	//stampa le f_out associate ai geni
	D2(for(i=0;i<s.gr.n;i++){
		printf("f_out gene %d\n",i);
		for(j=0;j<(1<<s.gr.l_geni[i].kin);j++)
			printf("%d ",s.gr.l_geni[i].f_out[j]);
		printf("\n");
	});
}

/**Funzione chiamata prima di uscire dal programma che dealloca tutta la memoria
 * allocata.
*/
static void deallocazione(){
	int i=0;
	//dealloco prima tutta la memoria dentro le liste di geni e proteine
	for(i=0; i<s.gr.n; i++){
		free(s.gr.l_geni[i].lp_in);	
		free(s.gr.l_geni[i].f_out);
		}
	//dealloco le liste di geni e proteine
	free(s.gr.l_geni);
	free(s.gr.l_proteine);	
	//dealloco tutta la mem dentro le funzioni booleane
	for(i=0;i<sg.n_k;i++){
		free(sg.l_fun[i].f);
		free(sg.l_fun[i].p);		
		}
	//dealloco le funzioni booleane
	free(sg.l_fun);
}
