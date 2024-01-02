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

#define MULHER 0
#define HOMEM 1

//Acabou a simulação
#define NAO_ACABOU 0
#define ACABOU 1

//Tamanhos
#define TAMANHO_CONFIG 34
#define TAMANHO_BUFFER 2048
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

//Ações
#define SAIR 0
#define ENTRAR 1
#define ENTRAR_FILA 2
#define SAIR_FILA 3
#define SAIR_FILA_ENTRAR 4
#define SAIR_SAIR 5
#define SAIR_FILA_ENFERMARIA 6

//cores
#define RESET "\x1B[0m"
#define VERMELHO "\x1B[31m"
#define VERDE "\x1B[32m"
#define AMARELO "\x1B[33m"
#define AZUL "\x1B[34m"
#define ROXO "\x1B[35m"
#define CIANO "\x1B[36m"
#define VERMELHO_CLARO "\x1B[91m"
#define VERDE_CLARO "\x1B[92m"
#define AMARELO_CLARO "\x1B[93m"
#define AZUL_CLARO "\x1B[94m"
#define ROXO_CLARO "\x1B[95m"
#define CIANO_CLARO "\x1B[96m"
#define CINZA "\x1B[90m"

struct configuracao{

	int numeroAtracoes;
	int tempoEsperaFilaParque;
	int tempoEntrarPraca;
	int tamanhoFilaParque;
	int tempoEsperaFilaNatacao;
	int tempoNatacao;
	int tamanhoFilaNatacao;
	int numeroMaximoNatacao;
	int tempoEsperaFilaMergulho;
	int tempoMergulho;
	int tamanhoFilaMergulho;
	int numeroMaximoMergulho;
	int tempoEsperaFilaTobogas;
	int tempoTobogas;
	int tamanhoFilaTobogas; // Tamanho
	int numeroMaximoTobogas; // Numero maximo de pessoas nos tobogãs
	int tempoEsperaFilaEnfermaria; // Tempo de espera na fila da Enfermaria
	int tempoEnfermaria; // Tempo passado na Enfermaria
	int tamanhoFilaEnfermaria; // Tamanho da fila da enfermaria 
	int numeroMaximoEnfermaria; // Numero maximo na enfermaria
	int tempoEsperaFilaRestauracao; // Tempo de espera na fila da restauração
	int tempoRestauracao; // Tempo que uma pessoa passa na restauração
	int tamanhoFilaRestauracao; // Tamanho da fila da restauração
	int numeroMaximoRestauracao; // Numero maximo de pessoas na restauração
	int tempoEsperaFilaBalnearios; // Tempo de espera na fila dos balnearios
	int tempoBalnearios; // Tempo que uma pessoa passa nos balnearios
	int tamanhoFilaBalnearios; // Tamanho da fila dos balnearios
	int numeroMaximoBalnearios; // Numero maximo de pessoas nos balnearios
	float probabilidadeMagoar; // Probabilidade de se magoar
	float probabilidadeDesistir; // Probabilidade de desistir da atração
	float probabilidadeMudarZona; // Probabilidade de querer mudar de zona depois de ter andado na atração
	float probabilidadeCurar; // Probabilidade de se curar ao ir à enfermaria
	int tempoSimulacao; // Tempo da simulação
	int tempoChegadaPessoas; // Tempo entre a chegada de uma pessoa e da próxima ao parque
	int quantidadePessoasParque; // Quantidade máxima de pessoas no parque
};

// Praça
// Natação
// Mergulho
// Tobogãs
// Enfermaria
// Restauração
// Balnearios

struct praca{
	int numeroPessoasNaFila; // Numero de pessoas à espera para entrar na praça do parque
	sem_t fila; // Fila de espera do Parque
};

struct natacao{
	int numeroAtualPessoas; // Número atual de pessoas na zona de natacao
	int numeroPessoasNaFila; // Numero de pessoas à espera para entrar na zona de natacao
	sem_t fila; // Fila de espera da natacao 
};

struct mergulho{
	int numeroAtualPessoas; // Número atual de pessoas na zona de mergulho
	int numeroPessoasNaFila; // Numero de pessoas à espera para entrar na zona de mergulho
	sem_t fila; // Fila de espera da zona 
};

struct tobogas{
	int numeroAtualPessoas; // Número atual de pessoas na zona de tobogas
	int numeroPessoasNaFila; // Numero de pessoas à espera para entrar na zona de tobogas
	sem_t fila; // Fila de espera da zona de tobogas
};

struct enfermaria{
	int numeroAtualPessoas; // Número atual de pessoas na zona da enfermaria
	int numeroPessoasNaFila; // Numero de pessoas à espera para entrar na zona da enfermaria
	sem_t fila; // Fila de espera da zona da enfermaria 
};

struct restauracao{
	int numeroAtualPessoas; // Número atual de pessoas na zona de restauração
	int numeroPessoasNaFila; // Numero de pessoas à espera para entrar na zona de restauração
	sem_t fila; // Fila de espera da zona 
};

struct balnearios{
	int numeroAtualPessoas; // Número atual de pessoas na zona de balneários
	int numeroPessoasNaFila; // Numero de pessoas à espera para entrar na zona de balneários
	sem_t fila; // Fila de espera da zona de balneários
};

struct pessoa {

	int idPessoa; // Id da pessoa
	int genero; // Gênero da pessoa (0 - Mulher / 1 - Homem)
	int idade; // Idade da pessoa
	int altura; // Altura da pessoa em centímetros
	int magoar; // Se se magoou ou não (0 - Não se magoou / 1 - Magoou-se)
	int zonaAtual; // Id da zona atual (0 - Bilheteria / ...)
	int tempoDeChegadaFila; // Tempo que a pessoa está na fila à espera 
	int desistir; // Se a pessoa desistiu da fila em que está (0 - não desistiu / 1 - desistiu)
	int visitas[5]; // Se já visitou uma atração guarda aqui (sem repetir)
	int totalVisitadas; // Guarda quantas visitou (pode repetir)
	bool dentroParque; // Variavel para saber se está no parque

};

//Métodos

//Simulador
int socketSimulador();
int configuracao (char *file);
int calculaProbabilidadeMudar(float probabilidade, struct pessoa *pessoa);
int calculaProbabilidadeDesistir(float probabilidade, struct pessoa *pessoa);
int randomEntreNumeros(int min, int max);
struct pessoa criarPessoa();
int visitarProximaAtracao(struct pessoa *pessoa);
void Fila (struct pessoa *pessoa);
void enviarDados(int acabou, int personId, int tempo, int acao, int zona);
void enviarPessoa(void *ptr);
void exclusaoMutua();
void simulador(char* config);

//Monitor
void socketMonitor();
void recebeDados(int newsockfd);
void imprimeDados();
void limpaFicheiro();
void escreveFicheiro(char *informacao);
void processarOsDados(int acabou, int idPessoa, int tempo, int acao, int zona);

