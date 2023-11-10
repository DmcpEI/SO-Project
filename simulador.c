#include "header.h"

void socketSimulador ()
{

	int sockfd, servlen;
	struct sockaddr_un serv_addr;

	if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0){
			printf("Erro ao abrir o Socket\n");
	}

	bzero((char *)&serv_addr,sizeof(serv_addr));
	serv_addr.sun_family = AF_UNIX;
	strcpy(serv_addr.sun_path, UNIXSTR_PATH);
	servlen = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);

	if(connect(sockfd, (struct sockaddr *)&serv_addr, servlen) < 0){
			printf("Não consegue conectar ao monitor\n");
			close(sockfd);
			exit(-1);
	}

	//str_cli("Conectou", sockfd);
	printf("Conectado com sucesso\n");

	close(sockfd);
	exit(0);
}

int main()
{

socketSimulador();

}

/*
	int main(int argc, char* argv[]){

    if(argc == 2){

	struct configuracao conf;
	FILE *ficheiro = fopen(argv[1], "r");

	if(ficheiro == NULL){
	    perror("Erro ao abrir o ficheiro");
	    return 1;
	}else if (strcmp(argv[1], "simulador.conf") != 0){
	perror("Ficheiro de configuração não existe.");
	return 1;
	}

	fseek(ficheiro, 0, SEEK_END);
        long tamanhoFicheiro = ftell(ficheiro);
        rewind(ficheiro);
        char buffer[tamanhoFicheiro];
        fread(buffer, 1, tamanhoFicheiro, ficheiro);
        fclose(ficheiro);

        int linhaAtual = 0;
        char *filtrada = strtok(buffer, "\n");
        char *linhas[40];
        while(filtrada != NULL){
	    linhas[linhaAtual++] = filtrada;
	    filtrada = strtok(NULL, "\n");
        }

	char *fim;
        char *array[2];
	char *valores[17];
	for(int index = 0; index < 17; index++){
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
        conf.probabilidadeMagoar = strtof(valores[13], &fim);
        conf.probabilidadeDesistir = strtof(valores[14], &fim);
        conf.tempoSimulacao = atoi(valores[15]);
    
        printf("Quantidade de pessoas no parque: %d\n", conf.quantidadePessoasParque);
        printf("Tempo de espera na entrada: %d\n", conf.tempoEsperaEntrada);
        printf("Número de atrações: %d\n", conf.numeroAtracoes);
        printf("Tempo de espera atracao 1: %d\n", conf.tempoEsperaAtracao1);
        printf("Tempo de espera atracao 2: %d\n", conf.tempoEsperaAtracao2);
        printf("Tempo de espera atracao 3: %d\n", conf.tempoEsperaAtracao3);
        printf("Tempo de espera atracao 4: %d\n", conf.tempoEsperaAtracao4);
        printf("Tempo de espera atracao 5: %d\n", conf.tempoEsperaAtracao5);
        printf("Tamanho da fila atracao 1: %d\n", conf.tamanhoFilaAtracao1);
        printf("Tamanho da fila atração 2: %d\n", conf.tamanhoFilaAtracao2);
        printf("Tamanho da fila atração 3: %d\n", conf.tamanhoFilaAtracao3);
        printf("Tamanho da fila atração 4: %d\n", conf.tamanhoFilaAtracao4);
        printf("Tamanho da fila atração 5: %d\n", conf.tamanhoFilaAtracao5);
        printf("Probabilidade de se magoar: %f\n", conf.probabilidadeMagoar);
        printf("Probabilidade de desistir: %f\n", conf.probabilidadeDesistir);
        printf("Tempo de Simulação: %d\n", conf.tempoSimulacao);
	return 0;
	}
	perror("É preciso como argumento o ficheiro de configuração.");
	return 1;
}
*/
