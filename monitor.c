#include "header.h"

int erro = 0;

//Final da simulação
int finalSim = FALSE;

//Arquivo do relatório
FILE *relatorioFicheiro;

//Variável para aparecer no início da execução
int simulacaoIniciada = 0;

//Contadores de pessoas nas zonas
int tempoSimulado = 0, numPessoas = 0, numDesistencias = 0,  numPraca = 0, numNatacao = 0, numMergulho = 0, numTobogas = 0, numEnfermaria = 0, 
	numRestauracao = 0, numBalnearios = 0;

//Contadores de pessoas em espera
int  espParque=0, espNatacao = 0, espMergulho = 0, espTobogas = 0, espEnfermaria = 0, 
	 espRestauracao = 0, espBalnearios = 0;

//variaveis para fazer contas
int desistirFilaNatacao = 0;
int entraramFilaNatacao = 0;


void socketMonitor () {

	int sockfd, newsockfd;
	int cli_size, server_size;
	struct sockaddr_un serv_end, serv_addr;

	// Verifica a criação do socket
	if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
		printf("Erro ao criar o Socket\n");
		exit(-1);
	}

	// Incializa os valores do buffer a zero
	bzero((char *)&serv_addr, sizeof(serv_addr));
	serv_addr.sun_family = AF_UNIX;
	strcpy(serv_addr.sun_path, UNIXSTR_PATH);
	server_size = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);
	unlink(UNIXSTR_PATH);

	// Liga o socket a um endereço
    if (bind(sockfd, (struct sockaddr *)&serv_addr, server_size) < 0) {
        printf("erro: nao foi possivel ligar o socket a um endereco. \n");
		exit(-1);
   	}

	// Só no início da simulação
	if(!simulacaoIniciada){	
		printf("Execute o simulador para começar...\n");
		simulacaoIniciada = 1;
	}

	// Servidor espera para aceitar 1 cliente para o socket stream
	listen(sockfd, 1);

	// Criação de um novo scoket
	cli_size = sizeof(serv_end);
	newsockfd = accept (sockfd, (struct sockaddr *) &serv_end, &cli_size);

	// Verifica se houve erro na aceitação da ligação
	if (newsockfd < 0) {
		printf("erro: nao foi possivel aceitar a ligacao. \n");
		exit(-1);
	}

	/*Criação de um processo filho para implementar o codigo 
	pois irão haver vários clientes simultaneos 
	e o processo pai não pode tratar de um vez a vez*/

	int pFilho;
	// Caso haja erro na criação do processo filho
	if((pFilho=fork()) < 0){
		printf("Erro na criação do processo filho\n");
		exit(-1);
	}
	else if( pFilho == 0 ) {	
		close(sockfd);
		recebeDados(newsockfd);
		exit(0);
	}
	close(newsockfd);
}

// Função que recebe os dados do socket
void recebeDados(int newsockfd){

	int acabou = 0;
	int idPessoa = 0;
	int tempo = 0;
	int acao = 0;
	int zona = 0;
	int recebido = 0;

	while (!finalSim){
		// Necessário criar um buffer para alocar temporariamnete memória 
		char buffer[TAMANHO_BUFFER+1];
		
		// Dados recebidos do socket
		memset(buffer, 0, sizeof(buffer));
		recebido = recv(newsockfd, buffer, TAMANHO_BUFFER, 0);

		if (recebido <= 0){
			printf(VERMELHO "Não foram recebidos dados\n");
			break;
		}

		char *token = strtok(buffer, "|");

		while(token != NULL){
			sscanf(token,"%d %d %d %d %d", &acabou, &idPessoa, &tempo, &acao, &zona);

			processarOsDados(acabou, idPessoa, tempo, acao, zona);

			token = strtok(NULL, "|");
		}

		// Converte a string para um número inteiro e 


		//printf(buffer);

		/*Caso a variável "acabou" seja um número diferente de 0 
		significa que a simulaçao acabou e sendo assim acaba a simulação 
		e imprime os ultimos dados*/
		
		//system("clear");
	}
}

