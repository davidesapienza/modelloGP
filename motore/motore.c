#include "motore.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

extern sistema s;
extern sistema_motore sm;

//dichairazione delle funzioni statiche
static bool fun_stato(int indice, int t_d[]);
static bool cerca_attrattore(bool *m_stati, int* t_dec, int idx_new, int idx_old, int id_cond);
static int trova_nome(bool *m_stati, int idx_new, int id_cond, int *t);


/** Funzione motore che fa evolvere il sistema di un numero prefissato di passi per
 * ogni condizione iniziale.
 * parametri in ingresso:
	- matrice degli stati da riempire
	- numero della condizione iniziale
	- matrice dei tempi di decadimento (solo per odalità tre)
 * Per ogni condizione iniziale, inizializza la matrice, dopodichè in base allo stato
 * della rete all'istante t calcola sulla base del t_decadimento e f_out lo stato 
 * del grafo all'istante t+1. decrementa di 1 tutti i t_decadimento, li riaggiorna
 * se il gene al t+1 è attivo (quindi la produce) e riparte da capo.
 */
void motore(bool* m_stati, int id_condiz, int *t_dec){
	//vettore d'appoggio per memorizzare i tempi di decadimento all'istante t	
	int t_d[s.gr.n];
	//dimciclo e indice variabili dipendenti dalla modalità scelta
	int j,k,dimciclo,indice;
	//variabile utilizzata solo in modalità tre per memorizzarmi l'indice del
	//più grande stato dell'attrattore
	int max_stato=0;
	//idx_old e idx_new rappresentano l'indice al più vecchio stato ottenuto e
	//all'ultimo stato, servono in modalità 3 per l'array circolare, e la ricerca
	//dell'attrattore.
	int idx_old=0,idx_new=0;
	//vettore ausiliare per gli stati, mi memorizza i nuovi stati al passo t 
	//ottenendoli dall'istante t-1
	bool ausstato[s.gr.n];
    //lunghezza del transiente
	int lungtrans=0;

	//in base alla modalità setto la dimensione del ciclo di inizializzazione e 
	//la dimensione del membro che calcola l'indice di indirizzamento
	switch(sm.modalita){
		case 1: dimciclo=sm.PMAX;
				break;
		case 2: dimciclo=1;
				break;
		case 3: dimciclo=sm.FINMAX;
				break;
		default://non potrà mai arrivare qui, perchè sarebbe già uscito nel main
				return ;
	}
	//INIZIALIZZO A ZERO TUTTA LA MATRICE
	for(j=0;j<dimciclo;j++){
		for(k=0;k<s.gr.n;k++)
			m_stati[(j*s.gr.n)+k]=0;
		}
	
	//QUESTO È VERO PER TUTTE E TRE LE MODALITÀ
	//riempio nella matrice la prima riga (condizione iniziale)
	for(j=0;j<s.gr.n;j++)
		m_stati[j]=s.stato[id_condiz*s.gr.n+j];  //non c'è il passaggio intermedio 
												 //con j*gr.n perchè si tratta 
												 //della prima riga
	/*QUESTO LO DEVO FARE PER TUTTE E TRE LE MODALITÀ*/
	//inizializzo i contatori dei t_decadimento
	for(j=0;j<s.gr.n;j++){
		//setto l'array dei tempi di decadimento ai t_dec passati in input dal gen_cond_ini
		t_d[j]=s.count_tdec[id_condiz*s.gr.n+j];
		//solo per la modalita3 setto anche t_dec ai tempi di decadimento iniziali
		t_dec[j]=t_d[j];		
		}	

	D2(printf("stampo i tempi di decadimento\n");
	for(j=0;j<s.gr.n;j++)
		printf("con mstati=%d --> t_dec è %d\n",m_stati[j],t_d[j]););

	/*TUTTE CICLANO PER PMAX CAMBIA PERÒ LA MEMORIZZAZIONE
	-	j*s.gr.n+k PER MOD1
	-	K PER MOD2
	-	j*s.gr.n+k PER MOD3 ma con j che va da 1..FINMAX*/ 
	//per ogni passo
	for(j=1;j<sm.PMAX;j++){
		//in base alla modalita devo settare l'indice di indirizzamento
		//quindi se la modalità è 1, allora l'indice rimane j, altrimenti...
		//se modalità è 2, allora l'indice è lo 0(così scrive sempre sulla stessa
		//riga, altrimenti so essere la modalità 3, e l'indice è il modulo tra j
		//e FINMAX.
		indice=(sm.modalita==1)?j:((sm.modalita==2)?0:j%sm.FINMAX);

		//setto gli indici idx_old e idx_new per la modalità 3
		if(sm.modalita==3){
			//incremento la lunghezza del transiente (info utile solo in modalità 3)
			lungtrans++;
			//prima di assegnare a new il nuovo indice, controllo che il new+1 in modulo
			//non sia == all'old, se uguale all'ora old incrementa(sempre in modulo)
			idx_old=((idx_new+1)%sm.FINMAX==idx_old)? (idx_old+1)%sm.FINMAX : idx_old; 		
			//assegno poi il nuovo indice a new
			idx_new=indice;
/*funziona perchè:
	se new è 10 (passo precedente), e old 11, 
	allora 10+1=11 --> old aumenta di uno, e poi assegno il nuovo indice che sarà 11 a new.
caso iniziale:
	new=0, old=0,
	new+1==old? no, quindi old rimane a zero, e new poi con indice non prende 1, ma rimane a 0.
passo successivo,
    la condizione sarà la stessa, ma sta volta indice contiene 1, quindi new prende 1.
*/
			}
		//per ogni gene
		D2(printf("gli stati sono:\n");
		for(k=0;k<s.gr.n;k++)
			printf("%d ",m_stati[(indice-1)*s.gr.n+k]);
		printf("il nuovo stato è:\n"););
		for(k=0;k<s.gr.n;k++){
			//calcolo il suo stato next
			ausstato[k]=fun_stato(k,t_d);
			D2(printf("%d",ausstato[k]));
			}
		D2(printf("\n----------\n"));
		//li memorizzo. ho dovuto farlo in due passaggi perchè in base alla modalità
		//avrei riscritto la stessa posizione andando a sballare poi il calcolo dei
		//nuovi stati 
		for(k=0;k<s.gr.n;k++)
			m_stati[indice*s.gr.n+k]=ausstato[k];
			
		//decremento il t_decadimento per tutte le proteine (è qualcosa che 
		//svanisce nel tempo)
		for (k=0;k<s.gr.n;k++){
			if(t_d[k]>0)			
				t_d[k]--;
			}
		//riaggiorno il t decadimento
		for(k=0;k<s.gr.n;k++){
			//non uso lo stato del gene perchè quelle info ce le ho già nella matrice
			//degli stati, e non avrebbe neanche senso che io tutte le volte mi 
			//metta ad riaggiornare gli stati per poi utilizzarli solo qui			
			if(m_stati[indice*s.gr.n+k])
				t_d[k]=s.gr.l_proteine[k].t_decadimento;			
			}
		for(k=0;k<s.gr.n;k++)
			t_dec[indice*s.gr.n+k]=t_d[k];
		//solo per la modalità 3, aggiorno la mia matrice t_dec e chiamo trova_attrattore()
		if(sm.modalita==3){
			
			//cerco l'attrattore;
			if(cerca_attrattore(m_stati,t_dec,idx_new,idx_old, id_condiz)){
				//allora devo trovare il vero nome dell'attrattore più grande
				//trova_nome mi ritorna l'indice dello stato più grande dell'attrattore
				max_stato=trova_nome(m_stati,idx_new,id_condiz, t_dec);
				//copio lo stato dell'attrattore e i suoi tempi di decadimento
				for(k=0;k<s.gr.n;k++){
					sm.l_attr[id_condiz].stato[k]=m_stati[max_stato*s.gr.n+k];
					//non max_stato, ma max_stato-1 in modulo
					//(max_stato+sm.FINMAX-1)%sm.FINMAX;
					sm.l_attr[id_condiz].tdec[k]=t_dec[max_stato*s.gr.n+k];
					//setto il transiente
					sm.l_attr[id_condiz].l_trans=lungtrans;
					}				
				D2(printf("ho trovato il vero attrattore\n"));				
				return ;
				}
			}
		}
}

