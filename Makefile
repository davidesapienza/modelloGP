#OBJ0 = struttura_dati.h struttura_path.h
OBJ1 = generatore_struttura/main_generatore.o generatore_struttura/generatore.o generatore_struttura/operazioni_file.o 
OBJ2 = motore/main_motore.o motore/motore.o motore/operazioni_file.o
OBJ3 = gen_condiz_ini/main_gen_condiz_ini.o gen_condiz_ini/operazioni_file.o
OBJ4 = esperimento/funzionalit.o esperimento/operazioni_file.o
OBJ5 = esperimento/esp_derrida/esp_derrida.o esperimento/funzionalit.o esperimento/operazioni_file.o esperimento/operazioni_comuni_derrida.o
OBJ6 = esperimento/esp_fig2/esp_fig2.o esperimento/funzionalit.o esperimento/operazioni_file.o
OBJ7 = esperimento/esp_fig345/esp_fig345.o esperimento/funzionalit.o esperimento/operazioni_file.o
OBJ8 = esperimento/esp_fig7/esp_fig7.o esperimento/funzionalit.o esperimento/operazioni_file.o
OBJ9 = esperimento/esp_derrida_geni/esp_derrida_geni.o esperimento/funzionalit.o esperimento/operazioni_file.o esperimento/operazioni_comuni_derrida.o
OBJ10 = esperimento/esp_derrida_geni_composto/esp_derrida_geni_composto.o esperimento/funzionalit.o esperimento/operazioni_file.o
OBJ11 = esperimento/esp_attrattori/esp_attrattori.o esperimento/funzionalit.o esperimento/operazioni_file.o esperimento/operazioni_comuni_derrida.o
OBJ12 = esperimento/esp_riporta_ciclo/esp_riporta_ciclo.o esperimento/funzionalit.o esperimento/operazioni_file.o
OBJ13 = esperimento/esp_attrattori_composto/esp_attrattori_composto.o esperimento/funzionalit.o esperimento/operazioni_file.o
OBJ14 = esperimento/esp_lancio/lancio.o esperimento/funzionalit.o esperimento/operazioni_file.o
#DEBUG_FLAGS=-D DEBUG_MODE 
CFLAGS=$(DEBUG_FLAGS) -Wall -lm
 
generatore: $(OBJ1) $(OBJ2) $(OBJ3) $(OBJ4) $(OBJ5) $(OBJ6) $(OBJ7) $(OBJ8) $(OBJ9) $(OBJ10) $(OBJ11) $(OBJ12) $(OBJ13) $(OBJ14)
	gcc -o generatore_struttura/generatore_struttura  $(OBJ1) $(CFLAGS)
	gcc -o motore/motore $(OBJ2) $(CFLAGS)
	gcc -o gen_condiz_ini/generatore_condizioni $(OBJ3) $(CFLAGS)
	gcc -c $(OBJ4) $(CFLAGS)
	gcc -o esperimento/esp_derrida/esp_derrida $(OBJ5) $(CFLAGS)
	gcc -o esperimento/esp_fig2/esp_fig2 $(OBJ6) $(CFLAGS)
	gcc -o esperimento/esp_fig345/esp_fig345 $(OBJ7) $(CFLAGS)
	gcc -o esperimento/esp_fig7/esp_fig7 $(OBJ8) $(CFLAGS)
	gcc -o esperimento/esp_derrida_geni/esp_derrida_geni $(OBJ9) $(CFLAGS)
	gcc -o esperimento/esp_derrida_geni_composto/esp_derrida_geni_composto $(OBJ10) $(CFLAGS)
	gcc -o esperimento/esp_attrattori/esp_attrattori $(OBJ11) $(CFLAGS)
	gcc -o esperimento/esp_riporta_ciclo/esp_riporta_ciclo $(OBJ12) $(CFLAGS) 
	gcc -o esperimento/esp_attrattori_composto/esp_attrattori_composto $(OBJ13) $(CFLAGS)
	gcc -o esperimento/esp_lancio/lancio $(OBJ14) $(CFLAGS)
