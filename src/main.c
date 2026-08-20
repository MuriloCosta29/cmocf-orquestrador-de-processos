#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 1024

int main(void) {
  char line[MAX_LINE];

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
    printf("%s\n", line);
  }
}
