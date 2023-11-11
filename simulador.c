#include "header.h"

int socketFD;
int idPessoa = 1; //Id para a pessoa criada, que começa a 1 e vai incrementando
int tempoSimulado = 0; //Tempo de Simulação
//int tempoParque = 0; //Tempo que o parque está aberto
//int pessoasParque = 0;

struct configuracao conf;

pthread_mutex_t mutexPessoa;
pthread_mutex_t mutexDados;
pthread_mutex_t mutexSimulacao;

pthread_t idThread[TAMANHO_TASK];
struct pessoa *pessoas[100000];

int socketSimulador(){
	int servlen, sockfd;
	struct sockaddr_un serv_addr;

	if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0){
		printf("Erro ao criar o Socket\n");
	}

	bzero((char *)&serv_addr,sizeof(serv_addr)); 
	serv_addr.sun_family = AF_UNIX; 
	strcpy(serv_addr.sun_path, UNIXSTR_PATH); 
	servlen = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);

	if(connect(sockfd, (struct sockaddr *)&serv_addr, servlen) < 0){
		printf("Execute o monitor primeiro\n");
		close(sockfd);
		exit(-1);
	}

	printf("Conectado com sucesso\n");
	return sockfd;
}

//Função que lê o ficheiro de configuração
int configuracao(char *file) {
    FILE *ficheiro = fopen(file, "r");

    if (ficheiro == NULL) {
        perror("Erro ao abrir o ficheiro");
        return 1;
    } else if (strcmp(file, "simulador.conf") != 0) {
        perror("Ficheiro de configuração não existe.");
        return 1;
    }

    fseek(ficheiro, 0, SEEK_END);
    long tamanhoFicheiro = ftell(ficheiro);
    rewind(ficheiro);
    char buffer[tamanhoFicheiro];
    fread(buffer, 1, tamanhoFicheiro, ficheiro);
    fclose(ficheiro);

    int linhaAtual = 0;
    char *filtrada = strtok(buffer, "\n");
    char *linhas[40];
    while (filtrada != NULL) {
        linhas[linhaAtual++] = filtrada;
        filtrada = strtok(NULL, "\n");
    }

    char *fim;
    char *array[2];
    char *valores[21];
    for (int index = 0; index < 21; index++) {
        char *aux = strtok(linhas[index], ":");
        int i = 0;
        while (aux != NULL) {
            array[i++] = aux;
            aux = strtok(NULL, ":");
        }
        valores[index] = array[1];
    }

    conf.quantidadePessoasParque = atoi(valores[0]);
    conf.numeroAtracoes = atoi(valores[1]);
    conf.tempoEsperaBilheteria = atoi(valores[2]);
    conf.tempoEsperaNatação = atoi(valores[3]);
    conf.tamanhoFilaNatação = atoi(valores[4]);
    conf.tempoEsperaMergulho = atoi(valores[5]);
    conf.tamanhoFilaMergulho = atoi(valores[6]);
    conf.tempoEsperaTobogãs = atoi(valores[7]);
    conf.tamanhoFilaTobogãs = atoi(valores[8]);
    conf.tempoEsperaEnfermaria = atoi(valores[9]);
    conf.tamanhoFilaEnfermaria = atoi(valores[10]);
    conf.tempoEsperaRestauração = atoi(valores[11]);
    conf.tamanhoFilaRestauração = atoi(valores[12]);
    conf.tempoEsperaBalnearios = atoi(valores[13]);
    conf.tamanhoFilaBalnearios = atoi(valores[14]);
    conf.probabilidadeMagoar = strtof(valores[15], &fim);
    conf.probabilidadeDesistir = strtof(valores[16], &fim);
    conf.probabilidadeVIP = strtof(valores[17], &fim);
    conf.tempoEsperaMax = atoi(valores[18]);
    conf.tempoSimulacao = atoi(valores[19]);
    conf.tempoChegadaPessoas = atoi(valores[20]);

    return 0;
}


int serVIP(float probabilidade)
{
	return ((rand() % 100) < (probabilidade * 100));
}

//Gera um número entre dois números
int randomEntreNumeros(int min, int max)
{
	if(min > max){ //Troca os números se o minimo for maior que o máximo
		int temp = min;
		min = max;
		max = temp;
	}

	return (rand() % (max - min +1) + min);
}

struct pessoa criarPessoa() {

	pthread_mutex_lock(&mutexPessoa);

	struct pessoa person;

	person.idPessoa = idPessoa;
	idPessoa++;
	person.genero = randomEntreNumeros(0,1); //0 - Mulher / 1 - Homem
	person.idade = randomEntreNumeros(0,90); //Idade randomizada entre 0 e 90 anos
	person.altura = randomEntreNumeros(60,220); //Altura randomizada entre 60 e 220 centímetros
	person.vip = serVIP(conf.probabilidadeVIP);
	person.magoar = 0; //Pessoa ainda não se magoo pois acabou de ser criada
	person.zonaAtual = 0; //Bilheteria
	person.tempoMaxEspera = randomEntreNumeros((conf.tempoEsperaMax / 2),conf.tempoEsperaMax);

	pthread_mutex_unlock(&mutexPessoa);

	return person;

}

void enviaDados() {

	pthread_mutex_lock(&mutexDados);

    char buffer[TAMANHO_BUFFER];
    int bytesEnviados;

    // Formata os dados como strings no buffer
    snprintf(buffer, TAMANHO_BUFFER, "%d", idPessoa);

    // Envia os dados no buffer para o cliente
    bytesEnviados = send(socketFD, buffer, strlen(buffer), 0);

    if (bytesEnviados == -1) {
        perror("Error sending data");
        // Trate o erro conforme necessário
    }

	pthread_mutex_unlock(&mutexDados);

}

void enviarPessoa(void *ptr)
{
	struct pessoa person = criarPessoa();
	pessoas[person.idPessoa] = &person;

	enviaDados();
}

void simulador(char* config)
{

	configuracao(config);

	while(conf.tempoSimulacao != tempoSimulado){

		tempoSimulado++;

		if(tempoSimulado % conf.tempoChegadaPessoas == 0){
			// Cria uma nova thread para representar uma pessoa
			pthread_mutex_lock(&mutexSimulacao);
            if (pthread_create(&idThread[idPessoa], NULL, enviarPessoa, NULL) != 0) {
                perror("Erro na criação da thread");
                exit(1);
            }
			pthread_mutex_unlock(&mutexSimulacao);
		}

		sleep(1);
	}

}

int main(int argc, char *argv[])
{

	socketFD = socketSimulador();

	simulador(argv[1]);
	close(socketFD);
	return 0;

}
