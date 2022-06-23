
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#define QUANTUN 3
#define SIMBOLO_DISCO 'D'
#define SIMBOLO_FITA 'F'
#define SIMBOLO_IMPRESSORA 'I'
#define TEMPO_DISCO 2
#define TEMPO_FITA 6
#define TEMPO_IMPRESSORA 12
FILE *out;
typedef struct p
{
    int pid;
    int tempoExecucao;
    char *operacoesIO;
    int *instantesIO;
    int tempoChegada;
    int numeroIO;
    int bloqueado;
    int tempoVoltaIO;
    int tempoCPU;
    int proxOperacaoIO;

} processo;
typedef struct Fila
{
    processo **array;
    int primeiro, ultimo;
    unsigned tamanho, maxTamanho;
} Fila;

// Cria uma fila de tamanho maxTamanho
Fila *criar_fila(unsigned maxTamanho);

// Verifica se a fila esta cheia
int fila_cheia(Fila *fila);

// Verifica se a fila esta vazia
int fila_vazia(Fila *fila);

// Retorna o primeiro elemento da fila.
processo *fila_get(Fila *fila);

// Coloca um elemento no final da fila.
void fila_add(Fila *fila, processo *item);

// Tira o elemento do início da fila e o retorna.
processo *fila_pop(Fila *fila);

Fila *criar_fila(unsigned maxTamanho)
{
    Fila *fila = (Fila *)malloc(sizeof(Fila));

    fila->primeiro = fila->tamanho = 0;
    fila->ultimo = maxTamanho - 1; // inicializando assim, se tornará 0 na primeira chamada de "add".
    fila->maxTamanho = maxTamanho;

    fila->array = (processo **)malloc(maxTamanho * sizeof(processo *));
    return fila;
}

int fila_cheia(Fila *fila)
{
    return (fila->tamanho == fila->maxTamanho);
}

int fila_vazia(Fila *fila)
{
    return !(fila->tamanho);
}

processo *fila_get(Fila *fila)
{
    return fila->array[fila->primeiro];
}

void fila_add(Fila *fila, processo *item)
{
    if (!fila_cheia(fila))
    {
        fila->ultimo = (fila->ultimo + 1) % fila->maxTamanho;
        fila->array[fila->ultimo] = item;
        fila->tamanho++;
    }
}

processo *fila_pop(Fila *fila)
{
    processo *item = fila_get(fila);
    fila->primeiro = (fila->primeiro + 1) % fila->maxTamanho;
    fila->tamanho--;
    return item;
}

int *parseInstantesIO(char *instantesString, int numeroIO)
{
    int i = 0, j = 0, l = 0, *instantesInt;
    char *numero;
    numero = (char *)malloc(3 * sizeof(char)); // a string esta preparada a priori pra receber no maximo 3 digitos
    if (numeroIO == 0)
    {
        return NULL;
    }
    instantesInt = (int *)malloc(numeroIO * sizeof(int));
    while (instantesString[i] != '\0')
    {

        if (instantesString[i] == '[' || instantesString[i] == ']' || instantesString[i] == ',')
        {
            i++;
            continue;
        }
        else
        {
            numero[j] = instantesString[i];
            j++;

            while (instantesString[i + 1] != '\0' && isdigit(instantesString[i + 1]))
            {

                numero[j] = instantesString[i + 1];
                j++;
                i++;
            }
        }
        instantesInt[l] = atoi(numero);
        l++;
        i++;
    }
    return instantesInt;
}

char *parseOperacoesIO(char *token, int numeroIO)
{
    int i = 0, j = 0;
    char *operacoesIO;
    operacoesIO = (char *)malloc((numeroIO + 1) * sizeof(char));
    while (j < numeroIO)
    {
        if (token[i] == '[' || token[i] == ',' || token[i] == ']')
        {
            i++;
            continue;
        }
        operacoesIO[j] = token[i];
        j++;
        i++;
    }
    operacoesIO[numeroIO] = '\0';
    return operacoesIO;
}
// Inicializa os atributos de um processo
processo *setProcesso(char *linhaTabela)
{
    processo *p;
    char *token;
    int i = 0;
    p = (processo *)malloc(sizeof(processo));
    token = strtok(linhaTabela, " ");
    while (token != NULL)
    {
        switch (i)
        {
        case 0:

            p->pid = atoi(token);
            break;
        case 1:

            p->tempoExecucao = atoi(token);
            break;
        case 2:
            p->numeroIO = atoi(token);
            break;
        case 3:
            p->operacoesIO = parseOperacoesIO(token, p->numeroIO);
            break;
        case 4:
            p->instantesIO = parseInstantesIO(linhaTabela, p->numeroIO);
            break;
        case 5:
            p->tempoChegada = atoi(token);
            break;
        default:
            printf("ERRO PARSE INSTANTES");
            break;
        }
        token = strtok(NULL, " ");
        i++;
    }
    p->bloqueado = 0;
    p->tempoVoltaIO = 0;
    p->tempoCPU = 0;
    p->proxOperacaoIO = 0;
    return p;
}

