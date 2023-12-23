#include "header.h"

//Variáveis globais
int socketFD;
int idPessoa = 1; // Id para a pessoa criada, que começa a 1 e vai incrementando
int tempoSimulado = 0; // Tempo de simulação que já foi simulado
//int minutosParque = 0; // Tempo de simulação que já foi simulado em minutos
//int tempoParque = 0; // Tempo que o parque está aberto
int pessoasParque = 0;

int zonas[5] = {NATACAO, MERGULHO, TOBOGAS, RESTAURACAO, BALNEARIOS};

int quantidadeZonas = sizeof(zonas)/sizeof(int);

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
sem_t semaforoParque;
sem_t semaforoBalneario;
sem_t semaforoMergulho;
sem_t semaforoTobogas;
sem_t semaforoRestauracao;
sem_t semaforoEnfermaria;

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
    conf.tamanhoFilaParque = atoi(valores[3]);
    conf.tempoEsperaNatação = atoi(valores[4]);
    conf.tamanhoFilaNatação = atoi(valores[5]);
    conf.numeroMaximoNatação = atoi(valores[6]);
    conf.tempoEsperaMergulho = atoi(valores[7]);
    conf.tamanhoFilaMergulho = atoi(valores[8]);
    conf.numeroMaximoMergulho = atoi(valores[9]);
    conf.tempoEsperaTobogãs = atoi(valores[10]);
    conf.tamanhoFilaTobogãs = atoi(valores[11]);
    conf.numeroMaximoTobogãs = atoi(valores[12]);
    conf.tempoEsperaEnfermaria = atoi(valores[13]);
    conf.tamanhoFilaEnfermaria = atoi(valores[14]);
    conf.numeroMaximoEnfermaria = atoi(valores[15]);
    conf.tempoEsperaRestauração = atoi(valores[16]);
    conf.tamanhoFilaRestauração = atoi(valores[17]);
    conf.numeroMaximoRestauração = atoi(valores[18]);
    conf.tempoEsperaBalnearios = atoi(valores[19]);
    conf.tamanhoFilaBalnearios = atoi(valores[20]);
    conf.numeroMaximoBalnearios = atoi(valores[21]);
    conf.probabilidadeMagoar = strtof(valores[22], &fim);
    conf.probabilidadeDesistir = strtof(valores[23], &fim);
    conf.probabilidadeVIP = strtof(valores[24], &fim);
    conf.tempoEsperaMax = atoi(valores[25]);
    conf.tempoSimulacao = atoi(valores[26]);
    conf.tempoChegadaPessoas = atoi(valores[27]);

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
    person.zonaAtual = FORADOPARQUE;
    person.tempoMaxEspera = randomEntreNumeros((conf.tempoEsperaMax / 2), conf.tempoEsperaMax); // Tempo de espera randomizado entre metade do tempo de espera máximo e o tempo de espera máximo
    for (int i = 0; i < NUM_ATRACOES; i++) {
        person.visitas[i] = 0;
    }
    person.totalVisitadas = 0;

    pthread_mutex_lock(&mutexPessoa);
    idPessoa++;
    pthread_mutex_unlock(&mutexPessoa);

    printf("Criou uma pessoa ao parque com ID %d\n", person.idPessoa);

    return person;// Retorna o descritor do socket conectado
}

// Visitando as atrações
int visitarProximaAtracao(struct pessoa *pessoa) {

    if (pessoa->totalVisitadas < quantidadeZonas) {
        int indice;
        do {
            indice = rand() % quantidadeZonas;
        } while (pessoa->visitas[indice] == 1);

        pessoa->visitas[indice] = 1;
        pessoa->totalVisitadas++;
        //printf("Visitando atração %d\n", indice + NATACAO);
        return indice + NATACAO; // Retorna o número da atração visitada
    } else {
        int indice = rand() % quantidadeZonas;
        //printf("Visitando atração %d novamente\n", indice + NATACAO);
        return indice + NATACAO; // Retorna número da atração visitada repetidamente
    }
}

