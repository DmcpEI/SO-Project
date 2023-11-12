all: simulador monitor

simulador: simulador.c header.h
	gcc -c -g simulador.c
	gcc -g -o simulador simulador.o -lpthread

monitor: monitor.c header.h
	gcc -c -g monitor.c
	gcc -g -o monitor monitor.o -lpthread

clean:
	rm *.o simulador monitor

fullClean:
	rm *.o simulador monitor
	rm *.txt
