#include <stlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

struct atracao {

	int idAtracao; //Id da atração
	int numeroMaximoPessoas; //Número máximo de pessoas na atração
	int numeroAtualPessoas; //Número atual de pessoas na atração
	int idadeMinima; //Idade mínima para aceder à atração
	int alturaMinima; //Altura mínima para aceder à atração
	int tempoMaxAtracao;
	float probMagoar; //Probabilidade de se magoar na atração
}

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

}
