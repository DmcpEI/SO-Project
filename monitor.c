#include "header.h"

int erro = 0;

//Final da simulação
int finalSim = FALSE;

//Arquivo do relatório
FILE *relatorioFicheiro;

//Variável para aparecer no início da execução
int simulacaoIniciada = 0;

float tempoSimulado = 0;

//Contadores de pessoas nas zonas
int numPessoas = 0, numPessoasSairam = 0, numPraca = 0, 
	numDesistencias = 0, numNatacao = 0, numMergulho = 0, numTobogas = 0,
	numEnfermaria = 0, numRestauracao = 0, numBalnearios = 0;

//Contadores de pessoas em espera
int  espParque=0, espNatacao = 0, espMergulho = 0, espTobogas = 0, espEnfermaria = 0, 
	 espRestauracao = 0, espBalnearios = 0;

//variaveis para fazer contas
unsigned int desistirFilaParque = 0;
unsigned int entraramFilaParque = 0;
int ratioFilaParque = 0;

unsigned int desistirFilaNatacao = 0;
unsigned int entraramFilaNatacao = 0;
int ratioFilaNatacao = 0;

unsigned int desistirFilaMergulho = 0;
unsigned int entraramFilaMergulho = 0;
int ratioFilaMergulho = 0;

unsigned int desistirFilaTobogas = 0;
unsigned int entraramFilaTobogas = 0;
int ratioFilaTobogas = 0;

unsigned int desistirFilaRestauracao = 0;
unsigned int entraramFilaRestauracao = 0;
int ratioFilaRestauracao = 0;

unsigned int desistirFilaBalnearios = 0;
unsigned int entraramFilaBalnearios = 0;
int ratioFilaBalnearios = 0;

unsigned int desistirFilaEnfermaria = 0;
unsigned int entraramFilaEnfermaria = 0;
int ratioFilaEnfermaria = 0;


unsigned int  totalEntrarParque = 0;

unsigned int totalEntrarNatacao = 0;
int ratioEntrarNatacao = 0;

unsigned int totalEntrarMergulho = 0;
int ratioEntrarMergulho = 0;

unsigned int totalEntrarTobogas = 0;
int ratioEntrarTobogas = 0;

unsigned int totalEntrarRestauracao = 0;
int ratioEntrarRestauracao = 0;

unsigned int totalEntrarBalnearios = 0;
int ratioEntrarBalnearios = 0;

unsigned int totalEntrarEnfermaria = 0;
int ratioEntrarEnfermaria = 0;


unsigned int totalTempoChegada = 0;
unsigned int totalTempoSaida = 0;
float mediaTempoParqueFinal = 0;

unsigned int totalChegadaNatacao = 0;
unsigned int totalSaidaNatacao = 0;
float mediaTempoNatacaoFinal = 0;

unsigned int totalChegadaMergulho = 0;
unsigned int totalSaidaMergulho = 0;
float mediaTempoMergulhoFinal = 0;

unsigned int totalChegadaTobogas = 0;
unsigned int totalSaidaTobogas = 0;
float mediaTempoTobogasFinal = 0;

unsigned int totalChegadaRestauracao = 0;
unsigned int totalSaidaRestauracao = 0;
float mediaTempoRestauracaoFinal = 0;

unsigned int totalChegadaBalnearios = 0;
unsigned int totalSaidaBalnearios = 0;
float mediaTempoBalneariosFinal = 0;

unsigned int totalChegadaEnfermaria = 0;
unsigned int totalSaidaEnfermaria = 0;
float mediaTempoEnfermariaFinal = 0;

/////////acabou variaveis para contas///////////

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

		acabou = 0;
		idPessoa = 0;
		tempo = 0;
		acao = 0;
		zona = 0;

	}
}

