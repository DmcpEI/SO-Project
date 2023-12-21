#include "header.h"

//Variáveis globais
int socketFD;
int idPessoa = 1; // Id para a pessoa criada, que começa a 1 e vai incrementando
int tempoSimulado = 0; // Tempo de simulação que já foi simulado
//int tempoParque = 0; // Tempo que o parque está aberto
int pessoasParque = 0;

//Structs
struct configuracao conf;
struct praca praca;
struct natacao natacao;
struct mergulho mergulho;
struct tobogas tobogas;
struct enfermaria enfermaria;
struct restauracao restauracao;
struct balnearios balnearios;

//Tricos
pthread_mutex_t mutexPessoa;
pthread_mutex_t mutexPessoaEnviar;
pthread_mutex_t mutexDados;
pthread_mutex_t mutexSimulacao;
pthread_mutex_t mutexFilas;

//Tarefas
pthread_t idThread[TAMANHO_TASK]; // Array das tarefas
struct pessoa *pessoas[100000]; // Array de todas as pessoas

int socketSimulador() {
    int servlen; 
    struct sockaddr_un serv_addr; 

    // Verifica a criação do socket 
    if ((socketFD = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        printf("Erro ao criar o Socket\n"); 
        exit(-1); 
    }

    bzero((char *)&serv_addr, sizeof(serv_addr)); // Limpa a estrutura serv_addr, prevenindo lixo de memória
    serv_addr.sun_family = AF_UNIX; // Configura o tipo de soquete para AF_UNIX
    strcpy(serv_addr.sun_path, UNIXSTR_PATH); // Configura o caminho do soquete no servidor
    servlen = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family); // Calcula o tamanho da estrutura serv_addr

    // Tenta estabelecer uma conexão com o servidor
    if (connect(socketFD, (struct sockaddr *)&serv_addr, servlen) < 0) {
        printf("Execute o monitor primeiro\n"); // Exibe uma mensagem de erro se o monitor não foi executado
        close(socketFD); 
        exit(-1);
    }

    printf("Conectado com sucesso\n");
    return socketFD; // Retorna o descritor do socket conectado
}


// Função que lê o arquivo de configuração
int configuracao(char *file) {
    FILE *ficheiro = fopen(file, "r"); // Abre o arquivo de configuração para leitura

    if (ficheiro == NULL) {
        perror("Erro ao abrir o arquivo"); // Exibe uma mensagem de erro se o arquivo não puder ser aberto
        return 1; // Retorna 1 para indicar um erro
    } else if (strcmp(file, "simulador.conf") != 0) {
        perror("Arquivo de configuração não existe."); // Exibe uma mensagem de erro se o arquivo de configuração não existe
        return 1; // Retorna 1 para indicar um erro
    }

    // Lê o tamanho do arquivo
    fseek(ficheiro, 0, SEEK_END);
    long tamanhoFicheiro = ftell(ficheiro);
    rewind(ficheiro);

    char buffer[tamanhoFicheiro];
    fread(buffer, 1, tamanhoFicheiro, ficheiro); // Lê o conteúdo do arquivo para um buffer
    fclose(ficheiro); // Fecha o arquivo

    int linhaAtual = 0;
    char *filtrada = strtok(buffer, "\n"); // Divide o buffer em linhas
    char *linhas[TAMANHO_CONFIG];
    while (filtrada != NULL) {
        linhas[linhaAtual++] = filtrada; // Armazena as linhas em um array
        filtrada = strtok(NULL, "\n");
    }

    char *fim;
    char *array[2];
    char *valores[TAMANHO_CONFIG];
    for (int index = 0; index < TAMANHO_CONFIG; index++) {
        char *aux = strtok(linhas[index], ":"); // Divide cada linha em partes usando ":"
        int i = 0;
        while (aux != NULL) {
            array[i++] = aux; // Armazena as partes em um array
            aux = strtok(NULL, ":");
        }
        valores[index] = array[1];
    }

    // Atribui os valores lidos do arquivo à estrutura de configuração "conf"
    conf.quantidadePessoasParque = atoi(valores[0]);
    conf.numeroAtracoes = atoi(valores[1]);
    conf.tempoEsperaParque = atoi(valores[2]);
    conf.tempoEsperaNatação = atoi(valores[3]);
    conf.tamanhoFilaNatação = atoi(valores[4]);
    conf.numeroMaximoNatação = atoi(valores[5]);
    conf.tempoEsperaMergulho = atoi(valores[6]);
    conf.tamanhoFilaMergulho = atoi(valores[7]);
    conf.numeroMaximoMergulho = atoi(valores[8]);
    conf.tempoEsperaTobogãs = atoi(valores[9]);
    conf.tamanhoFilaTobogãs = atoi(valores[10]);
    conf.numeroMaximoTobogãs = atoi(valores[11]);
    conf.tempoEsperaEnfermaria = atoi(valores[12]);
    conf.tamanhoFilaEnfermaria = atoi(valores[13]);
    conf.numeroMaximoEnfermaria = atoi(valores[14]);
    conf.tempoEsperaRestauração = atoi(valores[15]);
    conf.tamanhoFilaRestauração = atoi(valores[16]);
    conf.numeroMaximoRestauração = atoi(valores[17]);
    conf.tempoEsperaBalnearios = atoi(valores[18]);
    conf.tamanhoFilaBalnearios = atoi(valores[19]);
    conf.numeroMaximoBalnearios = atoi(valores[20]);
    conf.probabilidadeMagoar = strtof(valores[21], &fim);
    conf.probabilidadeDesistir = strtof(valores[22], &fim);
    conf.probabilidadeVIP = strtof(valores[23], &fim);
    conf.tempoEsperaMax = atoi(valores[24]);
    conf.tempoSimulacao = atoi(valores[25]);
    conf.tempoChegadaPessoas = atoi(valores[26]);

    return 0; // Retorna 0 para indicar sucesso
}

