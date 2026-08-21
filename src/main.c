#include <_string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 1024
#define MAX_ARGS 64
#define MAX_TASKS 64

// ----------------------

typedef struct Task {
  char *nome;
  char *argumentos[MAX_ARGS];
} Task;

Task tasks[MAX_TASKS];
int task_count = 0;

// ----------------------

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

    char *token = strtok(line, " ");

    while (token != NULL && count < MAX_ARGS - 1) {
      args[count] = token;
      count++;
      token = strtok(NULL, " ");
    }

    args[count] = NULL;

    for (int i = 0; i < count; i++) {
      printf("[%d] %s\n", i, args[i]);
    }

    if (count == 0) {
      continue;
    }

    if (strcmp(args[0], "task") == 0) {
      // cadastra na tabela
      if (count < 3) {
        fprintf(stderr, "depois coloco mensagem\n");
        continue;
      }
      if (task_count >= MAX_TASKS) {
        fprintf(stderr, "ERROR!");
        continue;
      }

      tasks[task_count].nome = strdup(args[1]);

      for (int i = 2; i < count; i++) {
        tasks[task_count].argumentos[i - 2] = strdup(args[i]);
      }
      tasks[task_count].argumentos[count - 2] = NULL;
      task_count++;
      printf("Cadastro confirmado.\n");
    } else if (strcmp(args[0], "run") == 0) {
      // procura na tabela, fork/exec/wait
    } else if (strcmp(args[0], "input") == 0) {
      // anota o arquivo de entrada na tarefa
    } else if (strcmp(args[0], "workdir") == 0) {
      // troca de diretório
    } else {
      // comando desconhecido: mensagem de erro, continua
    }
  }
}
