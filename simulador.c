#include "header.h"

//Variáveis globais
int socketFD = 0;
int idPessoa = 1; // Id para a pessoa criada, que começa a 1 e vai incrementando
int tempoSimulado = 0; // Tempo de simulação que já foi simulado
int pessoasParque = 0; // Número de pessoas dentro do parque

//Structs
struct configuracao conf;
struct praca praca;
struct natacao natacao;
struct mergulho mergulho;
struct tobogas tobogas;
struct enfermaria enfermaria;
struct restauracao restauracao;
struct balnearios balnearios;

//Tricos e Semáforos
pthread_mutex_t mutexPessoa = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexPessoaEnviar = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexDados = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexSimulacao = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexFilas = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexZonas = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexParque = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexTempo = PTHREAD_MUTEX_INITIALIZER;
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

// Função que calcula a probabilidade de uma pessoa mudar de zona dependendo a quantas zonas do parque já visitou
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

    return random_value < probabilidade;
}

// Função que calcula a probabilidade de uma pessoa desistir/sair do parque dependendo de quantas zonas do parque já visitou e devolve se desiste ou não
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

    return random_value > probabilidade;
}

// Função que devolve se sim ou não dependendo da probabilidade
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

    return person;// Retorna o descritor do socket conectado
}

// Visitando as atrações
int visitarProximaAtracao(struct pessoa *pessoa) {

    // Verifica se o número total de atrações visitadas é menor que o número total de atrações no parque
    if (pessoa->totalVisitadas < conf.numeroAtracoes) {

        int indice;

        do {

            // Gera um índice aleatório dentro do número total de atrações permitidas
            indice = rand() % conf.numeroAtracoes;

        } while (pessoa->visitas[indice] == 1); // Verifica se a atração já foi visitada

        pessoa->visitas[indice] = 1; // Marca a atração como visitada
        pessoa->totalVisitadas++; // Incrementa o número total de atrações visitadas
        return indice + NATACAO; // Retorna o número da atração visitada

    } else {

        // Se todas as atrações foram visitadas, gera um índice aleatório para visitar
        int indice = rand() % conf.numeroAtracoes;
        return indice + NATACAO; // Retorna o número da atração visitada

    }
}