// Função que verifica se uma pessoa é VIP com base na probabilidade
int serVIP(float probabilidade) {
    return ((rand() % 100) < (probabilidade * 100)); // Calcula aleatoriamente se a pessoa é VIP
}

// Gera um número aleatório entre dois valores
int randomEntreNumeros(int min, int max) {
    if (min > max) { // Troca os números se o mínimo for maior que o máximo
        int temp = min;
        min = max;
        max = temp;
    }
    return (rand() % (max - min + 1) + min); // Gera um número aleatório no intervalo especificado
}

// Função que cria uma nova pessoa com valores aleatórios
struct pessoa criarPessoa() {

    struct pessoa person;

    person.idPessoa = idPessoa;
    person.genero = randomEntreNumeros(0, 1); // 0 - Mulher / 1 - Homem
    person.idade = randomEntreNumeros(0, 90); // Idade randomizada entre 0 e 90 anos
    person.altura = randomEntreNumeros(60, 220); // Altura randomizada entre 60 e 220 centímetros
    person.vip = serVIP(conf.probabilidadeVIP);
    person.magoar = 0; // Pessoa ainda não se magoou, pois acabou de ser criada
    person.zonaAtual = PRACA; // Começa na bilheteria para esta entrega
    person.tempoMaxEspera = randomEntreNumeros((conf.tempoEsperaMax / 2), conf.tempoEsperaMax); // Tempo de espera randomizado entre metade do tempo de espera máximo e o tempo de espera máximo

    pthread_mutex_lock(&mutexPessoa);
    idPessoa++;
    pthread_mutex_unlock(&mutexPessoa);

    printf("Criou uma pessoa ao parque com ID %d\n", person.idPessoa);

    return person;// Retorna o descritor do socket conectado
}

