#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void) {
  printf("antes\n");

  pid_t pid = fork();

  if (pid == 0) {
    printf("Eu sou o filho\n");

    char *args[] = {"ls", "-l", NULL};
    execvp("ls", args);
    printf("depois do execvp\n");
    perror("execvp");

  } else {
    printf("eu sou o pai\n");
    wait(NULL);
  }

  printf("retorno de fork = %d | meu PID = %d\n", (int)pid, (int)getpid());
}