void bubbleSort(processo **vetor, int tam)
{
    if (tam < 1)
    {
        return;
    }
    for (size_t i = 0; i < tam - 1; i++)
    {
        if (vetor[i]->tempoChegada > vetor[i + 1]->tempoChegada)
        {

            processo *temp = vetor[i];
            vetor[i] = vetor[i + 1];
            vetor[i + 1] = temp;
        }
        bubbleSort(vetor, tam - 1);
    }
}

// Pega os processos descritos na tabela e os coloca num tipo de área de espera
processo **parseTabela(int *numeroProcessos)
{
    FILE *tabela = fopen("tabela.txt", "r");
    char *linha = NULL;
    short int flag = 1; // marca se a primeira linha da tabela ainda não foi lida
    int i = 0;
    size_t tam_linha = 0;
    processo **espera;
    if (tabela == NULL)
    {
        printf("Tabela não encontrada \n");
        return NULL;
    }

    while (getline(&linha, &tam_linha, tabela) != -1)
    {
        if (flag)
        {
            *numeroProcessos = atoi(linha);
            espera = (processo **)malloc(*numeroProcessos * sizeof(processo *)); // cria a área de espera
            flag = 0;
            continue;
        }
        espera[i] = setProcesso(linha);
        i++;
    }

    fclose(tabela);

    bubbleSort(espera, *numeroProcessos);
    return espera;
}

// Representa a chegada de um processo ao escalonador
void buscaEspera(processo **areaEspera, Fila *altaPrioridade, int tempoAtual, int *proxEspera, int numeroProcessos)
{

    if (*proxEspera < numeroProcessos)
    {

        while (1)
        {

            processo *primeiroEspera = areaEspera[*proxEspera]; // primeiro processo na área de espera
            if (primeiroEspera->tempoChegada == tempoAtual)
            {

                fprintf(out, "\tProcesso %d chegou na fila de alta prioridade\n", primeiroEspera->pid);
                fila_add(altaPrioridade, primeiroEspera);
                (*proxEspera)++;
                if (*proxEspera < numeroProcessos)
                {
                    continue;
                }
            }
            break;
        }
    }
}

// Retira os processos prontos de uma fila de IO e os coloca na sua fila de retorno designada
void buscaIO(Fila *filaIO, Fila *filaVolta, int tempoAtual)
{

    if (fila_vazia(filaIO))
    {
        return;
    }
    processo *primeiroFila;
    while (1)
    {
        primeiroFila = fila_get(filaIO);
        if (primeiroFila->tempoVoltaIO == tempoAtual)
        {
            fprintf(out, "\tProcesso %d voltou de uma fila de IO\n", primeiroFila->pid);
            fila_add(filaVolta, fila_pop(filaIO));
        }
        break;
    }
}

// Verifica em todas as filas de IO quem está pronto para o retorno
void buscaIOAll(Fila *discoFila, Fila *fitaFila, Fila *impressoraFila, Fila *altaPrioridade, Fila *baixaPrioridade, int tempAtual)
{
    buscaIO(discoFila, baixaPrioridade, tempAtual);
    buscaIO(fitaFila, altaPrioridade, tempAtual);
    buscaIO(impressoraFila, altaPrioridade, tempAtual);
}

// Escalona o próximo processo
processo *proximoEscalonamento(Fila *altaPrioridade, Fila *baixaPrioridade)
{
    if (!fila_vazia(altaPrioridade))
    {

        fprintf(out, "\tProcesso %d foi escalonado da fila de alta prioridade\n", fila_get(altaPrioridade)->pid);
        return fila_pop(altaPrioridade);
    }
    else if (!fila_vazia(baixaPrioridade))
    {
        fprintf(out, "\tProcesso %d foi escalonado da fila de baixa prioridade\n", fila_get(baixaPrioridade)->pid);
        return fila_pop(baixaPrioridade);
    }
    return NULL;
}

// Verifica se o processo na CPU terminou sua execução
int isTerminado(processo *cpu, int *processosFinalizados)
{
    cpu->tempoCPU++;
    int isTerminado = cpu->tempoCPU == cpu->tempoExecucao ? 1 : 0;
    if (isTerminado)
    {

        (*processosFinalizados)++;
        fprintf(out, "\tProcesso %d terminou sua execução\n", cpu->pid);
        return 1;
    }
    return 0;
}

