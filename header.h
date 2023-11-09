#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>
#include <semaphore.h>

extern int fd;
extern char *ptr;
extern int nbytes;
extern int readn(int fd, char *ptr, int nbytes);
extern int writen(int fd, char *ptr, int nbytes);
extern int readline(int fd, char *ptr, int maxlen);
extern void err_dump(char *msg);
extern void str_cli(FILE *fp, int sockfd);
extern void str_echo(int sockfd);

#define UNIXSTR_PATH "/tmp/s.unixstr"
#define UNIXDG_PATH  "/tmp/s.unixdgx"
#define UNIXDG_TMP   "/tmp/dgXXXXXXX"

struct configuracao{
	
	int quantidadePessoasParque;
	int tempoEsperaEntrada;
	int numeroAtracoes;
	int tempoEsperaAtracao1;
	int tempoEsperaAtracao2;
	int tempoEsperaAtracao3;
	int tempoEsperaAtracao4;
	int tempoEsperaAtracao5; 
	int tamanhoFilaAtracao1; 
	int tamanhoFilaAtracao2; 
	int tamanhoFilaAtracao3; 
	int tamanhoFilaAtracao4; 
	int tamanhoFilaAtracao5;
	float probabilidadeMagoar;
	float probabilidadeDesistir;
	int tempoSimulacao;
};


struct atracao {

	int idAtracao; //Id da atração
	int numeroMaximoPessoas; //Número máximo de pessoas na atração
	int numeroAtualPessoas; //Número atual de pessoas na atração
	int idadeMinima; //Idade mínima para aceder à atração
	int alturaMinima; //Altura mínima para aceder à atração
	int tempoMaxAtracao;
	float probMagoar; //Probabilidade de se magoar na atração
};

struct pessoa {

	int idPessoa; //Id da pessoa
	int genero; //Gênero da pessoa (0 - Mulher / 1 - Homem)
	int idade; //Idade da pessoa
	int altura; //Altura da pessoa em centímetros
	int onParque; //Se está no parque (0 - Não está / 1 - Está)
	int vip; //Se é VIP do parque ou não (passa à frente) (0 - Não é VIP / 1 - É VIP)
	int magoar; //Se se magoou ou não (0 - Não se magoou / 1 - Magoou-se)
	int atracaoAtual; //Id da atração atual (0 - Não está em nenhuma atração / ...)
	int tempoMaxEspera; //Tempo máximo de espera numa fila (em ciclos)

};