void Fila (struct pessoa *pessoa) {
    
    char buffer[TAMANHO_BUFFER];
    int tempo = tempoSimulado;
    int tempoDeEspera, valorDoSemaforo;
    
    if(pessoa->zonaAtual == PRACA){

        printf("A pessoa com ID %d chegou à fila para entrar no parque \n", pessoa->idPessoa);

        if(pessoasParque < conf.quantidadePessoasParque){
            sem_wait(&praca.fila);
            pthread_mutex_lock(&mutexFilas);
            pessoasParque++;
            pthread_mutex_unlock(&mutexFilas);
            printf("A pessoa com ID %d entrou no parque \n", pessoa->idPessoa);
            sem_post(&praca.fila);
        } else {
            printf("A pessoa com ID %d ficou na fila para entrar no parque \n", pessoa->idPessoa);
            pessoa->desistir = TRUE;
            sem_wait(&praca.fila);
        }
    } /*else if (pessoa->zonaAtual == NATACAO){

        pthread_mutex_lock(&mutexFilas);
        int pessoasNaFila = natacao.numeroPessoasNaFila;
        pthread_mutex_unlock(&mutexFilas);

        if(pessoasNaFila < conf.tamanhoFilaNatação){
            printf("A pessoa com ID %d chegou à fila da zona de Natação", pessoa->idPessoa);

            pessoa->tempoDeChegadaFila = tempo;

            pthread_mutex_lock(&mutexFilas);
            natacao.numeroPessoasNaFila++;
            pthread_mutex_unlock(&mutexFilas);

            sem_wait(&natacao.fila);
            pthread_mutex_lock(&mutexFilas);
            tempoDeEspera = tempoSimulado - pessoa->tempoDeChegadaFila;
            pthread_mutex_unlock(&mutexFilas);
            
            if(tempoDeEspera > pessoa->tempoMaxEspera){
                pessoa->desistir = TRUE;
                sem_post(&natacao.fila);
                pthread_mutex_lock(&mutexFilas);
                natacao.numeroPessoasNaFila--;

                sem_getvalue(&natacao.fila, &valorDoSemaforo);
                if(valorDoSemaforo < natacao.numeroAtualPessoas) {
                    sem_post(&natacao.fila);
                }

                pthread_mutex_unlock(&mutexFilas);

                printf("A pessoa com ID %d desistiu da zona de Natação pois esperou muito tempo", pessoa->idPessoa);
            } else {
                //if(natacao.numeroAtualPessoas < conf.numeroMaximoNatação){
                printf("A pessoa com ID %d entrou na zona de Natação", pessoa->idPessoa);
                pthread_mutex_lock(&mutexFilas);
                natacao.numeroPessoasNaFila--;
                natacao.numeroAtualPessoas++;
                pthread_mutex_unlock(&mutexFilas);
            }
        }

    }else if (pessoa->zonaAtual == MERGULHO){

        pthread_mutex_lock(&mutexFilas);
        int pessoasNaFila = mergulho.numeroPessoasNaFila;
        pthread_mutex_unlock(&mutexFilas);

    }else if (pessoa->zonaAtual == TOBOGAS){

        pthread_mutex_lock(&mutexFilas);
        int pessoasNaFila = tobogas.numeroPessoasNaFila;
        pthread_mutex_unlock(&mutexFilas);
        
    }else if (pessoa->zonaAtual == ENFERMARIA){

        pthread_mutex_lock(&mutexFilas);
        int pessoasNaFila = enfermaria.numeroPessoasNaFila;
        pthread_mutex_unlock(&mutexFilas);
        
    }else if (pessoa->zonaAtual == RESTAURACAO){

        pthread_mutex_lock(&mutexFilas);
        int pessoasNaFila = restauracao.numeroPessoasNaFila;
        pthread_mutex_unlock(&mutexFilas);
        
    }else{

        pthread_mutex_lock(&mutexFilas);
        int pessoasNaFila = balnearios.numeroPessoasNaFila;
        pthread_mutex_unlock(&mutexFilas);

    }   
    */
}

// Função para enviar dados (buffer) para o socket
void enviarDados(char *bufferEnviar) {
    pthread_mutex_lock(&mutexDados);

    char buffer[TAMANHO_BUFFER];

    if (strcpy(buffer, bufferEnviar) != 0) { // Copia o conteúdo de bufferEnviar para buffer

        if (send(socketFD, buffer, strlen(buffer), 0) == -1) {
            perror("Erro ao enviar dados"); // Exibe uma mensagem de erro se não conseguir enviar os dados
        }
    }

    pthread_mutex_unlock(&mutexDados);
}

void enviarPessoa(void *ptr) {

    struct pessoa person = criarPessoa();
    pessoas[person.idPessoa] = &person; // Adiciona a pessoa criada ao array de todas as pessoas criadas 

    int time = tempoSimulado;

    while(TRUE){

        Fila(&person);
        usleep(100);
        printf("Entrou aqui com ID %d\n", person.idPessoa);
        if(person.desistir == FALSE){

            printf("Entrou aqui com ID %d\n", person.idPessoa);

            if (person.zonaAtual == PRACA) {
                sem_post(&praca.fila);
            }
            if (person.zonaAtual == NATACAO) {
                sem_post(&natacao.fila);
            }
            if (person.zonaAtual == MERGULHO) {
                sem_post(&mergulho.fila);
            }
            if (person.zonaAtual == TOBOGAS) {
                sem_post(&tobogas.fila);
            }
            if (person.zonaAtual == ENFERMARIA) {
                sem_post(&enfermaria.fila);
            }
            if (person.zonaAtual == RESTAURACAO) {
                sem_post(&restauracao.fila);
            }
            if (person.zonaAtual == BALNEARIOS) {
                sem_post(&balnearios.fila);
            }
            printf("A pessoa com ID %d saiu do parque", person.idPessoa);
            //enviaInformacao(sockfd, people.id, timestamp, 1, 3, people.zona);
            person.desistir == TRUE;

        } else {
            break;
        }
    }

    //printf("Chegou uma pessoa ao parque com ID %d\n", person.idPessoa);

    //char bufferEnviar[TAMANHO_BUFFER];
    // Aqui envio primeiro se acabou ou não a simulação e depois o id da pessoa criada
    //snprintf(bufferEnviar, TAMANHO_BUFFER, "%d %d", NAO_ACABOU, person.idPessoa);
    //enviarDados(bufferEnviar);
    //praca.numeroAtualPessoas++; // Adiciona a pessoa à Bilheteria (na próxima entrega iremos melhorar)
}

