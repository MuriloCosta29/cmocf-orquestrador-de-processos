# ProcessFlow - Orquestrador de Processos

O ProcessFlow é um orquestrador de processos desenvolvido em C. Ele cadastra
programas como tarefas e os executa por meio de processos filhos, utilizando as
chamadas de sistema `fork()`, `execvp()`, `waitpid()`, `dup2()` e `pipe()`.

O programa funciona no modo interativo ou executando comandos de um arquivo de
workflow. A execução não utiliza `system()`, `popen()` ou outro shell.

## Funcionalidades

- Cadastro e execução de tarefas.
- Execução simples, sequencial e paralela.
- Comunicação entre tarefas utilizando pipes.
- Redirecionamento de entrada e saída.
- Escrita e acréscimo em arquivos.
- Alteração do diretório de trabalho.
- Execução de tarefas em background.
- Listagem e espera de jobs.
- Execução de arquivos de workflow.
- Tratamento de comandos e recursos inválidos.

## Requisitos

- Linux, Unix ou macOS.
- Compilador C compatível com as chamadas POSIX.
- `make`.

## Compilação

Para compilar o programa:

```bash
make
```

O executável `processflow` será criado na raiz do projeto.

Outros comandos disponíveis no Makefile:

```bash
make clean   # Remove o executável
make run     # Compila e executa no modo interativo
make debug   # Compila com AddressSanitizer e executa
```

## Ambiente limpo para evidências

O arquivo `.evidrc` não faz parte do funcionamento do ProcessFlow. Ele foi
criado somente para abrir uma sessão limpa do Bash durante a produção do
`evidencias.log`, evitando que temas, plugins e configurações pessoais do shell
apareçam no registro.

Essa configuração deixa o prompt apenas como `$` e mostra a data, o usuário e o
diretório atual no começo da sessão.

Para abrir o Bash utilizando essa configuração:

```bash
bash --noprofile --rcfile .evidrc
```

## Execução

### Modo interativo

```bash
./processflow
```

O programa apresentará o prompt:

```text
processflow>
```

### Modo workflow

```bash
./processflow arquivo.pf
```

Cada linha do arquivo será impressa antes de ser processada. O comando `exit` é
recomendado no final do workflow, mas o programa também termina normalmente ao
encontrar o fim do arquivo.

Exemplo de `fluxo.pf`:

```text
task primeira /bin/echo primeira
task segunda /bin/echo segunda
run sequential primeira segunda
exit
```

## Comandos

### Cadastrar uma tarefa

```text
task <nome> <programa> [argumentos...]
```

Exemplo:

```text
task listar /bin/ls -l
```

### Executar uma tarefa

```text
run <nome>
```

### Executar tarefas sequencialmente

Cada tarefa começa somente depois que a anterior termina:

```text
run sequential <tarefa1> <tarefa2> [tarefa3...]
```

### Executar tarefas em paralelo

Todas as tarefas são iniciadas antes que o ProcessFlow aguarde seus términos:

```text
run parallel <tarefa1> <tarefa2> [tarefa3...]
```

### Conectar tarefas com pipes

A saída de cada tarefa é utilizada como entrada da tarefa seguinte:

```text
run pipe <tarefa1> <tarefa2> [tarefa3...]
```

Exemplo:

```text
task ler /bin/cat
task ordenar /usr/bin/sort
task contar /usr/bin/wc -l
input ler nomes.txt
output contar resultado.txt
run pipe ler ordenar contar
```

Nesse exemplo, `ler` recebe os dados de `nomes.txt`, `ordenar` recebe a saída de
`ler` e `contar` grava o resultado em `resultado.txt`.

### Redirecionar a entrada

```text
input <tarefa> <arquivo>
```

### Redirecionar a saída

O comando `output` substitui o conteúdo anterior do arquivo:

```text
output <tarefa> <arquivo>
```

O comando `append` acrescenta a nova saída ao final do arquivo:

```text
append <tarefa> <arquivo>
```

### Alterar o diretório de trabalho

```text
workdir <diretório>
```

As tarefas executadas posteriormente utilizarão esse diretório.

### Executar em background

```text
start <tarefa>
```

O ProcessFlow informa o identificador do job e o PID do processo:

```text
[1] 1234
```

Para listar os jobs:

```text
jobs
```

Para esperar o término de um job específico:

```text
wait <jobId>
```

Exemplo:

```text
task pausa /bin/sleep 5
start pausa
jobs
wait 1
```

### Encerrar o programa

```text
exit
```

No modo interativo, `Ctrl-D` também encerra o ProcessFlow.

## Tratamento de erros

O ProcessFlow informa o erro e continua processando quando encontra:

- Uma tarefa inexistente.
- Um programa que não pode ser executado.
- Um arquivo de entrada ou saída que não pode ser aberto.
- Um job inexistente.
- Um diretório de trabalho inválido.
- Um comando desconhecido.
- Um processo que termina com código diferente de zero.

O programa encerra com erro quando recebe argumentos demais na inicialização ou
quando o arquivo de workflow não pode ser aberto.

## Estrutura do projeto

```text
.
├── src/
│   └── main.c    # Implementação do ProcessFlow
├── .evidrc       # Configuração de Bash limpo usada somente nas evidências
├── Makefile      # Compilação, limpeza, execução e debug
└── README.md     # Documentação do projeto
```

## Conclusão pt1

Projeto para disciplina de Infraestrutura de Software.
Nome: Carlos Murilo de Oliveira Costa Filho

## Conclusão pt2

Esse não foi o maior projeto que já fiz, mas foi sem dúvida um dos mais legais. Eu raramente comento sobre projetos de faculdade, mas esse me despertou uma vontade sincera de construir um shell.

Não como um shell para competir com zsh ou bash, mas por diversão mesmo! Quem sabe até mesmo criar um plugin para o zsh. Tenho algumas ideias em mente! E gostei demais do aprendizado.