// Retira o processo da CPU e o coloca numa fila de IO
void executarOperacaoIO(processo *cpu, int tempoAtual, int tempoOperacao, Fila *filaIO)
{
    cpu->tempoVoltaIO = tempoAtual + tempoOperacao;
    fila_add(filaIO, cpu);
    fprintf(out, " ate o instante %d\n", cpu->tempoVoltaIO);
}

// Trata o pedido de IO de um processo
void handleIO(processo *cpu, int tempoAtual, Fila *discoFila, Fila *fitaFila, Fila *impressoraFila)
{

    char tipoOperacao = cpu->operacoesIO[cpu->proxOperacaoIO];
    switch (tipoOperacao)
    {
    case SIMBOLO_DISCO:
        fprintf(out, "\tProcesso %d saiu para fazer uma operacao de disco", cpu->pid);
        executarOperacaoIO(cpu, tempoAtual, TEMPO_DISCO, discoFila);
        break;
    case SIMBOLO_FITA:
        fprintf(out, "\tProcesso %d saiu para fazer uma operacao de fita", cpu->pid);
        executarOperacaoIO(cpu, tempoAtual, TEMPO_FITA, fitaFila);
        break;
    case SIMBOLO_IMPRESSORA:
        fprintf(out, "\tProcesso %d saiu para fazer uma operacao de impressora", cpu->pid);
        executarOperacaoIO(cpu, tempoAtual, TEMPO_IMPRESSORA, impressoraFila);
        break;
    default:
        printf("ERRO: OPERAÇÃO DESCONHECIDA");
        break;
    }
}
// Verifica se eu processo faz IO nesse instante
int fezIO(processo *cpu, Fila *discoFila, Fila *fitaFila, Fila *impressoraFila, int tempoAtual)
{
    int podeFazerIO = cpu->numeroIO > cpu->proxOperacaoIO ? 1 : 0;

    if (podeFazerIO)
    {
        int fazIOAgora = cpu->tempoCPU == cpu->instantesIO[cpu->proxOperacaoIO];
        if (fazIOAgora)
        {
            handleIO(cpu, tempoAtual, discoFila, fitaFila, impressoraFila);
            return 1;
        }
    }
    return 0;
}

int sofreuTimeOut(processo *cpu, Fila *baixaPrioridade)
{
    int sofreTimeout = !(cpu->tempoCPU % QUANTUN);
    if (sofreTimeout)
    {
        fprintf(out, "\tProcesso %d sofreu timeout\n", cpu->pid);
        fila_add(baixaPrioridade, cpu);
    }

    return sofreTimeout;
}

// Gerencia o uso da CPU
processo *verificaCPU(processo *cpu, Fila *altaPrioridade, Fila *baixaPrioridade, Fila *discoFIla, Fila *fitaFila, Fila *impressoraFila, int tempoAtual, int *processoFinalizados)
{
    if (cpu == NULL || isTerminado(cpu, processoFinalizados) || sofreuTimeOut(cpu, baixaPrioridade) || fezIO(cpu, discoFIla, fitaFila, impressoraFila, tempoAtual))
    {
        // printf("%d",cpu==NULL);
        cpu = proximoEscalonamento(altaPrioridade, baixaPrioridade);
    }
    return cpu;
}
void escalonador()
{
    int numeroProcessos, proxEspera = 0, tempoAtual = 0, processosFinalizados = 0;
    processo **areaEspera, *cpu;                                                    // onde guardamos os processos que ainda não foram escalonados
    Fila *altaPrioridade, *baixaPrioridade, *discoFila, *impressoraFila, *fitaFila; // filas existentes no sistema
    out = fopen("out.txt", "w");
    cpu = NULL;

    areaEspera = parseTabela(&numeroProcessos);

    // Criação das filas
    altaPrioridade = criar_fila(numeroProcessos);
    baixaPrioridade = criar_fila(numeroProcessos);
    discoFila = criar_fila(numeroProcessos);
    impressoraFila = criar_fila(numeroProcessos);
    fitaFila = criar_fila(numeroProcessos);

    do
    {
        fprintf(out, "Instante %d:\n", tempoAtual);
        // Chegada de novos processos

        buscaEspera(areaEspera, altaPrioridade, tempoAtual, &proxEspera, numeroProcessos);

        // Chegada de processos voltando do IO
        buscaIOAll(discoFila, fitaFila, impressoraFila, altaPrioridade, baixaPrioridade, tempoAtual);
        // Gerenciamento da CPU
        cpu = verificaCPU(cpu, altaPrioridade, baixaPrioridade, discoFila, fitaFila, impressoraFila, tempoAtual, &processosFinalizados);
        tempoAtual++;
    } while (processosFinalizados < numeroProcessos);

    fclose(out);
}
int main(int argc, char const *argv[])
{
    escalonador();
    return 0;
}