/*
// Função para criar e enviar uma nova pessoa
void enviarPessoa(void *ptr) {
    pthread_mutex_lock(&mutexPessoaEnviar);

    struct pessoa person = criarPessoa();
    pessoas[person.idPessoa] = &person; // Adiciona a pessoa criada ao array de todas as pessoas criadas 

    printf("Chegou uma pessoa ao parque com ID %d\n", person.idPessoa);

    char bufferEnviar[TAMANHO_BUFFER];
    // Aqui envio primeiro se acabou ou não a simulação e depois o id da pessoa criada
    snprintf(bufferEnviar, TAMANHO_BUFFER, "%d %d", NAO_ACABOU, person.idPessoa);
    enviarDados(bufferEnviar);
    praca.numeroAtualPessoas++; // Adiciona a pessoa à Bilheteria (na próxima entrega iremos melhorar)

    pthread_mutex_unlock(&mutexPessoaEnviar);
}
*/

// Função para inicializar os trincos
void exclusaoMutua() {
	// Inicia mutex para criar pessoa
    if (pthread_mutex_init(&mutexPessoa, NULL) != 0) {
        perror("Erro na inicialização do mutexPessoa");
        exit(1);
    }

    // Inicia mutex para filas
    if (pthread_mutex_init(&mutexFilas, NULL) != 0) {
        perror("Erro na inicialização do mutexFilas");
        exit(1);
    }

	// Inicia mutex para enviar pessoa
    if (pthread_mutex_init(&mutexPessoaEnviar, NULL) != 0) {
        perror("Erro na inicialização do mutexPessoaEnviar");
        exit(1);
    }

	// Inicia mutex para enviar dados (buffer)
    if (pthread_mutex_init(&mutexDados, NULL) != 0) {
        perror("Erro na inicialização do mutexDados");
        exit(1);
    }

	// Inicia mutex para simulação
    if (pthread_mutex_init(&mutexSimulacao, NULL) != 0) {
        perror("Erro na inicialização do mutexSimulacao");
        exit(1);
    }

    sem_init(&praca.fila, 0, 1);
}

// Função principal do simulador
void simulador(char* config) {
    configuracao(config); // Lê as configurações do arquivo e inicializa as variáveis
    exclusaoMutua(); // Inicializa os mutexes

    while (conf.tempoSimulacao != tempoSimulado) { // Enquanto não acaba o tempo de simulação

        tempoSimulado++;

        if (tempoSimulado % conf.tempoChegadaPessoas == 0) {
            // Cria uma nova thread para representar uma pessoa
            pthread_mutex_lock(&mutexSimulacao);
            if (pthread_create(&idThread[idPessoa], NULL, enviarPessoa, NULL) != 0) {
                perror("Erro na criação da thread");
                exit(1);
            }
            pthread_mutex_unlock(&mutexSimulacao);
        }

        usleep(10000); // Aguarda um curto período de tempo (microssegundos)

    }

    if (conf.tempoSimulacao <= tempoSimulado) { // Chegou ao tempo final da simulação
        printf("Acabou a simulação\n");
        char bufferEnviar[TAMANHO_BUFFER];
        // Aqui envio que acabou a simulação e não envio nenhuma pessoa
        snprintf(bufferEnviar, TAMANHO_BUFFER, "%d %d", ACABOU, 0);
        enviarDados(bufferEnviar);
    }
}

// Função principal do programa
int main(int argc, char *argv[]) {

    if (argc == 2) { // Verifica se o número de argumentos da linha de comando é igual a 2
        socketFD = socketSimulador();
        simulador(argv[1]); // Inicia a simulação com o arquivo de configuração especificado
        close(socketFD);
        return 0;
    } else {
        printf("É preciso passar como argumento o ficheiro de configuração.\n");
        return -1;
    }
}
