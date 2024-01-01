#include "header.h"

// Variavel para indicar o final da simulação
int finalSim = FALSE;

// Arquivo do relatório
FILE *relatorioFicheiro;

// Variável para aparecer no início da execução
int simulacaoIniciada = 0;

// Variavel para guardar o tempo simulado
float tempoSimulado = 0;

// Variáveis para guardar os minutos e segundos simulados
int segundos = 0, minutos = 0;

// Contadores de pessoas nas zonas
int numPessoas = 0, numPessoasSairam = 0, numPraca = 0, 
	numDesistencias = 0, numNatacao = 0, numMergulho = 0, numTobogas = 0,
	numEnfermaria = 0, numRestauracao = 0, numBalnearios = 0;

// Contadores de pessoas em espera
int  espParque=0, espNatacao = 0, espMergulho = 0, espTobogas = 0, espEnfermaria = 0, 
	 espRestauracao = 0, espBalnearios = 0;

// variaveis para fazer contas

	// ----|Variáveis que auxiliam na conta da taxa de desistencia da fila de uma zona/atração|----
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
	// ---- |Fim das variáveis usadas para a taxa de desistencia de uma fila|----


	// ----|Variáveis usadas para contar quantas pessoas entraram numa dada atração/zona|----
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

	// ----|Fim das variáveis usadas para contar quantas pessoas entraram numa dada atração/zona|----

	// ----|Variáveis usadas para auxiliar na conta dos tempos médios de cada atração/zona|----
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
	
	// ----|Fim das variáveis usadas para calcular os tempos médios|----

//----acabou variaveis para contas----

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

	// Variáveis que guardam os valores recebidos pelo buffer
	int acabou = 0;
	int idPessoa = 0;
	int tempo = 0;
	int acao = 0;
	int zona = 0;
	int recebido = 0;

	// Enquanto a simulação não acaba vai recebendo os dados enviados pelo simulador
	while (!finalSim){
		// Necessário criar um buffer para alocar temporariamnete memória 
		char buffer[TAMANHO_BUFFER+1];
		
		// Limpa o buffer para receber novos dados do socket
		memset(buffer, 0, sizeof(buffer));
		recebido = recv(newsockfd, buffer, TAMANHO_BUFFER, 0);

		// Caso não haja dados para receber
		if (recebido <= 0){
			printf(VERMELHO "Não foram recebidos dados\n");
			break;
		}

		// Recebe os dados do token até encontrar "|"
		char *token = strtok(buffer, "|");

		// Caso seja recebido mais do qeu um pacote ao mesmo tempo já divide as informações para fazer as contas de maneira correta
		while(token != NULL){
			sscanf(token,"%d %d %d %d %d", &acabou, &idPessoa, &tempo, &acao, &zona);

			processarOsDados(acabou, idPessoa, tempo, acao, zona);

			token = strtok(NULL, "|");
		}

		// Reinicializa as variáveis a zero por segurança
		acabou = 0;
		idPessoa = 0;
		tempo = 0;
		acao = 0;
		zona = 0;

	}
}

