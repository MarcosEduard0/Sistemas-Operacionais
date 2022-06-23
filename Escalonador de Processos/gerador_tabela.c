#include <stdio.h>
#include <stdlib.h>
#include <time.h>

const int MAX_TEMPO_CHEGADA = 15;
const int MAX_NUMERO_PROCESSOS = 5;
const int MIN_NUMERO_PROCESSOS = 3;
const int MAX_TEMPO_EXECUCAO = 30;
const int MIN_TEMPO_EXECUCAO = 1;
const int MAX_OPERACOES_IO = 5;
const int MIN_OPERACOES_IO = 0;
#define SIMBOLO_DISCO 'D'
#define SIMBOLO_FITA 'F'
#define SIMBOLO_IMPRESSORA 'I'
char DISPOSITIVOS_IO[] = {SIMBOLO_DISCO, SIMBOLO_FITA, SIMBOLO_IMPRESSORA};
int numberInArray(int *vetor, int elemento, int tamanhoVetor)
{
    for (size_t i = 0; i < tamanhoVetor; i++)
    {
        if (vetor[i] == elemento)
        {
            return 1;
        }
    }
    return 0;
}
int setNumeroProcesso()
{

    return (rand() % (MAX_NUMERO_PROCESSOS - MIN_NUMERO_PROCESSOS + 1)) + MIN_NUMERO_PROCESSOS;
}
int setTempoExecucao()
{
    return (rand() % MAX_TEMPO_EXECUCAO) + MIN_TEMPO_EXECUCAO;
}
int setNumeroIO(int tempoExecucao)
{
    return (rand() % (MAX_OPERACOES_IO + 1)) + MIN_OPERACOES_IO;
}
int *setIntantesIO(int TempoExecucao, int numeroIO)
{
    int *InstantesIO, instante;
    InstantesIO = malloc(numeroIO * sizeof(int));
    for (size_t i = 0; i < numeroIO; i++)
    {
        instante = (rand() % (TempoExecucao - 1)) + 1;
        if (numberInArray(InstantesIO, instante, numeroIO))
        {
            i--;
            continue;
        }
        InstantesIO[i] = instante;
    }

    return InstantesIO;
}
char *setIO(int numeroIO)
{
    char *operacoesIO;
    operacoesIO = malloc(numeroIO * sizeof(char));
    ;
    for (size_t i = 0; i < numeroIO; i++)
    {
        operacoesIO[i] = DISPOSITIVOS_IO[rand() % 3];
    }
    return operacoesIO;
}
void printIntArrayOnFile(int *array, FILE *arquivo, int tamanhoVetor)
{
    if (tamanhoVetor == 0)
    {
        fprintf(arquivo, "[]");
        return;
    }

    fprintf(arquivo, "[");
    for (size_t i = 0; i < tamanhoVetor; i++)
    {

        if (!(i == tamanhoVetor - 1))
        {
            fprintf(arquivo, "%d,", array[i]);
        }
        else
        {
            fprintf(arquivo, "%d]", array[i]);
        }
    }
}
void printCharArrayOnFile(char *array, FILE *arquivo, int tamanhoVetor)
{
    if (tamanhoVetor == 0)
    {
        fprintf(arquivo, "[]");
        return;
    }

    fprintf(arquivo, "[");
    for (size_t i = 0; i < tamanhoVetor; i++)
    {

        if (!(i == tamanhoVetor - 1))
        {
            fprintf(arquivo, "%c,", array[i]);
        }
        else
        {
            fprintf(arquivo, "%c]", array[i]);
        }
    }
}
int main(int argc, char const *argv[])
{
    int numeroProcessos, processoPid, tempoExecucao, numeroIO;
    FILE *tabela;
    srand(time(NULL));
    tabela = fopen("tabela.txt", "w");
    numeroProcessos = setNumeroProcesso();
    fprintf(tabela, "%d\n", numeroProcessos);
    for (size_t i = 0; i < numeroProcessos; i++)
    {

        fprintf(tabela, "%d", i + 1);
        fprintf(tabela, " %d ", tempoExecucao = setTempoExecucao());
        numeroIO = setNumeroIO(tempoExecucao);
        fprintf(tabela, "%d ", numeroIO);
        printCharArrayOnFile(setIO(numeroIO), tabela, numeroIO);
        fprintf(tabela, " ");
        printIntArrayOnFile(setIntantesIO(tempoExecucao, numeroIO), tabela, numeroIO);
        fprintf(tabela, " %d", rand() % MAX_TEMPO_CHEGADA);
        fprintf(tabela, "\n");
    }
    fclose(tabela);

    return 0;
}
