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
#define TAMANHO_CONFIG 29
#define TAMANHO_BUFFER 1024
#define TAMANHO_TASK 1000000

//Zonas
#define FORADOPARQUE 0
#define PRACA 1
#define NATACAO 2
#define MERGULHO 3
#define TOBOGAS 4
#define RESTAURACAO 5
#define BALNEARIOS 6
#define ENFERMARIA 7

//cores
#define AMARELO "\x1B[33m"
#define AZUL "\x1B[34m"
#define CIANO "\x1B[36m"
#define VERDE "\x1B[32m"
#define VERMELHO "\x1B[31m"
#define RESET "\x1B[0m"
#define ROXO "\x1b[35m"

struct configuracao{
	
	int quantidadePessoasParque; //Quantidade máxima de pessoas no parque
	int numeroAtracoes;
	int tempoEsperaParque;
	int tamanhoFilaParque;	
	int tempoEsperaNatacao;
	int tamanhoFilaNatacao;	
	int numeroMaximoNatacao;
	int tempoEsperaMergulho;
	int tamanhoFilaMergulho;
	int numeroMaximoMergulho;
	int tempoEsperaTobogas;
	int tamanhoFilaTobogas;	
	int numeroMaximoTobogas;
	int tempoEsperaEnfermaria;
	int tamanhoFilaEnfermaria;	
	int numeroMaximoEnfermaria;
	int tempoEsperaRestauracao;
	int tamanhoFilaRestauracao;
	int numeroMaximoRestauracao;
	int tempoEsperaBalnearios;
	int tamanhoFilaBalnearios;
	int numeroMaximoBalnearios;
	float probabilidadeMagoar;
	float probabilidadeDesistir; //Não devia de estar em pessoa?
	float probabilidadeMudarZona; //Probabilidade de querer mudar de zona depois de ter andado na atração
	int tempoEsperaMax; //Tempo máximo que uma pessoa pode esperar numa fila antes de desistir
	int tempoSimulacao; //Tempo da simulação
	int tempoChegadaPessoas; //Tempo entre a chegada de uma pessoa e da próxima ao parque
	int pessoasCriar;
};

// Praça
// Natação
// Mergulho
// Tobogãs
// Enfermaria
// Restauração
// Balnearios

struct praca{
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
	int magoar; //Se se magoou ou não (0 - Não se magoou / 1 - Magoou-se)
	int zonaAtual; //Id da zona atual (0 - Bilheteria / ...)
	int tempoMaxEspera; //Tempo máximo de espera numa fila (em ciclos)
	int tempoDeChegadaFila; //Tempo que a pessoa está na fila à espera 
	int desistir; //Se a pessoa desistiu da fila em que está (0 - não desistiu / 1 - desistiu)
	int visitas[5];
	int totalVisitadas;
	bool dentroParque;

};

//Métodos

//Simulador
int socketSimulador();
int configuracao (char *file);
int calculaProbabilidadeMudar(float probabilidade, struct pessoa *pessoa);
int calculaProbabilidadeDesistir(float probabilidade, struct pessoa *pessoa);
int serVIP(float probabilidade);
int randomEntreNumeros(int min, int max);
struct pessoa criarPessoa();
int visitarProximaAtracao(struct pessoa *pessoa);
void Fila (struct pessoa *pessoa);
void enviarDados(char *bufferEnviar);
void enviarPessoa(void *ptr);
void exclusaoMutua();
void simulador(char* config);

//Monitor
void socketMonitor();
void recebeDados(int newsockfd);
void imprimeDados();
void limpaFicheiro();
void escreveFicheiro(char *informacao);