motore: $(OBJ2)
	gcc -o motore/motore $(OBJ2) $(CFLAGS)
generatore_condiz: $(OBJ3)
	gcc -o gen_condiz_ini/generatore_condizioni $(OBJ3) $(CFLAGS)
file_comuni_esperimenti: $(OBJ4)
	gcc -c $(OBJ0) $(OBJ4) $(CFLAGS)
esperimento_derrida: $(OBJ5)
	gcc -o esperimento/esp_derrida/esp_derrida $(OBJ5) $(CFLAGS)
esperimento_fig2: $(OBJ6)
	gcc -o esperimento/esp_fig2/esp_fig2 $(OBJ6) $(CFLAGS)
esperimento_fig345: $(OBJ7)
	gcc -o esperimento/esp_fig345/esp_fig345 $(OBJ7) $(CFLAGS)
esperimento_fig7: $(OBJ8)
	gcc -o esperimento/esp_fig7/esp_fig7 $(OBJ8) $(CFLAGS)
esperimento_derrida_geni: $(OBJ9)
	gcc -o esperimento/esp_derrida_geni/esp_derrida_geni $(OBJ9) $(CFLAGS)
esperimento_derrida_geni_composto: $(OBJ10)
	gcc -o esperimento/esp_derrida_geni_composto/esp_derrida_geni_composto $(OBJ10) $(CFLAGS)
esp_attrattori: $(OBJ11)
	gcc -o esperimento/esp_attrattori/esp_attrattori $(OBJ11) $(CFLAGS)
esp_riporta_ciclo: $(OBJ12)
	gcc -o esperimento/esp_riporta_ciclo/esp_riporta_ciclo $(OBJ12) $(CFLAGS) 
esp_attrattori_composto: $(OBJ13)
	gcc -o esperimento/esp_attrattori_composto/esp_attrattori_composto $(OBJ13) $(CFLAGS)
lancio: $(OBJ14)
	gcc -o esperimento/esp_lancio/lancio $(OBJ14) $(CFLAGS)

-include dependencies

.PHONY: depend clean cleanall

install: 
	ln -n generatore_struttura/generatore_struttura /usr/bin/reti_gpbn/generatore_struttura
	ln -n gen_condiz_ini/generatore_condizioni /usr/bin/reti_gpbn/generatore_condizioni
	ln -n motore/motore /usr/bin/reti_gpbn/motore
	ln -n esperimento/esp_derrida/esp_derrida /usr/bin/reti_gpbn/esp_derrida
	ln -n esperimento/esp_fig2/esp_fig2 /usr/bin/reti_gpbn/esp_fig2
	ln -n esperimento/esp_fig345/esp_fig345 /usr/bin/reti_gpbn/esp_fig345
	ln -n esperimento/esp_fig7/esp_fig7 /usr/bin/reti_gpbn/esp_fig7
	ln -n esperimento/esp_lancio/lancio /usr/bin/reti_gpbn/lancio
	ln -n esperimento/esp_derrida_geni/esp_derrida_geni /usr/bin/reti_gpbn/esp_derrida_geni	
	ln -n esperimento/esp_derrida_geni_composto/esp_derrida_geni_composto /usr/bin/reti_gpbn/esp_derrida_geni_composto
	ln -n esperimento/esp_attrattori/esp_attrattori /usr/bin/reti_gpbn/esp_attrattori
	ln -n esperimento/esp_riporta_ciclo/esp_riporta_ciclo /usr/bin/reti_gpbn/esp_riporta_ciclo
	ln -n esperimento/esp_attrattori_composto/esp_attrattori_composto /usr/bin/reti_gpbn/esp_attrattori_composto

uninstall:
	rm /usr/bin/reti_gpbn/*

depend:
	gcc -MM *.c > dependencies
clean:
	find . -name '*.o' -o -name '*.gch' -delete
cleanall:
	find . -name '*.o' -o -name '*.gch' -o -name '*~' -delete 