// Função que vai processar os dados recebidos
void processarOsDados(int acabou, int idPessoa, int tempo, int acao, int zona){

	// Limpa o monitor antes de colocar a informação atualizada
	system("clear");

	// Converte os segundos para minutos e segundos
	if(tempo<60){
		tempoSimulado = tempo/100.0;
		segundos = tempo;
	} else {
		segundos = tempo % 60;  
    	minutos = tempo / 60.0;  

    	tempoSimulado = minutos + (segundos / 100.0);
	}

	// Switch case para cada tipo de dados que recebe
	switch (acabou){
		
		// Se a simulação ainda não acabou
		case NAO_ACABOU:

			// Se entrou em alguma atração/zona
			if (acao == ENTRAR){

				// Se for a Praça incrementa o número atual de pessoas na Praça, o total que entraram no parque
				// e o tempo que chegou ao Parque
				if(zona == PRACA){
					printf("A pessoa com ID %d entrou no Parque\n", idPessoa);
					numPraca++;
					totalEntrarParque++;
					totalTempoChegada += tempo;

				// Se for para a Natação incrementa o número atual de pessoas na Natação, diminui o número de pessoas na Praça,
				// aumenta o numero total de pessoas que entraram na atração e o tempo que chegou à atração
				}else if(zona == NATACAO){
					printf("A pessoa com ID %d entrou na atração da Natação\n", idPessoa);
					numNatacao++;
					numPraca--;
					totalEntrarNatacao++;
					totalChegadaNatacao += tempo;

				// Se for para o Mergulho incrementa o número atual de pessoas no Mergulho, diminui o número de pessoas na Praça,
				// aumenta o numero total de pessoas que entraram na atração e o tempo que chegou à atração
				}else if(zona == MERGULHO){
					printf("A pessoa com ID %d entrou na atração de Mergulho\n", idPessoa);
					numMergulho++;
					numPraca--;
					totalEntrarMergulho++;
					totalChegadaMergulho += tempo;

				// Se for para os Tobogãs incrementa o número atual de pessoas nos Tobogãs, diminui o número de pessoas na Praça,
				// aumenta o numero total de pessoas que entraram na atração e o tempo que chegou à atração
				}else if(zona == TOBOGAS){
					printf("A pessoa com ID %d entrou na atração dos Tobogãs\n", idPessoa);
					numTobogas++;
					numPraca--;
					totalEntrarTobogas++;
					totalChegadaTobogas += tempo;

				// Se for para a Restauração incrementa o número atual de pessoas na Restauração, diminui o número de pessoas na Praça,
				// aumenta o numero total de pessoas que entraram na zona e o tempo que chegou à zona
				}else if(zona == RESTAURACAO){
					printf("A pessoa com ID %d entrou na Restauração\n", idPessoa);
					numRestauracao++;
					numPraca--;
					totalEntrarRestauracao++;
					totalChegadaRestauracao += tempo;

				// Se for para os Balnearios incrementa o número atual de pessoas nos Balnearios, diminui o número de pessoas na Praça,
				// aumenta o numero total de pessoas que entraram na zona e o tempo que chegou à zona
				}else if(zona == BALNEARIOS){
					printf("A pessoa com ID %d entrou nos Balneários\n", idPessoa);
					numBalnearios++;
					numPraca--;
					totalEntrarBalnearios++;
					totalChegadaBalnearios += tempo;

				// Se for para a Enfermaria incrementa o número atual de pessoas na Enfermaria, diminui o número de pessoas na Praça,
				// aumenta o numero total de pessoas que entraram na zona e o tempo que chegou à zona
				}else if(zona == ENFERMARIA){
					printf("A pessoa com ID %d entrou na Enfermaria\n", idPessoa);
					numEnfermaria++;
					numPraca--;
					totalEntrarEnfermaria++;
					totalChegadaEnfermaria += tempo;

				}

			// Se saiu de uma atração/zona
			} else if(acao == SAIR) {

				// Se saiu da Praça significa sair do Parque, diminuindo o número atual de pessoas no Parque assim como na Praça,
				// aumentado o número total de pessoas que sairam do Parque e registando o tempo de saída
				if(zona == PRACA){

					printf("A pessoa com ID %d saiu do Parque\n", idPessoa);
					numPraca--;
					numPessoasSairam++;
					totalTempoSaida += tempo;

				// Se saiu da Natação diminui o número atual de pessoas na Natação, aumenta o número atual de pessoas na Praça
				// e regista o tempo de saída 
				}else if(zona == NATACAO){

					printf("A pessoa com ID %d saiu da atração da Natação\n", idPessoa);
					numNatacao--;
					numPraca++;
					totalSaidaNatacao += tempo;

				// Se saiu do Mergulho diminui o número atual de pessoas no Mergulho, aumenta o número atual de pessoas na Praça
				// e regista o tempo de saída 
				}else if(zona == MERGULHO){

					printf("A pessoa com ID %d saiu da atração de Mergulho\n", idPessoa);
					numMergulho--;
					numPraca++;
					totalSaidaMergulho += tempo;

				// Se saiu dos Tobogãs diminui o número atual de pessoas nos Tobogãs, aumenta o número atual de pessoas na Praça
				// e regista o tempo de saída 
				}else if(zona == TOBOGAS){

					printf("A pessoa com ID %d saiu da atração dos Tobogãs\n", idPessoa);
					numTobogas--;
					numPraca++;
					totalSaidaTobogas += tempo;

				// Se saiu da Restauração diminui o número atual de pessoas na Restauração, aumenta o número atual de pessoas na Praça
				// e regista o tempo de saída 
				}else if(zona == RESTAURACAO){

					printf("A pessoa com ID %d saiu da Restauração\n", idPessoa);
					numRestauracao--;
					numPraca++;
					totalSaidaRestauracao += tempo;

				// Se saiu dos Balnearios diminui o número atual de pessoas nos Balnearios, aumenta o número atual de pessoas na Praça
				// e regista o tempo de saída 
				}else if(zona == BALNEARIOS){

					printf("A pessoa com ID %d saiu dos Balneários\n", idPessoa);
					numBalnearios--;
					numPraca++;
					totalSaidaBalnearios += tempo;

				// Se saiu da Enfermaria diminui o número atual de pessoas na Enfermaria, aumenta o número atual de pessoas na Praça
				// e regista o tempo de saída 
				}else if(zona == ENFERMARIA){

					printf("A pessoa com ID %d saiu da Enfermaria\n", idPessoa);
					numEnfermaria--;
					numPraca++;
					totalSaidaEnfermaria += tempo;

				}
			
			// Se entrou na fila da atração/zona
			} else if (acao == ENTRAR_FILA) {
				
				// Se entrou na fila do Parque, aumenta o numero atual de pessoas na fila e regista o tempo de entrada na fila
				if(zona == PRACA){
					printf("A pessoa com ID %d entrou na fila do Parque\n", idPessoa);
					espParque++;
					entraramFilaParque++;

				// Se entrou na fila da Natação, aumenta o numero atual de pessoas na fila, diminui o número atual de pessoas na Praça
				// e aumenta o total de pessoas que entraram na fila
				}else if(zona == NATACAO){
					printf("A pessoa com ID %d entrou na fila da atração da Natação\n", idPessoa);
					espNatacao++;
					numPraca--;
					entraramFilaNatacao++;

				// Se entrou na fila do Mergulho, aumenta o numero atual de pessoas na fila, diminui o número atual de pessoas na Praça
				// e aumenta o total de pessoas que entraram na fila
				}else if(zona == MERGULHO){
					printf("A pessoa com ID %d entrou na fila da atração de Mergulho\n", idPessoa);
					espMergulho++;
					numPraca--;
					entraramFilaMergulho++;

				// Se entrou na fila dos Tobogãs, aumenta o numero atual de pessoas na fila, diminui o número atual de pessoas na Praça
				// e aumenta o total de pessoas que entraram na fila
				}else if(zona == TOBOGAS){
					printf("A pessoa com ID %d entrou na fila da atração dos Tobogãs\n", idPessoa);
					espTobogas++;
					numPraca--;
					entraramFilaTobogas++;

				// Se entrou na fila da Restauração, aumenta o numero atual de pessoas na fila, diminui o número atual de pessoas na Praça
				// e aumenta o total de pessoas que entraram na fila
				}else if(zona == RESTAURACAO){
					printf("A pessoa com ID %d entrou na fila da Restauração\n", idPessoa);
					espRestauracao++;
					numPraca--;
					entraramFilaRestauracao++;

				// Se entrou na fila dos Balnearios, aumenta o numero atual de pessoas na fila, diminui o número atual de pessoas na Praça
				// e aumenta o total de pessoas que entraram na fila
				}else if(zona == BALNEARIOS){
					printf("A pessoa com ID %d entrou na fila dos Balneários\n", idPessoa);
					espBalnearios++;
					numPraca--;
					entraramFilaBalnearios++;

				// Se entrou na fila da Enfermaria, aumenta o numero atual de pessoas na fila, diminui o número atual de pessoas na Praça
				// e aumenta o total de pessoas que entraram na fila
				}else if(zona == ENFERMARIA){
					printf("A pessoa com ID %d entrou na fila da Enfermaria\n", idPessoa);
					espEnfermaria++;
					numPraca--;
					entraramFilaEnfermaria++;
					
				}
			
			// Se desistiu de esperar na fila
			} else if (acao == SAIR_FILA) {

				// Se saiu da fila do Parque, diminui o número atual de pessoas na fila, aumenta o número de desistências do Parque
				// e o número total de pessoas que desistiram do Parque
				if(zona == PRACA){
					
					printf("A pessoa com ID %d saiu da fila do Parque\n", idPessoa);
					espParque--;
					numDesistencias++;
					desistirFilaParque++;

				// Se saiu da fila da Natação, diminui o número atual de pessoas na fila, aumenta o número de pessoas na Praça e
				// aumenta o número total de desistencias da fila da zona/atração
				}else if(zona == NATACAO){
					
					printf("A pessoa com ID %d saiu da fila da atração da Natação\n", idPessoa);
					espNatacao--;
					numPraca++;
					desistirFilaNatacao++;

				// Se saiu da fila do Mergulho, diminui o número atual de pessoas na fila, aumenta o número de pessoas na Praça e
				// aumenta o número total de desistencias da fila da zona/atração
				}else if(zona == MERGULHO){

					printf("A pessoa com ID %d saiu da fila da atração de Mergulho\n", idPessoa);
					espMergulho--;
					numPraca++;
					desistirFilaMergulho++;

				// Se saiu da fila dos Tobogãs, diminui o número atual de pessoas na fila, aumenta o número de pessoas na Praça e
				// aumenta o número total de desistencias da fila da zona/atração
				}else if(zona == TOBOGAS){

					printf("A pessoa com ID %d saiu da fila da atração dos Tobogãs\n", idPessoa);
					espTobogas--;
					numPraca++;
					desistirFilaTobogas++;

				// Se saiu da fila da Restauração, diminui o número atual de pessoas na fila, aumenta o número de pessoas na Praça e
				// aumenta o número total de desistencias da fila da zona/atração
				}else if(zona == RESTAURACAO){

					printf("A pessoa com ID %d saiu da fila da Restauração\n", idPessoa);
					espRestauracao--;
					numPraca++;
					desistirFilaRestauracao++;

				// Se saiu da fila dos Balnearios, diminui o número atual de pessoas na fila, aumenta o número de pessoas na Praça e
				// aumenta o número total de desistencias da fila da zona/atração
				}else if(zona == BALNEARIOS){

					printf("A pessoa com ID %d saiu da fila dos Balneários\n", idPessoa);
					espBalnearios--;
					numPraca++;
					desistirFilaBalnearios++;

				// Se saiu da fila da Enfermaria, diminui o número atual de pessoas na fila, aumenta o número de pessoas na Praça e
				// aumenta o número total de desistencias da fila da zona/atração
				}else if(zona == ENFERMARIA){

					printf("A pessoa com ID %d saiu da fila da Enfermaria\n", idPessoa);
					espEnfermaria--;
					numPraca++;
					desistirFilaEnfermaria++;

				}

			// Se saiu da fila para entrar na atração/zona
			} else if (acao == SAIR_FILA_ENTRAR) {

				// Se entra no Parque, diminui o número de pessoas atual na fila do Parque, aumenta o número atual de pessoas na Praça,
				// aumenta o total de pessoas que entraram no Parque e regista o tempo de chegada no Parque
				if(zona == PRACA){
					
					printf("A pessoa com ID %d saiu da fila e entrou do Parque\n", idPessoa);
					espParque--;
					numPraca++;
					totalEntrarParque++;
					totalTempoChegada += tempo;

				// Se entra na Natação, diminui o número de pessoas atual na fila da zona/atração, aumenta o número atual de pessoas na atração/zona,
				// aumenta o total de pessoas que entraram na atração/zona e regista o tempo de chegada na atração/zona
				}else if(zona == NATACAO){
					
					printf("A pessoa com ID %d saiu da fila e entrou na atração da Natação\n", idPessoa);
					espNatacao--;
					numNatacao++;
					totalEntrarNatacao++;
					totalChegadaNatacao += tempo;

				// Se entra no Mergulho, diminui o número de pessoas atual na fila da zona/atração, aumenta o número atual de pessoas na atração/zona,
				// aumenta o total de pessoas que entraram na atração/zona e regista o tempo de chegada na atração/zona
				}else if(zona == MERGULHO){

					printf("A pessoa com ID %d saiu da fila e entrou na atração de Mergulho\n", idPessoa);
					espMergulho--;
					numMergulho++;
					totalEntrarMergulho++;
					totalChegadaMergulho += tempo;

				// Se entra nos Tobogãs, diminui o número de pessoas atual na fila da zona/atração, aumenta o número atual de pessoas na atração/zona,
				// aumenta o total de pessoas que entraram na atração/zona e regista o tempo de chegada na atração/zona
				}else if(zona == TOBOGAS){

					printf("A pessoa com ID %d saiu da fila e entrou na atração dos Tobogãs\n", idPessoa);
					espTobogas--;
					numTobogas++;
					totalEntrarTobogas++;
					totalChegadaTobogas += tempo;

				// Se entra na Restauração, diminui o número de pessoas atual na fila da zona/atração, aumenta o número atual de pessoas na atração/zona,
				// aumenta o total de pessoas que entraram na atração/zona e regista o tempo de chegada na atração/zona
				}else if(zona == RESTAURACAO){

					printf("A pessoa com ID %d saiu da fila e entrou na Restauração\n", idPessoa);
					espRestauracao--;
					numRestauracao++;
					totalEntrarRestauracao++;
					totalChegadaRestauracao += tempo;

				// Se entra nos Balnearios, diminui o número de pessoas atual na fila da zona/atração, aumenta o número atual de pessoas na atração/zona,
				// aumenta o total de pessoas que entraram na atração/zona e regista o tempo de chegada na atração/zona
				}else if(zona == BALNEARIOS){

					printf("A pessoa com ID %d saiu da fila e entrou nos Balneários\n", idPessoa);
					espBalnearios--;
					numBalnearios++;
					totalEntrarBalnearios++;
					totalChegadaBalnearios += tempo;

				// Se entra na Enfermaria, diminui o número de pessoas atual na fila da zona/atração, aumenta o número atual de pessoas na atração/zona,
				// aumenta o total de pessoas que entraram na atração/zona e regista o tempo de chegada na atração/zona
				}else if(zona == ENFERMARIA){

					printf("A pessoa com ID %d saiu da fila e entrou na Enfermaria\n", idPessoa);
					espEnfermaria--;
					numEnfermaria++;
					totalEntrarEnfermaria++;
					totalChegadaEnfermaria += tempo;

				}

			// Se saiu da atração/zona e do parque
			}else if (acao == SAIR_SAIR){

				// Se saiu da Enfermaria, diminui o número atual de pessoas na atração, aumenta o número total de pessoas que sairam do Parque,
				// regista o tempo de saida da atração/zona e do parque
				if(zona==ENFERMARIA){
					printf("A pessoa com ID %d saiu da Enfermaria e foi para o hospital\n", idPessoa);
					numEnfermaria--;
					numPessoasSairam++;
					totalTempoSaida += tempo;
					totalSaidaEnfermaria += tempo;

				// Se saiu da Balnearios, diminui o número atual de pessoas na atração, aumenta o número total de pessoas que sairam do Parque,
				// regista o tempo de saida da atração/zona e do parque
				} else if (zona==BALNEARIOS){
					printf("A pessoa com ID %d saiu dos Balnearios e saiu do Parque\n", idPessoa);
					numBalnearios--;
					numPessoasSairam++;
					totalTempoSaida += tempo;
					totalSaidaBalnearios += tempo;

				// Se saiu dos Tobogãs, diminui o número atual de pessoas na atração, aumenta o número total de pessoas que sairam do Parque,
				// regista o tempo de saida da atração/zona e do parque
				} else if (zona==TOBOGAS){
					printf("A pessoa com ID %d saiu dos Tobogas e saiu do Parque\n", idPessoa);
					numTobogas--;
					numPessoasSairam++;
					totalTempoSaida += tempo;
					totalSaidaTobogas += tempo;

				// Se saiu da Natação, diminui o número atual de pessoas na atração, aumenta o número total de pessoas que sairam do Parque,
				// regista o tempo de saida da atração/zona e do parque
				} else if (zona==NATACAO){
					printf("A pessoa com ID %d saiu da Natacao e saiu do Parque\n", idPessoa);
					numNatacao--;
					numPessoasSairam++;
					totalTempoSaida += tempo;
					totalSaidaNatacao += tempo;

				// Se saiu do Mergulho, diminui o número atual de pessoas na atração, aumenta o número total de pessoas que sairam do Parque,
				// regista o tempo de saida da atração/zona e do parque
				} else if (zona==MERGULHO){
					printf("A pessoa com ID %d saiu do Mergulho e saiu do Parque\n", idPessoa);
					numMergulho--;
					numPessoasSairam++;
					totalTempoSaida += tempo;
					totalSaidaMergulho += tempo;

				// Se saiu da Restauração, diminui o número atual de pessoas na atração, aumenta o número total de pessoas que sairam do Parque,
				// regista o tempo de saida da atração/zona e do parque
				} else if (zona==RESTAURACAO){
					printf("A pessoa com ID %d saiu da Restauracao e saiu do Parque\n", idPessoa);
					numRestauracao--;
					numPessoasSairam++;
					totalTempoSaida += tempo;
					totalSaidaRestauracao += tempo;
				}

			// Se saiu da fila da Enfermaria, significa ir para o hospital, diminui o número atual de pessoas à espera na fila,
			// aumenta o número total de pessoas que sairam do Parque e da fila e regista o tempo de saida do Parque
			}else if (acao == SAIR_FILA_ENFERMARIA){
				printf("A pessoa com ID %d saiu da fila da Enfermaria e foi para o hospital\n", idPessoa);
				espEnfermaria--;
				numPessoasSairam++;
				desistirFilaEnfermaria++;
				totalTempoSaida += tempo;
			}
			
			// Variavel que guarda o número de pessoas no Parque atualmente
			numPessoas = numBalnearios + numEnfermaria + numMergulho + numNatacao + numPraca +
						 numRestauracao + numTobogas + espBalnearios + espEnfermaria + 
						 espMergulho + espNatacao + espRestauracao + espTobogas;


			// Variaveis que vão guardar as taxas de desistencia de uma fila
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

			// Variaveis que vão guardar as taxas de utilização de uma atração/zona
			ratioEntrarNatacao = (int)(((float)totalEntrarNatacao / totalEntrarParque) * 100);
			ratioEntrarTobogas = (int)(((float)totalEntrarTobogas / totalEntrarParque) * 100);
			ratioEntrarRestauracao = (int)(((float)totalEntrarRestauracao / totalEntrarParque) * 100);
			ratioEntrarMergulho = (int)(((float)totalEntrarMergulho / totalEntrarParque) * 100);
			ratioEntrarBalnearios = (int)(((float)totalEntrarBalnearios / totalEntrarParque) * 100);
			ratioEntrarEnfermaria = (int)(((float)totalEntrarEnfermaria / totalEntrarParque) * 100);
			
			// Vai imprimir os dados recebidos
			imprimeDados();
			break;
		
		// Se acabou a simulação
		case ACABOU:

			// Muda a variavel para verdadeiro
			finalSim = TRUE;

			// Variaveis que vão guardar os segundos e os minutos médios de cada atração

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

			// Imprime os dados finais
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
		"Tempo Simulado(minutos) --> %02d:%02d\n"
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
		"Taxas de utilização das zonas:\n"//qual a atracao que as pessoas vao mais
		"---> Natacao:		        %d%%\n"
		"---> Mergulho:			%d%%\n"
        "---> Tobogas:			%d%%\n"
        "---> Enfermaria:		%d%%\n"
        "---> Restauracao:		%d%%\n"
        "---> Balnearios:		%d%%\n"
		"--------------------------------------\n",
        (!finalSim) ? "A decorrer" : "Finalizado",
        minutos, segundos, numPessoas, numPessoasSairam, numDesistencias,
        numPraca, numNatacao, numMergulho, numTobogas, numEnfermaria, numRestauracao, numBalnearios,
        espParque, espNatacao, espMergulho, espTobogas, espEnfermaria, espRestauracao, espBalnearios,
		entraramFilaParque, ratioFilaParque, entraramFilaNatacao, ratioFilaNatacao, entraramFilaMergulho, ratioFilaMergulho, entraramFilaTobogas, 
		ratioFilaTobogas, entraramFilaEnfermaria, ratioFilaEnfermaria, entraramFilaRestauracao, ratioFilaRestauracao, entraramFilaBalnearios, ratioFilaBalnearios, 
		ratioEntrarNatacao, ratioEntrarMergulho, ratioEntrarTobogas, ratioEntrarEnfermaria, ratioEntrarRestauracao, ratioEntrarBalnearios);

    // Imprime a informação na consola
    printf("%s", informacao);

	// caso seja o final da simulação
	if(finalSim) {

		// Limpa os dados anteriores para mostrar só os calculos feitos
		system("clear");

		sprintf(informacao,
        "======================================\n"
        "            PARQUE AQUATICO\n"
        "======================================\n"
        "Estado de execucao --> Finalizado\n"
		"Tempo Simulado Final(minutos) --> %02d:%02d\n"
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
		"Taxas de utilização das zonas:\n"
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
        minutos, segundos, totalEntrarParque, numDesistencias,
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