/** Funzione che calcola lo stato al passo t+1.
 * Prende in ingresso il gene per cui deve calcolare lo stato e l'array dei 
 * contatori dei tempi di decadimento. 
 * solo se il tempo di dec è >0 allora tale proteina sarà considerata a 1 
 * (altrimenti considerata a 0); si calcola per tutte le proteine in ingresso la 
 * somma dei corrispondenti valori decimali, così da ottenere l'indice esatto della 
 * f_out del gene.
 * Ritorna il novo stato del gene indice-esimo. 
 */
static bool fun_stato(int indice, int t_d[]){
	int i;
	//somma mi rappresenta l'indice della tabella di verità
	int somma=0;
	D2(printf("per il gene %d\n",indice));
	//per il gene selezionato, per ogni proteina in ingresso
	for(i=0;i<s.gr.l_geni[indice].kin;i++){
		//se il t_decadimento è attivo (vuol dire proteina attiva)
		if(t_d[s.gr.l_geni[indice].lp_in[i]]>0){
			D2(printf("--%d\n",s.gr.l_geni[indice].lp_in[i]));
			//sommo 2^(kin-1-i)
			//la proteina più significativa è la prima della lista delle adiacenze
			/*in questo caso con kin=2 avrò la più significativa che varrà 
				2^(kin-1-i) (i=0 più signif) --> =2^1=2 GIUSTO!!*/
			somma+=1<<(s.gr.l_geni[indice].kin-1-i);
			D2(printf("somma=%d\n",somma));		
			}
		//altrimenti è come se dovessi sommare 0*2^.. = 0
		}
	//ottenuto il numero decimale, accedo a f_out per sapere lo stato next
	return s.gr.l_geni[indice].f_out[somma];
}

