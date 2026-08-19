#include <stdio.h>
#include <stdlib.h>

#define MAX_LINE 1024

int main(void) {
  char linha[MAX_LINE];

  while (1) {
    printf("processflow> ");

    char *resultado = fgets(linha, sizeof(linha), stdin);

    if (resultado == NULL) {
      break;
    }

    printf("%s", linha);
  }
}
