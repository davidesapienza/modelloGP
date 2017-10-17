/**Lancio standard su reti geni-proteine. 
 * Crea la struttura della rete.
 * Crea le condizioni iniziali.
 * Fa evolvere il sistema.

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

//Nomi degli eseguibili da utilizzare.
const char INVOCA_GS[]="/usr/bin/reti_gpbn/generatore_struttura";

const char INVOCA_GCI[]="/usr/bin/reti_gpbn/generatore_condizioni";

const char INVOCA_M[]="/usr/bin/reti_gpbn/motore";

/**Funzione principale che semplicemente lancia una sola volta il G_struttura, il 
 * G_C.I e il M.
 */
int main( int argc, char **argv){
	
	D1(printf("inizio\n"));
	if(system(INVOCA_GS)!=0){
		printf("Errore!! esecuzione fallita di %s\n",INVOCA_GS);
		return -1;}
	D1(printf("generatore ok\n"));
    if(system(INVOCA_GCI)!=0){
		printf("Errore!! esecuzione fallita di %s\n",INVOCA_GCI);
		return -1;}
	D1(printf("generatore condiz ok\n"));
	if(system(INVOCA_M)!=0){
		printf("Errore!! esecuzione fallita di %s\n",INVOCA_M);
		return -1;}
	D1(printf("motore ok\n"));
	return 0;
}