/** Funzione che cerca l'attrattore nell'array circolare di dimensione FINMAX.
 * Paramentri in ingresso:
	-m_stati: matrice delgi stati
	-t_dec: matrice dei tempi di decadimento
	-idx_new: indice dell'ultimo stato calcolato
	-idx_old: indice del più vecchio stato mantenuto nell'array
	-id_cond: indice della condizione iniziale corrente
 * Ritorna true se trova l'attrattore e false altrimenti.
 * La funzione prima ricerca l'ugualianza dello stato e poi confronta i tempi di 
 * decadimento. parte dall'ultimo stato trovato (per gli altri, l'attrattore, sarebbe 
 * esistito al passo precedente) e lo confronta con tutti gli stati precedenti. 
 * se i nomi conbaciano allora confronta i t_decadimento, e se anch'essi combaciano, 
 * allora è un attrattore (il più corto possibile).
 * Altrimenti continua fino a scorrere tutta la lista circolare.
 */
static bool cerca_attrattore(bool *m_stati, int* t_dec, int idx_new, int idx_old, int id_cond){
	int i,j;
	//per ora faccio il base: l'ultimo lo ricerco per tutti gli stai precedenti
	//in futuro potrò fare una ricerca binaria dello stato su un array tenuto ordinato
	//se esiste, allora conto quante volte esiste e lo ricerco nella lista
	//circolare per tutte quelle volte finchè non trovo l'attrattore; se invece
	//con la ricerca binaria non lo trovo, allora evito anche di scorrere tutta 
	//la lista.

	//nome dell'ultimo stato trovato
	char last[s.gr.n+1];
	//nome dello stato corrente
	char nome[s.gr.n+1];
	//lunghezza dell'attrattore
	int len_attr=0;
	//idx mi indica l'elemento che devo confrontare con l'ultimo (parto da new -1)
	int idx=(idx_new+sm.FINMAX-1)%sm.FINMAX;
	//fine mi indica l'ultimo elemento della lista -1 (così lo confronto anche 
	//con l'ultimo) e a -1 esce
	int fine=(idx_old+sm.FINMAX-1)%sm.FINMAX;
	
	for(i=0;i<s.gr.n;i++)
		last[i]=(char)m_stati[idx_new*s.gr.n+i]+'0';
	last[i]='\0';
	D2(printf("ULTIMO %s\n",last););
	//guardo tutti i nomi degli stati precedenti
	for(i=1;idx!=fine;i++,len_attr++){
		//mi costruisco il nuovo nome
		for(j=0;j<s.gr.n;j++)
			nome[j]=(char)m_stati[idx*s.gr.n+j]+'0';
		nome[j]='\0';
		//se last è uguale al nome corrente allora guarda i tempi di decadimento
		if(strcmp(last,nome)==0){
			D2(printf("sono uguali!-----confronto i t_dec\n"););
			//controllo i t_dec
			for(j=0;j<s.gr.n;j++){
				//se trovo almeno un numero diverso tra i due, allora esco e 
				//continuo la ricerca				
				if(t_dec[idx_new*s.gr.n+j]!=t_dec[idx*s.gr.n+j]){
					D2(printf("tempi diversi!!!:(\n l'iterazione e' j=%d\n",j););
					D2(printf("i due tempi sono: %d e %d\n", t_dec[idx_new*s.gr.n+j],
											t_dec[idx*s.gr.n+j]));
					break;			
					}
				}
			//se mi sono fermato prima, vuol dire che non è un attrattore quindi 
			//continuo
			if(j!=s.gr.n){
				idx=(idx+sm.FINMAX-1)%sm.FINMAX;
				continue;
				}
			//se sono qui, vuol dire che ho trovato il mio attrattore
			//memorizzo la lunghezza e il nome che ho appena trovato ed esco
			sm.l_attr[id_cond].lung=len_attr+1;
			for(j=0;j<s.gr.n;j++)
				sm.l_attr[id_cond].stato[j]=m_stati[idx_new*s.gr.n+j];
			return true;
			}
		//altrimenti niente, passa al successivo e incrementa len_attr(dal for)		
		idx=(idx+sm.FINMAX-1)%sm.FINMAX;
		}
return false;
}

