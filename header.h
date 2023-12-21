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

#define TRUE 1
#define FALSE 0

//Acabou a simulação
#define NAO_ACABOU 0
#define ACABOU 1

//Tamanhos
#define TAMANHO_CONFIG 27
#define TAMANHO_BUFFER 1024
#define TAMANHO_TASK 1000000

//Zonas
#define PRACA 0
#define NATACAO 1
#define MERGULHO 2
#define TOBOGAS 3
#define ENFERMARIA 4
#define RESTAURACAO 5
#define BALNEARIOS 6


struct configuracao{
	
	int quantidadePessoasParque; //Quantidade máxima de pessoas no parque
	int numeroAtracoes;
	int tempoEsperaParque;
	int tempoEsperaNatação;
	int tamanhoFilaNatação;	
	int numeroMaximoNatação;
	int tempoEsperaMergulho;
	int tamanhoFilaMergulho;
	int numeroMaximoMergulho;
	int tempoEsperaTobogãs;
	int tamanhoFilaTobogãs;	
	int numeroMaximoTobogãs;
	int tempoEsperaEnfermaria;
	int tamanhoFilaEnfermaria;	
	int numeroMaximoEnfermaria;
	int tempoEsperaRestauração;
	int tamanhoFilaRestauração;
	int numeroMaximoRestauração;
	int tempoEsperaBalnearios;
	int tamanhoFilaBalnearios;
	int numeroMaximoBalnearios;
	float probabilidadeMagoar;
	float probabilidadeDesistir; //Não devia de estar em pessoa?
	float probabilidadeVIP; //Probabilidade der ser VIP do parque e passar sempre a frente nas filas
	int tempoEsperaMax; //Tempo máximo que uma pessoa pode esperar numa fila antes de desistir
	int tempoSimulacao; //Tempo da simulação
	int tempoChegadaPessoas; //Tempo entre a chegada de uma pessoa e da próxima ao parque
};

// Praça
// Natação
// Mergulho
// Tobogãs
// Enfermaria
// Restauração
// Balnearios

struct praca{
	int numeroAtualPessoas; //Número atual de pessoas na parça
	int numeroPessoasNaFila; //Numero de pessoas à espera para entrar na praça do parque
	//int tempoMaxZona;
	sem_t fila; //Fila de espera do Parque
};

struct natacao{
	int numeroAtualPessoas; //Número atual de pessoas na zona de natacao
	int numeroPessoasNaFila; //Numero de pessoas à espera para entrar na zona de natacao
	//int idadeMinima; //Idade mínima para aceder à natacao
	//int alturaMinima; //Altura mínima para aceder à natacao
	//int tempoMaxZona;
	//float probMagoar; //Probabilidade de se magoar na natacao
	sem_t fila; //Fila de espera da natacao 
};

struct mergulho{
	int numeroAtualPessoas; //Número atual de pessoas na zona de mergulho
	int numeroPessoasNaFila; //Numero de pessoas à espera para entrar na zona de mergulho
	//int idadeMinima; //Idade mínima para aceder à zona
	//int alturaMinima; //Altura mínima para aceder à zona
	//int tempoMaxZona;
	//float probMagoar; //Probabilidade de se magoar na zona
	sem_t fila; //Fila de espera da zona 
};

struct tobogas{
	int numeroAtualPessoas; //Número atual de pessoas na zona de tobogas
	int numeroPessoasNaFila; //Numero de pessoas à espera para entrar na zona de tobogas
	//int idadeMinima; //Idade mínima para aceder aos tobogas
	//int alturaMinima; //Altura mínima para aceder aos tobogas
	//int tempoMaxZona;
	//float probMagoar; //Probabilidade de se magoar nos tobogas
	sem_t fila; //Fila de espera da zona de tobogas
};

struct enfermaria{
	int numeroAtualPessoas; //Número atual de pessoas na zona da enfermaria
	int numeroPessoasNaFila; //Numero de pessoas à espera para entrar na zona da enfermaria
	//int tempoMaxZona;
	sem_t fila; //Fila de espera da zona da enfermaria 
};

struct restauracao{
	int numeroAtualPessoas; //Número atual de pessoas na zona de restauração
	int numeroPessoasNaFila; //Numero de pessoas à espera para entrar na zona de restauração
	//int tempoMaxZona;
	//float probMagoar; //Probabilidade de se magoar na zona de restauração
	sem_t fila; //Fila de espera da zona 
};

struct balnearios{
	int numeroAtualPessoas; //Número atual de pessoas na zona de balneários
	int numeroPessoasNaFila; //Numero de pessoas à espera para entrar na zona de balneários
	//int tempoMaxZona;
	//float probMagoar; //Probabilidade de se magoar na zona de balneários
	sem_t fila; //Fila de espera da zona de balneários
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
	int tempoDeChegadaFila; //Tempo que a pessoa está na fila à espera 
	int desistir; //Se a pessoa desistiu da zona em que esta (0 - não desistiu / 1 - desistiu)

};

//Métodos

//Simulador
int socketSimulador();
int configuracao (char *file);
int serVIP(float probabilidade);
int randomEntreNumeros(int min, int max);
struct pessoa criarPessoa();
void enviarDados(char *bufferEnviar);
void enviarPessoa(void *ptr);
void simulador(char* config);

//Monitor
void socketMonitor();
void recebeDados(int newsockfd);
void imprimeDados();
void limpaFicheiro();
void escreveFicheiro(char *informacao);
