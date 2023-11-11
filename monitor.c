#include "header.h"

//final da simulação
int finalSim = FALSE;

//arquivo do relatorio
FILE *relatorioFicheiro;

//contadores de pessoas nas zonas
int numPessoas = 0, numDesistencias = 0,  numBilheteria = 0, numNatacao = 0, numTobogas = 0, numEnfermaria = 0, 
	numRestauracao = 0, numBalnearios = 0;

//contadores de pessoas em espera
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

	// Liga o socket a um endereco
    if (bind(sockfd, (struct sockaddr *)&serv_addr, server_size) < 0) {
        printf("erro: nao foi possivel ligar o socket a um endereco. \n");
		exit(-1);
   	 }

    	// Espera a conexao com o simulador
	printf("Começando a simulacao. Espera pelo simulador...\n");

		//servidor espera para aceitar 1 cliente para o socket stream
	listen(sockfd, 1);

	// Criacao de um novo scoket
	cli_size = sizeof(serv_end);
	newsockfd = accept (sockfd, (struct sockaddr *) &serv_end, &cli_size);

	if (newsockfd < 0) {        //verifica se houve erro na aceitacao da ligacao
		printf("erro: nao foi possivel aceitar a ligacao. \n");
		exit(-1);
	}

	printf("Espera concluída, conectado com sucesso!\n");

	/*criação de um processo filho para implementar o codigo 
	pois irão haver vários clientes simultaneos 
	e o pai não pode tratar de um vez a vez*/

	int pFilho = fork();

	if(pFilho == 0){
		close(sockfd);
		recebeDados(newsockfd);
	} 
	else if(pFilho == -1){
		printf("Erro na criação do processo filho\n");
		exit(-1);
	}

	close(newsockfd);

}


void recebeDados(int newsockfd){

	int idPessoa = 0;
	int acabou = 0;
	int recebido = 0;

	while (!finalSim)
	{
		// necessário criar um buffer para alocar temporariamnete memória 
		char buffer[TAMANHO_BUFFER];
		
		//dados recebidos do socket
		recebido = recv(newsockfd, buffer, (TAMANHO_BUFFER-1) , 0);

		//converte a string para um número inteiro e 
		sscanf(buffer,"%d %d", &idPessoa, &acabou); 

		printf("Chegou uma pessoa ao Parque, o seu ID é: %d\n", idPessoa);
		printf("%d\n",acabou);
		numPessoas++;
		numBilheteria++;

		if(acabou == 1){
			finalSim = TRUE;
		}

		imprimeDados();
	}

}

//EXPORTAÇÃO PARA O FICHEIRO
void limpaFicheiro(){
	fclose(fopen("Relatorio.txt","w"));
}

void escreveFicheiro(char *informacao){

	limpaFicheiro();

	relatorioFicheiro = fopen("Relatorio.txt","a");
	if(relatorioFicheiro == NULL){
		perror("Nao foi possivel abrir o ficheiro");
	}
	fprintf(relatorioFicheiro,"%s", informacao);
	fclose(relatorioFicheiro);
}



void imprimeDados() {

	//poderá ser preciso mudar o 1000 caso se acrescente mais informação
    char informacao[1000]; 

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

    // Imprime a informação na consosa
    printf("%s", informacao);
}

int main (void) {
	while (!finalSim)
	{
		socketMonitor();
	}
}