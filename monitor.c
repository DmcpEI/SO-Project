#include "header.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    // Verifica se o número de argumentos é válido
    if (argc != 3) {
        printf("Deve escrever como argumentos: %s <ficheiro> <texto>\n", argv[0]);
        return 1;
    }

    char *nomeFicheiro = argv[1];
    char *texto = argv[2];

    // Verifica se o arquivo já existe
    FILE *verificador = fopen(nomeFicheiro, "r");
    
    if (!verificador) {
        printf("Criou ficheiro %s.\n", nomeFicheiro);
    }

    // Abre o arquivo no modo de acrescentar ou criar
    FILE *ficheiro = fopen(nomeFicheiro, "a");

    // Verifica se o arquivo foi aberto com sucesso
    if (ficheiro == NULL) {
        printf("Erro ao abrir o ficheiro.\n");
        return 1;
    }
    else {
        // Escreve o texto no arquivo
        fprintf(ficheiro, "%s\n", texto);

        // Fecha o arquivo
        fclose(ficheiro);

        printf("Texto adicionado com sucesso em %s.\n", nomeFicheiro);

        return 0;
    }
}
