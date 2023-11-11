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

#define TRUE 1
#define False 0

#define UNIXSTR_PATH "/tmp/s.unixstr"
#define UNIXDG_PATH  "/tmp/s.unixdgx"
#define UNIXDG_TMP   "/tmp/dgXXXXXXX"

#define TRUE 1
#define FALSE 0

#define TAMANHO_BUFFER 1024
#define TAMANHO_TASK 1000000

//zona
#define BILHETERIA 0
#define NATACAO 1
#define MERGULHO 2
#define TOBOGAS 3
#define ENFERMARIA 4
#define RESTAURACAO 5
#define BALNEARIOS 6


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
	float probabilidadeVIP; //Probabilidade der ser VIP do parque e passar sempre a frente nas filas
	int tempoEsperaMax; //Tempo máximo que uma pessoa pode esperar numa fila antes de desistir
	int tempoSimulacao; //Tempo da simulação
	int tempoChegadaPessoas; //Tempo entre a chegada de uma pessoa e da próxima ao parque
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

	int idZona; //Id da zona
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
	int vip; //Se é VIP do parque ou não (passa à frente) (0 - Não é VIP / 1 - É VIP)
	int magoar; //Se se magoou ou não (0 - Não se magoou / 1 - Magoou-se)
	int zonaAtual; //Id da zona atual (0 - Bilheteria / ...)
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
int socketSimulador();
int configuracao (char *file);
int serVIP(float probabilidade);
int randomEntreNumeros(int min, int max);
struct pessoa criarPessoa();
void enviaDados();
void enviarPessoa(void *ptr);
void simulador(char* config);

//Monitor
void socketMonitor();
void recebeDados(int newsockfd);
void imprimeDados();