void Fila (struct pessoa *pessoa) {
    
    char buffer[TAMANHO_BUFFER];
    //int tempo = tempoSimulado;
    int tempoDeEspera, valorDoSemaforo;
    
    if(pessoa->zonaAtual == FORADOPARQUE){

        if(pessoasParque < conf.quantidadePessoasParque){
            sem_wait(&semaforoParque);
            pthread_mutex_lock(&mutexFilas);
            pessoasParque++;
            pthread_mutex_unlock(&mutexFilas);
            pessoa->zonaAtual = PRACA;
            printf("\033[0;32m A pessoa com ID %d entrou no parque \n\033[0m", pessoa->idPessoa);
            //enviarInformação
        } else {

            if (praca.numeroPessoasNaFila < conf.tamanhoFilaParque) {

                sem_wait(&praca.fila);
                printf("\033[0;33mA pessoa com ID %d chegou à fila para entrar no parque\n\033[0m", pessoa->idPessoa);

                pthread_mutex_lock(&mutexFilas);
                //pessoa->tempoDeChegadaFila = tempoSimulado;
                praca.numeroPessoasNaFila++;
                pthread_mutex_unlock(&mutexFilas);

                printf("Pessoas na fila: %d\n", praca.numeroPessoasNaFila);

                pthread_mutex_lock(&mutexSimulacao);
                //Fazer dependendo do tempo que chegou e de quantas pessoas estão na fila!!!!!!
                tempoDeEspera = randomEntreNumeros(conf.tempoEsperaParque, conf.tempoEsperaMax);
                pthread_mutex_unlock(&mutexSimulacao);

                printf("Tempo de Espera: %d\n", tempoDeEspera);
                printf("Tempo máximo de espera: %d\n", pessoa->tempoMaxEspera);
                
                if (tempoDeEspera > pessoa->tempoMaxEspera) {

                    sem_post(&praca.fila);
                    pthread_mutex_lock(&mutexFilas);
                    praca.numeroPessoasNaFila--;
                    pthread_mutex_unlock(&mutexFilas);
                    printf("\033[0;31mA pessoa com ID %d desistiu de entrar no parque\n\033[0m", pessoa->idPessoa);
                    pessoa->desistir = TRUE;
                    //enviarInformação

                } else {
                    sem_wait(&semaforoParque);
                    sem_post(&praca.fila);
                    pthread_mutex_lock(&mutexFilas);
                    praca.numeroPessoasNaFila--;
                    pthread_mutex_unlock(&mutexFilas);
                    pessoa->zonaAtual = PRACA;
                    printf("\033[0;32m A pessoa com ID %d entrou no parque depois de esperar na fila\n\033[0m", pessoa->idPessoa);
                    //enviarInformação
                }

            }
        }
    } else if (pessoa->zonaAtual == BALNEARIOS) {

        if(balnearios.numeroAtualPessoas < conf.numeroMaximoBalnearios){
            sem_wait(&semaforoBalneario);
            pthread_mutex_lock(&mutexFilas);
            balnearios.numeroAtualPessoas++;
            pthread_mutex_unlock(&mutexFilas);
            //pessoa->zonaAtual = BALNEARIOS;
            printf("\033[0;32m A pessoa com ID %d entrou nos balneários \n\033[0m", pessoa->idPessoa);
            //enviarInformação
        } else {

            if (balnearios.numeroPessoasNaFila < conf.tamanhoFilaBalnearios) {

                sem_wait(&balnearios.fila);
                printf("\033[0;33mA pessoa com ID %d chegou à fila para entrar nos balneários\n\033[0m", pessoa->idPessoa);

                pthread_mutex_lock(&mutexFilas);
                //pessoa->tempoDeChegadaFila = tempoSimulado;
                balnearios.numeroPessoasNaFila++;
                pthread_mutex_unlock(&mutexFilas);

                pthread_mutex_lock(&mutexSimulacao);
                //Fazer dependendo do tempo que chegou e de quantas pessoas estão na fila!!!!!!
                tempoDeEspera = randomEntreNumeros(conf.tempoEsperaBalnearios, conf.tempoEsperaMax);
                pthread_mutex_unlock(&mutexSimulacao);
                
                if (tempoDeEspera > pessoa->tempoMaxEspera) {

                    sem_post(&balnearios.fila);
                    pthread_mutex_lock(&mutexFilas);
                    balnearios.numeroPessoasNaFila--;
                    pthread_mutex_unlock(&mutexFilas);
                    printf("\033[0;31mA pessoa com ID %d desistiu de entrar nos balneários\n\033[0m", pessoa->idPessoa);
                    pessoa->zonaAtual = PRACA;
                    //enviarInformação

                } else {

                    sem_wait(&semaforoBalneario);
                    sem_post(&balnearios.fila);
                    pthread_mutex_lock(&mutexFilas);
                    balnearios.numeroPessoasNaFila--;
                    pthread_mutex_unlock(&mutexFilas);
                    printf("\033[0;32m A pessoa com ID %d entrou nos balneários depois de esperar na fila\n\033[0m", pessoa->idPessoa);
                    //enviarInformação

                }
            }
        }
    } else if (pessoa->zonaAtual == NATACAO) {

        if(natacao.numeroAtualPessoas < conf.numeroMaximoNatação){
            sem_wait(&semaforoNatacao);
            pthread_mutex_lock(&mutexFilas);
            natacao.numeroAtualPessoas++;
            pthread_mutex_unlock(&mutexFilas);
            //pessoa->zonaAtual = BALNEARIOS;
            printf("\033[0;32m A pessoa com ID %d entrou na atração de natação \n\033[0m", pessoa->idPessoa);
            //enviarInformação
        } else {

            if (natacao.numeroPessoasNaFila < conf.tamanhoFilaBalnearios) {

                sem_wait(&natacao.fila);
                printf("\033[0;33mA pessoa com ID %d chegou à fila para entrar na atração de natação\n\033[0m", pessoa->idPessoa);

                pthread_mutex_lock(&mutexFilas);
                //pessoa->tempoDeChegadaFila = tempoSimulado;
                natacao.numeroPessoasNaFila++;
                pthread_mutex_unlock(&mutexFilas);

                pthread_mutex_lock(&mutexSimulacao);
                //Fazer dependendo do tempo que chegou e de quantas pessoas estão na fila!!!!!!
                tempoDeEspera = randomEntreNumeros(conf.tempoEsperaNatação, conf.tempoEsperaMax);
                pthread_mutex_unlock(&mutexSimulacao);
                
                if (tempoDeEspera > pessoa->tempoMaxEspera) {

                    sem_post(&natacao.fila);
                    pthread_mutex_lock(&mutexFilas);
                    natacao.numeroPessoasNaFila--;
                    pthread_mutex_unlock(&mutexFilas);
                    printf("\033[0;31mA pessoa com ID %d desistiu de entrar na atração de natação\n\033[0m", pessoa->idPessoa);
                    pessoa->zonaAtual = PRACA;
                    //enviarInformação

                } else {

                    sem_wait(&semaforoNatacao);
                    sem_post(&natacao.fila);
                    pthread_mutex_lock(&mutexFilas);
                    natacao.numeroPessoasNaFila--;
                    pthread_mutex_unlock(&mutexFilas);
                    printf("\033[0;32m A pessoa com ID %d entrou na atração de natação depois de esperar na fila\n\033[0m", pessoa->idPessoa);
                    //enviarInformação

                }
            }
        }
    } else if (pessoa->zonaAtual == MERGULHO) {

        if(mergulho.numeroAtualPessoas < conf.numeroMaximoMergulho){
            sem_wait(&semaforoMergulho);
            pthread_mutex_lock(&mutexFilas);
            mergulho.numeroAtualPessoas++;
            pthread_mutex_unlock(&mutexFilas);
            //pessoa->zonaAtual = BALNEARIOS;
            printf("\033[0;32m A pessoa com ID %d entrou na atração de mergulho \n\033[0m", pessoa->idPessoa);
            //enviarInformação
        } else {

            if (mergulho.numeroPessoasNaFila < conf.tamanhoFilaMergulho) {

                sem_wait(&mergulho.fila);
                printf("\033[0;33mA pessoa com ID %d chegou à fila para entrar na atração de mergulho\n\033[0m", pessoa->idPessoa);

                pthread_mutex_lock(&mutexFilas);
                //pessoa->tempoDeChegadaFila = tempoSimulado;
                mergulho.numeroPessoasNaFila++;
                pthread_mutex_unlock(&mutexFilas);

                pthread_mutex_lock(&mutexSimulacao);
                //Fazer dependendo do tempo que chegou e de quantas pessoas estão na fila!!!!!!
                tempoDeEspera = randomEntreNumeros(conf.tempoEsperaMergulho, conf.tempoEsperaMax);
                pthread_mutex_unlock(&mutexSimulacao);
                
                if (tempoDeEspera > pessoa->tempoMaxEspera) {

                    sem_post(&mergulho.fila);
                    pthread_mutex_lock(&mutexFilas);
                    mergulho.numeroPessoasNaFila--;
                    pthread_mutex_unlock(&mutexFilas);
                    printf("\033[0;31mA pessoa com ID %d desistiu de entrar na atração de mergulho\n\033[0m", pessoa->idPessoa);
                    pessoa->zonaAtual = PRACA;
                    //enviarInformação

                } else {

                    sem_wait(&semaforoMergulho);
                    sem_post(&mergulho.fila);
                    pthread_mutex_lock(&mutexFilas);
                    mergulho.numeroPessoasNaFila--;
                    pthread_mutex_unlock(&mutexFilas);
                    printf("\033[0;32m A pessoa com ID %d entrou na atração de mergulho depois de esperar na fila\n\033[0m", pessoa->idPessoa);
                    //enviarInformação

                }
            }
        }
    } else if (pessoa->zonaAtual == TOBOGAS) {

        if(tobogas.numeroAtualPessoas < conf.numeroMaximoTobogãs){
            sem_wait(&semaforoTobogas);
            pthread_mutex_lock(&mutexFilas);
            tobogas.numeroAtualPessoas++;
            pthread_mutex_unlock(&mutexFilas);
            printf("\033[0;32m A pessoa com ID %d entrou na atração de tobogãs \n\033[0m", pessoa->idPessoa);
            //enviarInformação
        } else {

            if (tobogas.numeroPessoasNaFila < conf.tamanhoFilaTobogãs) {

                sem_wait(&tobogas.fila);
                printf("\033[0;33mA pessoa com ID %d chegou à fila para entrar na atração de tobogãs\n\033[0m", pessoa->idPessoa);

                pthread_mutex_lock(&mutexFilas);
                //pessoa->tempoDeChegadaFila = tempoSimulado;
                tobogas.numeroPessoasNaFila++;
                pthread_mutex_unlock(&mutexFilas);

                pthread_mutex_lock(&mutexSimulacao);
                //Fazer dependendo do tempo que chegou e de quantas pessoas estão na fila!!!!!!
                tempoDeEspera = randomEntreNumeros(conf.tempoEsperaTobogãs, conf.tempoEsperaMax);
                pthread_mutex_unlock(&mutexSimulacao);
                
                if (tempoDeEspera > pessoa->tempoMaxEspera) {

                    sem_post(&tobogas.fila);
                    pthread_mutex_lock(&mutexFilas);
                    tobogas.numeroPessoasNaFila--;
                    pthread_mutex_unlock(&mutexFilas);
                    printf("\033[0;31mA pessoa com ID %d desistiu de entrar na atração de tobogãs\n\033[0m", pessoa->idPessoa);
                    pessoa->zonaAtual = PRACA;
                    //enviarInformação

                } else {

                    sem_wait(&semaforoTobogas);
                    sem_post(&tobogas.fila);
                    pthread_mutex_lock(&mutexFilas);
                    tobogas.numeroPessoasNaFila--;
                    pthread_mutex_unlock(&mutexFilas);
                    printf("\033[0;32m A pessoa com ID %d entrou na atração de tobogãs depois de esperar na fila\n\033[0m", pessoa->idPessoa);
                    //enviarInformação

                }
            }
        }
    } else if (pessoa->zonaAtual == RESTAURACAO) {

        if(restauracao.numeroAtualPessoas < conf.numeroMaximoRestauração){
            sem_wait(&semaforoRestauracao);
            pthread_mutex_lock(&mutexFilas);
            restauracao.numeroAtualPessoas++;
            pthread_mutex_unlock(&mutexFilas);
            printf("\033[0;32m A pessoa com ID %d entrou na atração de restauração \n\033[0m", pessoa->idPessoa);
            //enviarInformação
        } else {

            if (restauracao.numeroPessoasNaFila < conf.tamanhoFilaRestauração) {

                sem_wait(&restauracao.fila);
                printf("\033[0;33mA pessoa com ID %d chegou à fila para entrar na restauração\n\033[0m", pessoa->idPessoa);

                pthread_mutex_lock(&mutexFilas);
                //pessoa->tempoDeChegadaFila = tempoSimulado;
                restauracao.numeroPessoasNaFila++;
                pthread_mutex_unlock(&mutexFilas);

                pthread_mutex_lock(&mutexSimulacao);
                //Fazer dependendo do tempo que chegou e de quantas pessoas estão na fila!!!!!!
                tempoDeEspera = randomEntreNumeros(conf.tempoEsperaRestauração, conf.tempoEsperaMax);
                pthread_mutex_unlock(&mutexSimulacao);
                
                if (tempoDeEspera > pessoa->tempoMaxEspera) {

                    sem_post(&restauracao.fila);
                    pthread_mutex_lock(&mutexFilas);
                    restauracao.numeroPessoasNaFila--;
                    pthread_mutex_unlock(&mutexFilas);
                    printf("\033[0;31mA pessoa com ID %d desistiu de entrar na restauração\n\033[0m", pessoa->idPessoa);
                    pessoa->zonaAtual = PRACA;
                    //enviarInformação

                } else {

                    sem_wait(&semaforoRestauracao);
                    sem_post(&restauracao.fila);
                    pthread_mutex_lock(&mutexFilas);
                    restauracao.numeroPessoasNaFila--;
                    pthread_mutex_unlock(&mutexFilas);
                    printf("\033[0;32m A pessoa com ID %d entrou na restauração depois de esperar na fila\n\033[0m", pessoa->idPessoa);
                    //enviarInformação

                }
            }
        }
    } /*else if (pessoa->zonaAtual == ENFERMARIA) {

    } */
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
        //break;
        if(person.desistir == FALSE){
            if(person.zonaAtual == PRACA) {

                visitarProximaAtracao(&person);

                //ifs

                //Se quer ir ao balneario
                //Escolhe a zona que quer ir 
                //Volta a dar o ciclo
                
            }

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

    sem_init(&praca.fila, 0, conf.tamanhoFilaParque);
    sem_init(&semaforoParque, 0, conf.quantidadePessoasParque);

    sem_init(&balnearios.fila, 0, conf.tamanhoFilaBalnearios);
    sem_init(&semaforoBalneario, 0, conf.numeroMaximoBalnearios);

    sem_init(&natacao.fila, 0, conf.tamanhoFilaNatação);
    sem_init(&semaforoNatacao, 0, conf.numeroMaximoNatação);

    sem_init(&mergulho.fila, 0, conf.tamanhoFilaMergulho);
    sem_init(&semaforoMergulho, 0, conf.numeroMaximoMergulho);

    sem_init(&tobogas.fila, 0, conf.tamanhoFilaTobogãs);
    sem_init(&semaforoTobogas, 0, conf.numeroMaximoTobogãs);

    sem_init(&restauracao.fila, 0, conf.tamanhoFilaRestauração);
    sem_init(&semaforoRestauracao, 0, conf.numeroMaximoRestauração);

    sem_init(&enfermaria.fila, 0, conf.tamanhoFilaEnfermaria);
    sem_init(&semaforoEnfermaria, 0, conf.numeroMaximoEnfermaria);
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
        printf("Pessoas no parque: %d\n", pessoasParque);
        printf("Pessoas na fila: %d\n", praca.numeroPessoasNaFila);
        char bufferEnviar[TAMANHO_BUFFER];
        // Aqui envio que acabou a simulação e não envio nenhuma pessoa
        snprintf(bufferEnviar, TAMANHO_BUFFER, "%d %d", ACABOU, 0);
        enviarDados(bufferEnviar);
    }
}

// Função principal do programa
int main(int argc, char *argv[]) {

    srand(time(NULL));

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
