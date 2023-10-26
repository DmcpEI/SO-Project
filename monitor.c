#include "header.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    // Verifica se o número de argumentos é válido
    if (argc != 3) {
        printf("Uso: %s <ficheiro> <texto>\n", argv[0]);
        return 1;
    }

    char *nomeFicheiro = argv[1];
    char *texto = argv[2];

    // Abre o ficheiro no modo de escrita (acrescentar)
    FILE *ficheiro = fopen(nomeFicheiro, "a");

    // Verifica se o ficheiro foi aberto com sucesso
    if (ficheiro == NULL) {
        printf("Erro ao abrir o ficheiro.\n");
        return 1;
    }

    // Escreve o texto no ficheiro
    fprintf(ficheiro, "%s\n", texto);

    // Fecha o ficheiro
    fclose(ficheiro);

    printf("Texto adicionado com sucesso!\n");

    return 0;
}

