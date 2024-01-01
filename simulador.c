#include "header.h"

//Variáveis globais
int socketFD = 0;
int idPessoa = 1; // Id para a pessoa criada, que começa a 1 e vai incrementando
int tempoSimulado = 0; // Tempo de simulação que já foi simulado
//int minutosParque = 0; // Tempo de simulação que já foi simulado em minutos
//int tempoParque = 0; // Tempo que o parque está aberto
int pessoasParque = 0;

int zonas[5] = {NATACAO, MERGULHO, TOBOGAS, RESTAURACAO, BALNEARIOS};

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
pthread_mutex_t mutexPessoa = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexPessoaEnviar = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexDados = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexSimulacao = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexFilas = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexZonas = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexParque = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexTempo = PTHREAD_MUTEX_INITIALIZER;
sem_t semaforoCriar;
sem_t semaforoDados;
sem_t semaforoParque;
sem_t semaforoBalneario;
sem_t semaforoNatacao;
sem_t semaforoMergulho;
sem_t semaforoTobogas;
sem_t semaforoRestauracao;
sem_t semaforoEnfermaria;

time_t tempoInicial;

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
    conf.numeroAtracoes = atoi(valores[0]);
    conf.tempoEsperaFilaParque = atoi(valores[1]);
    conf.tempoEntrarPraca = atoi(valores[2]);
    conf.tamanhoFilaParque = atoi(valores[3]);
    conf.tempoEsperaFilaNatacao = atoi(valores[4]);
    conf.tempoNatacao = atoi(valores[5]);
    conf.tamanhoFilaNatacao = atoi(valores[6]);
    conf.numeroMaximoNatacao = atoi(valores[7]);
    conf.tempoEsperaFilaMergulho = atoi(valores[8]);
    conf.tempoMergulho = atoi(valores[9]);
    conf.tamanhoFilaMergulho = atoi(valores[10]);
    conf.numeroMaximoMergulho = atoi(valores[11]);
    conf.tempoEsperaFilaTobogas = atoi(valores[12]);
    conf.tempoTobogas = atoi(valores[13]);
    conf.tamanhoFilaTobogas = atoi(valores[14]);
    conf.numeroMaximoTobogas = atoi(valores[15]);
    conf.tempoEsperaFilaEnfermaria = atoi(valores[16]);
    conf.tempoEnfermaria = atoi(valores[17]);
    conf.tamanhoFilaEnfermaria = atoi(valores[18]);
    conf.numeroMaximoEnfermaria = atoi(valores[19]);
    conf.tempoEsperaFilaRestauracao = atoi(valores[20]);
    conf.tempoRestauracao = atoi(valores[21]);
    conf.tamanhoFilaRestauracao = atoi(valores[22]);
    conf.numeroMaximoRestauracao = atoi(valores[23]);
    conf.tempoEsperaFilaBalnearios = atoi(valores[24]);
    conf.tempoBalnearios = atoi(valores[25]);
    conf.tamanhoFilaBalnearios = atoi(valores[26]);
    conf.numeroMaximoBalnearios = atoi(valores[27]);
    conf.probabilidadeMagoar = strtof(valores[28], &fim);
    conf.probabilidadeDesistir = strtof(valores[29], &fim);
    conf.probabilidadeMudarZona = strtof(valores[30], &fim);
    conf.probabilidadeCurar = strtof(valores[31], &fim);
    conf.tempoSimulacao = atoi(valores[32]);
    conf.tempoChegadaPessoas = atoi(valores[33]);

    conf.quantidadePessoasParque = conf.tamanhoFilaNatacao + conf.numeroMaximoNatacao + 
                                    conf.tamanhoFilaMergulho + conf.numeroMaximoMergulho + 
                                    conf.tamanhoFilaTobogas + conf.numeroMaximoTobogas + 
                                    conf.tamanhoFilaEnfermaria + conf.numeroMaximoEnfermaria + 
                                    conf.tamanhoFilaBalnearios + conf.numeroMaximoBalnearios + 
                                    conf.tamanhoFilaRestauracao + conf.numeroMaximoRestauracao;

    return 0; // Retorna 0 para indicar sucesso
}

int calculaProbabilidadeMudar(float probabilidade, struct pessoa *pessoa) {

    float random_value = (float)rand() / RAND_MAX; // Normaliza o valor entre 0 e 1

    // Aumenta a probabilidade linearmente com o número de visitas
    if (pessoa->totalVisitadas > 4) {
        probabilidade = probabilidade - (pessoa->totalVisitadas * 0.05);
    }

    // Verifica se a probabilidade não ultrapassa 1.0
    if (probabilidade > 1.0) {
        probabilidade = 1.0;
    }

    //printf("Probabilidade de mudar da pessoa com ID %d: %f\n", pessoa->idPessoa ,probabilidade);
    return random_value < probabilidade;
}

int calculaProbabilidadeDesistir(float probabilidade, struct pessoa *pessoa) {

    float random_value = (float)rand() / RAND_MAX; // Normaliza o valor entre 0 e 1

    // Aumenta a probabilidade linearmente com o número de visitas
    if (pessoa->totalVisitadas > 4) {
        probabilidade = probabilidade + (pessoa->totalVisitadas * 0.03);
    }

    // Verifica se a probabilidade não ultrapassa 1.0
    if ((probabilidade > 1.0) || (tempoSimulado>conf.tempoSimulacao)) {
        probabilidade = 1.0;
    }

    //printf("Probabilidade de desistir da pessoa com ID %d: %f\n", pessoa->idPessoa ,probabilidade);
    return random_value > probabilidade;
}

int calcularProbabilidade(float probabilidade){

    float random_value = (float)rand() / RAND_MAX;

    return random_value < probabilidade;
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
    person.magoar = 0; // Pessoa ainda não se magoou, pois acabou de ser criada
    person.zonaAtual = FORADOPARQUE;
    for (int i = 0; i < conf.numeroAtracoes; i++) {
        person.visitas[i] = 0;
    }
    person.totalVisitadas = 0;
    person.dentroParque = FALSE;

    pthread_mutex_lock(&mutexPessoa);
    idPessoa++;
    pthread_mutex_unlock(&mutexPessoa);

    //printf("Chegou uma pessoa ao parque com ID %d | Tempo: %d\n", person.idPessoa, tempoSimulado);

    return person;// Retorna o descritor do socket conectado
}

// Visitando as atrações
int visitarProximaAtracao(struct pessoa *pessoa) {

    if (pessoa->totalVisitadas < conf.numeroAtracoes) {
        int indice;
        do {
            indice = rand() % conf.numeroAtracoes;
        } while (pessoa->visitas[indice] == 1);

        pessoa->visitas[indice] = 1;
        pessoa->totalVisitadas++;
        //printf("Pessoa com ID: %d visitando atração %d\n", pessoa->idPessoa, indice + NATACAO);
        return indice + NATACAO; // Retorna o número da atração visitada
    } else {
        int indice = rand() % conf.numeroAtracoes;
        //printf("Pessoa com ID: %d Visitando atração %d novamente\n", pessoa->idPessoa,indice + NATACAO);
        return indice + NATACAO; // Retorna número da atração visitada repetidamente
    }
}

