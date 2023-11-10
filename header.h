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

#define UNIXSTR_PATH "/tmp/s.unixstr"
#define UNIXDG_PATH  "/tmp/s.unixdgx"
#define UNIXDG_TMP   "/tmp/dgXXXXXXX"

struct configuracao{
	
	int quantidadePessoasParque;
	int numeroAtracoes;
	int tempoEsperaBilheteria;
	int tempoEsperaNatação;
	int tamanhoFilaNatação;	
	int tempoEsperaMergulho;
	int tamanhoFilaMergulho;
	int tempoEsperaTobogãs;
	int tamanhoFilaTobogãs;	
	int tempoEsperaEnfermaria;
	int tamanhoFilaEnfermaria;	
	int tempoEsperaRestauração;
	int tamanhoFilaRestauração;
	int tempoEsperaBalnearios;
	int tamanhoFilaBalnearios;
	//int tempoEsperaParqueEstacionamento;
	//int tamanhoFilaParqueEstacionamento;
	float probabilidadeMagoar;
	float probabilidadeDesistir; //Não devia de estar em pessoa?
	//float probabilidadeMembro (Probabilidade der ser membro do parque e passar sempre a frente nas filas, pode ser uma boa politica)
	int tempoSimulacao;
};

// Bilheteria
// Natação
// Mergulho
// Tobogãs
// Enfermaria
// Restauração
// Balnearios
// Parque de estacionamento(talvez)
struct zona {

	int idZona; //Id da zona (Começa em 1)
	int numeroMaximoPessoas; //Número máximo de pessoas na zona
	int numeroAtualPessoas; //Número atual de pessoas na zona
	int idadeMinima; //Idade mínima para aceder à zona
	int alturaMinima; //Altura mínima para aceder à zona
	int tempoMaxZona;
	float probMagoar; //Probabilidade de se magoar na zona
	//sem_t fila //Fila de espera da zona 
};

struct pessoa {

	int idPessoa; //Id da pessoa
	int genero; //Gênero da pessoa (0 - Mulher / 1 - Homem)
	int idade; //Idade da pessoa
	int altura; //Altura da pessoa em centímetros
	int onParque; //Se está no parque (0 - Não está / 1 - Está)
	int vip; //Se é VIP do parque ou não (passa à frente) (0 - Não é VIP / 1 - É VIP)
	int magoar; //Se se magoou ou não (0 - Não se magoou / 1 - Magoou-se)
	int zonaAtual; //Id da zona atual (0 - Não está em nenhuma atração / ...)
	int tempoMaxEspera; //Tempo máximo de espera numa fila (em ciclos)

};

extern int fd;
extern char *ptr;
extern int nbytes;
extern int readn(int fd, char *ptr, int nbytes);
extern int writen(int fd, char *ptr, int nbytes);
extern int readline(int fd, char *ptr, int maxlen);
extern void err_dump(char *msg);
extern void str_cli(FILE *fp, int sockfd);
extern void str_echo(int sockfd);

//Métodos

//Simulador
void socketSimulador();
struct pessoa criarPessoa();
//void enviarDados(char* dados);

//Monitor
void socketMonitor();
//void escreveDados(char* dados);