void processarOsDados(int acabou, int idPessoa, int tempo, int acao, int zona){

	system("clear");

	if(tempo<60){
		tempoSimulado = tempo/100.0;
	} else {
		int segundos = tempo % 60;  
    	int minutos = tempo / 60.0;  

    	tempoSimulado = minutos + (segundos / 100.0);
	}
	
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
					printf("A pessoa com ID %d entrou no Parque\n", idPessoa);
					numPraca++;
					totalEntrarParque++;
					totalTempoChegada += tempo;

				}else if(zona == NATACAO){
					printf("A pessoa com ID %d entrou na atração da Natação\n", idPessoa);
					numNatacao++;
					numPraca--;
					totalEntrarNatacao++;
					totalChegadaNatacao += tempo;

				}else if(zona == MERGULHO){
					printf("A pessoa com ID %d entrou na atração de Mergulho\n", idPessoa);
					numMergulho++;
					numPraca--;
					totalEntrarMergulho++;
					totalChegadaMergulho += tempo;

				}else if(zona == TOBOGAS){
					printf("A pessoa com ID %d entrou na atração dos Tobogãs\n", idPessoa);
					numTobogas++;
					numPraca--;
					totalEntrarTobogas++;
					totalChegadaTobogas += tempo;

				}else if(zona == RESTAURACAO){
					printf("A pessoa com ID %d entrou na Restauração\n", idPessoa);
					numRestauracao++;
					numPraca--;
					totalEntrarRestauracao++;
					totalChegadaRestauracao += tempo;

				}else if(zona == BALNEARIOS){
					printf("A pessoa com ID %d entrou nos Balneários\n", idPessoa);
					numBalnearios++;
					numPraca--;
					totalEntrarBalnearios++;
					totalChegadaBalnearios += tempo;

				}else if(zona == ENFERMARIA){
					printf("A pessoa com ID %d entrou na Enfermaria\n", idPessoa);
					numEnfermaria++;
					numPraca--;
					totalEntrarEnfermaria++;
					totalChegadaEnfermaria += tempo;

				}

			} else if(acao == SAIR) {

				if(zona == PRACA){

					printf("A pessoa com ID %d saiu do Parque\n", idPessoa);
					numPraca--;
					numPessoasSairam++;
					totalTempoSaida += tempo;

				}else if(zona == NATACAO){

					printf("A pessoa com ID %d saiu da atração da Natação\n", idPessoa);
					numNatacao--;
					numPraca++;
					totalSaidaNatacao += tempo;

				}else if(zona == MERGULHO){

					printf("A pessoa com ID %d saiu da atração de Mergulho\n", idPessoa);
					numMergulho--;
					numPraca++;
					totalSaidaMergulho += tempo;

				}else if(zona == TOBOGAS){

					printf("A pessoa com ID %d saiu da atração dos Tobogãs\n", idPessoa);
					numTobogas--;
					numPraca++;
					totalSaidaTobogas += tempo;

				}else if(zona == RESTAURACAO){

					printf("A pessoa com ID %d saiu da Restauração\n", idPessoa);
					numRestauracao--;
					numPraca++;
					totalSaidaRestauracao += tempo;

				}else if(zona == BALNEARIOS){

					printf("A pessoa com ID %d saiu dos Balneários\n", idPessoa);
					numBalnearios--;
					numPraca++;
					totalSaidaBalnearios += tempo;

				}else if(zona == ENFERMARIA){

					printf("A pessoa com ID %d saiu da Enfermaria\n", idPessoa);
					numEnfermaria--;
					numPraca++;
					totalSaidaEnfermaria += tempo;

				}
				
			} else if (acao == ENTRAR_FILA) {
					
				if(zona == PRACA){
					printf("A pessoa com ID %d entrou na fila do Parque\n", idPessoa);
					espParque++;
					entraramFilaParque++;

				}else if(zona == NATACAO){
					printf("A pessoa com ID %d entrou na fila da atração da Natação\n", idPessoa);
					espNatacao++;
					numPraca--;
					entraramFilaNatacao++;

				}else if(zona == MERGULHO){
					printf("A pessoa com ID %d entrou na fila da atração de Mergulho\n", idPessoa);
					espMergulho++;
					numPraca--;
					entraramFilaMergulho++;

				}else if(zona == TOBOGAS){
					printf("A pessoa com ID %d entrou na fila da atração dos Tobogãs\n", idPessoa);
					espTobogas++;
					numPraca--;
					entraramFilaTobogas++;

				}else if(zona == RESTAURACAO){
					printf("A pessoa com ID %d entrou na fila da Restauração\n", idPessoa);
					espRestauracao++;
					numPraca--;
					entraramFilaRestauracao++;

				}else if(zona == BALNEARIOS){
					printf("A pessoa com ID %d entrou na fila dos Balneários\n", idPessoa);
					espBalnearios++;
					numPraca--;
					entraramFilaBalnearios++;

				}else if(zona == ENFERMARIA){
					printf("A pessoa com ID %d entrou na fila da Enfermaria\n", idPessoa);
					espEnfermaria++;
					numPraca--;
					entraramFilaEnfermaria++;
					
				}
				
			} else if (acao == SAIR_FILA) {

				if(zona == PRACA){
					
					printf("A pessoa com ID %d saiu da fila do Parque\n", idPessoa);
					espParque--;
					numDesistencias++;
					desistirFilaParque++;

				}else if(zona == NATACAO){
					
					printf("A pessoa com ID %d saiu da fila da atração da Natação\n", idPessoa);
					espNatacao--;
					numPraca++;
					desistirFilaNatacao++;

				}else if(zona == MERGULHO){

					printf("A pessoa com ID %d saiu da fila da atração de Mergulho\n", idPessoa);
					espMergulho--;
					numPraca++;
					desistirFilaMergulho++;

				}else if(zona == TOBOGAS){

					printf("A pessoa com ID %d saiu da fila da atração dos Tobogãs\n", idPessoa);
					espTobogas--;
					numPraca++;
					desistirFilaTobogas++;

				}else if(zona == RESTAURACAO){

					printf("A pessoa com ID %d saiu da fila da Restauração\n", idPessoa);
					espRestauracao--;
					numPraca++;
					desistirFilaRestauracao++;

				}else if(zona == BALNEARIOS){

					printf("A pessoa com ID %d saiu da fila dos Balneários\n", idPessoa);
					espBalnearios--;
					numPraca++;
					desistirFilaBalnearios++;

				}else if(zona == ENFERMARIA){

					printf("A pessoa com ID %d saiu da fila da Enfermaria\n", idPessoa);
					espEnfermaria--;
					numPraca++;
					desistirFilaEnfermaria++;

				}

			} else if (acao == SAIR_FILA_ENTRAR) {

				if(zona == PRACA){
					
					printf("A pessoa com ID %d saiu da fila e entrou do Parque\n", idPessoa);
					espParque--;
					numPraca++;
					totalEntrarParque++;
					totalTempoChegada += tempo;

				}else if(zona == NATACAO){
					
					printf("A pessoa com ID %d saiu da fila e entrou na atração da Natação\n", idPessoa);
					espNatacao--;
					numNatacao++;
					totalEntrarNatacao++;
					totalChegadaNatacao += tempo;

				}else if(zona == MERGULHO){

					printf("A pessoa com ID %d saiu da fila e entrou na atração de Mergulho\n", idPessoa);
					espMergulho--;
					numMergulho++;
					totalEntrarMergulho++;
					totalChegadaMergulho += tempo;

				}else if(zona == TOBOGAS){

					printf("A pessoa com ID %d saiu da fila e entrou na atração dos Tobogãs\n", idPessoa);
					espTobogas--;
					numTobogas++;
					totalEntrarTobogas++;
					totalChegadaTobogas += tempo;

				}else if(zona == RESTAURACAO){

					printf("A pessoa com ID %d saiu da fila e entrou na Restauração\n", idPessoa);
					espRestauracao--;
					numRestauracao++;
					totalEntrarRestauracao++;
					totalChegadaRestauracao += tempo;

				}else if(zona == BALNEARIOS){

					printf("A pessoa com ID %d saiu da fila e entrou nos Balneários\n", idPessoa);
					espBalnearios--;
					numBalnearios++;
					totalEntrarBalnearios++;
					totalChegadaBalnearios += tempo;

				}else if(zona == ENFERMARIA){

					printf("A pessoa com ID %d saiu da fila e entrou na Enfermaria\n", idPessoa);
					espEnfermaria--;
					numEnfermaria++;
					totalEntrarEnfermaria++;
					totalChegadaEnfermaria += tempo;

				}

			}else if (acao == SAIR_SAIR){
				if(zona==ENFERMARIA){
					printf("A pessoa com ID %d saiu da Enfermaria e foi para o hospital\n", idPessoa);
					numEnfermaria--;
					numPessoasSairam++;
					totalTempoSaida += tempo;
					totalSaidaEnfermaria += tempo;

				} else if (zona==BALNEARIOS){
					printf("A pessoa com ID %d saiu dos Balnearios e saiu do Parque\n", idPessoa);
					numBalnearios--;
					numPessoasSairam++;
					totalTempoSaida += tempo;
					totalSaidaBalnearios += tempo;

				} else if (zona==TOBOGAS){
					printf("A pessoa com ID %d saiu dos Tobogas e saiu do Parque\n", idPessoa);
					numTobogas--;
					numPessoasSairam++;
					totalTempoSaida += tempo;
					totalSaidaTobogas += tempo;

				} else if (zona==NATACAO){
					printf("A pessoa com ID %d saiu da Natacao e saiu do Parque\n", idPessoa);
					numNatacao--;
					numPessoasSairam++;
					totalTempoSaida += tempo;
					totalSaidaNatacao += tempo;

				} else if (zona==MERGULHO){
					printf("A pessoa com ID %d saiu do Mergulho e saiu do Parque\n", idPessoa);
					numMergulho--;
					numPessoasSairam++;
					totalTempoSaida += tempo;
					totalSaidaMergulho += tempo;

				} else if (zona==RESTAURACAO){
					printf("A pessoa com ID %d saiu da Restauracao e saiu do Parque\n", idPessoa);
					numRestauracao--;
					numPessoasSairam++;
					totalTempoSaida += tempo;
					totalSaidaRestauracao += tempo;
				}

			}else if (acao == SAIR_FILA_ENFERMARIA){
				printf("A pessoa com ID %d saiu da fila da Enfermaria e foi para o hospital\n", idPessoa);
				espEnfermaria--;
				numPessoasSairam++;
				desistirFilaEnfermaria++;
				totalTempoSaida += tempo;
			}
			
			numPessoas = numBalnearios + numEnfermaria + numMergulho + numNatacao + numPraca +
						 numRestauracao + numTobogas + espBalnearios + espEnfermaria + 
						 espMergulho + espNatacao + espRestauracao + espTobogas;

			if(entraramFilaParque != 0){
				ratioFilaParque = (int)(((float)desistirFilaParque / entraramFilaParque) * 100);
			}
			if(entraramFilaNatacao != 0){
				ratioFilaNatacao = (int)(((float)desistirFilaNatacao / entraramFilaNatacao) * 100);
			}
			if(entraramFilaTobogas != 0){
				ratioFilaTobogas = (int)(((float)desistirFilaTobogas / entraramFilaTobogas) * 100);
			}
			if(entraramFilaRestauracao != 0){
				ratioFilaRestauracao = (int)(((float)desistirFilaRestauracao / entraramFilaRestauracao) * 100);
			}
			if(entraramFilaMergulho != 0){
				ratioFilaMergulho = (int)(((float)desistirFilaMergulho / entraramFilaMergulho) * 100);
			}
			if(entraramFilaBalnearios != 0){
				ratioFilaBalnearios = (int)(((float)desistirFilaBalnearios / entraramFilaBalnearios) * 100);
			}
			if(entraramFilaEnfermaria != 0){
				ratioFilaEnfermaria = (int)(((float)desistirFilaEnfermaria / entraramFilaEnfermaria) * 100);
			}

			ratioEntrarNatacao = (int)(((float)totalEntrarNatacao / totalEntrarParque) * 100);
			ratioEntrarTobogas = (int)(((float)totalEntrarTobogas / totalEntrarParque) * 100);
			ratioEntrarRestauracao = (int)(((float)totalEntrarRestauracao / totalEntrarParque) * 100);
			ratioEntrarMergulho = (int)(((float)totalEntrarMergulho / totalEntrarParque) * 100);
			ratioEntrarBalnearios = (int)(((float)totalEntrarBalnearios / totalEntrarParque) * 100);
			ratioEntrarEnfermaria = (int)(((float)totalEntrarEnfermaria / totalEntrarParque) * 100);
			
			imprimeDados();
			break;
			
		case ACABOU:
			finalSim = TRUE;

			int mediaTempoParqueSec = (((totalTempoSaida - totalTempoChegada) / totalEntrarParque) % 60);
			int mediaTempoParqueMin = (((totalTempoSaida - totalTempoChegada) / totalEntrarParque) / 60.0);
			mediaTempoParqueFinal = mediaTempoParqueMin + (mediaTempoParqueSec / 100.0);

			int mediaTempoNatacaoSec = (((totalSaidaNatacao - totalChegadaNatacao) / totalEntrarNatacao) % 60);
			int mediaTempoNatacaoMin = (((totalSaidaNatacao - totalChegadaNatacao) / totalEntrarNatacao) / 60.0);
			mediaTempoNatacaoFinal = mediaTempoNatacaoMin + (mediaTempoNatacaoSec / 100.0);

			int mediaTempoMergulhoSec = (((totalSaidaMergulho - totalChegadaMergulho) / totalEntrarMergulho) % 60);
			int mediaTempoMergulhoMin = (((totalSaidaMergulho - totalChegadaMergulho) / totalEntrarMergulho) / 60.0);
			mediaTempoMergulhoFinal = mediaTempoMergulhoMin + (mediaTempoMergulhoSec / 100.0);

			int mediaTempoTobogasSec = (((totalSaidaTobogas - totalChegadaTobogas) / totalEntrarTobogas) % 60);
			int mediaTempoTobogasMin = (((totalSaidaTobogas - totalChegadaTobogas) / totalEntrarTobogas) / 60.0);
			mediaTempoTobogasFinal = mediaTempoTobogasMin + (mediaTempoTobogasSec / 100.0);

			int mediaTempoEnfermariaSec = (((totalSaidaEnfermaria - totalChegadaEnfermaria) / totalEntrarEnfermaria) % 60);
			int mediaTempoEnfermariaMin = (((totalSaidaEnfermaria - totalChegadaEnfermaria) / totalEntrarEnfermaria) / 60.0);
			mediaTempoEnfermariaFinal = mediaTempoEnfermariaMin + (mediaTempoEnfermariaSec / 100.0);

			int mediaTempoRestauracaoSec = (((totalSaidaRestauracao - totalChegadaRestauracao) / totalEntrarRestauracao) % 60);
			int mediaTempoRestauracaoMin = (((totalSaidaRestauracao - totalChegadaRestauracao) / totalEntrarRestauracao) / 60.0);
			mediaTempoRestauracaoFinal = mediaTempoRestauracaoMin + (mediaTempoRestauracaoSec / 100.0);

			int mediaTempoBalneariosSec = (((totalSaidaBalnearios - totalChegadaBalnearios) / totalEntrarBalnearios) % 60);
			int mediaTempoBalneariosMin = (((totalSaidaBalnearios - totalChegadaBalnearios) / totalEntrarBalnearios) / 60.0);
			mediaTempoBalneariosFinal = mediaTempoBalneariosMin + (mediaTempoBalneariosSec / 100.0);

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
	fclose(fopen("Resumo_Execucao.txt","w"));
}