/** Funzione che cerca il nome più grande dell'attrattore.
 * Parametri in ingresso:
	- m_stati: la matrice degli stati
	- idx_new: l'indice del nuovo stato trovato (per il quale è stato trovato 
				l'attrattore)
	- id_cond: indice della condizione iniziale corrente
 * Ritorna la posizione del più grande stato dell'attrattore.
 * La funzione cerca il più grande stato tra idx_new e i precedenti lun_attr della
 * matrice degli stati(lun_attr è già stato settato in cerca_attrattore).
 */
static int trova_nome(bool *m_stati, int idx_new, int id_cond, int *t){
	//stringa ausiliaria che memorizza il nome del più grande stato
	char max[s.gr.n+1];
	char nome[s.gr.n+1];
	//idx mi serve come indice di nomi. parte da idx_new e decrementerà mano a 
	//mano fino a lung
	int i,j,k,idx=idx_new;
	//indice mi mantiene l'indice del massimo, che andrò a ritornare
	int indice=idx_new;

	for(i=0;i<s.gr.n;i++)
		max[i]=m_stati[idx*s.gr.n+i];
	max[i]='\0';
	//l'indice parte del penultimo stato trovato e scorre indietro nella lista 
	//circolare
	idx=(idx+sm.FINMAX-1)%sm.FINMAX;
	//per tutta la lunghezza dell'attrattore, partendo da uno, cerca il massimo
	for(i=1;i<sm.l_attr[id_cond].lung;i++){
		//mi copio in nome il nuovo stato da confrontare
		for(j=0;j<s.gr.n;j++)
			nome[j]=m_stati[idx*s.gr.n+j];
		nome[j]='\0';
		D2(printf("max=%s e nuovo=%s\n",max,nome));
		if(strcmp(max,nome)<0){
			D2(printf("max più piccolo\n"));
			strcpy(max,nome);
			indice=idx;
			}
/*
devo aggiungere:
		else if(strcmp(...)==0){
			allora guarda i tempi di decadimento
			cosa faccio mi creo un'altra stringa... o confronto uno per uno? 
			cofrontare uno per uno vuol dire che partendo dal primo, appena ne trovo uno diverso tra i due, scelgo come nome quello di quello che ha il t_dec[i] più alto

*/
		//se invece i gli stati sono uguali, allora devo verificare i t_dec
		else if(strcmp(max,nome)==0){
			//per ogni tempo, parto dal "più grande" 0 fino ad n
			for(k=0;k<s.gr.n;k++){
				//se il più grande è quello attuale, allora aggiorno max ed esco
				if(t[idx*s.gr.n+k]>t[indice*s.gr.n+k]){
					//il nome è ugual, basta solo aggiornare l'indice
					indice=idx;
					break;
					}
				//viceversa se l'attuale è più piccolo, non devo aggiornare e devo solo uscire
				else if(t[idx*s.gr.n+k]<t[indice*s.gr.n+k]){
					break;
					}
				//altrimenti sono uguali e passa al successivo
				}
			//if(k==s.gr.n) --> l'attrattore può essere solo di lunghezza uno!!	
			//e non dovrei mai essere qui perchè i parte da 1!!!!
			if(k==s.gr.n)
				printf("attrattore di lung 1!--- ma qui non dovrei mai esserci\n");
			}
		//idx decrementa in modulo
		idx=(idx+sm.FINMAX-1)%sm.FINMAX;
		}
	return indice;
}