void Fila (struct pessoa *pessoa) {
    
    char buffer[TAMANHO_BUFFER];
    int tempoDeEspera, valorDoSemaforo;

    sleep(1);

    if(pessoa->zonaAtual == FORADOPARQUE){

        if(pessoasParque < conf.quantidadePessoasParque){
            
            sem_wait(&semaforoParque);

            pthread_mutex_lock(&mutexFilas);
            pessoasParque++;
            pthread_mutex_unlock(&mutexFilas);

            pessoa->zonaAtual = PRACA;
            pessoa->dentroParque = TRUE;

            enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, ENTRAR, PRACA);
            printf(VERDE_CLARO "A pessoa com ID %d entrou no parque | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);
            
            sleep(conf.tempoEntrarPraca);

        } else {

            if (praca.numeroPessoasNaFila < conf.tamanhoFilaParque) {

                sem_wait(&praca.fila);
                
                pthread_mutex_lock(&mutexFilas);
                praca.numeroPessoasNaFila++;
                pthread_mutex_unlock(&mutexFilas);

                pthread_mutex_lock(&mutexSimulacao);
                tempoDeEspera = conf.tempoEntrarPraca*praca.numeroPessoasNaFila;
                pthread_mutex_unlock(&mutexSimulacao);
                
                enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, ENTRAR_FILA, PRACA);
                printf(AMARELO_CLARO "A pessoa com ID %d chegou à fila para entrar no parque | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);

                sleep(tempoDeEspera);
                
                if(pessoasParque<conf.quantidadePessoasParque){

                    if (tempoSimulado > conf.tempoSimulacao){
                        sem_post(&praca.fila);

                        pthread_mutex_lock(&mutexFilas);
                        praca.numeroPessoasNaFila--;
                        pthread_mutex_unlock(&mutexFilas);

                        enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, SAIR_FILA, PRACA);
                        printf(VERMELHO_CLARO "A pessoa com ID %d desistiu de entrar no parque porque este estava a fechar | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);

                        pessoa->desistir = TRUE;

                    } else {

                        sem_post(&praca.fila);
                        sem_wait(&semaforoParque);

                        pthread_mutex_lock(&mutexFilas);
                        praca.numeroPessoasNaFila--;
                        pessoasParque++;
                        pthread_mutex_unlock(&mutexFilas);

                        pessoa->zonaAtual = PRACA;
                        pessoa->dentroParque = TRUE;

                        enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, SAIR_FILA_ENTRAR, PRACA);
                        printf(VERDE_CLARO "A pessoa com ID %d entrou no parque depois de esperar %d segundos | Tempo: %d\n" RESET, pessoa->idPessoa, tempoDeEspera, tempoSimulado);
                        
                        sleep(conf.tempoEntrarPraca);

                    }

                }else {
                    
                    sem_post(&praca.fila);

                    pthread_mutex_lock(&mutexFilas);
                    praca.numeroPessoasNaFila--;
                    pthread_mutex_unlock(&mutexFilas);

                    enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, SAIR_FILA, PRACA);
                    printf(VERMELHO_CLARO "A pessoa com ID %d desistiu de entrar no parque porque não queria esperar na fila | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);
                    
                    pessoa->desistir = TRUE;
                }
            } else {

                printf(VERMELHO "A pessoa com ID %d desistiu de entrar no parque porque não havia lugar na fila | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);

                pessoa->desistir = TRUE;
            }
        } 
    } else if (pessoa->zonaAtual == BALNEARIOS) {

        if (tempoSimulado > conf.tempoSimulacao) {

            pessoa->zonaAtual = PRACA;

        } else {

            if(balnearios.numeroAtualPessoas < conf.numeroMaximoBalnearios){
                
                sem_wait(&semaforoBalneario);
                
                pthread_mutex_lock(&mutexFilas);
                balnearios.numeroAtualPessoas++;
                pthread_mutex_unlock(&mutexFilas);
                
                enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, ENTRAR, BALNEARIOS);
                printf(CIANO_CLARO "A pessoa com ID %d entrou nos balneários | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);

                sleep(conf.tempoBalnearios);
                
            } else {

                if (balnearios.numeroPessoasNaFila < conf.tamanhoFilaBalnearios) {

                    sem_wait(&balnearios.fila);

                    pthread_mutex_lock(&mutexFilas);
                    balnearios.numeroPessoasNaFila++;
                    pthread_mutex_unlock(&mutexFilas);

                    pthread_mutex_lock(&mutexSimulacao);
                    tempoDeEspera = conf.tempoEsperaFilaBalnearios * balnearios.numeroPessoasNaFila;
                    pthread_mutex_unlock(&mutexSimulacao);
                    
                    enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, ENTRAR_FILA, BALNEARIOS);
                    printf(CIANO "A pessoa com ID %d chegou à fila para entrar nos balneários | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);

                    sleep(tempoDeEspera);

                    if (tempoSimulado > conf.tempoSimulacao){
                        sem_post(&balnearios.fila);

                        pthread_mutex_lock(&mutexFilas);
                        balnearios.numeroPessoasNaFila--;
                        pthread_mutex_unlock(&mutexFilas);

                        enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, SAIR_FILA, BALNEARIOS);
                        printf(VERMELHO "A pessoa com ID %d teve que sair da fila dos balneários porque o parque está a fechar | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);

                        pessoa->zonaAtual = PRACA;
                    } else{
                        if (balnearios.numeroAtualPessoas >= conf.numeroMaximoBalnearios) {

                            sem_post(&balnearios.fila);
                            sem_wait(&semaforoParque);
                            
                            pthread_mutex_lock(&mutexFilas);
                            balnearios.numeroPessoasNaFila--;
                            pthread_mutex_unlock(&mutexFilas);

                            enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, SAIR_FILA, BALNEARIOS);
                            printf(VERMELHO "A pessoa com ID %d desistiu de entrar nos balneários porque não queria esperar na fila | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);

                            pessoa->zonaAtual = PRACA;

                        } else {

                            sem_post(&balnearios.fila);
                            sem_wait(&semaforoBalneario);
                            
                            pthread_mutex_lock(&mutexFilas);
                            balnearios.numeroPessoasNaFila--;
                            balnearios.numeroAtualPessoas++;
                            pthread_mutex_unlock(&mutexFilas);
                            
                            enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, SAIR_FILA_ENTRAR, BALNEARIOS);
                            printf(CIANO_CLARO "A pessoa com ID %d entrou nos balneários depois de esperar na fila | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);

                            sleep(conf.tempoBalnearios);
                        }
                    }

                } else {

                    printf(VERMELHO "A pessoa com ID %d desistiu de entrar nos balneários porque não havia lugar na fila | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);
                    
                    sem_wait(&semaforoParque);
                    
                    pessoa->zonaAtual = PRACA;
                }
            }
        }

    } else if (pessoa->zonaAtual == NATACAO) {

        if (tempoSimulado > conf.tempoSimulacao) {

            pessoa->zonaAtual = PRACA;

        } else {

            if(pessoa->altura >= 100 && pessoa->idade >= 5){

                if(natacao.numeroAtualPessoas < conf.numeroMaximoNatacao){

                    sem_wait(&semaforoNatacao);

                    pthread_mutex_lock(&mutexFilas);
                    natacao.numeroAtualPessoas++;
                    pthread_mutex_unlock(&mutexFilas);

                    enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, ENTRAR, NATACAO);
                    printf(CIANO_CLARO "A pessoa com ID %d entrou na atração de natação | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);
                    
                    sleep(conf.tempoNatacao);

                    if((tempoSimulado > conf.tempoSimulacao)) {

                        sem_post(&semaforoNatacao);

                        pthread_mutex_lock(&mutexFilas);
                        natacao.numeroAtualPessoas--;
                        pthread_mutex_unlock(&mutexFilas);

                        enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, SAIR, NATACAO);
                        printf(VERMELHO "A pessoa com ID %d teve que sair da atração de natação porque o parque está a fechar | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);

                        pessoa->zonaAtual = PRACA;

                    } else if (calcularProbabilidade(conf.probabilidadeMagoar)) {

                        printf(ROXO "A pessoa com ID %d magoou-se e tem de ir para a enfermaria | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);
                        
                        sem_post(&semaforoNatacao);
                        sem_wait(&semaforoParque);

                        pthread_mutex_lock(&mutexZonas);
                        natacao.numeroAtualPessoas--;
                        pthread_mutex_unlock(&mutexZonas);

                        enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, SAIR, NATACAO);
                        printf(VERMELHO "A pessoa com ID %d saiu da atração de natação | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);
                        
                        pessoa->magoar = TRUE;
                        pessoa->zonaAtual = ENFERMARIA;
                    }

                } else {

                    if (natacao.numeroPessoasNaFila < conf.tamanhoFilaNatacao) {

                        sem_wait(&natacao.fila);

                        pthread_mutex_lock(&mutexFilas);
                        natacao.numeroPessoasNaFila++;
                        pthread_mutex_unlock(&mutexFilas);

                        pthread_mutex_lock(&mutexSimulacao);
                        tempoDeEspera = conf.tempoEsperaFilaNatacao * natacao.numeroPessoasNaFila;
                        pthread_mutex_unlock(&mutexSimulacao);

                        enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, ENTRAR_FILA, NATACAO);
                        printf(CIANO "A pessoa com ID %d chegou à fila para entrar na atração de natação | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);
                        
                        sleep(tempoDeEspera);

                        if (tempoSimulado > conf.tempoSimulacao){
                            sem_post(&natacao.fila);

                            pthread_mutex_lock(&mutexFilas);
                            natacao.numeroPessoasNaFila--;
                            pthread_mutex_unlock(&mutexFilas);

                            enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, SAIR_FILA, NATACAO);
                            printf(VERMELHO "A pessoa com ID %d teve que sair da fila da natação porque o parque está a fechar | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);

                            pessoa->zonaAtual = PRACA;

                        } else{

                            if (natacao.numeroAtualPessoas >= conf.numeroMaximoNatacao) {

                                sem_post(&natacao.fila);
                                sem_wait(&semaforoParque);

                                pthread_mutex_lock(&mutexFilas);
                                natacao.numeroPessoasNaFila--;
                                pthread_mutex_unlock(&mutexFilas);

                                enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, SAIR_FILA, NATACAO);
                                printf(VERMELHO "A pessoa com ID %d desistiu de entrar na atração de natação porque não queria esperar na fila | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);
                                
                                pessoa->zonaAtual = PRACA;

                            } else {

                                sem_post(&natacao.fila);
                                sem_wait(&semaforoNatacao);
                                
                                pthread_mutex_lock(&mutexFilas);
                                natacao.numeroPessoasNaFila--;
                                natacao.numeroAtualPessoas++;
                                pthread_mutex_unlock(&mutexFilas);

                                enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, SAIR_FILA_ENTRAR, NATACAO);
                                printf(CIANO_CLARO "A pessoa com ID %d entrou na atração de natação depois de esperar na fila | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);

                                sleep(conf.tempoNatacao);

                                if((tempoSimulado > conf.tempoSimulacao)) {

                                    sem_post(&semaforoNatacao);

                                    pthread_mutex_lock(&mutexFilas);
                                    natacao.numeroAtualPessoas--;
                                    pthread_mutex_unlock(&mutexFilas);

                                    enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, SAIR, NATACAO);
                                    printf(VERMELHO "A pessoa com ID %d teve que sair da atração de natação porque o parque está a fechar | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);

                                    pessoa->zonaAtual = PRACA;

                                }else if(calcularProbabilidade(conf.probabilidadeMagoar)){
                            
                                    sem_post(&semaforoNatacao);
                                    sem_wait(&semaforoParque);
                                    
                                    pthread_mutex_lock(&mutexZonas);
                                    natacao.numeroAtualPessoas--;
                                    pthread_mutex_unlock(&mutexZonas);

                                    enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, SAIR, NATACAO);
                                    printf(ROXO "A pessoa com ID %d magoou-se e tem de ir para a enfermaria | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);
                                    printf(VERMELHO "A pessoa com ID %d saiu da atração de natação | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);
                                    
                                    pessoa->magoar = TRUE;
                                    pessoa->zonaAtual = ENFERMARIA;
                                }
                            }
                    }
                    } else {
                        printf(VERMELHO "A pessoa com ID %d desistiu de entrar na atração de natação porque não havia lugar na fila | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);
                        
                        sem_wait(&semaforoParque);
                        
                        pessoa->zonaAtual = PRACA;
                    }
                }
            }else{
                printf(CINZA "A pessoa com ID %d não tem altura e idade mínima requirida para entrar na natação | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);
                
                sem_wait(&semaforoParque);
                
                pessoa->zonaAtual = PRACA;

            }
        }

    } else if (pessoa->zonaAtual == MERGULHO) {

        if (tempoSimulado > conf.tempoSimulacao) {

            pessoa->zonaAtual = PRACA;

        } else {

            if((pessoa->altura <= 200 && pessoa->altura >= 100) && (pessoa->idade <= 55 && pessoa->idade >= 10)){

                if(mergulho.numeroAtualPessoas < conf.numeroMaximoMergulho){
                    sem_wait(&semaforoMergulho);
                    
                    pthread_mutex_lock(&mutexFilas);
                    mergulho.numeroAtualPessoas++;
                    pthread_mutex_unlock(&mutexFilas);

                    enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, ENTRAR, MERGULHO);
                    printf(CIANO_CLARO "A pessoa com ID %d entrou na atração de mergulho | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);
                    
                    sleep(conf.tempoMergulho);

                    if((tempoSimulado > conf.tempoSimulacao)) {

                        sem_post(&semaforoMergulho);

                        pthread_mutex_lock(&mutexFilas);
                        mergulho.numeroAtualPessoas--;
                        pthread_mutex_unlock(&mutexFilas);

                        enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, SAIR, MERGULHO);
                        printf(VERMELHO "A pessoa com ID %d teve que sair da atração de mergulho porque o parque está a fechar | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);

                        pessoa->zonaAtual = PRACA;

                    } else if(calcularProbabilidade(conf.probabilidadeMagoar)){
                        
                        sem_post(&semaforoMergulho);
                        sem_wait(&semaforoParque);
                        
                        pthread_mutex_lock(&mutexZonas);
                        mergulho.numeroAtualPessoas--;
                        pthread_mutex_unlock(&mutexZonas);
                        
                        enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, SAIR, MERGULHO);
                        printf(ROXO "A pessoa com ID %d magoou-se e tem de ir para a enfermaria | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);
                        printf(VERMELHO "A pessoa com ID %d saiu da atração de mergulho | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);
                        
                        pessoa->magoar = TRUE;
                        pessoa->zonaAtual = ENFERMARIA;
                    }

                } else {

                    if (mergulho.numeroPessoasNaFila < conf.tamanhoFilaMergulho) {

                        sem_wait(&mergulho.fila);

                        pthread_mutex_lock(&mutexFilas);
                        mergulho.numeroPessoasNaFila++;
                        pthread_mutex_unlock(&mutexFilas);

                        pthread_mutex_lock(&mutexSimulacao);
                        tempoDeEspera = conf.tempoEsperaFilaMergulho * mergulho.numeroPessoasNaFila;
                        pthread_mutex_unlock(&mutexSimulacao);

                        enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, ENTRAR_FILA, MERGULHO);
                        printf(CIANO "A pessoa com ID %d chegou à fila para entrar na atração de mergulho | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);

                        sleep(tempoDeEspera);

                        if (tempoSimulado > conf.tempoSimulacao){
                            sem_post(&mergulho.fila);

                            pthread_mutex_lock(&mutexFilas);
                            mergulho.numeroPessoasNaFila--;
                            pthread_mutex_unlock(&mutexFilas);

                            enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, SAIR_FILA, MERGULHO);
                            printf(VERMELHO "A pessoa com ID %d teve que sair da fila do mergulho porque o parque está a fechar | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);

                            pessoa->zonaAtual = PRACA;

                        } else{
                        
                            if (mergulho.numeroAtualPessoas >= conf.numeroMaximoMergulho) {

                                sem_post(&mergulho.fila);
                                sem_wait(&semaforoParque);

                                pthread_mutex_lock(&mutexFilas);
                                mergulho.numeroPessoasNaFila--;
                                pthread_mutex_unlock(&mutexFilas);

                                enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, SAIR_FILA, MERGULHO);
                                printf(VERMELHO "A pessoa com ID %d desistiu de entrar na atração de mergulho porque não queria esperar na fila | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);
                                
                                pessoa->zonaAtual = PRACA;

                            } else {

                                sem_post(&mergulho.fila);
                                sem_wait(&semaforoMergulho);
                                
                                pthread_mutex_lock(&mutexFilas);
                                mergulho.numeroPessoasNaFila--;
                                mergulho.numeroAtualPessoas++;
                                pthread_mutex_unlock(&mutexFilas);

                                enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, SAIR_FILA_ENTRAR, MERGULHO);
                                printf(CIANO_CLARO "A pessoa com ID %d entrou na atração de mergulho depois de esperar na fila | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);
                                
                                sleep(conf.tempoMergulho);

                                if((tempoSimulado > conf.tempoSimulacao)) {

                                    sem_post(&semaforoMergulho);

                                    pthread_mutex_lock(&mutexFilas);
                                    mergulho.numeroAtualPessoas--;
                                    pthread_mutex_unlock(&mutexFilas);

                                    enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, SAIR, TOBOGAS);
                                    printf(VERMELHO "A pessoa com ID %d teve que sair da natação porque o parque está a fechar | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);

                                    pessoa->zonaAtual = PRACA;

                                }else if(calcularProbabilidade(conf.probabilidadeMagoar)){
                                
                                    sem_post(&semaforoMergulho);
                                    sem_wait(&semaforoParque);
                                    
                                    pthread_mutex_lock(&mutexZonas);
                                    mergulho.numeroAtualPessoas--;
                                    pthread_mutex_unlock(&mutexZonas);
                                    
                                    enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, SAIR, MERGULHO);
                                    printf(ROXO "A pessoa com ID %d magoou-se e tem de ir para a enfermaria | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);
                                    printf(VERMELHO "A pessoa com ID %d saiu da atração de mergulho | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);
                                    
                                    pessoa->magoar = TRUE;
                                    pessoa->zonaAtual = ENFERMARIA;
                                }

                            }
                        }
                    } else {
                        printf(VERMELHO "A pessoa com ID %d desistiu de entrar na atração de mergulho porque não havia lugar na fila | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);
                        
                        sem_wait(&semaforoParque);
                        
                        pessoa->zonaAtual = PRACA;
                    }
                }
            }else{
                printf(CINZA "A pessoa com ID %d não tem altura e idade mínima requirida para entrar na atração mergulhos | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);
                
                sem_wait(&semaforoParque);
                
                pessoa->zonaAtual = PRACA;
                
            }
        }
        
    } else if (pessoa->zonaAtual == TOBOGAS) {

        if (tempoSimulado > conf.tempoSimulacao) {

            pessoa->zonaAtual = PRACA;

        } else {

            if((pessoa->altura >= 110) && (pessoa->idade <= 70 && pessoa->idade >= 6)){ //criar no conf as alturas minimas e idade das atrações

                if(tobogas.numeroAtualPessoas < conf.numeroMaximoTobogas){
                    sem_wait(&semaforoTobogas);
                    
                    pthread_mutex_lock(&mutexFilas);
                    tobogas.numeroAtualPessoas++;
                    pthread_mutex_unlock(&mutexFilas);
                    
                    enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, ENTRAR, TOBOGAS);
                    printf(CIANO_CLARO "A pessoa com ID %d entrou na atração de tobogãs | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);
                    
                    sleep(conf.tempoTobogas);

                    if((tempoSimulado > conf.tempoSimulacao)) {

                        sem_post(&semaforoTobogas);

                        pthread_mutex_lock(&mutexFilas);
                        tobogas.numeroAtualPessoas--;
                        pthread_mutex_unlock(&mutexFilas);

                        enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, SAIR, TOBOGAS);
                        printf(VERMELHO "A pessoa com ID %d teve que sair da atração de tobogãs porque o parque está a fechar | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);

                        pessoa->zonaAtual = PRACA;

                    }else if(calcularProbabilidade(conf.probabilidadeMagoar)){
                    
                        sem_post(&semaforoTobogas);
                        sem_wait(&semaforoParque);
                        
                        pthread_mutex_lock(&mutexZonas);
                        tobogas.numeroAtualPessoas--;
                        pthread_mutex_unlock(&mutexZonas);

                        enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, SAIR, TOBOGAS);
                        printf(ROXO "A pessoa com ID %d magoou-se e tem de ir para a enfermaria | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);                    
                        printf(VERMELHO "A pessoa com ID %d saiu da atração de tobogãs | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);
                        
                        pessoa->magoar = TRUE;
                        pessoa->zonaAtual = ENFERMARIA;
                    }

                } else {

                    if (tobogas.numeroPessoasNaFila < conf.tamanhoFilaTobogas) {

                        sem_wait(&tobogas.fila);

                        pthread_mutex_lock(&mutexFilas);
                        tobogas.numeroPessoasNaFila++;
                        pthread_mutex_unlock(&mutexFilas);

                        pthread_mutex_lock(&mutexSimulacao);
                        tempoDeEspera = conf.tempoEsperaFilaTobogas * tobogas.numeroPessoasNaFila;
                        pthread_mutex_unlock(&mutexSimulacao);

                        enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, ENTRAR_FILA, TOBOGAS);
                        printf(CIANO "A pessoa com ID %d chegou à fila para entrar na atração de tobogãs | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);

                        sleep(tempoDeEspera);

                        if (tempoSimulado > conf.tempoSimulacao){
                            sem_post(&tobogas.fila);

                            pthread_mutex_lock(&mutexFilas);
                            tobogas.numeroPessoasNaFila--;
                            pthread_mutex_unlock(&mutexFilas);

                            enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, SAIR_FILA, TOBOGAS);
                            printf(VERMELHO "A pessoa com ID %d teve que sair da fila dos tobogãs porque o parque está a fechar | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);

                            pessoa->zonaAtual = PRACA;

                        } else{
                        
                            if (tobogas.numeroAtualPessoas >= conf.numeroMaximoTobogas) {

                                sem_post(&tobogas.fila);
                                sem_wait(&semaforoParque);

                                pthread_mutex_lock(&mutexFilas);
                                tobogas.numeroPessoasNaFila--;
                                pthread_mutex_unlock(&mutexFilas);

                                enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, SAIR_FILA, TOBOGAS);
                                printf(VERMELHO "A pessoa com ID %d desistiu de entrar na atração de tobogãs porque não queria esperar na fila | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);
                                
                                pessoa->zonaAtual = PRACA;

                            } else {

                                sem_post(&tobogas.fila);
                                sem_wait(&semaforoTobogas);

                                pthread_mutex_lock(&mutexFilas);
                                tobogas.numeroPessoasNaFila--;
                                tobogas.numeroAtualPessoas++;
                                pthread_mutex_unlock(&mutexFilas);
                                
                                enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, SAIR_FILA_ENTRAR, TOBOGAS);
                                printf(CIANO_CLARO "A pessoa com ID %d entrou na atração de tobogãs depois de esperar na fila | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);
                                
                                sleep(conf.tempoTobogas);

                                if((tempoSimulado > conf.tempoSimulacao)) {

                                    sem_post(&semaforoTobogas);

                                    pthread_mutex_lock(&mutexFilas);
                                    tobogas.numeroAtualPessoas--;
                                    pthread_mutex_unlock(&mutexFilas);

                                    enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, SAIR, TOBOGAS);
                                    printf(VERMELHO "A pessoa com ID %d teve que sair da atração de tobogãs porque o parque está a fechar | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);

                                    pessoa->zonaAtual = PRACA;

                                }else if(calcularProbabilidade(conf.probabilidadeMagoar)){
                                    
                                    sem_post(&semaforoTobogas);
                                    sem_wait(&semaforoParque);

                                    pthread_mutex_lock(&mutexZonas);
                                    tobogas.numeroAtualPessoas--;
                                    pthread_mutex_unlock(&mutexZonas);

                                    enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, SAIR, TOBOGAS);
                                    printf(ROXO "A pessoa com ID %d magoou-se e tem de ir para a enfermaria | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);
                                    printf(VERMELHO "A pessoa com ID %d saiu da atração de tobogãs | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);
                                    
                                    pessoa->magoar = TRUE;
                                    pessoa->zonaAtual = ENFERMARIA;
                                }
                            }
                        }
                    } else {
                        printf(VERMELHO "A pessoa com ID %d desistiu de entrar na atração de tobogãs porque não havia lugar na fila | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);
                        
                        sem_wait(&semaforoParque);
                        
                        pessoa->zonaAtual = PRACA;
                    }
                }
            }else{
                printf(CINZA "A pessoa com ID %d não tem altura e idade mínima requirida para entrar nos tobogãs | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);
                
                sem_wait(&semaforoParque);
                
                pessoa->zonaAtual = PRACA;
            }
        }
        
    } else if (pessoa->zonaAtual == RESTAURACAO) {

        if (tempoSimulado > conf.tempoSimulacao) {

            pessoa->zonaAtual = PRACA;

        } else {

            if(restauracao.numeroAtualPessoas < conf.numeroMaximoRestauracao){
                
                sem_wait(&semaforoRestauracao);

                pthread_mutex_lock(&mutexFilas);
                restauracao.numeroAtualPessoas++;
                pthread_mutex_unlock(&mutexFilas);

                enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, ENTRAR, RESTAURACAO);
                printf(CIANO_CLARO "A pessoa com ID %d entrou na restauração | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);

                sleep(conf.tempoRestauracao);
                
            } else {

                if (restauracao.numeroPessoasNaFila < conf.tamanhoFilaRestauracao) {

                    sem_wait(&restauracao.fila);

                    pthread_mutex_lock(&mutexFilas);
                    restauracao.numeroPessoasNaFila++;
                    pthread_mutex_unlock(&mutexFilas);

                    pthread_mutex_lock(&mutexSimulacao);
                    tempoDeEspera = conf.tempoEsperaFilaRestauracao * restauracao.numeroPessoasNaFila;
                    pthread_mutex_unlock(&mutexSimulacao);

                    enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, ENTRAR_FILA, RESTAURACAO);
                    printf(CIANO "A pessoa com ID %d chegou à fila para entrar na restauração | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);

                    sleep(tempoDeEspera);

                    if (tempoSimulado > conf.tempoSimulacao){
                        sem_post(&restauracao.fila);

                        pthread_mutex_lock(&mutexFilas);
                        restauracao.numeroPessoasNaFila--;
                        pthread_mutex_unlock(&mutexFilas);

                        enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, SAIR_FILA, RESTAURACAO);
                        printf(VERMELHO "A pessoa com ID %d teve que sair da fila da restauração porque o parque está a fechar | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);

                        pessoa->zonaAtual = PRACA;

                    } else{
                    
                        if (restauracao.numeroAtualPessoas >= conf.numeroMaximoRestauracao) {

                            sem_post(&restauracao.fila);
                            sem_wait(&semaforoParque);

                            pthread_mutex_lock(&mutexFilas);
                            restauracao.numeroPessoasNaFila--;
                            pthread_mutex_unlock(&mutexFilas);

                            enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, SAIR_FILA, RESTAURACAO);
                            printf(VERMELHO "A pessoa com ID %d desistiu de entrar na restauração porque não queria esperar na fila | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);

                            pessoa->zonaAtual = PRACA;

                        } else {

                            sem_post(&restauracao.fila);
                            sem_wait(&semaforoRestauracao);

                            pthread_mutex_lock(&mutexFilas);
                            restauracao.numeroPessoasNaFila--;
                            restauracao.numeroAtualPessoas++;
                            pthread_mutex_unlock(&mutexFilas);

                            enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, SAIR_FILA_ENTRAR, RESTAURACAO);
                            printf(CIANO_CLARO "A pessoa com ID %d entrou na restauração depois de esperar na fila | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);

                            sleep(conf.tempoRestauracao);

                        }
                    }
                } else {
                    printf(VERMELHO "A pessoa com ID %d desistiu de entrar na restauração porque não havia lugar na fila | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);

                    sem_wait(&semaforoParque);

                    pessoa->zonaAtual = PRACA;
                }
            }
        }
    }
}

// Função para enviar dados (buffer) para o socket
void enviarDados(int acabou, int personId, int tempo, int acao, int zona) {
    //sem_wait(&semaforoDados);
    pthread_mutex_lock(&mutexDados);

    // tempo = tempo/conf.tempoChegadaPessoas;

    char buffer[TAMANHO_BUFFER];
    snprintf(buffer, TAMANHO_BUFFER, "%d %d %d %d %d|", acabou, personId, tempo, acao, zona);

    if (send(socketFD, buffer, strlen(buffer), 0) == -1) {
        perror("Erro ao enviar dados"); // Exibe uma mensagem de erro se não conseguir enviar os dados
    }

    pthread_mutex_unlock(&mutexDados);
    //sem_post(&semaforoDados);
}

void enviarPessoa(void *ptr) {

    struct pessoa person = criarPessoa();
    pessoas[person.idPessoa] = &person; // Adiciona a pessoa criada ao array de todas as pessoas criadas 
    
    int time = tempoSimulado;

    while(TRUE){
        if (!person.dentroParque) { //Se a pessoa entra no parque
            Fila(&person);
        } else if (person.dentroParque && person.zonaAtual != PRACA) { //Se a pessoa vai da praça para uma atração ou quer ficar na mesma atração
            Fila(&person);
        }
        sleep(1);
        if(person.desistir == FALSE){
            if(person.zonaAtual == PRACA) {

                if(tempoSimulado>conf.tempoSimulacao){

                    sem_post(&semaforoParque);

                    pthread_mutex_lock(&mutexParque);
                    pessoasParque--;
                    pthread_mutex_unlock(&mutexParque);

                    printf(VERMELHO_CLARO "A pessoa com ID %d saiu do parque porque está a fechar | Tempo: %d\n" RESET, person.idPessoa, tempoSimulado);
                    enviarDados(NAO_ACABOU, person.idPessoa, tempoSimulado, SAIR, PRACA);

                    break;

                }

                if (calculaProbabilidadeDesistir(conf.probabilidadeDesistir, &person)) {

                    int proximaAtracao = visitarProximaAtracao(&person);
                    person.zonaAtual = proximaAtracao;

                    if (proximaAtracao == BALNEARIOS) {

                        sem_post(&semaforoParque);
                        // printf( "A pessoa com ID %d está na Praça vai para os balneários \n", person.idPessoa);

                    } else if (proximaAtracao == NATACAO) {

                        sem_post(&semaforoParque);
                        // printf( "A pessoa com ID %d está na Praça vai para a atração de natação \n", person.idPessoa);

                    } else if (proximaAtracao == MERGULHO) {

                        sem_post(&semaforoParque);
                        // printf( "A pessoa com ID %d está na Praça vai para a atração de mergulho \n", person.idPessoa);
                        
                    } else if (proximaAtracao == TOBOGAS) {

                        sem_post(&semaforoParque);
                        // printf( "A pessoa com ID %d está na Praça vai para a atração dos tobogãs \n", person.idPessoa);                     
                        
                    } else if (proximaAtracao == RESTAURACAO) {

                        sem_post(&semaforoParque);
                        // printf( "A pessoa com ID %d está na Praça vai para a restauração \n", person.idPessoa);
                        
                    }
                } else {

                    sem_post(&semaforoParque);

                    pthread_mutex_lock(&mutexParque);
                    pessoasParque--;
                    pthread_mutex_unlock(&mutexParque);

                    printf(VERMELHO_CLARO "A pessoa com ID %d foi para a Praça e saiu do parque | Tempo: %d\n" RESET, person.idPessoa, tempoSimulado);
                    enviarDados(NAO_ACABOU, person.idPessoa, tempoSimulado, SAIR, PRACA);
                    
                    break;
                }
                
            } else if (person.zonaAtual == ENFERMARIA) {
                
                if(tempoSimulado>conf.tempoSimulacao){

                    sem_post(&semaforoParque);

                    pthread_mutex_lock(&mutexParque);
                    pessoasParque--;
                    pthread_mutex_unlock(&mutexParque);

                    printf(VERMELHO_CLARO "A pessoa com ID %d saiu do parque porque está a fechar e foi para o hospital | Tempo: %d\n" RESET, person.idPessoa, tempoSimulado);
                    enviarDados(NAO_ACABOU, person.idPessoa, tempoSimulado, SAIR, PRACA);

                    break;

                }
                
                if(enfermaria.numeroAtualPessoas < conf.numeroMaximoEnfermaria){

                    sem_post(&semaforoParque);
                    sem_wait(&semaforoEnfermaria);

                    pthread_mutex_lock(&mutexFilas);
                    enfermaria.numeroAtualPessoas++;
                    pthread_mutex_unlock(&mutexFilas);

                    printf(ROXO "A pessoa com ID %d entrou na enfermaria | Tempo: %d\n" RESET, person.idPessoa, tempoSimulado);
                    enviarDados(NAO_ACABOU, person.idPessoa, tempoSimulado, ENTRAR, ENFERMARIA);

                    sleep(conf.tempoEnfermaria);
                    
                    if(calcularProbabilidade(conf.probabilidadeCurar)){ 
                        
                        person.magoar = FALSE;

                        sem_post(&semaforoEnfermaria);
                        sem_wait(&semaforoParque);

                        pthread_mutex_lock(&mutexZonas);
                        enfermaria.numeroAtualPessoas--;
                        pthread_mutex_unlock(&mutexZonas);

                        printf(ROXO_CLARO "A pessoa com ID %d saiu da enfermaria curada | Tempo: %d\n" RESET, person.idPessoa, tempoSimulado);
                        enviarDados(NAO_ACABOU, person.idPessoa, tempoSimulado, SAIR, ENFERMARIA);

                        person.zonaAtual = PRACA;

                    }else{ //sai do parque

                        sem_post(&semaforoEnfermaria);
                        
                        pthread_mutex_lock(&mutexZonas);
                        enfermaria.numeroAtualPessoas--;
                        pthread_mutex_unlock(&mutexZonas);

                        pthread_mutex_lock(&mutexParque);
                        pessoasParque--;
                        pthread_mutex_unlock(&mutexParque);

                        printf(VERMELHO_CLARO "A pessoa com ID %d entrou na enfermaria mas não foi curada e foi para o hospital | Tempo: %d\n" RESET, person.idPessoa, tempoSimulado);
                        enviarDados(NAO_ACABOU, person.idPessoa, tempoSimulado, SAIR_SAIR, ENFERMARIA);

                        break;
                    }

                } else {

                    if (enfermaria.numeroPessoasNaFila < conf.tamanhoFilaEnfermaria) {

                        sem_post(&semaforoParque);
                        sem_wait(&enfermaria.fila);

                        pthread_mutex_lock(&mutexFilas);
                        enfermaria.numeroPessoasNaFila++;
                        pthread_mutex_unlock(&mutexFilas);

                        pthread_mutex_lock(&mutexSimulacao);
                        int tempoDeEspera = conf.tempoEsperaFilaEnfermaria * enfermaria.numeroPessoasNaFila;
                        pthread_mutex_unlock(&mutexSimulacao);

                        printf(CIANO "A pessoa com ID %d chegou à fila para entrar na enfermaria | Tempo: %d\n" RESET, person.idPessoa, tempoSimulado);
                        enviarDados(NAO_ACABOU, person.idPessoa, tempoSimulado, ENTRAR_FILA, ENFERMARIA);

                        sleep(tempoDeEspera);
                        
                        if (enfermaria.numeroAtualPessoas >= conf.numeroMaximoEnfermaria) {

                            sem_post(&enfermaria.fila);

                            pthread_mutex_lock(&mutexFilas);
                            enfermaria.numeroPessoasNaFila--;
                            pthread_mutex_unlock(&mutexFilas);

                            pthread_mutex_lock(&mutexParque);
                            pessoasParque--;
                            pthread_mutex_unlock(&mutexParque);
                            
                            printf(VERMELHO_CLARO "A pessoa com ID %d desistiu de entrar na enfermaria porque não queria esperar na fila e foi para o hospital | Tempo: %d\n" RESET, person.idPessoa, tempoSimulado);
                            enviarDados(NAO_ACABOU, person.idPessoa, tempoSimulado, SAIR_FILA_ENFERMARIA, ENFERMARIA);

                            break;

                        } else {

                            sem_post(&enfermaria.fila);
                            sem_wait(&semaforoEnfermaria);

                            pthread_mutex_lock(&mutexFilas);
                            enfermaria.numeroPessoasNaFila--;
                            enfermaria.numeroAtualPessoas++;
                            pthread_mutex_unlock(&mutexFilas);

                            printf(ROXO "A pessoa com ID %d entrou na enfermaria depois de esperar na fila | Tempo: %d\n" RESET, person.idPessoa, tempoSimulado);
                            enviarDados(NAO_ACABOU, person.idPessoa, tempoSimulado, SAIR_FILA_ENTRAR, ENFERMARIA);

                            sleep(conf.tempoEnfermaria);

                            if(calcularProbabilidade(conf.probabilidadeCurar)){ 
                                person.magoar = FALSE;

                                sem_post(&semaforoEnfermaria);
                                sem_wait(&semaforoParque);

                                pthread_mutex_lock(&mutexZonas);
                                enfermaria.numeroAtualPessoas--;
                                pthread_mutex_unlock(&mutexZonas);

                                printf(ROXO_CLARO "A pessoa com ID %d saiu da enfermaria curada | Tempo: %d\n" RESET, person.idPessoa, tempoSimulado);
                                enviarDados(NAO_ACABOU, person.idPessoa, tempoSimulado, SAIR, ENFERMARIA);

                                person.zonaAtual = PRACA;

                            }else{ //sai do parque

                                sem_post(&semaforoEnfermaria);

                                pthread_mutex_lock(&mutexZonas);
                                enfermaria.numeroAtualPessoas--;
                                pthread_mutex_unlock(&mutexZonas);

                                pthread_mutex_lock(&mutexParque);
                                pessoasParque--;
                                pthread_mutex_unlock(&mutexParque);

                                printf(VERMELHO_CLARO "A pessoa com ID %d entrou na enfermaria mas não foi curada e foi para o hospital | Tempo: %d\n" RESET, person.idPessoa, tempoSimulado);
                                enviarDados(NAO_ACABOU, person.idPessoa, tempoSimulado, SAIR_SAIR, ENFERMARIA);
                                break;
                            }

                        }

                    } else {
                        sem_post(&semaforoParque);

                        pthread_mutex_lock(&mutexParque);
                        pessoasParque--;
                        pthread_mutex_unlock(&mutexParque);

                        printf(VERMELHO_CLARO "A pessoa com ID %d desistiu de entrar na enfermaria porque não havia lugar na fila e foi para o hospital | Tempo: %d\n" RESET, person.idPessoa, tempoSimulado);
                        enviarDados(NAO_ACABOU, person.idPessoa, tempoSimulado, SAIR, PRACA);

                        break;
                    }
                }
                
                

            } else { //Está em qualquer outra zona
                if(tempoSimulado > conf.tempoSimulacao) {

                        if (person.zonaAtual == BALNEARIOS) {
                            sem_post(&semaforoBalneario);

                            pthread_mutex_lock(&mutexZonas);
                            balnearios.numeroAtualPessoas--;
                            pthread_mutex_unlock(&mutexZonas);

                        } else if (person.zonaAtual == NATACAO) {

                            sem_post(&semaforoNatacao);

                            pthread_mutex_lock(&mutexZonas);
                            natacao.numeroAtualPessoas--;
                            pthread_mutex_unlock(&mutexZonas);
                        } else if (person.zonaAtual == MERGULHO) {

                            sem_post(&semaforoMergulho);

                            pthread_mutex_lock(&mutexZonas);
                            mergulho.numeroAtualPessoas--;
                            pthread_mutex_unlock(&mutexZonas);
                        } else if (person.zonaAtual == TOBOGAS) {

                            sem_post(&semaforoTobogas);

                            pthread_mutex_lock(&mutexZonas);
                            tobogas.numeroAtualPessoas--;
                            pthread_mutex_unlock(&mutexZonas);
                        } else if (person.zonaAtual == RESTAURACAO) {

                            sem_post(&semaforoRestauracao);

                            pthread_mutex_lock(&mutexZonas);
                            restauracao.numeroAtualPessoas--;
                            pthread_mutex_unlock(&mutexZonas);
                        }

                        pthread_mutex_lock(&mutexParque);
                        pessoasParque--;
                        pthread_mutex_unlock(&mutexParque);
                        
                        printf(VERMELHO_CLARO "A pessoa com ID %d saiu do parque porque está a fechar | Tempo: %d\n" RESET, person.idPessoa, tempoSimulado);
                        enviarDados(NAO_ACABOU, person.idPessoa, tempoSimulado, SAIR_SAIR, person.zonaAtual);
                        
                        break;

                    }
                if (calculaProbabilidadeMudar(conf.probabilidadeMudarZona, &person)) { //Se quer mudar de zona, probabilidade aumenta se já andou por todas ou não
                    //printf(AZUL "A pessoa com o ID %d quer mudar para outra zona e vai para a Praça\n" RESET, person.idPessoa);
                    
                    if (person.zonaAtual == BALNEARIOS) {

                        sem_post(&semaforoBalneario);
                        sem_wait(&semaforoParque);

                        pthread_mutex_lock(&mutexZonas);
                        balnearios.numeroAtualPessoas--;
                        pthread_mutex_unlock(&mutexZonas);

                        printf(VERMELHO "A pessoa com ID %d saiu dos balneários | Tempo: %d\n" RESET, person.idPessoa, tempoSimulado);
                        enviarDados(NAO_ACABOU, person.idPessoa, tempoSimulado, SAIR, BALNEARIOS);

                        sleep(1);

                    } else if (person.zonaAtual == NATACAO) {

                        sem_post(&semaforoNatacao);
                        sem_wait(&semaforoParque);

                        pthread_mutex_lock(&mutexZonas);
                        natacao.numeroAtualPessoas--;
                        pthread_mutex_unlock(&mutexZonas);

                        printf(VERMELHO "A pessoa com ID %d saiu da atração de natação | Tempo: %d\n" RESET, person.idPessoa, tempoSimulado);
                        enviarDados(NAO_ACABOU, person.idPessoa, tempoSimulado, SAIR, NATACAO);

                        sleep(1);

                    } else if (person.zonaAtual == MERGULHO) {

                        sem_post(&semaforoMergulho);
                        sem_wait(&semaforoParque);

                        pthread_mutex_lock(&mutexZonas);
                        mergulho.numeroAtualPessoas--;
                        pthread_mutex_unlock(&mutexZonas);

                        printf(VERMELHO "A pessoa com ID %d saiu da atração de mergulho | Tempo: %d\n" RESET, person.idPessoa, tempoSimulado);
                        enviarDados(NAO_ACABOU, person.idPessoa, tempoSimulado, SAIR, MERGULHO);

                        sleep(1);
                        
                    } else if (person.zonaAtual == TOBOGAS) {

                        sem_post(&semaforoTobogas);
                        sem_wait(&semaforoParque);

                        pthread_mutex_lock(&mutexZonas);
                        tobogas.numeroAtualPessoas--;
                        pthread_mutex_unlock(&mutexZonas);

                        printf(VERMELHO "A pessoa com ID %d saiu da atração de tobogãs | Tempo: %d\n" RESET, person.idPessoa, tempoSimulado);
                        enviarDados(NAO_ACABOU, person.idPessoa, tempoSimulado, SAIR, TOBOGAS);
                        sleep(1);
                        
                    } else if (person.zonaAtual == RESTAURACAO) {

                        sem_post(&semaforoRestauracao);
                        sem_wait(&semaforoParque);

                        pthread_mutex_lock(&mutexZonas);
                        restauracao.numeroAtualPessoas--;
                        pthread_mutex_unlock(&mutexZonas);

                        printf(VERMELHO "A pessoa com ID %d saiu da restauração | Tempo: %d\n" RESET, person.idPessoa, tempoSimulado);
                        enviarDados(NAO_ACABOU, person.idPessoa, tempoSimulado, SAIR, RESTAURACAO);

                        sleep(1);
                        
                    }

                    person.zonaAtual = PRACA;

                } else { //Se não quer mudar de zona

                    printf(AZUL "A pessoa com o ID %d quer continuar na zona | Tempo: %d\n" RESET, person.idPessoa, tempoSimulado);
                    //sleep(1);
                    //probabilidade de ele se magoar, vai para a enfermaria

                    //não muda de zona e vai para a fila da zona (abrir semaforo da zona)
                    if (person.zonaAtual == BALNEARIOS) {

                        sem_post(&semaforoBalneario);

                        pthread_mutex_lock(&mutexZonas);
                        balnearios.numeroAtualPessoas--;
                        pthread_mutex_unlock(&mutexZonas);

                        // printf(VERMELHO "A pessoa com ID %d saiu dos balneários\n" RESET, person.idPessoa);

                        enviarDados(NAO_ACABOU, person.idPessoa, tempoSimulado, SAIR, BALNEARIOS);

                    } else if (person.zonaAtual == NATACAO) {

                        sem_post(&semaforoNatacao);

                        pthread_mutex_lock(&mutexZonas);
                        natacao.numeroAtualPessoas--;
                        pthread_mutex_unlock(&mutexZonas);
                        
                        // printf(VERMELHO "A pessoa com ID %d saiu da atração de natação\n" RESET, person.idPessoa);

                        enviarDados(NAO_ACABOU, person.idPessoa, tempoSimulado, SAIR, NATACAO);

                    } else if (person.zonaAtual == MERGULHO) {

                        sem_post(&semaforoMergulho);

                        pthread_mutex_lock(&mutexZonas);
                        mergulho.numeroAtualPessoas--;
                        pthread_mutex_unlock(&mutexZonas);

                        // printf(VERMELHO "A pessoa com ID %d saiu da atração de mergulho\n" RESET, person.idPessoa);

                        enviarDados(NAO_ACABOU, person.idPessoa, tempoSimulado, SAIR, MERGULHO);
                        
                    } else if (person.zonaAtual == TOBOGAS) {

                        sem_post(&semaforoTobogas);

                        pthread_mutex_lock(&mutexZonas);
                        tobogas.numeroAtualPessoas--;
                        pthread_mutex_unlock(&mutexZonas);

                        // printf(VERMELHO "A pessoa com ID %d saiu da atração de tobogãs\n" RESET, person.idPessoa);

                        enviarDados(NAO_ACABOU, person.idPessoa, tempoSimulado, SAIR, TOBOGAS);
                        
                    } else if (person.zonaAtual == RESTAURACAO) {

                        sem_post(&semaforoRestauracao);

                        pthread_mutex_lock(&mutexZonas);
                        restauracao.numeroAtualPessoas--;
                        pthread_mutex_unlock(&mutexZonas);

                        // printf(VERMELHO "A pessoa com ID %d saiu da restauração\n" RESET, person.idPessoa);

                        enviarDados(NAO_ACABOU, person.idPessoa, tempoSimulado, SAIR, RESTAURACAO);
                        
                    }
                }
            }

        } else {
            break;
        }
    }
}

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

	// Inicia mutex para simulação
    if (pthread_mutex_init(&mutexSimulacao, NULL) != 0) {
        perror("Erro na inicialização do mutexSimulacao");
        exit(1);
    }

    // Inicia mutex para mudar de zona
    if (pthread_mutex_init(&mutexZonas, NULL) != 0) {
        perror("Erro na inicialização do mutexZonas");
        exit(1);
    }

    // Inicia mutex para sair do parque
    if (pthread_mutex_init(&mutexParque, NULL) != 0) {
        perror("Erro na inicialização do mutexParque");
        exit(1);
    }

    // Inicia mutex para incrementar tempo
    if (pthread_mutex_init(&mutexTempo, NULL) != 0) {
        perror("Erro na inicialização do mutexTempo");
        exit(1);
    }

    if (pthread_mutex_init(&mutexDados, NULL) != 0) {
        perror("Erro na inicialização do mutexDados");
        exit(1);
    }

    sem_init(&semaforoCriar, 0, 1);

    sem_init(&semaforoDados, 0, 1);

    sem_init(&praca.fila, 0, conf.tamanhoFilaParque);
    sem_init(&semaforoParque, 0, conf.quantidadePessoasParque);

    sem_init(&balnearios.fila, 0, conf.tamanhoFilaBalnearios);
    sem_init(&semaforoBalneario, 0, conf.numeroMaximoBalnearios);

    sem_init(&natacao.fila, 0, conf.tamanhoFilaNatacao);
    sem_init(&semaforoNatacao, 0, conf.numeroMaximoNatacao);

    sem_init(&mergulho.fila, 0, conf.tamanhoFilaMergulho);
    sem_init(&semaforoMergulho, 0, conf.numeroMaximoMergulho);

    sem_init(&tobogas.fila, 0, conf.tamanhoFilaTobogas);
    sem_init(&semaforoTobogas, 0, conf.numeroMaximoTobogas);

    sem_init(&restauracao.fila, 0, conf.tamanhoFilaRestauracao);
    sem_init(&semaforoRestauracao, 0, conf.numeroMaximoRestauracao);

    sem_init(&enfermaria.fila, 0, conf.tamanhoFilaEnfermaria);
    sem_init(&semaforoEnfermaria, 0, conf.numeroMaximoEnfermaria);
}

// Função principal do simulador
void simulador(char* config) {
    configuracao(config); // Lê as configurações do arquivo e inicializa as variáveis
    exclusaoMutua(); // Inicializa os mutexes

    printf("Vai começar uma simulaçao de um parque com capacidade para %d pessoas.\n" RESET, conf.quantidadePessoasParque);

    while (TRUE) { // Loop infinito para continuar a simulação

        pthread_mutex_lock(&mutexTempo);
        // tempoSimulado++;
        time_t tempoAtual = time(NULL);

        time_t diferencaTempo = tempoAtual - tempoInicial;

        tempoSimulado = diferencaTempo;

        pthread_mutex_unlock(&mutexTempo); 

              
        if ((tempoSimulado % conf.tempoChegadaPessoas == 0) && (tempoSimulado < conf.tempoSimulacao)) {
            if (praca.numeroPessoasNaFila <= conf.tamanhoFilaParque) {
                // Cria uma nova thread para representar uma pessoa
                pthread_mutex_lock(&mutexSimulacao);
                if (pthread_create(&idThread[idPessoa], NULL, enviarPessoa, NULL) != 0) {
                    perror("Erro na criação da thread");
                    exit(1);
                }
                pthread_mutex_unlock(&mutexSimulacao);
                sleep(1);
            }
        }

        usleep(10000);

        if ((pessoasParque < 1) && (tempoSimulado >= conf.tempoSimulacao)) { // Verifica se o tempo simulado atingiu ou ultrapassou o tempo de simulação
            break; // Sai do loop quando a simulação terminar
        }
    }

    printf("Acabou a simulação\n");
    printf("Pessoas no parque: %d\n", pessoasParque);
    printf("Pessoas no natacao: %d\n", natacao.numeroAtualPessoas);
    printf("Pessoas no mergulho: %d\n", mergulho.numeroAtualPessoas);
    printf("Pessoas no tobogas: %d\n", tobogas.numeroAtualPessoas);
    printf("Pessoas no enfermaria: %d\n", enfermaria.numeroAtualPessoas);
    printf("Pessoas no restauracao: %d\n", restauracao.numeroAtualPessoas);
    printf("Pessoas no balnearios: %d\n", balnearios.numeroAtualPessoas);
    // Aqui envio que acabou a simulação e não envio nenhuma pessoa
    enviarDados(ACABOU, 0, tempoSimulado, 0, 0);
   

}

// Função principal do programa
int main(int argc, char *argv[]) {

    srand(time(NULL));
    tempoInicial = time(NULL); 

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
