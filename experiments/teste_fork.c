#include <stdio.h>
#include <unistd.h>

int main(void) {
  printf("antes\n");

  fork();

  printf("depois\n");
  return 0;
}

// Quando rodo: ./teste_fork o shell cria um processo rodando o código. Esse
// processo é o processo Pai! O pai é o processo que estava executando  o
// código, então o fork o duplica e o filho que nasce dele, faz com que APARTIR
// DALI o processo seja executado duas vezes, porque a dois processos de
// execução de código sendo rodados.

/*
o "APARTIR DALI" (está em caixa alta, porque é importante entender que depois
que "forko" um processo ele continua de onde ele foi "forkado").
*/
