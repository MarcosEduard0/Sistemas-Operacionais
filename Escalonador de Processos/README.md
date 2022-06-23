# Escalonador de Processos Round-Robin com Feedback

Trabalho desenvolvido para a disciplina de Sistemas Operacionais.

## Round-Robin

O Round-Robin é um algoritmo escalonador de processos que consiste em dividir o tempo de uso da CPU. Cada processo recebe uma fatia de tempo, esse tempo é chamado Time-Slice, também conhecido por Quantum. Os processos são todos armazenados em Fila (Buffer) circular. O escalonador executa cada tarefa pelo tempo determinado pelo Time-Slice e ao fim deste período é executada a troca de contexto, onde o próximo processo fila passa a ser executado pela CPU até percorrer o período do Time-Slice. Após percorrer todos os processos da fila, essas atividades se repetem e o escalonador aponta para a primeira tarefa. A figura a seguir ilustra bem todo esse processo.

## Criação da tabela

A fim de ter uma simulação mais próxima da realidade e cumprir com um certo nível de aleatoriedade pedido no trabalho, resolvemos criar nossa tabela de processos através de um programa separado que seguindo algumas restrições gera uma tabela de forma randômica. Essas restrições são parâmetros definidos com um certo intervalo de valores para eles, tendo assim para a criação de tabelas:

- Tempo de Chegada sendo no máximo 15;
- Número de Processos sendo no máximo 5 e no mínimo 3;
- Tempo de Execução sendo no máximo 30 e no mínimo 1;
- Quantidade de operações de IO por processo sendo no máximo 5;
  A tabela é escrita em um arquivo separado que vai ser lido pelo código do escalonador e tem a seguinte forma:

A primeira linha contém a quantidade de processos gerados. Cada linha seguinte contém as informações de um processo sendo essas informações:

- Identificação do processo
- Tempo de execução
- Quantidade de IO
- Tipos de IO
- Tempo de execução do IO (quando ele é executado)
- Tempo de chegada do processo.

## Execução do Escalonador

Na execução do escalonador temos ainda a definição de mais três parâmetros, a quantidade do quantum e os tempos de execução de cada operação de IO. Os valores decididos são arbitrários, os tempos de IO seguem uma lógica de velocidade, e o quantum baseado em exercícios já feitos.
| Parâmetro | Tempo |
| ------- | ------- |
| Quantum| 3 |
| Disco| 2 |
| Fita| 6 |
| Impressora| 12 |

Tabela 1: Definição do tempo de cada parâmetro.
Tendo a tabela e esses parâmetros definidos podemos agora realizar escalonamento dos processos. Os processos lidos da tabela são postos numa área de espera até que seja o momento de simulação de sua chegada. Essa área está sempre ordenada crescentemente .Nosso escalonamento é explicitado através de descrições das ações tomadas em cada instante de tempo e armazenado em um arquivo separado, portanto a saída do nosso escalonador é um arquivo contendo um log separado em instantes e as ações que ocorreram neles. As ações possíveis são as seguintes:

- Processo X chegou na fila de alta prioridade
- Processo X foi escalonado da fila de alta prioridade
- Processo X foi escalonado da fila de baixa prioridade
- Processo X sofreu timeout
- Processo X saiu para fazer uma operação de Y até o instante Z
- Processo X voltou de uma fila de IO
- Processo X terminou sua execução
  Decidimos deixar no log todos os instantes, mesmo aqueles em que não temos nenhuma ação pois julgamos que facilita o entendimento das ações e a visualização geral de como está se comportando o escalonador.