void processarOsDados(int acabou, int idPessoa, int tempo, int acao, int zona){

	system("clear");

	tempoSimulado = tempo;

	if(zona > 7) {
		printf("ERRO!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		printf("Ação: %d | Zona: %d | Erros: %d | Acabou: %d | ID: %d | Tempo: %d\n", acao, zona, erro, acabou, idPessoa, tempo);
		erro++;
	}

	switch (acabou){
		/*Caso a simulação esteja a decorrer este imprime o ID da pessoa que chegou
		ao Parque e incrementa o número de Pessoas no Parque, como as pessoas começam
		na praça este também irá incrementar o número de pessoas que estão lá*/
		case NAO_ACABOU:

			//printf("Ação: %d | Zona: %d | Erros: %d\n", acao, zona, erro);
			
			if (acao == ENTRAR){

				if(zona == PRACA){
					//printf("Tempo: %d",", tempoSimulado);
					printf("A pessoa com ID %d entrou no Parque\n", idPessoa);
					numPessoas++;

				}else if(zona == NATACAO){
					//printf("Tempo: %d", |", tempoSimulado);
					printf("A pessoa com ID %d entrou na atração da Natação\n", idPessoa);
					numNatacao++;

				}else if(zona == MERGULHO){
					//printf("Tempo: %d", |", tempoSimulado);
					printf("A pessoa com ID %d entrou na atração de Mergulho\n", idPessoa);
					numMergulho++;

				}else if(zona == TOBOGAS){
					//printf("Tempo: %d", |", tempoSimulado);
					printf("A pessoa com ID %d entrou na atração dos Tobogãs\n", idPessoa);
					numTobogas++;

				}else if(zona == RESTAURACAO){
					//printf("Tempo: %d", |", tempoSimulado);
					printf("A pessoa com ID %d entrou na Restauração\n", idPessoa);
					numRestauracao++;

				}else if(zona == BALNEARIOS){
					//printf("Tempo: %d", |", tempoSimulado);
					printf("A pessoa com ID %d entrou nos Balneários\n", idPessoa);
					numBalnearios++;

				}else if(zona == ENFERMARIA){
					//printf("Tempo: %d", |", tempoSimulado);
					printf("A pessoa com ID %d entrou na Enfermaria\n", idPessoa);
					numEnfermaria++;

				}

			} else if(acao == SAIR) {

				if(zona == PRACA){
					//printf("Tempo: %d", |", tempoSimulado);
					printf("A pessoa com ID %d saiu do Parque\n", idPessoa);
					numPessoas--;

				}else if(zona == NATACAO){
					//printf("Tempo: %d", |", tempoSimulado);
					printf("A pessoa com ID %d saiu da atração da Natação\n", idPessoa);
					numNatacao--;

				}else if(zona == MERGULHO){
					//printf("Tempo: %d", |", tempoSimulado);
					printf("A pessoa com ID %d saiu da atração de Mergulho\n", idPessoa);
					numMergulho--;

				}else if(zona == TOBOGAS){
					//printf("Tempo: %d", |", tempoSimulado);
					printf("A pessoa com ID %d saiu da atração dos Tobogãs\n", idPessoa);
					numTobogas--;

				}else if(zona == RESTAURACAO){
					//printf("Tempo: %d", |", tempoSimulado);
					printf("A pessoa com ID %d saiu da Restauração\n", idPessoa);
					numRestauracao--;

				}else if(zona == BALNEARIOS){
					//printf("Tempo: %d", |", tempoSimulado);
					printf("A pessoa com ID %d saiu dos Balneários\n", idPessoa);
					numBalnearios--;

				}else if(zona == ENFERMARIA){
					//printf("Tempo: %d", |", tempoSimulado);
					printf("A pessoa com ID %d saiu da Enfermaria\n", idPessoa);
					numEnfermaria--;

				}
				
			} else if (acao == ENTRAR_FILA) {
					
				if(zona == PRACA){
					//printf("Tempo: %d", |", tempoSimulado);
					printf("A pessoa com ID %d entrou na fila do Parque\n", idPessoa);
					espParque++;

				}else if(zona == NATACAO){
					//printf("Tempo: %d", |", tempoSimulado);
					printf("A pessoa com ID %d entrou na fila da atração da Natação\n", idPessoa);
					espNatacao++;
					entraramFilaNatacao++;

				}else if(zona == MERGULHO){
					//printf("Tempo: %d", |", tempoSimulado);
					printf("A pessoa com ID %d entrou na fila da atração de Mergulho\n", idPessoa);
					espMergulho++;

				}else if(zona == TOBOGAS){
					//printf("Tempo: %d", |", tempoSimulado);
					printf("A pessoa com ID %d entrou na fila da atração dos Tobogãs\n", idPessoa);
					espTobogas++;

				}else if(zona == RESTAURACAO){
					//printf("Tempo: %d", |", tempoSimulado);
					printf("A pessoa com ID %d entrou na fila da Restauração\n", idPessoa);
					espRestauracao++;

				}else if(zona == BALNEARIOS){
					//printf("Tempo: %d", |", tempoSimulado);
					printf("A pessoa com ID %d entrou na fila dos Balneários\n", idPessoa);
					espBalnearios++;

				}else if(zona == ENFERMARIA){
					//printf("Tempo: %d", |", tempoSimulado);
					printf("A pessoa com ID %d entrou na fila da Enfermaria\n", idPessoa);
					espEnfermaria++;
					
				}
				
			} else if (acao == SAIR_FILA) {

				if(zona == PRACA){
					
					//printf("Tempo: %d", |", tempoSimulado);
					printf("A pessoa com ID %d saiu da fila do Parque\n", idPessoa);
					espParque--;

				}else if(zona == NATACAO){
					
					//printf("Tempo: %d", |", tempoSimulado);
					printf("A pessoa com ID %d saiu da fila da atração da Natação\n", idPessoa);
					espNatacao--;
					desistirFilaNatacao++;

				}else if(zona == MERGULHO){

					//printf("Tempo: %d", |", tempoSimulado);
					printf("A pessoa com ID %d saiu da fila da atração de Mergulho\n", idPessoa);
					espMergulho--;

				}else if(zona == TOBOGAS){

					//printf("Tempo: %d", |", tempoSimulado);
					printf("A pessoa com ID %d saiu da fila da atração dos Tobogãs\n", idPessoa);
					espTobogas--;

				}else if(zona == RESTAURACAO){

					//printf("Tempo: %d", |", tempoSimulado);
					printf("A pessoa com ID %d saiu da fila da Restauração\n", idPessoa);
					espRestauracao--;

				}else if(zona == BALNEARIOS){

					//printf("Tempo: %d", |", tempoSimulado);
					printf("A pessoa com ID %d saiu da fila dos Balneários\n", idPessoa);
					espBalnearios--;

				}else if(zona == ENFERMARIA){

					//printf("Tempo: %d", |", tempoSimulado);
					printf("A pessoa com ID %d saiu da fila da Enfermaria\n", idPessoa);
					espEnfermaria--;

				}

			} else if (acao == SAIR_FILA_ENTRAR) {

				if(zona == PRACA){
					
					//printf("Tempo: %d", |", tempoSimulado);
					printf("A pessoa com ID %d saiu da fila e entrou do Parque\n", idPessoa);
					espParque--;
					numPessoas++;

				}else if(zona == NATACAO){
					
					//printf("Tempo: %d", |", tempoSimulado);
					printf("A pessoa com ID %d saiu da fila e entrou na atração da Natação\n", idPessoa);
					espNatacao--;
					numNatacao++;

				}else if(zona == MERGULHO){

					//printf("Tempo: %d", |", tempoSimulado);
					printf("A pessoa com ID %d saiu da fila e entrou na atração de Mergulho\n", idPessoa);
					espMergulho--;
					numMergulho++;

				}else if(zona == TOBOGAS){

					//printf("Tempo: %d", |", tempoSimulado);
					printf("A pessoa com ID %d saiu da fila e entrou na atração dos Tobogãs\n", idPessoa);
					espTobogas--;
					numTobogas++;

				}else if(zona == RESTAURACAO){

					//printf("Tempo: %d", |", tempoSimulado);
					printf("A pessoa com ID %d saiu da fila e entrou na Restauração\n", idPessoa);
					espRestauracao--;
					numRestauracao++;

				}else if(zona == BALNEARIOS){

					//printf("Tempo: %d", |", tempoSimulado);
					printf("A pessoa com ID %d saiu da fila e entrou nos Balneários\n", idPessoa);
					espBalnearios--;
					numBalnearios++;

				}else if(zona == ENFERMARIA){

					//printf("Tempo: %d", |", tempoSimulado);
					printf("A pessoa com ID %d saiu da fila e entrou na Enfermaria\n", idPessoa);
					espEnfermaria--;
					numEnfermaria++;

				}

			}else if (acao == SAIR_SAIR){
				if(zona==ENFERMARIA){
					//printf("Tempo: %d", |", tempoSimulado);
					printf("A pessoa com ID %d saiu da Enfermaria e foi para o hospital\n", idPessoa);
					numEnfermaria--;
					numPessoas--;
				} else if (zona==BALNEARIOS){
					//printf("Tempo: %d", |", tempoSimulado);
					printf("A pessoa com ID %d saiu dos Balnearios e saiu do Parque\n", idPessoa);
					numBalnearios--;
					numPessoas--;
				} else if (zona==TOBOGAS){
					//printf("Tempo: %d", |", tempoSimulado);
					printf("A pessoa com ID %d saiu dos Tobogas e saiu do Parque\n", idPessoa);
					numTobogas--;
					numPessoas--;
				} else if (zona==NATACAO){
					//printf("Tempo: %d", |", tempoSimulado);
					printf("A pessoa com ID %d saiu da Natacao e saiu do Parque\n", idPessoa);
					numNatacao--;
					numPessoas--;
				} else if (zona==MERGULHO){
					//printf("Tempo: %d", |", tempoSimulado);
					printf("A pessoa com ID %d saiu do Mergulho e saiu do Parque\n", idPessoa);
					numMergulho--;
					numPessoas--;
				} else if (zona==RESTAURACAO){
					//printf("Tempo: %d", |", tempoSimulado);
					printf("A pessoa com ID %d saiu da Restauracao e saiu do Parque\n", idPessoa);
					numRestauracao--;
					numPessoas--;
				}

			}
			
			
			imprimeDados();
			break;
			
		case ACABOU:
			finalSim = TRUE;
			imprimeDados();
			break;
		default:
			printf("Foi para o default!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
			break;
	
	}
}

// Exportação para o ficheiro

/*Função que limpa o ficheiro, 
usamos "w" pois caso exista algo no ficheiro este elimina o que já existe nele
caso contrario ele cria o ficheiro*/
void limpaFicheiro(){
	fclose(fopen("Relatorio.txt","w"));
}


// Função que escreve no ficheiro
void escreveFicheiro(char *informacao){

	// Começa por limpar o ficheiro pois só queremos os dados quando acabou a simulação
	limpaFicheiro();

	/*Abrimos o ficheiro e este usa "a" para dar append ou seja, 
	caso o ficheiro exista este adiciona ao ficheiro o texto que queremos, 
	caso contrário cria um novo*/

	relatorioFicheiro = fopen("Relatorio.txt","a");

	// Caso haja um erro o usuario seá notificado
	if(relatorioFicheiro == NULL){
		perror("Nao foi possivel abrir o ficheiro");
	}

	// Irá adicionar o ficherio a informação que queremos e depois irá fechar o ficheiro
	fprintf(relatorioFicheiro,"%s", informacao);
	fclose(relatorioFicheiro);
}


// Função que imprime na consola e escreve no ficheiro o estado atual da simulação
void imprimeDados() {

	// Poderá ser preciso mudar o 1000 caso se acrescente mais informação
    char informacao[1000]; 

	/*Irá colocar na variavel local informação a seguinte mensagem, 
	decidimos utilizar este formato pois ajuda na visualização da mensagem que 
	irá aparecer na consola*/
    sprintf(informacao,
        "======================================\n"
        "            PARQUE AQUATICO\n"
        "======================================\n"
        "Estado de execucao --> %s\n"
		"Tempo Simulado --> %d\n"
        "--------------------------------------\n"
        "Pessoas no Parque:		%d\n"
        "Desistencias:			%d\n"
        "--------------------------------------\n"
        "Pessoas na zona:\n"
        "---> Natacao:			%d\n"
		"---> Mergulho:			%d\n"
        "---> Tobogas:			%d\n"
        "---> Enfermaria:		%d\n"
        "---> Restauracao:		%d\n"
        "---> Balnearios:		%d\n"
        "--------------------------------------\n"
        "Pessoas a espera na fila:\n"
        "---> Parque:		        %d\n"
        "---> Natacao:			%d\n"
		"---> Mergulho:			%d\n"
        "---> Tobogas:			%d\n"
        "---> Enfermaria:		%d\n"
        "---> Restauracao:		%d\n"
        "---> Balnearios:		%d\n"
        "--------------------------------------\n",
        (!finalSim) ? "A decorrer" : "Finalizado",
        tempoSimulado,numPessoas, numDesistencias,
        numNatacao, numMergulho, numTobogas, numEnfermaria, numRestauracao, numBalnearios,
        espParque, espNatacao, espMergulho, espTobogas, espEnfermaria, espRestauracao, espBalnearios);

	//float conta = entraramFilaNatacao/desistirFilaNatacao;

	//printf("ratio de pessoas que desistiram da fila de natação: %f", conta);

   	// Escreve no ficheiro a informação
  	escreveFicheiro(informacao);	

    // Imprime a informação na consola
    printf("%s", informacao);
}

int main (void) {
	while (!finalSim){
		socketMonitor();
	}
	return 0;
}