// Função que escreve no ficheiro
void escreveFicheiro(char *informacao){

	// Começa por limpar o ficheiro pois só queremos os dados quando acabou a simulação
	limpaFicheiro();

	/*Abrimos o ficheiro e este usa "a" para dar append ou seja, 
	caso o ficheiro exista este adiciona ao ficheiro o texto que queremos, 
	caso contrário cria um novo*/

	relatorioFicheiro = fopen("Resumo_Execucao.txt","a");

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
    char informacao[2000]; 

	/*Irá colocar na variavel local informação a seguinte mensagem, 
	decidimos utilizar este formato pois ajuda na visualização da mensagem que 
	irá aparecer na consola*/
    sprintf(informacao,
        "======================================\n"
        "            PARQUE AQUATICO\n"
        "======================================\n"
        "Estado de execucao --> %s\n"
		"Tempo Simulado(minutos) --> %.2f\n"
        "--------------------------------------\n"
        "Pessoas no Parque:		%d\n"
        "Pessoas que saíram:		%d\n"
		"Desistencias:			%d\n"
        "--------------------------------------\n"
        "Pessoas na zona:\n"
		"---> Praça:			%d\n"
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
        "--------------------------------------\n"
		"Taxas de desistência ao estar na fila:\n"
		"---> Parque (%d):		%d%%\n"
        "---> Natacao (%d):		%d%%\n"
		"---> Mergulho (%d):		%d%%\n"
        "---> Tobogas (%d):		%d%%\n"
        "---> Enfermaria (%d):		%d%%\n"
        "---> Restauracao (%d):		%d%%\n"
        "---> Balnearios (%d):		%d%%\n"
		"--------------------------------------\n"
		"Taxas de acesso da zona:\n"//qual a atracao que as pessoas vao mais
		"---> Natacao:		        %d%%\n"
		"---> Mergulho:			%d%%\n"
        "---> Tobogas:			%d%%\n"
        "---> Enfermaria:		%d%%\n"
        "---> Restauracao:		%d%%\n"
        "---> Balnearios:		%d%%\n"
		"--------------------------------------\n",
        (!finalSim) ? "A decorrer" : "Finalizado",
        tempoSimulado, numPessoas, numPessoasSairam, numDesistencias,
        numPraca, numNatacao, numMergulho, numTobogas, numEnfermaria, numRestauracao, numBalnearios,
        espParque, espNatacao, espMergulho, espTobogas, espEnfermaria, espRestauracao, espBalnearios,
		entraramFilaParque, ratioFilaParque, entraramFilaNatacao, ratioFilaNatacao, entraramFilaMergulho, ratioFilaMergulho, entraramFilaTobogas, 
		ratioFilaTobogas, entraramFilaEnfermaria, ratioFilaEnfermaria, entraramFilaRestauracao, ratioFilaRestauracao, entraramFilaBalnearios, ratioFilaBalnearios, 
		ratioEntrarNatacao, ratioEntrarMergulho, ratioEntrarTobogas, ratioEntrarEnfermaria, ratioEntrarRestauracao, ratioEntrarBalnearios);

    // Imprime a informação na consola
    printf("%s", informacao);

	if(finalSim) {

		system("clear");

		sprintf(informacao,
        "======================================\n"
        "            PARQUE AQUATICO\n"
        "======================================\n"
        "Estado de execucao --> Finalizado\n"
		"Tempo Simulado Final(minutos) --> %.2f\n"
        "--------------------------------------\n"
        "Número de entradas:		%d\n"
		"Desistencias:			%d\n"
        "--------------------------------------\n"
		"Taxas de desistência ao estar na fila:\n"
		"---> Parque:		        %d%%\n"
        "---> Natacao:			%d%%\n"
		"---> Mergulho:			%d%%\n"
        "---> Tobogas:			%d%%\n"
        "---> Enfermaria:		%d%%\n"
        "---> Restauracao:		%d%%\n"
        "---> Balnearios:		%d%%\n"
		"--------------------------------------\n"
		"Taxas de acesso da zona:\n"//qual a atracao que as pessoas vao mais
		"---> Natacao:		        %d%%\n"
		"---> Mergulho:			%d%%\n"
        "---> Tobogas:			%d%%\n"
        "---> Enfermaria:		%d%%\n"
        "---> Restauracao:		%d%%\n"
        "---> Balnearios:		%d%%\n"
		"--------------------------------------\n"
		"Médias(minutos):\n"
		"---> Tempo no Parque:		%.2f\n"
		"---> Tempo na Natacao:		%.2f\n"
		"---> Tempo no Mergulho:		%.2f\n"
		"---> Tempo nos Tobogas:		%.2f\n"
		"---> Tempo na Enfermaria:	%.2f\n"
		"---> Tempo na Restauracao:	%.2f\n"
		"---> Tempo nos Balnearios:	%.2f\n"
		"--------------------------------------\n",
        tempoSimulado, totalEntrarParque, numDesistencias,
		ratioFilaParque, ratioFilaNatacao, ratioFilaMergulho, ratioFilaTobogas, ratioFilaEnfermaria,
		ratioFilaRestauracao, ratioFilaBalnearios, ratioEntrarNatacao, ratioEntrarMergulho, ratioEntrarTobogas,
		ratioEntrarEnfermaria, ratioEntrarRestauracao, ratioEntrarBalnearios,
		mediaTempoParqueFinal, mediaTempoNatacaoFinal, mediaTempoMergulhoFinal, mediaTempoTobogasFinal,
		mediaTempoEnfermariaFinal, mediaTempoRestauracaoFinal, mediaTempoBalneariosFinal);

		// Escreve no ficheiro a informação
  		escreveFicheiro(informacao);	

		// Imprime a informação na consola
    	printf("%s", informacao);
	}
}

int main (void) {
	while (!finalSim){
		socketMonitor();
	}
	return 0;
}