// Função que serve para gerir as pessoas que entram e saiem de zonas dentro do parque e do parque
void Fila (struct pessoa *pessoa) {

    int tempoDeEspera;

    sleep(1);

    // Verifica se a pessoa está fora do parque
    if (pessoa->zonaAtual == FORADOPARQUE) {

        // Verifica se o parque tem capacidade para mais pessoas
        if (pessoasParque < conf.quantidadePessoasParque) {

            sem_wait(&semaforoParque);

            // Incrementa o número de pessoas no parque
            pthread_mutex_lock(&mutexFilas);
            pessoasParque++;
            pthread_mutex_unlock(&mutexFilas);

            // Atualiza o estado da pessoa para dentro do parque e a sua zona para Praça
            pessoa->zonaAtual = PRACA;
            pessoa->dentroParque = TRUE;

            // Envia dados sobre a entrada da pessoa para o monitor
            enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, ENTRAR, PRACA);
            // Imprime mensagem indicando a entrada da pessoa no parque
            printf(VERDE_CLARO "A pessoa com ID %d entrou no parque | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);
            
            // Aguarda um tempo específico para entrada na praça
            sleep(conf.tempoEntrarPraca);

        // Se o parque estiver cheio
        } else {

            // Verifica se há espaço na fila da praça
            if (praca.numeroPessoasNaFila < conf.tamanhoFilaParque) {

                sem_wait(&praca.fila);

                // Incrementa o número de pessoas na fila da praça
                pthread_mutex_lock(&mutexFilas);
                praca.numeroPessoasNaFila++;
                pthread_mutex_unlock(&mutexFilas);

                // Calcula o tempo de espera baseado na posição na fila
                tempoDeEspera = conf.tempoEsperaFilaParque * praca.numeroPessoasNaFila;

                // Envia dados sobre a entrada na fila para o monitor
                enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, ENTRAR_FILA, PRACA);
                // Imprime mensagem indicando a chegada à fila do parque
                printf(AMARELO_CLARO "A pessoa com ID %d chegou à fila para entrar no parque | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);

                // Aguarda o tempo de espera na fila
                sleep(tempoDeEspera);
                
                // Se for possível entrar no parque após a espera
                if (pessoasParque < conf.quantidadePessoasParque) {

                    // Verifica se o tempo de simulação acabou enquanto estáva na fila
                    if (tempoSimulado > conf.tempoSimulacao) {
                        
                        sem_post(&praca.fila);

                        // Decrementa o número de pessoas na fila do parque após a desistência
                        pthread_mutex_lock(&mutexFilas);
                        praca.numeroPessoasNaFila--;
                        pthread_mutex_unlock(&mutexFilas);

                        // Envia dados sobre a saída da fila para o monitor
                        enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, SAIR_FILA, PRACA);
                        // Imprime mensagem indicando a desistência
                        printf(VERMELHO_CLARO "A pessoa com ID %d desistiu de entrar no parque porque este estava a fechar | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);

                        // Atualiza o status da pessoa para desistir
                        pessoa->desistir = TRUE;

                    // Se o tempo de simulação ainda não acabou
                    } else {

                        sem_post(&praca.fila);
                        sem_wait(&semaforoParque);

                        // Decrementa o número de pessoas na fila do parque e incrementa o número de pessoas no parque
                        pthread_mutex_lock(&mutexFilas);
                        praca.numeroPessoasNaFila--;
                        pessoasParque++;
                        pthread_mutex_unlock(&mutexFilas);

                        // Atualiza o estado da pessoa para dentro do parque e a sua zona para Praça
                        pessoa->zonaAtual = PRACA;
                        pessoa->dentroParque = TRUE;

                        // Envia dados sobre a entrada no parque após a espera na fila para o monitor
                        enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, SAIR_FILA_ENTRAR, PRACA);
                        // Imprime mensagem indicando a entrada no parque após a espera
                        printf(VERDE_CLARO "A pessoa com ID %d entrou no parque depois de esperar %d segundos | Tempo: %d\n" RESET, pessoa->idPessoa, tempoDeEspera, tempoSimulado);

                        // Aguarda um tempo específico para entrada na praça
                        sleep(conf.tempoEntrarPraca);
                    }

                // Se não for possível entrar no parque após espera
                } else {

                    sem_post(&praca.fila);

                    // Decrementa o número de pessoas na fila do parque após a desistência
                    pthread_mutex_lock(&mutexFilas);
                    praca.numeroPessoasNaFila--;
                    pthread_mutex_unlock(&mutexFilas);

                    // Envia dados sobre a saída da fila para o monitor
                    enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, SAIR_FILA, PRACA);
                    // Imprime mensagem indicando a desistência
                    printf(VERMELHO_CLARO "A pessoa com ID %d desistiu de entrar no parque porque não queria esperar na fila | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);

                    // Atualiza o status da pessoa para desistir
                    pessoa->desistir = TRUE;
                }
            
            // Se não houver espaço na fila
            } else {
                // Imprime mensagem indicando a desistência por falta de espaço na fila
                printf(VERMELHO "A pessoa com ID %d desistiu de entrar no parque porque não havia lugar na fila | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);

                // Atualiza o status da pessoa para desistir
                pessoa->desistir = TRUE;
            }
        }

    // Verifica se a pessoa está na zona dos balneários
    } else if (pessoa->zonaAtual == BALNEARIOS) {

        // Verifica se o tempo de simulação acabou
        if (tempoSimulado > conf.tempoSimulacao) {

            // Se o tempo de simulação acabou, direciona a pessoa de volta para a praça
            pessoa->zonaAtual = PRACA;

        // Se o tempo de simulação ainda não acabou
        } else {

            // Verifica se há espaço nos balneários para mais pessoas
            if (balnearios.numeroAtualPessoas < conf.numeroMaximoBalnearios) {
                
                sem_wait(&semaforoBalneario);
                
                // Incrementa o número de pessoas nos balneários
                pthread_mutex_lock(&mutexFilas);
                balnearios.numeroAtualPessoas++;
                pthread_mutex_unlock(&mutexFilas);
                
                // Envia dados sobre a entrada nos balneários para o monitor
                enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, ENTRAR, BALNEARIOS);
                // Imprime mensagem indicando a entrada nos balneários
                if (pessoa->genero == MULHER) {
                    printf(CIANO_CLARO "A pessoa com ID %d entrou nos balneários femininos| Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);
                } else {
                    printf(CIANO_CLARO "A pessoa com ID %d entrou nos balneários masculinos| Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);
                }

                // Aguarda um tempo específico nos balneários
                sleep(conf.tempoBalnearios);

                // Se o tempo de simulação acabou durante a permanência na zona
                if (tempoSimulado > conf.tempoSimulacao) {

                    sem_post(&semaforoBalneario);

                    // Decrementa o número de pessoas nos balneários
                    pthread_mutex_lock(&mutexFilas);
                    balnearios.numeroAtualPessoas--;
                    pthread_mutex_unlock(&mutexFilas);

                    // Envia dados sobre a saída dos balneários para o monitor
                    enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, SAIR, BALNEARIOS);
                    // Imprime mensagem indicando a saída dos balneários
                    printf(VERMELHO "A pessoa com ID %d teve que sair dos balneários porque o parque está a fechar | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);

                    // Direciona a pessoa de volta para a praça
                    pessoa->zonaAtual = PRACA;
                }
                
            // Se não há espaço nos balneários
            } else {

                // Verifica se há espaço na fila dos balneários
                if (balnearios.numeroPessoasNaFila < conf.tamanhoFilaBalnearios) {

                    sem_wait(&balnearios.fila);

                    // Incrementa o número de pessoas na fila dos balneários
                    pthread_mutex_lock(&mutexFilas);
                    balnearios.numeroPessoasNaFila++;
                    pthread_mutex_unlock(&mutexFilas);

                    // Calcula o tempo de espera baseado na posição na fila
                    pthread_mutex_lock(&mutexSimulacao);
                    tempoDeEspera = conf.tempoEsperaFilaBalnearios * balnearios.numeroPessoasNaFila;
                    pthread_mutex_unlock(&mutexSimulacao);

                    // Envia dados sobre a entrada na fila dos balneários para o monitor
                    enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, ENTRAR_FILA, BALNEARIOS);
                    // Imprime mensagem indicando a chegada à fila dos balneários
                    printf(CIANO "A pessoa com ID %d chegou à fila para entrar nos balneários | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);

                    // Aguarda o tempo de espera na fila dos balneários
                    sleep(tempoDeEspera);

                    // Se o tempo de simulação acabou enquanto esperava na fila
                    if (tempoSimulado > conf.tempoSimulacao) {
                        sem_post(&balnearios.fila);

                        // Decrementa o número de pessoas na fila dos balneários
                        pthread_mutex_lock(&mutexFilas);
                        balnearios.numeroPessoasNaFila--;
                        pthread_mutex_unlock(&mutexFilas);

                        // Envia dados sobre a saída da fila dos balneários para o monitor
                        enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, SAIR_FILA, BALNEARIOS);
                        // Imprime mensagem indicando a saída da fila dos balneários
                        printf(VERMELHO "A pessoa com ID %d teve que sair da fila dos balneários porque o parque está a fechar | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);

                        // Direciona a pessoa de volta para a praça
                        pessoa->zonaAtual = PRACA;

                    // Se não acabou
                    } else {

                        // Se a zona está cheia
                        if (balnearios.numeroAtualPessoas >= conf.numeroMaximoBalnearios) {

                            sem_post(&balnearios.fila);
                            sem_wait(&semaforoParque);
                            
                            // Decrementa o número de pessoas na fila dos balneários após a desistência
                            pthread_mutex_lock(&mutexFilas);
                            balnearios.numeroPessoasNaFila--;
                            pthread_mutex_unlock(&mutexFilas);

                            // Envia dados sobre a saída da fila dos balneários para o monitor
                            enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, SAIR_FILA, BALNEARIOS);
                            // Imprime mensagem indicando a desistência de entrar nos balneários
                            printf(VERMELHO "A pessoa com ID %d desistiu de entrar nos balneários porque não queria esperar na fila | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);

                            // Direciona a pessoa de volta para a praça
                            pessoa->zonaAtual = PRACA;

                        // Se não está cheia
                        } else {

                            sem_post(&balnearios.fila);
                            sem_wait(&semaforoBalneario);
                            
                            // Decrementa o número de pessoas na fila e incremeta o dos balneários
                            pthread_mutex_lock(&mutexFilas);
                            balnearios.numeroPessoasNaFila--;
                            balnearios.numeroAtualPessoas++;
                            pthread_mutex_unlock(&mutexFilas);
                            
                            // Envia dados sobre a entrada nos balneários após a espera na fila para o monitor
                            enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, SAIR_FILA_ENTRAR, BALNEARIOS);
                            // Imprime mensagem indicando a entrada nos balneários após a espera
                            if (pessoa->genero == MULHER) {
                                printf(CIANO_CLARO "A pessoa com ID %d entrou nos balneários femininos depois de esperar na fila| Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);
                            } else {
                                printf(CIANO_CLARO "A pessoa com ID %d entrou nos balneários masculinos depois de esperar na fila| Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);
                            }

                            // Aguarda o tempo de permanência nos balneários
                            sleep(conf.tempoBalnearios);

                            // Se o tempo de simulação acabou durante a permanência na zona
                            if (tempoSimulado > conf.tempoSimulacao) {

                                sem_post(&semaforoBalneario);

                                // Decrementa o número de pessoas nos balneários
                                pthread_mutex_lock(&mutexFilas);
                                balnearios.numeroAtualPessoas--;
                                pthread_mutex_unlock(&mutexFilas);

                                // Envia dados sobre a saída dos balneários para o monitor
                                enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, SAIR, BALNEARIOS);
                                // Imprime mensagem indicando a saída dos balneários
                                printf(VERMELHO "A pessoa com ID %d teve que sair dos balneários porque o parque está a fechar | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);

                                // Direciona a pessoa de volta para a praça
                                pessoa->zonaAtual = PRACA;
                            }
                        }
                    }

                // Se não houver espaço na fila
                } else {

                    // Imprime mensagem indicando a desistência por falta de espaço na fila dos balneários
                    printf(VERMELHO "A pessoa com ID %d desistiu de entrar nos balneários porque não havia lugar na fila | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);
                    
                    sem_wait(&semaforoParque);
                    
                    // Direciona a pessoa de volta para a praça
                    pessoa->zonaAtual = PRACA;
                }
            }
        }

    // Verifica se a pessoa está na zona de natação
    } else if (pessoa->zonaAtual == NATACAO) {

        // Verifica se o tempo de simulação acabou
        if (tempoSimulado > conf.tempoSimulacao) {

            // Direciona a pessoa de volta para a praça
            pessoa->zonaAtual = PRACA;

        // Se o tempo de simulação ainda não acabou
        } else {

            // Verifica se a pessoa tem altura e idade mínima para entrar na atração de natação
            if (pessoa->altura >= 100 && pessoa->idade >= 5) {

                // Verifica se há espaço na atração de natação
                if (natacao.numeroAtualPessoas < conf.numeroMaximoNatacao) {

                    sem_wait(&semaforoNatacao);

                    // Incrementa o número de pessoas na atração de natação
                    pthread_mutex_lock(&mutexFilas);
                    natacao.numeroAtualPessoas++;
                    pthread_mutex_unlock(&mutexFilas);

                    // Envia dados sobre a entrada na atração de natação para o monitor
                    enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, ENTRAR, NATACAO);
                    // Imprime mensagem indicando a entrada na atração de natação
                    printf(CIANO_CLARO "A pessoa com ID %d entrou na atração de natação | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);
                    
                    // Aguarda o tempo de permanência na atração de natação
                    sleep(conf.tempoNatacao);

                    // Se o tempo de simulação acabou durante a permanência na atração
                    if ((tempoSimulado > conf.tempoSimulacao)) {

                        sem_post(&semaforoNatacao);

                        // Decrementa o número de pessoas na atração de natação
                        pthread_mutex_lock(&mutexFilas);
                        natacao.numeroAtualPessoas--;
                        pthread_mutex_unlock(&mutexFilas);

                        // Envia dados sobre a saída da atração de natação para o monitor
                        enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, SAIR, NATACAO);
                        // Imprime mensagem indicando a saída da atração de natação
                        printf(VERMELHO "A pessoa com ID %d teve que sair da atração de natação porque o parque está a fechar | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);

                        // Direciona a pessoa de volta para a praça
                        pessoa->zonaAtual = PRACA;

                    // Se não acabou e se magoou-se
                    } else if (calcularProbabilidade(conf.probabilidadeMagoar)) {

                        // Imprime mensagem indicando que a pessoa magoou-se na atração de natação
                        printf(ROXO "A pessoa com ID %d magoou-se e tem de ir para a enfermaria | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);
                        
                        sem_post(&semaforoNatacao);
                        sem_wait(&semaforoParque);

                        // Decrementa o número de pessoas na atração de natação
                        pthread_mutex_lock(&mutexZonas);
                        natacao.numeroAtualPessoas--;
                        pthread_mutex_unlock(&mutexZonas);

                        // Envia dados sobre a saída da atração de natação para o monitor
                        enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, SAIR, NATACAO);
                        // Imprime mensagem indicando a saída da atração de natação
                        printf(VERMELHO "A pessoa com ID %d saiu da atração de natação | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);
                        
                        // Define a condição de magoar-se como verdadeira e direciona a pessoa para a enfermaria
                        pessoa->magoar = TRUE;
                        pessoa->zonaAtual = ENFERMARIA;
                    }

                // Se não houver espaço na atração
                } else {

                    // Verifica se há espaço na fila da atração de natação
                    if (natacao.numeroPessoasNaFila < conf.tamanhoFilaNatacao) {

                        sem_wait(&natacao.fila);

                        // Incrementa o número de pessoas na fila da atração de natação
                        pthread_mutex_lock(&mutexFilas);
                        natacao.numeroPessoasNaFila++;
                        pthread_mutex_unlock(&mutexFilas);

                        // Calcula o tempo de espera baseado na posição na fila
                        pthread_mutex_lock(&mutexSimulacao);
                        tempoDeEspera = conf.tempoEsperaFilaNatacao * natacao.numeroPessoasNaFila;
                        pthread_mutex_unlock(&mutexSimulacao);

                        // Envia dados sobre a entrada na fila da atração de natação para o monitor
                        enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, ENTRAR_FILA, NATACAO);
                        // Imprime mensagem indicando a chegada à fila da atração de natação
                        printf(CIANO "A pessoa com ID %d chegou à fila para entrar na atração de natação | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);
                        
                        // Aguarda o tempo de espera na fila da atração de natação
                        sleep(tempoDeEspera);

                        // Se o tempo de simulação acabou durante a espera na fila
                        if (tempoSimulado > conf.tempoSimulacao) {

                            sem_post(&natacao.fila);

                            // Decrementa o número de pessoas na fila da atração de natação
                            pthread_mutex_lock(&mutexFilas);
                            natacao.numeroPessoasNaFila--;
                            pthread_mutex_unlock(&mutexFilas);

                            // Envia dados sobre a saída da fila da atração de natação para o monitor
                            enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, SAIR_FILA, NATACAO);
                            // Imprime mensagem indicando a saída da fila da atração de natação
                            printf(VERMELHO "A pessoa com ID %d teve que sair da fila da natação porque o parque está a fechar | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);

                            // Direciona a pessoa de volta para a praça
                            pessoa->zonaAtual = PRACA;

                        // Se não acabou
                        } else {

                            // Se a atração está cheia
                            if (natacao.numeroAtualPessoas >= conf.numeroMaximoNatacao) {

                                sem_post(&natacao.fila);
                                sem_wait(&semaforoParque);

                                // Decrementa o número de pessoas na fila da atração de natação após a desistência
                                pthread_mutex_lock(&mutexFilas);
                                natacao.numeroPessoasNaFila--;
                                pthread_mutex_unlock(&mutexFilas);

                                // Envia dados sobre a saída da fila da atração de natação para o monitor
                                enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, SAIR_FILA, NATACAO);
                                // Imprime mensagem indicando a desistência de entrar na atração de natação
                                printf(VERMELHO "A pessoa com ID %d desistiu de entrar na atração de natação porque não queria esperar na fila | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);
                                
                                // Direciona a pessoa de volta para a praça
                                pessoa->zonaAtual = PRACA;

                            // Se não está cheia
                            } else {

                                sem_post(&natacao.fila);
                                sem_wait(&semaforoNatacao);
                                
                                // Decrementa o número de pessoas na fila e incremeta o da atração de natação
                                pthread_mutex_lock(&mutexFilas);
                                natacao.numeroPessoasNaFila--;
                                natacao.numeroAtualPessoas++;
                                pthread_mutex_unlock(&mutexFilas);

                                // Envia dados sobre a entrada na atração de natação após a espera na fila para o monitor
                                enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, SAIR_FILA_ENTRAR, NATACAO);
                                // Imprime mensagem indicando a entrada na atração de natação após a espera
                                printf(CIANO_CLARO "A pessoa com ID %d entrou na atração de natação depois de esperar na fila | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);

                                // Aguarda o tempo de permanência na atração de natação
                                sleep(conf.tempoNatacao);

                                // Se o tempo de simulação acabou durante a permanência na atração
                                if ((tempoSimulado > conf.tempoSimulacao)) {

                                    sem_post(&semaforoNatacao);

                                    // Decrementa o número de pessoas na atração de natação após a desistência
                                    pthread_mutex_lock(&mutexFilas);
                                    natacao.numeroAtualPessoas--;
                                    pthread_mutex_unlock(&mutexFilas);

                                    // Envia dados sobre a saída da atração de natação para o monitor
                                    enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, SAIR, NATACAO);
                                    // Imprime mensagem indicando a saída da atração de natação
                                    printf(VERMELHO "A pessoa com ID %d teve que sair da atração de natação porque o parque está a fechar | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);

                                    // Direciona a pessoa de volta para a praça
                                    pessoa->zonaAtual = PRACA;

                                // Se não acabou e se magoou-se
                                } else if (calcularProbabilidade(conf.probabilidadeMagoar)) {

                                    // Imprime mensagem indicando que a pessoa magoou-se na atração de natação
                                    printf(ROXO "A pessoa com ID %d magoou-se e tem de ir para a enfermaria | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);
                            
                                    sem_post(&semaforoNatacao);
                                    sem_wait(&semaforoParque);
                                    
                                    // Decrementa o número de pessoas da atração de natação após magoar-se
                                    pthread_mutex_lock(&mutexZonas);
                                    natacao.numeroAtualPessoas--;
                                    pthread_mutex_unlock(&mutexZonas);

                                    // Envia dados sobre a saida da atração de natação para o monitor
                                    enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, SAIR, NATACAO);
                                    // Imprime mensagem indicando a saída da atração de natação após o acidente
                                    printf(VERMELHO "A pessoa com ID %d saiu da atração de natação | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);
                                    
                                    // Define a condição de magoar-se como verdadeira e direciona a pessoa para a enfermaria
                                    pessoa->magoar = TRUE;
                                    pessoa->zonaAtual = ENFERMARIA;
                                }
                            }
                        }
                    // Se não há espaço na fila da atração de natação
                    } else {
                        
                        // Imprime mensagem indicando a desitência da atração de natação
                        printf(VERMELHO "A pessoa com ID %d desistiu de entrar na atração de natação porque não havia lugar na fila | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);
                        
                        sem_wait(&semaforoParque);
                        
                        // Direciona a pessoa de volta para a praça
                        pessoa->zonaAtual = PRACA;
                    }
                }

            // Se a pessoa não atende aos requisitos mínimos de idade e altura
            } else {

                // Imprime mensagem indicando a desitência da atração de natação por causa dos requisitos
                printf(CINZA "A pessoa com ID %d não tem altura e idade mínima requerida para entrar na natação | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);
                
                sem_wait(&semaforoParque);
                
                // Direciona a pessoa de volta para a praça
                pessoa->zonaAtual = PRACA;
            }
        }
       
    // Verifica se a pessoa está na zona de mergulho
    } else if (pessoa->zonaAtual == MERGULHO) {

        // Verifica se o tempo de simulação já terminou
        if (tempoSimulado > conf.tempoSimulacao) {

            // Direciona a pessoa de volta para a praça
            pessoa->zonaAtual = PRACA;

        // Se o tempo de simulação ainda não acabou
        } else {

            // Verifica se a pessoa tem altura e idade adequadas para a atração de mergulho
            if ((pessoa->altura <= 200 && pessoa->altura >= 100) && (pessoa->idade <= 55 && pessoa->idade >= 10)) {

                // Verifica se há espaço na atração de mergulho
                if (mergulho.numeroAtualPessoas < conf.numeroMaximoMergulho) {

                    sem_wait(&semaforoMergulho);
                    
                    // Incrementa o número atual de pessoas na atração de mergulho
                    pthread_mutex_lock(&mutexFilas);
                    mergulho.numeroAtualPessoas++;
                    pthread_mutex_unlock(&mutexFilas);

                    // Envia dados sobre a entrada na atração de mergulho para o monitor
                    enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, ENTRAR, MERGULHO);
                    // Imprime mensagem indicando a entrada na atração de mergulho
                    printf(CIANO_CLARO "A pessoa com ID %d entrou na atração de mergulho | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);
                    
                    // Aguarda o tempo de permanência na atração de mergulho
                    sleep(conf.tempoMergulho);

                    // Se o tempo de simulação acabou durante a permanência na atração
                    if ((tempoSimulado > conf.tempoSimulacao)) {

                        sem_post(&semaforoMergulho);

                        // Decrementa o número atual de pessoas na atração de mergulho
                        pthread_mutex_lock(&mutexFilas);
                        mergulho.numeroAtualPessoas--;
                        pthread_mutex_unlock(&mutexFilas);

                        // Envia dados sobre a saída da atração de mergulho para o monitor
                        enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, SAIR, MERGULHO);
                        // Imprime mensagem indicando a saída da atração de mergulho
                        printf(VERMELHO "A pessoa com ID %d teve que sair da atração de mergulho porque o parque está a fechar | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);

                        // Direciona a pessoa de volta para a praça
                        pessoa->zonaAtual = PRACA;

                    // Se não acabou e magoou-se
                    } else if (calcularProbabilidade(conf.probabilidadeMagoar)) {

                        // Imprime mensagem indicando que a pessoa magoou-se na atração de mergulho
                        printf(ROXO "A pessoa com ID %d magoou-se e tem de ir para a enfermaria | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);

                        sem_post(&semaforoMergulho);
                        sem_wait(&semaforoParque);
                        
                        // Decrementa o número atual de pessoas na atração de mergulho
                        pthread_mutex_lock(&mutexZonas);
                        mergulho.numeroAtualPessoas--;
                        pthread_mutex_unlock(&mutexZonas);
                        
                        // Envia dados sobre o acidente na atração de mergulho para o monitor
                        enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, SAIR, MERGULHO);
                        // Imprime mensagem indicando a saída da atração de mergulho
                        printf(VERMELHO "A pessoa com ID %d saiu da atração de mergulho | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);
                        
                        // Define a condição de magoar-se como verdadeira e direciona a pessoa para a enfermaria
                        pessoa->magoar = TRUE;
                        pessoa->zonaAtual = ENFERMARIA;
                    }
                
                // Se não houver espaço na atração
                } else {

                    // Verifica se há espaço na fila da atração de mergulho
                    if (mergulho.numeroPessoasNaFila < conf.tamanhoFilaMergulho) {

                        sem_wait(&mergulho.fila);

                        // Incrementa o número de pessoas na fila da atração de mergulho
                        pthread_mutex_lock(&mutexFilas);
                        mergulho.numeroPessoasNaFila++;
                        pthread_mutex_unlock(&mutexFilas);

                        // Calcula o tempo de espera baseado na posição na fila
                        pthread_mutex_lock(&mutexSimulacao);
                        tempoDeEspera = conf.tempoEsperaFilaMergulho * mergulho.numeroPessoasNaFila;
                        pthread_mutex_unlock(&mutexSimulacao);

                        // Envia dados sobre a entrada na fila da atração de mergulho para o monitor
                        enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, ENTRAR_FILA, MERGULHO);
                        // Imprime mensagem indicando a chegada à fila da atração de mergulho
                        printf(CIANO "A pessoa com ID %d chegou à fila para entrar na atração de mergulho | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);

                        // Aguarda o tempo de espera na fila da atração de mergulho
                        sleep(tempoDeEspera);

                        // Se o tempo de simulação acabou durante a espera na fila
                        if (tempoSimulado > conf.tempoSimulacao){

                            sem_post(&mergulho.fila);

                            // Decrementa o número de pessoas na fila da atração de mergulho
                            pthread_mutex_lock(&mutexFilas);
                            mergulho.numeroPessoasNaFila--;
                            pthread_mutex_unlock(&mutexFilas);

                            // Envia dados sobre a saída da fila da atração de mergulho para o monitor
                            enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, SAIR_FILA, MERGULHO);
                            // Imprime mensagem indicando a saída da fila da atração de mergulho
                            printf(VERMELHO "A pessoa com ID %d teve que sair da fila do mergulho porque o parque está a fechar | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);

                            // Direciona a pessoa de volta para a praça
                            pessoa->zonaAtual = PRACA;

                        // Se não acabou
                        } else {
                        
                            // Se a atração está cheia
                            if (mergulho.numeroAtualPessoas >= conf.numeroMaximoMergulho) {

                                sem_post(&mergulho.fila);
                                sem_wait(&semaforoParque);

                                // Decrementa o número de pessoas na fila da atração de mergulho após a desistência
                                pthread_mutex_lock(&mutexFilas);
                                mergulho.numeroPessoasNaFila--;
                                pthread_mutex_unlock(&mutexFilas);

                                // Envia dados sobre a saída da fila da atração de mergulho para o monitor
                                enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, SAIR_FILA, MERGULHO);
                                // Imprime mensagem indicando a desistência de entrar na atração de mergulho
                                printf(VERMELHO "A pessoa com ID %d desistiu de entrar na atração de mergulho porque não queria esperar na fila | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);
                                
                                // Direciona a pessoa de volta para a praça
                                pessoa->zonaAtual = PRACA;

                            // Se não está cheia
                            } else {

                                sem_post(&mergulho.fila);
                                sem_wait(&semaforoMergulho);
                                
                                // Decrementa o número de pessoas na fila e incremeta o da atração de mergulho
                                pthread_mutex_lock(&mutexFilas);
                                mergulho.numeroPessoasNaFila--;
                                mergulho.numeroAtualPessoas++;
                                pthread_mutex_unlock(&mutexFilas);

                                // Envia dados sobre a entrada na atração de mergulho após a espera na fila para o monitor
                                enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, SAIR_FILA_ENTRAR, MERGULHO);
                                // Imprime mensagem indicando a entrada na atração de mergulho após a espera
                                printf(CIANO_CLARO "A pessoa com ID %d entrou na atração de mergulho depois de esperar na fila | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);
                                
                                // Aguarda o tempo de permanência na atração de mergulho
                                sleep(conf.tempoMergulho);

                                // Se o tempo de simulação acabou durante a permanência na atração
                                if((tempoSimulado > conf.tempoSimulacao)) {

                                    sem_post(&semaforoMergulho);

                                    // Decrementa o número de pessoas da atração de mergulho após a desistência
                                    pthread_mutex_lock(&mutexFilas);
                                    mergulho.numeroAtualPessoas--;
                                    pthread_mutex_unlock(&mutexFilas);

                                    // Envia dados sobre a saída da atração de mergulho para o monitor
                                    enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, SAIR, TOBOGAS);
                                    // Imprime mensagem indicando a saída da atração de natação
                                    printf(VERMELHO "A pessoa com ID %d teve que sair da natação porque o parque está a fechar | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);

                                    // Direciona a pessoa de volta para a praça
                                    pessoa->zonaAtual = PRACA;

                                // Se não acabou e se magoou-se
                                }else if(calcularProbabilidade(conf.probabilidadeMagoar)){

                                    // Imprime mensagem indicando que a pessoa magoou-se na atração de mergulho
                                    printf(ROXO "A pessoa com ID %d magoou-se e tem de ir para a enfermaria | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);
                                
                                    sem_post(&semaforoMergulho);
                                    sem_wait(&semaforoParque);
                                    
                                    // Decrementa o número de pessoas da atração de mergulho após magoar-se
                                    pthread_mutex_lock(&mutexZonas);
                                    mergulho.numeroAtualPessoas--;
                                    pthread_mutex_unlock(&mutexZonas);
                                    
                                    // Envia dados sobre a saida da atração de mergulho para o monitor
                                    enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, SAIR, MERGULHO);
                                    // Imprime mensagem indicando a saída da atração de mergulho após o acidente
                                    printf(VERMELHO "A pessoa com ID %d saiu da atração de mergulho | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);
                                    
                                    // Define a condição de magoar-se como verdadeira e direciona a pessoa para a enfermaria
                                    pessoa->magoar = TRUE;
                                    pessoa->zonaAtual = ENFERMARIA;
                                }

                            }
                        }

                    // Se não há espaço na fila da atração de mergulho
                    } else {

                        // Imprime mensagem indicando a desitência da atração de mergulho
                        printf(VERMELHO "A pessoa com ID %d desistiu de entrar na atração de mergulho porque não havia lugar na fila | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);
                        
                        sem_wait(&semaforoParque);
                        
                        // Direciona a pessoa de volta para a praça
                        pessoa->zonaAtual = PRACA;
                    }
                }

            // Se a pessoa não atende aos requisitos mínimos de idade e altura
            }else{

                // Imprime mensagem indicando a desitência da atração de mergulho por causa dos requisitos
                printf(CINZA "A pessoa com ID %d não tem altura e idade mínima requirida para entrar na atração mergulhos | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);
                
                sem_wait(&semaforoParque);
                
                // Direciona a pessoa de volta para a praça
                pessoa->zonaAtual = PRACA;
                
            }
        }
        
    // Verifica se a pessoa está na zona de tobogãs
    } else if (pessoa->zonaAtual == TOBOGAS) {

        // Verifica se o tempo de simulação acabou
        if (tempoSimulado > conf.tempoSimulacao) {

            // Direciona a pessoa de volta para a praça
            pessoa->zonaAtual = PRACA;

        // Se o tempo de simulação ainda não acabou
        } else {

            // Verifica se a pessoa tem altura e idade mínima para entrar na atração de tobogãs
            if((pessoa->altura >= 110) && (pessoa->idade <= 70 && pessoa->idade >= 6)){ //criar no conf as alturas minimas e idade das atrações

                // Verifica se há espaço na atração de tobogãs
                if(tobogas.numeroAtualPessoas < conf.numeroMaximoTobogas){
                    sem_wait(&semaforoTobogas);
                    
                    // Incrementa o número de pessoas na atração de tobogãs
                    pthread_mutex_lock(&mutexFilas);
                    tobogas.numeroAtualPessoas++;
                    pthread_mutex_unlock(&mutexFilas);
                    
                    // Envia dados sobre a entrada na atração de tobogãs para o monitor
                    enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, ENTRAR, TOBOGAS);
                    // Imprime mensagem indicando a entrada na atração de tobogãs
                    printf(CIANO_CLARO "A pessoa com ID %d entrou na atração de tobogãs | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);
                    
                    // Aguarda o tempo de permanência na atração de tobogãs
                    sleep(conf.tempoTobogas);

                    // Se o tempo de simulação acabou durante a permanência na atração
                    if((tempoSimulado > conf.tempoSimulacao)) {

                        sem_post(&semaforoTobogas);

                        // Decrementa o número de pessoas na atração de tobogãs
                        pthread_mutex_lock(&mutexFilas);
                        tobogas.numeroAtualPessoas--;
                        pthread_mutex_unlock(&mutexFilas);

                        // Envia dados sobre a saída da atração de tobogãs para o monitor
                        enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, SAIR, TOBOGAS);
                        // Imprime mensagem indicando a saída da atração de tobogãs
                        printf(VERMELHO "A pessoa com ID %d teve que sair da atração de tobogãs porque o parque está a fechar | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);

                        // Direciona a pessoa de volta para a praça
                        pessoa->zonaAtual = PRACA;

                    // Se não acabou e se magoou-se
                    } else if (calcularProbabilidade(conf.probabilidadeMagoar)){

                        // Imprime mensagem indicando que a pessoa magoou-se na atração de tobogãs
                        printf(ROXO "A pessoa com ID %d magoou-se e tem de ir para a enfermaria | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);                    
                    
                        sem_post(&semaforoTobogas);
                        sem_wait(&semaforoParque);
                        
                        // Decrementa o número de pessoas na atração de tobogãs
                        pthread_mutex_lock(&mutexZonas);
                        tobogas.numeroAtualPessoas--;
                        pthread_mutex_unlock(&mutexZonas);

                        // Envia dados sobre a saída da atração de tobogãs para o monitor
                        enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, SAIR, TOBOGAS);
                        // Imprime mensagem indicando a saída da atração de tobogãs
                        printf(VERMELHO "A pessoa com ID %d saiu da atração de tobogãs | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);
                        
                        // Define a condição de magoar-se como verdadeira e direciona a pessoa para a enfermaria
                        pessoa->magoar = TRUE;
                        pessoa->zonaAtual = ENFERMARIA;
                    }

                // Se não houver espaço na atração
                } else {

                    // Verifica se há espaço na fila da atração de tobogãs
                    if (tobogas.numeroPessoasNaFila < conf.tamanhoFilaTobogas) {

                        sem_wait(&tobogas.fila);

                        // Incrementa o número de pessoas na fila da atração de tobogãs
                        pthread_mutex_lock(&mutexFilas);
                        tobogas.numeroPessoasNaFila++;
                        pthread_mutex_unlock(&mutexFilas);

                        // Calcula o tempo de espera baseado na posição na fila
                        pthread_mutex_lock(&mutexSimulacao);
                        tempoDeEspera = conf.tempoEsperaFilaTobogas * tobogas.numeroPessoasNaFila;
                        pthread_mutex_unlock(&mutexSimulacao);

                        // Envia dados sobre a entrada na fila da atração de tobogãs para o monitor
                        enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, ENTRAR_FILA, TOBOGAS);
                        // Imprime mensagem indicando a chegada à fila da atração de tobogãs
                        printf(CIANO "A pessoa com ID %d chegou à fila para entrar na atração de tobogãs | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);

                        // Aguarda o tempo de espera na fila da atração de tobogãs
                        sleep(tempoDeEspera);

                        // Se o tempo de simulação acabou durante a espera na fila
                        if (tempoSimulado > conf.tempoSimulacao){

                            sem_post(&tobogas.fila);

                            // Decrementa o número de pessoas na fila da atração de tobogãs
                            pthread_mutex_lock(&mutexFilas);
                            tobogas.numeroPessoasNaFila--;
                            pthread_mutex_unlock(&mutexFilas);

                            // Envia dados sobre a saída da fila da atração de tobogãs para o monitor
                            enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, SAIR_FILA, TOBOGAS);
                            // Imprime mensagem indicando a saída da fila da atração de tobogãs
                            printf(VERMELHO "A pessoa com ID %d teve que sair da fila dos tobogãs porque o parque está a fechar | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);

                            // Direciona a pessoa de volta para a praça
                            pessoa->zonaAtual = PRACA;

                        // Se não acabou
                        } else {
                        
                            // Se a atração está cheia
                            if (tobogas.numeroAtualPessoas >= conf.numeroMaximoTobogas) {

                                sem_post(&tobogas.fila);
                                sem_wait(&semaforoParque);

                                // Decrementa o número de pessoas na fila da atração de tobogãs após a desistência
                                pthread_mutex_lock(&mutexFilas);
                                tobogas.numeroPessoasNaFila--;
                                pthread_mutex_unlock(&mutexFilas);

                                // Envia dados sobre a saída da fila da atração de tobogãs para o monitor
                                enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, SAIR_FILA, TOBOGAS);
                                // Imprime mensagem indicando a desistência de entrar na atração de tobogãs
                                printf(VERMELHO "A pessoa com ID %d desistiu de entrar na atração de tobogãs porque não queria esperar na fila | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);
                                
                                // Direciona a pessoa de volta para a praça
                                pessoa->zonaAtual = PRACA;

                            // Se não está cheia
                            } else {

                                sem_post(&tobogas.fila);
                                sem_wait(&semaforoTobogas);

                                // Decrementa o número de pessoas na fila e incremeta o da atração de tobogãs
                                pthread_mutex_lock(&mutexFilas);
                                tobogas.numeroPessoasNaFila--;
                                tobogas.numeroAtualPessoas++;
                                pthread_mutex_unlock(&mutexFilas);
                                
                                // Envia dados sobre a entrada na atração de tobogãs após a espera na fila para o monitor
                                enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, SAIR_FILA_ENTRAR, TOBOGAS);
                                // Imprime mensagem indicando a entrada na atração de tobogãs após a espera
                                printf(CIANO_CLARO "A pessoa com ID %d entrou na atração de tobogãs depois de esperar na fila | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);
                                
                                // Aguarda o tempo de permanência na atração de tobogãs
                                sleep(conf.tempoTobogas);

                                // Se o tempo de simulação acabou durante a permanência na atração
                                if((tempoSimulado > conf.tempoSimulacao)) {

                                    sem_post(&semaforoTobogas);

                                    // Decrementa o número de pessoas na atração de tobogãs após a desistência
                                    pthread_mutex_lock(&mutexFilas);
                                    tobogas.numeroAtualPessoas--;
                                    pthread_mutex_unlock(&mutexFilas);

                                    // Envia dados sobre a saída da atração de tobogãs para o monitor
                                    enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, SAIR, TOBOGAS);
                                    // Imprime mensagem indicando a saída da atração de tobogãs
                                    printf(VERMELHO "A pessoa com ID %d teve que sair da atração de tobogãs porque o parque está a fechar | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);

                                    // Direciona a pessoa de volta para a praça
                                    pessoa->zonaAtual = PRACA;

                                // Se não acabou e se magoou-se
                                }else if(calcularProbabilidade(conf.probabilidadeMagoar)){

                                    // Imprime mensagem indicando que a pessoa magoou-se na atração de tobogãs
                                    printf(ROXO "A pessoa com ID %d magoou-se e tem de ir para a enfermaria | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);
                                    
                                    sem_post(&semaforoTobogas);
                                    sem_wait(&semaforoParque);

                                    // Decrementa o número de pessoas da atração de tobogãs após magoar-se
                                    pthread_mutex_lock(&mutexZonas);
                                    tobogas.numeroAtualPessoas--;
                                    pthread_mutex_unlock(&mutexZonas);

                                    // Envia dados sobre a saida da atração de tobogãs para o monitor
                                    enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, SAIR, TOBOGAS);
                                    // Imprime mensagem indicando a saída da atração de tobogãs após o acidente
                                    printf(VERMELHO "A pessoa com ID %d saiu da atração de tobogãs | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);
                                    
                                    // Define a condição de magoar-se como verdadeira e direciona a pessoa para a enfermaria
                                    pessoa->magoar = TRUE;
                                    pessoa->zonaAtual = ENFERMARIA;
                                }
                            }
                        }

                    // Se não há espaço na fila da atração de tobogãs    
                    } else {

                        // Imprime mensagem indicando a desitência da atração de tobogãs
                        printf(VERMELHO "A pessoa com ID %d desistiu de entrar na atração de tobogãs porque não havia lugar na fila | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);
                        
                        sem_wait(&semaforoParque);
                        
                        // Direciona a pessoa de volta para a praça
                        pessoa->zonaAtual = PRACA;
                    }
                }

            // Se a pessoa não atende aos requisitos mínimos de idade e altura    
            } else {

                // Imprime mensagem indicando a desitência da atração de tobogãs por causa dos requisitos
                printf(CINZA "A pessoa com ID %d não tem altura e idade mínima requirida para entrar nos tobogãs | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);
                
                sem_wait(&semaforoParque);
                
                // Direciona a pessoa de volta para a praça
                pessoa->zonaAtual = PRACA;
            }
        }

    // Verifica se a pessoa está na zona de restauração    
    } else if (pessoa->zonaAtual == RESTAURACAO) {

        // Verifica se o tempo de simulação acabou
        if (tempoSimulado > conf.tempoSimulacao) {

            // Direciona a pessoa de volta para a praça
            pessoa->zonaAtual = PRACA;

        // Se o tempo de simulação ainda não acabou
        } else {

            // Verifica se há espaço na restauração
            if(restauracao.numeroAtualPessoas < conf.numeroMaximoRestauracao){
                
                sem_wait(&semaforoRestauracao);

                // Incrementa o número de pessoas na restauração
                pthread_mutex_lock(&mutexFilas);
                restauracao.numeroAtualPessoas++;
                pthread_mutex_unlock(&mutexFilas);

                // Envia dados sobre a entrada na restauração para o monitor
                enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, ENTRAR, RESTAURACAO);
                // Imprime mensagem indicando a entrada na restauração
                printf(CIANO_CLARO "A pessoa com ID %d entrou na restauração | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);

                // Aguarda o tempo de permanência na restauração
                sleep(conf.tempoRestauracao);

                // Se o tempo de simulação acabou durante a permanência na zona
                if ((tempoSimulado > conf.tempoSimulacao)) {

                    sem_post(&semaforoRestauracao);

                    // Decrementa o número de pessoas na restauração
                    pthread_mutex_lock(&mutexFilas);
                    restauracao.numeroAtualPessoas--;
                    pthread_mutex_unlock(&mutexFilas);

                    // Envia dados sobre a saída da restauração para o monitor
                    enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, SAIR, RESTAURACAO);
                    // Imprime mensagem indicando a saída da restauração
                    printf(VERMELHO "A pessoa com ID %d teve que sair da restauração porque o parque está a fechar | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);

                    // Direciona a pessoa de volta para a praça
                    pessoa->zonaAtual = PRACA;
                }
                
            // Se não houver espaço na zona
            } else {

                // Verifica se há espaço na fila da restauração
                if (restauracao.numeroPessoasNaFila < conf.tamanhoFilaRestauracao) {

                    sem_wait(&restauracao.fila);

                    // Incrementa o número de pessoas na fila da restauração
                    pthread_mutex_lock(&mutexFilas);
                    restauracao.numeroPessoasNaFila++;
                    pthread_mutex_unlock(&mutexFilas);

                    // Calcula o tempo de espera baseado na posição na fila
                    pthread_mutex_lock(&mutexSimulacao);
                    tempoDeEspera = conf.tempoEsperaFilaRestauracao * restauracao.numeroPessoasNaFila;
                    pthread_mutex_unlock(&mutexSimulacao);

                    // Envia dados sobre a entrada na fila da restauração para o monitor
                    enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, ENTRAR_FILA, RESTAURACAO);
                    // Imprime mensagem indicando a chegada à fila da restauração
                    printf(CIANO "A pessoa com ID %d chegou à fila para entrar na restauração | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);

                    // Aguarda o tempo de espera na fila da restauração
                    sleep(tempoDeEspera);

                    // Se o tempo de simulação acabou durante a espera na fila
                    if (tempoSimulado > conf.tempoSimulacao){

                        sem_post(&restauracao.fila);

                        // Decrementa o número de pessoas na fila da restauração
                        pthread_mutex_lock(&mutexFilas);
                        restauracao.numeroPessoasNaFila--;
                        pthread_mutex_unlock(&mutexFilas);

                        // Envia dados sobre a saída da fila da restauração para o monitor
                        enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, SAIR_FILA, RESTAURACAO);
                        // Imprime mensagem indicando a saída da fila da restauração
                        printf(VERMELHO "A pessoa com ID %d teve que sair da fila da restauração porque o parque está a fechar | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);

                        // Direciona a pessoa de volta para a praça
                        pessoa->zonaAtual = PRACA;

                    // Se não acabou
                    } else {
                    
                        // Se a zona está cheia
                        if (restauracao.numeroAtualPessoas >= conf.numeroMaximoRestauracao) {

                            sem_post(&restauracao.fila);
                            sem_wait(&semaforoParque);

                            // Decrementa o número de pessoas na fila da restauração após a desistência
                            pthread_mutex_lock(&mutexFilas);
                            restauracao.numeroPessoasNaFila--;
                            pthread_mutex_unlock(&mutexFilas);

                            // Envia dados sobre a saída da fila da restauração para o monitor
                            enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, SAIR_FILA, RESTAURACAO);
                            // Imprime mensagem indicando a desistência de entrar na restauração
                            printf(VERMELHO "A pessoa com ID %d desistiu de entrar na restauração porque não queria esperar na fila | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);

                            // Direciona a pessoa de volta para a praça
                            pessoa->zonaAtual = PRACA;

                        // Se não está cheia
                        } else {

                            sem_post(&restauracao.fila);
                            sem_wait(&semaforoRestauracao);

                            // Decrementa o número de pessoas na fila e incremeta o da restauração
                            pthread_mutex_lock(&mutexFilas);
                            restauracao.numeroPessoasNaFila--;
                            restauracao.numeroAtualPessoas++;
                            pthread_mutex_unlock(&mutexFilas);

                            // Envia dados sobre a entrada na restauração após a espera na fila para o monitor
                            enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, SAIR_FILA_ENTRAR, RESTAURACAO);
                            // Imprime mensagem indicando a entrada na restauração após a espera
                            printf(CIANO_CLARO "A pessoa com ID %d entrou na restauração depois de esperar na fila | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);

                            // Aguarda o tempo de permanência na restauração
                            sleep(conf.tempoRestauracao);

                            // Se o tempo de simulação acabou durante a permanência na zona
                            if (tempoSimulado > conf.tempoSimulacao){

                                sem_post(&semaforoRestauracao);

                                // Decrementa o número de pessoas na restauração após a desistência
                                pthread_mutex_lock(&mutexFilas);
                                restauracao.numeroAtualPessoas--;
                                pthread_mutex_unlock(&mutexFilas);

                                // Envia dados sobre a saída da restauração para o monitor
                                enviarDados(NAO_ACABOU, pessoa->idPessoa, tempoSimulado, SAIR, RESTAURACAO);
                                // Imprime mensagem indicando a saída da restauração
                                printf(VERMELHO "A pessoa com ID %d teve que sair da restauração porque o parque está a fechar | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);

                                // Direciona a pessoa de volta para a praça
                                pessoa->zonaAtual = PRACA;

                            }
                        }
                    }

                // Se não houver espaço na fila
                } else {

                    // Imprime mensagem indicando a desistência por falta de espaço na fila da restauração
                    printf(VERMELHO "A pessoa com ID %d desistiu de entrar na restauração porque não havia lugar na fila | Tempo: %d\n" RESET, pessoa->idPessoa, tempoSimulado);

                    sem_wait(&semaforoParque);

                    // Direciona a pessoa de volta para a praça
                    pessoa->zonaAtual = PRACA;
                }
            }
        }
    }
}

// Função para enviar dados (buffer) para o socket
void enviarDados(int acabou, int personId, int tempo, int acao, int zona) {

    pthread_mutex_lock(&mutexDados);

    char buffer[TAMANHO_BUFFER];
    snprintf(buffer, TAMANHO_BUFFER, "%d %d %d %d %d|", acabou, personId, tempo, acao, zona);

    if (send(socketFD, buffer, strlen(buffer), 0) == -1) {
        perror("Erro ao enviar dados"); // Exibe uma mensagem de erro se não conseguir enviar os dados
    }

    pthread_mutex_unlock(&mutexDados);

}

void enviarPessoa(void *ptr) {

    // Cria uma pessoa
    struct pessoa person = criarPessoa();

    while(TRUE){

        //Se a pessoa entra no parque, logo se é a primeira vez
        if (!person.dentroParque) {

            Fila(&person);

        //Se a pessoa vai da praça para uma atração ou quer ficar na mesma atração
        } else if (person.dentroParque && person.zonaAtual != PRACA) {

            Fila(&person);

        }

        sleep(1);

        // Se a pessoa ainda não desistiu
        if(person.desistir == FALSE){

            // Se a zona atual for a Praça
            if(person.zonaAtual == PRACA) {

                // Verifica se o tempo de simulação acabou
                if(tempoSimulado>conf.tempoSimulacao){

                    sem_post(&semaforoParque);

                    // Decrementa o número de pessoas no parque
                    pthread_mutex_lock(&mutexParque);
                    pessoasParque--;
                    pthread_mutex_unlock(&mutexParque);

                    // Envia dados sobre a saída do parque para o monitor
                    enviarDados(NAO_ACABOU, person.idPessoa, tempoSimulado, SAIR, PRACA);
                    // Imprime mensagem indicando a saída do parque
                    printf(VERMELHO_CLARO "A pessoa com ID %d saiu do parque porque está a fechar | Tempo: %d\n" RESET, person.idPessoa, tempoSimulado);

                    break;

                }

                // Verifica se a pessoa desistiu de estar no parque
                if (calculaProbabilidadeDesistir(conf.probabilidadeDesistir, &person)) {

                    // Determina a próxima atração da pessoa
                    int proximaAtracao = visitarProximaAtracao(&person);
                    // Muda a zona atual da pessoa para a próxima
                    person.zonaAtual = proximaAtracao;

                    if (person.zonaAtual == BALNEARIOS) {

                        sem_post(&semaforoParque);

                    } else if (person.zonaAtual == NATACAO) {

                        sem_post(&semaforoParque);

                    } else if (person.zonaAtual == MERGULHO) {

                        sem_post(&semaforoParque);
                        
                    } else if (person.zonaAtual == TOBOGAS) {

                        sem_post(&semaforoParque);
                        
                    } else if (person.zonaAtual == RESTAURACAO) {

                        sem_post(&semaforoParque);
                        
                    }
                
                // Se a pessoa desistiu
                } else {

                    sem_post(&semaforoParque);

                    // Decrementa o número de pessoas no parque
                    pthread_mutex_lock(&mutexParque);
                    pessoasParque--;
                    pthread_mutex_unlock(&mutexParque);

                    // Envia dados sobre a saída do parque para o monitor
                    enviarDados(NAO_ACABOU, person.idPessoa, tempoSimulado, SAIR, PRACA);
                    // Imprime mensagem indicando a saída do parque
                    printf(VERMELHO_CLARO "A pessoa com ID %d foi para a Praça e saiu do parque | Tempo: %d\n" RESET, person.idPessoa, tempoSimulado);
                    
                    break;
                }
                
            // Verifica se a pessoa está na zona da enfermaria
            } else if (person.zonaAtual == ENFERMARIA) {
                
                // Verifica se o tempo de simulação acabou
                if(tempoSimulado > conf.tempoSimulacao){

                    sem_post(&semaforoParque);

                    // Decrementa o número de pessoas no parque
                    pthread_mutex_lock(&mutexParque);
                    pessoasParque--;
                    pthread_mutex_unlock(&mutexParque);

                    // Envia dados sobre a saída do parque para o monitor
                    enviarDados(NAO_ACABOU, person.idPessoa, tempoSimulado, SAIR, PRACA);
                    // Imprime mensagem indicando a saída do parque
                    printf(VERMELHO_CLARO "A pessoa com ID %d saiu do parque porque está a fechar e foi para o hospital | Tempo: %d\n" RESET, person.idPessoa, tempoSimulado);

                    break;

                }
                
                // Verifica se há espaço na enfermaria
                if(enfermaria.numeroAtualPessoas < conf.numeroMaximoEnfermaria){

                    sem_post(&semaforoParque);
                    sem_wait(&semaforoEnfermaria);

                    // Incrementa o número de pessoas na enfermaria
                    pthread_mutex_lock(&mutexFilas);
                    enfermaria.numeroAtualPessoas++;
                    pthread_mutex_unlock(&mutexFilas);

                    // Envia dados sobre a entrada na enfermaria para o monitor
                    enviarDados(NAO_ACABOU, person.idPessoa, tempoSimulado, ENTRAR, ENFERMARIA);
                    // Imprime mensagem indicando a entrada na enfermaria
                    printf(ROXO "A pessoa com ID %d entrou na enfermaria | Tempo: %d\n" RESET, person.idPessoa, tempoSimulado);

                    // Aguarda o tempo de permanência na enfermaria
                    sleep(conf.tempoEnfermaria);
                    
                    // Verifica se a pessoa foi curada
                    if(calcularProbabilidade(conf.probabilidadeCurar)){ 
                        
                        // Define a condição de magoar-se como falsa
                        person.magoar = FALSE;

                        sem_post(&semaforoEnfermaria);
                        sem_wait(&semaforoParque);

                        // Decrementa o número de pessoas na enfermaria
                        pthread_mutex_lock(&mutexZonas);
                        enfermaria.numeroAtualPessoas--;
                        pthread_mutex_unlock(&mutexZonas);

                        // Envia dados sobre a saída da enfermaria para o monitor
                        enviarDados(NAO_ACABOU, person.idPessoa, tempoSimulado, SAIR, ENFERMARIA);
                        // Imprime mensagem indicando a saída da enfermaria
                        printf(ROXO_CLARO "A pessoa com ID %d saiu da enfermaria curada | Tempo: %d\n" RESET, person.idPessoa, tempoSimulado);

                        // Direciona a pessoa de volta para a praça
                        person.zonaAtual = PRACA;

                    // Se não foi curada tem de ir para o hospital
                    }else{

                        sem_post(&semaforoEnfermaria);
                        
                        // Decrementa o número de pessoas na enfermaria
                        pthread_mutex_lock(&mutexZonas);
                        enfermaria.numeroAtualPessoas--;
                        pthread_mutex_unlock(&mutexZonas);

                        // Decrementa o número de pessoas no parque
                        pthread_mutex_lock(&mutexParque);
                        pessoasParque--;
                        pthread_mutex_unlock(&mutexParque);

                        // Envia dados sobre a saída da enfermaria e do parque para o monitor
                        enviarDados(NAO_ACABOU, person.idPessoa, tempoSimulado, SAIR_SAIR, ENFERMARIA);
                        // Imprime mensagem indicando a saída da enfermaria e do parque
                        printf(VERMELHO_CLARO "A pessoa com ID %d entrou na enfermaria mas não foi curada e foi para o hospital | Tempo: %d\n" RESET, person.idPessoa, tempoSimulado);

                        break;
                    }

                // Se não houver espaço na enfermaria
                } else {

                    // Verifica se há espaço na fila da enfermaria
                    if (enfermaria.numeroPessoasNaFila < conf.tamanhoFilaEnfermaria) {

                        sem_post(&semaforoParque);
                        sem_wait(&enfermaria.fila);

                        // Incrementa o número de pessoas na fila da enfermaria
                        pthread_mutex_lock(&mutexFilas);
                        enfermaria.numeroPessoasNaFila++;
                        pthread_mutex_unlock(&mutexFilas);

                        // Calcula o tempo de espera baseado na posição na fila
                        pthread_mutex_lock(&mutexSimulacao);
                        int tempoDeEspera = conf.tempoEsperaFilaEnfermaria * enfermaria.numeroPessoasNaFila;
                        pthread_mutex_unlock(&mutexSimulacao);

                        // Envia dados sobre a entrada na fila da enfermaria para o monitor
                        enviarDados(NAO_ACABOU, person.idPessoa, tempoSimulado, ENTRAR_FILA, ENFERMARIA);
                        // Imprime mensagem indicando a chegada à fila da enfermaria
                        printf(CIANO "A pessoa com ID %d chegou à fila para entrar na enfermaria | Tempo: %d\n" RESET, person.idPessoa, tempoSimulado);

                        // Aguarda o tempo de espera na fila da enfermaria
                        sleep(tempoDeEspera);
                        
                        // Se a zona continua cheia
                        if (enfermaria.numeroAtualPessoas >= conf.numeroMaximoEnfermaria) {

                            sem_post(&enfermaria.fila);

                            // Decrementa o número de pessoas na fila da enfermaria após a desistência
                            pthread_mutex_lock(&mutexFilas);
                            enfermaria.numeroPessoasNaFila--;
                            pthread_mutex_unlock(&mutexFilas);

                            // Decrementa o número de pessoas no parque
                            pthread_mutex_lock(&mutexParque);
                            pessoasParque--;
                            pthread_mutex_unlock(&mutexParque);
                            
                            // Envia dados sobre a saída da fila da enfermaria e do parque para o monitor
                            enviarDados(NAO_ACABOU, person.idPessoa, tempoSimulado, SAIR_FILA_ENFERMARIA, ENFERMARIA);
                            // Imprime mensagem indicando a saída da fila da enfermaria e do parque
                            printf(VERMELHO_CLARO "A pessoa com ID %d desistiu de entrar na enfermaria porque não queria esperar na fila e foi para o hospital | Tempo: %d\n" RESET, person.idPessoa, tempoSimulado);

                            break;

                        // Se a zona já não está cheia
                        } else {

                            sem_post(&enfermaria.fila);
                            sem_wait(&semaforoEnfermaria);

                            // Decrementa o número de pessoas na fila e incremeta o da enfermaria
                            pthread_mutex_lock(&mutexFilas);
                            enfermaria.numeroPessoasNaFila--;
                            enfermaria.numeroAtualPessoas++;
                            pthread_mutex_unlock(&mutexFilas);

                            // Envia dados sobre a entrada na enfermaria após a espera na fila para o monitor
                            enviarDados(NAO_ACABOU, person.idPessoa, tempoSimulado, SAIR_FILA_ENTRAR, ENFERMARIA);
                            // Imprime mensagem indicando a entrada na enfermaria após a espera
                            printf(ROXO "A pessoa com ID %d entrou na enfermaria depois de esperar na fila | Tempo: %d\n" RESET, person.idPessoa, tempoSimulado);

                            // Aguarda o tempo de permanência na enfermaria
                            sleep(conf.tempoEnfermaria);

                            // Verifica se a pessoa foi curada
                            if(calcularProbabilidade(conf.probabilidadeCurar)) { 
                                
                                // Define a condição de magoar-se como falsa
                                person.magoar = FALSE;

                                sem_post(&semaforoEnfermaria);
                                sem_wait(&semaforoParque);

                                // Decrementa o número de pessoas na enfermaria
                                pthread_mutex_lock(&mutexZonas);
                                enfermaria.numeroAtualPessoas--;
                                pthread_mutex_unlock(&mutexZonas);

                                // Envia dados sobre a saída da enfermaria para o monitor
                                enviarDados(NAO_ACABOU, person.idPessoa, tempoSimulado, SAIR, ENFERMARIA);
                                // Imprime mensagem indicando a saída da enfermaria
                                printf(ROXO_CLARO "A pessoa com ID %d saiu da enfermaria curada | Tempo: %d\n" RESET, person.idPessoa, tempoSimulado);

                                // Direciona a pessoa de volta para a praça
                                person.zonaAtual = PRACA;

                            // Se não foi curada tem de ir para o hospital
                            } else {

                                sem_post(&semaforoEnfermaria);

                                // Decrementa o número de pessoas na enfermaria
                                pthread_mutex_lock(&mutexZonas);
                                enfermaria.numeroAtualPessoas--;
                                pthread_mutex_unlock(&mutexZonas);

                                // Decrementa o número de pessoas no parque
                                pthread_mutex_lock(&mutexParque);
                                pessoasParque--;
                                pthread_mutex_unlock(&mutexParque);

                                // Envia dados sobre a saída da enfermaria e do parque para o monitor
                                enviarDados(NAO_ACABOU, person.idPessoa, tempoSimulado, SAIR_SAIR, ENFERMARIA);
                                // Imprime mensagem indicando a saída da enfermaria e do parque
                                printf(VERMELHO_CLARO "A pessoa com ID %d entrou na enfermaria mas não foi curada e foi para o hospital | Tempo: %d\n" RESET, person.idPessoa, tempoSimulado);
                                
                                break;
                            }

                        }

                    // Se não houver espaço na fila
                    } else {
                        
                        sem_post(&semaforoParque);

                        // Decrementa o número de pessoas no parque
                        pthread_mutex_lock(&mutexParque);
                        pessoasParque--;
                        pthread_mutex_unlock(&mutexParque);

                        // Envia dados sobre a saída do parque para o monitor
                        enviarDados(NAO_ACABOU, person.idPessoa, tempoSimulado, SAIR, PRACA);
                        // Imprime mensagem indicando a desistência e a saida do parque por falta de espaço na fila da enfermaria
                        printf(VERMELHO_CLARO "A pessoa com ID %d desistiu de entrar na enfermaria porque não havia lugar na fila e foi para o hospital | Tempo: %d\n" RESET, person.idPessoa, tempoSimulado);

                        break;
                    }
                } 

            //Está em qualquer outra zona
            } else {
            
                // Se o tempo de simulação já acabou
                if(tempoSimulado > conf.tempoSimulacao) {

                    // Se a pessoa está nos Balnearios
                    if (person.zonaAtual == BALNEARIOS) {
                        sem_post(&semaforoBalneario);

                        // Decrementa o número atual de pessoas 
                        pthread_mutex_lock(&mutexZonas);
                        balnearios.numeroAtualPessoas--;
                        pthread_mutex_unlock(&mutexZonas);

                    // Se a pessoa está na Natação
                    } else if (person.zonaAtual == NATACAO) {

                        sem_post(&semaforoNatacao);

                        // Decrementa o número atual de pessoas
                        pthread_mutex_lock(&mutexZonas);
                        natacao.numeroAtualPessoas--;
                        pthread_mutex_unlock(&mutexZonas);

                    // Se a pessoa está no Mergulho
                    } else if (person.zonaAtual == MERGULHO) {

                        sem_post(&semaforoMergulho);

                        // Decrementa o número atual de pessoas
                        pthread_mutex_lock(&mutexZonas);
                        mergulho.numeroAtualPessoas--;
                        pthread_mutex_unlock(&mutexZonas);

                    // Se a pessoa esta nos Tobogãs
                    } else if (person.zonaAtual == TOBOGAS) {

                        sem_post(&semaforoTobogas);

                        // Decrementa o número atual de pessoas
                        pthread_mutex_lock(&mutexZonas);
                        tobogas.numeroAtualPessoas--;
                        pthread_mutex_unlock(&mutexZonas);

                    // Se a pessoa está na Restauração
                    } else if (person.zonaAtual == RESTAURACAO) {

                        sem_post(&semaforoRestauracao);

                        // Decrementa o número atual de pessoas
                        pthread_mutex_lock(&mutexZonas);
                        restauracao.numeroAtualPessoas--;
                        pthread_mutex_unlock(&mutexZonas);
                    }

                    // Decrementa o número atual de pessoas no parque
                    pthread_mutex_lock(&mutexParque);
                    pessoasParque--;
                    pthread_mutex_unlock(&mutexParque);
                    
                    // Imprime a informação que a pessoa saiu do parque porque está a fechar
                    printf(VERMELHO_CLARO "A pessoa com ID %d saiu do parque porque está a fechar | Tempo: %d\n" RESET, person.idPessoa, tempoSimulado);
                    // Envia os dados de como a pessoa saiu da atração/zona e do Parque para o monitor
                    enviarDados(NAO_ACABOU, person.idPessoa, tempoSimulado, SAIR_SAIR, person.zonaAtual);
                    
                    break;

                }

                // Se a pessoa quer mudar de zona
                if (calculaProbabilidadeMudar(conf.probabilidadeMudarZona, &person)) {
                    
                    // Se está na zona dos Balnearios
                    if (person.zonaAtual == BALNEARIOS) {

                        sem_post(&semaforoBalneario);
                        sem_wait(&semaforoParque);

                        // Decrementa o número atual de pessoas nos Balnearios
                        pthread_mutex_lock(&mutexZonas);
                        balnearios.numeroAtualPessoas--;
                        pthread_mutex_unlock(&mutexZonas);

                        // Imprime a informação de que a pessoa saiu dos Balnearios
                        printf(VERMELHO "A pessoa com ID %d saiu dos balneários | Tempo: %d\n" RESET, person.idPessoa, tempoSimulado);
                        // Envia os dados a informar que a pessoa saiu dos Balnearios para o monitor
                        enviarDados(NAO_ACABOU, person.idPessoa, tempoSimulado, SAIR, BALNEARIOS);

                        sleep(1);
                    
                    // Se está na atração de Natação
                    } else if (person.zonaAtual == NATACAO) {

                        sem_post(&semaforoNatacao);
                        sem_wait(&semaforoParque);

                        // Decrementa o número atual de pessoas na Natação
                        pthread_mutex_lock(&mutexZonas);
                        natacao.numeroAtualPessoas--;
                        pthread_mutex_unlock(&mutexZonas);

                        // Imprime a informação de que a pessoa saiu da Natação
                        printf(VERMELHO "A pessoa com ID %d saiu da atração de natação | Tempo: %d\n" RESET, person.idPessoa, tempoSimulado);
                        // Envia os dados a informar que a pessoa saiu da Natação para o monitor
                        enviarDados(NAO_ACABOU, person.idPessoa, tempoSimulado, SAIR, NATACAO);

                        sleep(1);

                    // Se está na atração de Mergulho
                    } else if (person.zonaAtual == MERGULHO) {

                        sem_post(&semaforoMergulho);
                        sem_wait(&semaforoParque);

                        // Decrementa o número atual de pessoas no Mergulho
                        pthread_mutex_lock(&mutexZonas);
                        mergulho.numeroAtualPessoas--;
                        pthread_mutex_unlock(&mutexZonas);

                        // Imprime a informação de que a pessoa saiu do Mergulho
                        printf(VERMELHO "A pessoa com ID %d saiu da atração de mergulho | Tempo: %d\n" RESET, person.idPessoa, tempoSimulado);
                        // Envia os dados a informar que a pessoa saiu do Mergulho para o monitor
                        enviarDados(NAO_ACABOU, person.idPessoa, tempoSimulado, SAIR, MERGULHO);

                        sleep(1);
                        
                    // Se está na atração dos Tobogãs
                    } else if (person.zonaAtual == TOBOGAS) {

                        sem_post(&semaforoTobogas);
                        sem_wait(&semaforoParque);

                        // Decrementa o número atual de pessoas nos Tobogãs
                        pthread_mutex_lock(&mutexZonas);
                        tobogas.numeroAtualPessoas--;
                        pthread_mutex_unlock(&mutexZonas);

                        // Imprime a informação de que a pessoa saiu dos Tobogãs
                        printf(VERMELHO "A pessoa com ID %d saiu da atração de tobogãs | Tempo: %d\n" RESET, person.idPessoa, tempoSimulado);
                        // Envia os dados a informar que a pessoa saiu dos Tobogãs para o monitor
                        enviarDados(NAO_ACABOU, person.idPessoa, tempoSimulado, SAIR, TOBOGAS);
                        sleep(1);
                        
                    // Se está na zona da Restauração
                    } else if (person.zonaAtual == RESTAURACAO) {

                        sem_post(&semaforoRestauracao);
                        sem_wait(&semaforoParque);

                        // Decrementa o número atual de pessoas na Restauração
                        pthread_mutex_lock(&mutexZonas);
                        restauracao.numeroAtualPessoas--;
                        pthread_mutex_unlock(&mutexZonas);

                        // Imprime a informação de que a pessoa saiu da Restauração
                        printf(VERMELHO "A pessoa com ID %d saiu da restauração | Tempo: %d\n" RESET, person.idPessoa, tempoSimulado);
                        // Envia os dados a informar que a pessoa saiu da Restauração para o monitor
                        enviarDados(NAO_ACABOU, person.idPessoa, tempoSimulado, SAIR, RESTAURACAO);

                        sleep(1);
                        
                    }

                    // Muda a zona atual da pessoa para Praça
                    person.zonaAtual = PRACA;

                } else { //Se não quer mudar de zona, sai da zona para entrar outra vez

                    // Imprime a informação que a pessoa que continuar na mesma zona
                    printf(AZUL "A pessoa com o ID %d quer continuar na zona | Tempo: %d\n" RESET, person.idPessoa, tempoSimulado);

                    // Se está nos Balnearios
                    if (person.zonaAtual == BALNEARIOS) {

                        sem_post(&semaforoBalneario);

                        // Decrementa o número atual de pessoas nos Balnearios
                        pthread_mutex_lock(&mutexZonas);
                        balnearios.numeroAtualPessoas--;
                        pthread_mutex_unlock(&mutexZonas);

                        // Envia os dados de que a pessoa saiu dos Balnearios para o monitor
                        enviarDados(NAO_ACABOU, person.idPessoa, tempoSimulado, SAIR, BALNEARIOS);

                    // Se está na Natação
                    } else if (person.zonaAtual == NATACAO) {

                        sem_post(&semaforoNatacao);

                        // Decrementa o número atual de pessoas na Natação
                        pthread_mutex_lock(&mutexZonas);
                        natacao.numeroAtualPessoas--;
                        pthread_mutex_unlock(&mutexZonas);

                        // Envia os dados de que a pessoa saiu da Natação para o monitor
                        enviarDados(NAO_ACABOU, person.idPessoa, tempoSimulado, SAIR, NATACAO);

                    // Se está no Mergulho
                    } else if (person.zonaAtual == MERGULHO) {

                        sem_post(&semaforoMergulho);

                        // Decrementa o número atual de pessoas no Mergulho
                        pthread_mutex_lock(&mutexZonas);
                        mergulho.numeroAtualPessoas--;
                        pthread_mutex_unlock(&mutexZonas);

                        // Envia os dados de que a pessoa saiu do Mergulho para o monitor
                        enviarDados(NAO_ACABOU, person.idPessoa, tempoSimulado, SAIR, MERGULHO);

                    // Se está nos Tobogãs 
                    } else if (person.zonaAtual == TOBOGAS) {

                        sem_post(&semaforoTobogas);

                        // Decrementa o número atual de pessoas nos Tobogãs
                        pthread_mutex_lock(&mutexZonas);
                        tobogas.numeroAtualPessoas--;
                        pthread_mutex_unlock(&mutexZonas);

                        // Envia os dados de que a pessoa saiu dos Tobogãs para o monitor
                        enviarDados(NAO_ACABOU, person.idPessoa, tempoSimulado, SAIR, TOBOGAS);
                    
                    // Se está na Restauração
                    } else if (person.zonaAtual == RESTAURACAO) {

                        sem_post(&semaforoRestauracao);

                        // Decrementa o número atual de pessoas na Restauração
                        pthread_mutex_lock(&mutexZonas);
                        restauracao.numeroAtualPessoas--;
                        pthread_mutex_unlock(&mutexZonas);

                        // Envia os dados de que a pessoa saiu da Restauração para o monitor
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

    // Inicia mutex para enviar os dados para o monit
    if (pthread_mutex_init(&mutexDados, NULL) != 0) {
        perror("Erro na inicialização do mutexDados");
        exit(1);
    }

    // Inicialização dos semaforos 

    //Semáforo para a fila de entrada no parque
    sem_init(&praca.fila, 0, conf.tamanhoFilaParque);
    //Semáforo para a praça do parque
    sem_init(&semaforoParque, 0, conf.quantidadePessoasParque);

    //Semáforo para a fila de entrada nos balneários
    sem_init(&balnearios.fila, 0, conf.tamanhoFilaBalnearios);
    //Semáforo para os balneários
    sem_init(&semaforoBalneario, 0, conf.numeroMaximoBalnearios);

    //Semáforo para a fila de entrada na atração da natação
    sem_init(&natacao.fila, 0, conf.tamanhoFilaNatacao);
    //Semáforo para a atração da natação
    sem_init(&semaforoNatacao, 0, conf.numeroMaximoNatacao);

    //Semáforo para a fila de entrada na atração de mergulho
    sem_init(&mergulho.fila, 0, conf.tamanhoFilaMergulho);
    //Semáforo para a atração de mergulho
    sem_init(&semaforoMergulho, 0, conf.numeroMaximoMergulho);

    //Semáforo para a fila de entrada na atração dos tobogãs
    sem_init(&tobogas.fila, 0, conf.tamanhoFilaTobogas);
    //Semáforo para a atração dos tobogãs
    sem_init(&semaforoTobogas, 0, conf.numeroMaximoTobogas);

    //Semáforo para a fila de entrada na restauração
    sem_init(&restauracao.fila, 0, conf.tamanhoFilaRestauracao);
    //Semáforo para a restauração
    sem_init(&semaforoRestauracao, 0, conf.numeroMaximoRestauracao);

    //Semáforo para a fila de entrada na enfermaria
    sem_init(&enfermaria.fila, 0, conf.tamanhoFilaEnfermaria);
    //Semáforo para a enfermaria
    sem_init(&semaforoEnfermaria, 0, conf.numeroMaximoEnfermaria);

}

// Função principal do simulador
void simulador(char* config) {

    // Lê as configurações do arquivo e inicializa as variáveis
    configuracao(config);
    // Inicializa os trincos e semaforos
    exclusaoMutua();

    printf("Vai começar uma simulaçao de um parque com capacidade para %d pessoas.\n" RESET, conf.quantidadePessoasParque);

    // Loop para continuar a simulação
    while (TRUE) {

        pthread_mutex_lock(&mutexTempo);

        // Variavel que guarda o tempo atual real
        time_t tempoAtual = time(NULL);

        // Variavel que guarda a diferença de tempo do tempo atual para o atual, para saber quanto tempo real passou
        time_t diferencaTempo = tempoAtual - tempoInicial;

        // A varial tempoSimulado vai ser igual à variavel de cima
        tempoSimulado = diferencaTempo;

        pthread_mutex_unlock(&mutexTempo);
              
        // Verfica o tempo de chegada das pessoas e se já acabou o tempo de simulação
        if ((tempoSimulado % conf.tempoChegadaPessoas == 0) && (tempoSimulado < conf.tempoSimulacao)) {

            // Verifica se tem espaço na fila do parque
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

        // Verifica se o tempo simulado atingiu ou ultrapassou o tempo de simulação
        if ((pessoasParque < 1) && (tempoSimulado >= conf.tempoSimulacao)) {
            break; // Sai do loop quando a simulação terminar
        }
    }

    // Imprime no final da simulação que sairam todas as pessoas das atrações e que acabou a simulação
    printf("Acabou a simulação\n");

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
