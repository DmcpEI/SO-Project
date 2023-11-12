#include "header.h"

//Final da simulação
int finalSim = FALSE;

//Arquivo do relatorio
FILE *relatorioFicheiro;

//Contadores de pessoas nas zonas
int numPessoas = 0, numDesistencias = 0,  numBilheteria = 0, numNatacao = 0, numTobogas = 0, numEnfermaria = 0, 
	numRestauracao = 0, numBalnearios = 0;

//Contadores de pessoas em espera
int  espBilheteria=0, espNatacao = 0, espTobogas = 0, espEnfermaria = 0, 
	 espRestauracao = 0, espBalnearios = 0;


void socketMonitor () {

	int sockfd, newsockfd;
	int cli_size, server_size;
	struct sockaddr_un serv_end, serv_addr;

	// Verifica a criacao do socket
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

	//Liga o socket a um endereco
    if (bind(sockfd, (struct sockaddr *)&serv_addr, server_size) < 0) {
        printf("erro: nao foi possivel ligar o socket a um endereco. \n");
		exit(-1);
   	}

	//Servidor espera para aceitar 1 cliente para o socket stream
	listen(sockfd, 1);

	// Criação de um novo scoket
	cli_size = sizeof(serv_end);
	newsockfd = accept (sockfd, (struct sockaddr *) &serv_end, &cli_size);

	//Verifica se houve erro na aceitacao da ligacao
	if (newsockfd < 0) {
		printf("erro: nao foi possivel aceitar a ligacao. \n");
		exit(-1);
	}

	/*Criação de um processo filho para implementar o codigo 
	pois irão haver vários clientes simultaneos 
	e o processo pai não pode tratar de um vez a vez*/

	int pFilho;
	//Caso haja erro na criação do processo filho
	if((pFilho=fork()) < 0){
		printf("Erro na criação do processo filho\n");
		exit(-1);
	}
	else if(pFilho > 0){
		printf("Começando a simulacao. Espera pelo simulador...\n");
	} 
	else if( pFilho == 0 ) {	
		close(sockfd);
		recebeDados(newsockfd);
	}
	close(newsockfd);
}

//Função que recebe os dados do socket
void recebeDados(int newsockfd){

	int idPessoa = 0;
	int acabou = 0;
	int recebido = 0;

	while (!finalSim){
		// necessário criar um buffer para alocar temporariamnete memória 
		char buffer[TAMANHO_BUFFER];
		
		//dados recebidos do socket
		recebido = recv(newsockfd, buffer, (TAMANHO_BUFFER-1) , 0);

		//converte a string para um número inteiro e 
		sscanf(buffer,"%d %d", &acabou, &idPessoa);

		/*Caso a variável "acabou" seja um número diferente de 0 
		significa que a simulaçao acabou e sendo assim acaba a simulação 
		e imprime os ultimos dados*/

		switch (acabou){
			/*Caso a simulação esteja a decorrer este imprime o ID da pessoa que chegou
		ao Parque e incrementa o número de Pessoas no Parque, como as pessoas começam
		na bilheteria este também irá incrementar o número de pessoas que estão lá
		*/
		case NAO_ACABOU:
			printf("Chegou uma pessoa ao Parque, o seu ID é: %d\n", idPessoa);

			numPessoas++;
			numBilheteria++;

			imprimeDados();
			break;

		case ACABOU:
			finalSim = TRUE;
			imprimeDados();
			break;
		}
		
	}
}

//Exportação para o ficheiro

/*Função que limpa o ficheiro, 
usamos "w" pois caso exista algo no ficheiro este elimina o que já existe nele
caso contrario ele cria o ficheiro*/
void limpaFicheiro(){
	fclose(fopen("Relatorio.txt","w"));
}


//Função que escreve no ficheiro
void escreveFicheiro(char *informacao){

	//começa por limpar o ficheiro pois só queremos os dados quando acabou a simulação
	limpaFicheiro();

	/*Abrimos o ficheiro e este usa "a" para dar append ou seja, 
	caso o ficheiro exista este adiciona ao ficheiro o texto que queremos, 
	caso contrário cria um novo*/

	relatorioFicheiro = fopen("Relatorio.txt","a");

	//Caso haja um erro o usuario seá notificado
	if(relatorioFicheiro == NULL){
		perror("Nao foi possivel abrir o ficheiro");
	}

	//Irá adicionar o ficherio a informação que queremos e depois irá fechar o ficheiro
	fprintf(relatorioFicheiro,"%s", informacao);
	fclose(relatorioFicheiro);
}



void imprimeDados() {

	//poderá ser preciso mudar o 1000 caso se acrescente mais informação
    char informacao[1000]; 

	/*Irá colocar na variavel local informação a seguinte mensagem, 
	decidimos utilizar este formato pois ajuda na visualização da mensagem que 
	irá aparecer na consola*/
    sprintf(informacao,
        "======================================\n"
        "            PARQUE AQUATICO\n"
        "======================================\n"
        "Estado de execucao --> %s\n"
        "--------------------------------------\n"
        "Pessoas no Parque:		%d\n"
        "Desistencias:			%d\n"
        "--------------------------------------\n"
        "Pessoas na zona:\n"
        "---> Bilheteria:		%d\n"
        "---> Natacao:			%d\n"
        "---> Tobogas:			%d\n"
        "---> Enfermaria:		%d\n"
        "---> Restauracao:		%d\n"
        "---> Balnearios:		%d\n"
        "--------------------------------------\n"
        "Pessoas a espera na zona:\n"
        "---> Bilheteria:		%d\n"
        "---> Natacao:			%d\n"
        "---> Tobogas:			%d\n"
        "---> Enfermaria:		%d\n"
        "---> Restauracao:		%d\n"
        "---> Balnearios:		%d\n"
        "--------------------------------------\n",
        (!finalSim) ? "A decorrer" : "Finalizado",
        numPessoas, numDesistencias,
        numBilheteria, numNatacao, numTobogas, numEnfermaria, numRestauracao, numBalnearios,
        espBilheteria, espNatacao, espTobogas, espEnfermaria, espRestauracao, espBalnearios);

   	//escreve no ficheiro a informação
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
