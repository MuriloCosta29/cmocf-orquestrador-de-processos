#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

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

    if (count == 0) {
      continue;
    }

    // ------------
    // TASK - cadastra na tabela
    if (strcmp(args[0], "task") == 0) {

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

      // ------------
      // RUN - procura na tabela, fork/exec/wait

    } else if (strcmp(args[0], "run") == 0) {

      if (count < 2) {
        fprintf(stderr, "Uso: run <tarefa>\n");
        continue;
      }

      int first_task = 1;
      int parallel = 0;

      if (strcmp(args[1], "sequential") == 0) {
        if (count < 3) {
          fprintf(stderr, "Uso: run sequential <tarefa1> [tarefa2...]\n");
          continue;
        }

        first_task = 2;

      } else if (strcmp(args[1], "parallel") == 0) {
        if (count < 3) {
          fprintf(stderr, "Uso: run parallel <tarefa1> [tarefa2...]\n");
          continue;
        }

        first_task = 2;
        parallel = 1;

      } else if (count != 2) {
        fprintf(stderr, "Uso: run <tarefa>\n");
        continue;
      }

      int task_quantity = count - first_task;
      int task_indexes[MAX_ARGS];
      int missing_task = 0;

      for (int i = 0; i < task_quantity; i++) {
        task_indexes[i] = -1;

        for (int j = 0; j < task_count; j++) {
          if (strcmp(tasks[j].nome, args[first_task + i]) == 0) {
            task_indexes[i] = j;
            break;
          }
        }

        if (task_indexes[i] == -1) {
          fprintf(stderr, "Tarefa '%s' não existe.\n", args[first_task + i]);
          missing_task = 1;
        }
      }

      if (missing_task) {
        continue;
      }

      if (!parallel) {
        for (int i = 0; i < task_quantity; i++) {
          int task_index = task_indexes[i];
          pid_t pid = fork();

          if (pid < 0) {
            perror("Erro ao criar processo");
            continue;
          }

          if (pid == 0) {
            execvp(tasks[task_index].argumentos[0],
                   tasks[task_index].argumentos);

            perror("Erro ao executar programa");
            exit(EXIT_FAILURE);
          }

          int status;

          if (waitpid(pid, &status, 0) == -1) {
            perror("Erro ao esperar processo");
            continue;
          }

          if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);

            if (exit_code != 0) {
              fprintf(stderr, "Tarefa '%s' terminou com código %d.\n",
                      tasks[task_index].nome, exit_code);
            }
          }
        }

      } else {
        pid_t pids[MAX_ARGS];

        for (int i = 0; i < task_quantity; i++) {
          int task_index = task_indexes[i];
          pids[i] = fork();

          if (pids[i] < 0) {
            perror("Erro ao criar processo");
            continue;
          }

          if (pids[i] == 0) {
            execvp(tasks[task_index].argumentos[0],
                   tasks[task_index].argumentos);

            perror("Erro ao executar programa");
            exit(EXIT_FAILURE);
          }
        }

        for (int i = 0; i < task_quantity; i++) {
          if (pids[i] < 0) {
            continue;
          }

          int status;
          int task_index = task_indexes[i];

          if (waitpid(pids[i], &status, 0) == -1) {
            perror("Erro ao esperar processo");
            continue;
          }

          if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);

            if (exit_code != 0) {
              fprintf(stderr, "Tarefa '%s' terminou com código %d.\n",
                      tasks[task_index].nome, exit_code);
            }
          }
        }
      }
      // ------------
      // INPUT - anota o arquivo de entrada na tarefa
    } else if (strcmp(args[0], "input") == 0) {

      // ------------
      // WORKDIR - troca de diretório
    } else if (strcmp(args[0], "workdir") == 0) {

      if (count != 2) {
        fprintf(stderr, "Uso: workdir <diretório>\n");
        continue;
      }

      if (chdir(args[1]) == -1) {
        perror("Erro ao alterar diretório");
        continue;
      }

      // ------------
    } else {
      // comando desconhecido: mensagem de erro, continua
    }
  }
}
