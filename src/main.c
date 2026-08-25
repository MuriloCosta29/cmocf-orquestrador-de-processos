#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_LINE 1024
#define MAX_ARGS 64
#define MAX_TASKS 64
#define MAX_JOBS 64

typedef struct Task {
  char *nome;
  char *argumentos[MAX_ARGS];
  char *arquivo_entrada;
  char *arquivo_saida;
  int append;
} Task;

typedef struct Job {
  int id;
  pid_t pid;
  int task_index;
  int finished;
  int status;
} Job;

Task tasks[MAX_TASKS];
int task_count = 0;
Job jobs[MAX_JOBS];
int job_count = 0;

void update_jobs(void) {
  for (int i = 0; i < job_count; i++) {
    if (!jobs[i].finished) {
      pid_t result = waitpid(jobs[i].pid, &jobs[i].status, WNOHANG);

      if (result == jobs[i].pid) {
        jobs[i].finished = 1;
      } else if (result == -1) {
        perror("Erro ao verificar job");
      }
    }
  }
}

int main(int argc, char *argv[]) {
  char line[MAX_LINE];
  char *args[MAX_ARGS];
  FILE *source = stdin;
  int interactive = 1;

  if (argc > 2) {
    fprintf(stderr, "Uso: %s [workflowFile]\n", argv[0]);
    return EXIT_FAILURE;
  }

  if (argc == 2) {
    source = fopen(argv[1], "r");

    if (source == NULL) {
      perror("Erro ao abrir arquivo workflow");
      return EXIT_FAILURE;
    }

    interactive = 0;
  }

  while (1) {

    update_jobs();

    if (interactive) {
      printf("processflow> ");
      fflush(stdout);
    }

    char *result = fgets(line, sizeof(line), source);

    if (result == NULL) {
      break;
    }

    if (!interactive) {
      printf("%s", line);

      if (strchr(line, '\n') == NULL) {
        printf("\n");
      }

      fflush(stdout);
    }

    line[strcspn(line, "\n")] = '\0';

    int count = 0;

    char *token = strtok(line, " \t");

    while (token != NULL && count < MAX_ARGS - 1) {
      args[count] = token;
      count++;
      token = strtok(NULL, " \t");
    }

    args[count] = NULL;

    if (count == 0) {
      continue;
    }

    if (strcmp(args[0], "exit") == 0) {
      if (count != 1) {
        fprintf(stderr, "Uso: exit\n");
        continue;
      }

      break;

    } else if (strcmp(args[0], "task") == 0) {

      if (count < 3) {
        fprintf(stderr, "Uso: task <nome> <programa> [argumentos...]\n");
        continue;
      }
      if (task_count >= MAX_TASKS) {
        fprintf(stderr, "Limite máximo de tarefas atingido.\n");
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
            if (tasks[task_index].arquivo_entrada != NULL) {
              int fd = open(tasks[task_index].arquivo_entrada, O_RDONLY);

              if (fd == -1) {
                perror("Erro ao abrir arquivo de entrada");
                exit(EXIT_FAILURE);
              }

              if (dup2(fd, STDIN_FILENO) == -1) {
                perror("Erro ao redirecionar entrada");
                close(fd);
                exit(EXIT_FAILURE);
              }

              close(fd);
            }

            if (tasks[task_index].arquivo_saida != NULL) {
              int flags = O_WRONLY | O_CREAT;

              if (tasks[task_index].append) {
                flags |= O_APPEND;
              } else {
                flags |= O_TRUNC;
              }

              int fd = open(tasks[task_index].arquivo_saida, flags, 0644);

              if (fd == -1) {
                perror("Erro ao abrir arquivo de saída");
                exit(EXIT_FAILURE);
              }

              if (dup2(fd, STDOUT_FILENO) == -1) {
                perror("Erro ao redirecionar saída");
                close(fd);
                exit(EXIT_FAILURE);
              }

              close(fd);
            }

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
            if (tasks[task_index].arquivo_entrada != NULL) {
              int fd = open(tasks[task_index].arquivo_entrada, O_RDONLY);

              if (fd == -1) {
                perror("Erro ao abrir arquivo de entrada");
                exit(EXIT_FAILURE);
              }

              if (dup2(fd, STDIN_FILENO) == -1) {
                perror("Erro ao redirecionar entrada");
                close(fd);
                exit(EXIT_FAILURE);
              }

              close(fd);
            }

            if (tasks[task_index].arquivo_saida != NULL) {
              int flags = O_WRONLY | O_CREAT;

              if (tasks[task_index].append) {
                flags |= O_APPEND;
              } else {
                flags |= O_TRUNC;
              }

              int fd = open(tasks[task_index].arquivo_saida, flags, 0644);

              if (fd == -1) {
                perror("Erro ao abrir arquivo de saída");
                exit(EXIT_FAILURE);
              }

              if (dup2(fd, STDOUT_FILENO) == -1) {
                perror("Erro ao redirecionar saída");
                close(fd);
                exit(EXIT_FAILURE);
              }

              close(fd);
            }

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
    } else if (strcmp(args[0], "start") == 0) {

      if (count != 2) {
        fprintf(stderr, "Uso: start <tarefa>\n");
        continue;
      }

      if (job_count >= MAX_JOBS) {
        fprintf(stderr, "Limite máximo de jobs atingido.\n");
        continue;
      }

      int found = -1;

      for (int i = 0; i < task_count; i++) {
        if (strcmp(tasks[i].nome, args[1]) == 0) {
          found = i;
          break;
        }
      }

      if (found == -1) {
        fprintf(stderr, "Tarefa '%s' não existe.\n", args[1]);
        continue;
      }

      pid_t pid = fork();

      if (pid < 0) {
        perror("Erro ao criar processo");
        continue;
      }

      if (pid == 0) {
        if (tasks[found].arquivo_entrada != NULL) {
          int fd = open(tasks[found].arquivo_entrada, O_RDONLY);

          if (fd == -1) {
            perror("Erro ao abrir arquivo de entrada");
            exit(EXIT_FAILURE);
          }

          if (dup2(fd, STDIN_FILENO) == -1) {
            perror("Erro ao redirecionar entrada");
            close(fd);
            exit(EXIT_FAILURE);
          }

          close(fd);
        }

        if (tasks[found].arquivo_saida != NULL) {
          int flags = O_WRONLY | O_CREAT;

          if (tasks[found].append) {
            flags |= O_APPEND;
          } else {
            flags |= O_TRUNC;
          }

          int fd = open(tasks[found].arquivo_saida, flags, 0644);

          if (fd == -1) {
            perror("Erro ao abrir arquivo de saída");
            exit(EXIT_FAILURE);
          }

          if (dup2(fd, STDOUT_FILENO) == -1) {
            perror("Erro ao redirecionar saída");
            close(fd);
            exit(EXIT_FAILURE);
          }

          close(fd);
        }

        execvp(tasks[found].argumentos[0], tasks[found].argumentos);
        perror("Erro ao executar programa");
        exit(EXIT_FAILURE);
      }

      jobs[job_count].id = job_count + 1;
      jobs[job_count].pid = pid;
      jobs[job_count].task_index = found;
      jobs[job_count].finished = 0;

      printf("[%d] %d\n", jobs[job_count].id, (int)pid);
      job_count++;

    } else if (strcmp(args[0], "jobs") == 0) {

      if (count != 1) {
        fprintf(stderr, "Uso: jobs\n");
        continue;
      }

      update_jobs();

      for (int i = 0; i < job_count; i++) {
        if (jobs[i].finished) {
          printf("[%d] %d concluído %s\n", jobs[i].id, (int)jobs[i].pid,
                 tasks[jobs[i].task_index].nome);
        } else {
          printf("[%d] %d executando %s\n", jobs[i].id, (int)jobs[i].pid,
                 tasks[jobs[i].task_index].nome);
        }
      }

    } else if (strcmp(args[0], "wait") == 0) {

      if (count != 2) {
        fprintf(stderr, "Uso: wait <jobId>\n");
        continue;
      }

      int job_id = atoi(args[1]);
      int found_job = -1;

      for (int i = 0; i < job_count; i++) {
        if (jobs[i].id == job_id) {
          found_job = i;
          break;
        }
      }

      if (found_job == -1) {
        fprintf(stderr, "Job '%s' não existe.\n", args[1]);
        continue;
      }

      if (!jobs[found_job].finished) {
        if (waitpid(jobs[found_job].pid, &jobs[found_job].status, 0) == -1) {
          perror("Erro ao esperar job");
          continue;
        }

        jobs[found_job].finished = 1;
      }

      printf("[%d] %d concluído %s\n", jobs[found_job].id,
             (int)jobs[found_job].pid, tasks[jobs[found_job].task_index].nome);

      if (WIFEXITED(jobs[found_job].status)) {
        int exit_code = WEXITSTATUS(jobs[found_job].status);

        if (exit_code != 0) {
          fprintf(stderr, "Tarefa '%s' terminou com código %d.\n",
                  tasks[jobs[found_job].task_index].nome, exit_code);
        }
      }

    } else if (strcmp(args[0], "input") == 0) {

      if (count != 3) {
        fprintf(stderr, "Uso: input <tarefa> <arquivo>\n");
        continue;
      }

      int found = -1;

      for (int i = 0; i < task_count; i++) {
        if (strcmp(tasks[i].nome, args[1]) == 0) {
          found = i;
          break;
        }
      }

      if (found == -1) {
        fprintf(stderr, "Tarefa '%s' não existe.\n", args[1]);
        continue;
      }

      tasks[found].arquivo_entrada = strdup(args[2]);

    } else if (strcmp(args[0], "output") == 0) {

      if (count != 3) {
        fprintf(stderr, "Uso: output <tarefa> <arquivo>\n");
        continue;
      }

      int found = -1;

      for (int i = 0; i < task_count; i++) {
        if (strcmp(tasks[i].nome, args[1]) == 0) {
          found = i;
          break;
        }
      }

      if (found == -1) {
        fprintf(stderr, "Tarefa '%s' não existe.\n", args[1]);
        continue;
      }

      tasks[found].arquivo_saida = strdup(args[2]);
      tasks[found].append = 0;

    } else if (strcmp(args[0], "append") == 0) {

      if (count != 3) {
        fprintf(stderr, "Uso: append <tarefa> <arquivo>\n");
        continue;
      }

      int found = -1;

      for (int i = 0; i < task_count; i++) {
        if (strcmp(tasks[i].nome, args[1]) == 0) {
          found = i;
          break;
        }
      }

      if (found == -1) {
        fprintf(stderr, "Tarefa '%s' não existe.\n", args[1]);
        continue;
      }

      tasks[found].arquivo_saida = strdup(args[2]);
      tasks[found].append = 1;

    } else if (strcmp(args[0], "workdir") == 0) {

      if (count != 2) {
        fprintf(stderr, "Uso: workdir <diretório>\n");
        continue;
      }

      if (chdir(args[1]) == -1) {
        perror("Erro ao alterar diretório");
        continue;
      }

    } else {
      fprintf(stderr, "Comando desconhecido: '%s'\n", args[0]);
    }
  }

  for (int i = 0; i < job_count; i++) {
    if (!jobs[i].finished) {
      if (waitpid(jobs[i].pid, &jobs[i].status, 0) == jobs[i].pid) {
        jobs[i].finished = 1;
      }
    }
  }

  if (!interactive) {
    fclose(source);
  }

  return EXIT_SUCCESS;
}
