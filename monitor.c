#include "header.h"

void socketMonitor () 
{

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
}

int main (void)
{

socketMonitor();

}
