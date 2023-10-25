#include "header.h"
#include <stdlib.h>

int main(int argc, char* argv[]){

    struct configuracao conf;
    
    if(argc != 2){
	printf("É preciso passar como argumento um ficheiro de configuração.\n");
	return 1;
    }

    FILE *ficheiro = fopen(argv[1], "r");

    if(ficheiro == NULL){
	perror("Erro ao abrir o ficheiro");
	return 1;
    }else{
	
	fseek(ficheiro, 0, SEEK_END);
	long tamanhoFicheiro = ftell(ficheiro);
	rewind(ficheiro);
	char buffer[tamanhoFicheiro];
	fread(buffer, 1, tamanhoFicheiro, ficheiro);
	fclose(ficheiro);

	int linhaAtual = 0;
	char *filtrada = strtok(buffer, "\n");
	char *linhas[30];
	while(filtrada != NULL){
	    linhas[linhaAtual++] = filtrada;
	    filtrada = strtok(NULL, "\n");
	}

	char *fim;
	char *array[2];
	char *valores[16];
	for(int index = 0; index < 12; index++){
	    char *aux = strtok(linhas[index], ":");
	    int i = 0;
	    while (aux != NULL){
		array[i++] = aux;
		aux = strtok(NULL, ":");
	    }
	    valores[index] = array[1];
	}
    
	conf.quantidadePessoasParque = atoi(valores[0]);
	conf.tempoEsperaEntrada = atoi(valores[1]);
	conf.numeroAtracoes = atoi(valores[2]);
	conf.tempoEsperaAtracao1 = atoi(valores[3]);
	conf.tempoEsperaAtracao2 = atoi(valores[4]);
	conf.tempoEsperaAtracao3 = atoi(valores[5]);
	conf.tempoEsperaAtracao4 = atoi(valores[6]);
	conf.tempoEsperaAtracao5 = atoi(valores[7]); 
	conf.tamanhoFilaAtracao1 = atoi(valores[8]); 
	conf.tamanhoFilaAtracao2 = atoi(valores[9]); 
	conf.tamanhoFilaAtracao3 = atoi(valores[10]); 
	conf.tamanhoFilaAtracao4 = atoi(valores[11]); 
	conf.tamanhoFilaAtracao5 = atoi(valores[12]);
	conf.probabilidadeMagoar = strtof(valores[13], &fim, 10);
	conf.probabilidadeDesistir = strtof(valores[14], &fim, 10);
	conf.tempoSimulacao = atoi(valores[15]);

	printf("Quantidade de pessoas no parque: %d\n", conf.quantidadePessoasParque);
	

	return 0;
    }
}
