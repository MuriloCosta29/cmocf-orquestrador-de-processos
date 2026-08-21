#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 1024
#define MAX_ARGS 64

int main(void) {
  char line[MAX_LINE];

  char *args[MAX_ARGS];

  while (1) {

    printf("processflow> ");

    char *result = fgets(line, sizeof(line), stdin);

    if (result == NULL) {
      break;
    }

    line[strcspn(line, "\n")] = '\0';

    if (strcmp(line, "exit") == 0) {
      break;
    }

    int count = 0;
    // Contar quantas palavras foram contadas.

    char *token = strtok(line, " ");
    // strtok divide uma string em multiplos pedaços

    while (token != NULL && count < MAX_ARGS - 1) {
      args[count] = token;
      // Guarda a palavra atual na próxima posição do array.
      count++;
      // avança posição agora count é = 1, então
      // próxima palavra vai para args[1].
      token = strtok(NULL, " ");
      // Pede a prox palavra. O NULL significa:
      // Continue de onde parou.
      // Essa linha faz o loop avançar! Sem ela
      // token nunca mudaria e o loop rodaria para sempre.
    }

    args[count] = NULL;
    // Marca o fim da lista
    // pq o args aqui vai ficar assim:
    // args[<ultimo-indice>] = NULL
    // porque o execvp percorre array até achar NULL,
    // então tem que coloca-ló para saber onde lista argumentos acaba.

    for (int i = 0; i < count; i++) { // não é <=, pq <= leria até '\0'.
      printf("[%d] %s\n", i, args[i]);
    }
  }
}
