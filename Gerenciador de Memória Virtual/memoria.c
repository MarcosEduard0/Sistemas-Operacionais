#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#define QUANTUN 3

//---------- NOVO ------------
#define WORKING_SET_LIMIT 4

int memoria[64][2];

FILE *out;
typedef struct p
{
    int pid;
    int tempoExecucao;
    int tempoChegada;
    int bloqueado;
    int tempoCPU;

    // ----------------- NOVO ---------------
    int num_pag_virtuais;               // Quantidade de páginas virtuais que o processo possui
    int page_table[4][4];               // Guarda os endereços reais e virtuais das páginas
    int working_set[WORKING_SET_LIMIT]; // Paginas do processo atualmente carregadas
    int *paginas_virtuais;              // Todas as páginas virtuais do processo

} processo;
typedef struct Fila
{
    processo **array;
    int primeiro, ultimo;
    unsigned tamanho, maxTamanho;
} Fila;

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
    srand((unsigned)time(NULL) * *linhaTabela);

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
        case 3:
        case 4:
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
    p->tempoCPU = 0;

    // --------------------- NOVO--------------------
    p->num_pag_virtuais = ((rand() % 64) + 1);
    p->paginas_virtuais = (int *)malloc(sizeof(int) * (p->num_pag_virtuais));

    for (i = 0; i < p->num_pag_virtuais; i++)
    {
        p->paginas_virtuais[i] = i;
    }
    for (i = 0; i < 4; i++)
    {
        p->page_table[i][0] = i;
        p->page_table[i][1] = -1;
        p->page_table[i][2] = -1;
        p->page_table[i][2] = -1;
    }
    printf("Processo de PID %d com %d paginas\n", p->pid, p->num_pag_virtuais);
    printf("\n================================================\n\n");
    return p;
}

//---------------------------------------------------------- NOVO -----------------------------------------------------
void getPage(processo *p, int ut)
{
    int pagina_escolhida = rand() % p->num_pag_virtuais;
    printf("Pagina a ser alocada: %d\n", pagina_escolhida);
    int i;
    int condicao = 0;

    for (i = 0; i < 4; i++)
    {
        if (p->page_table[i][1] == pagina_escolhida)
        {
            p->page_table[i][3] = ut;
            // nao faz nada
            condicao = 1;
            i = 4;
        }
    }
    if (condicao == 0)
    {
        for (i = 0; i < 4; i++)
        {
            if (p->page_table[i][1] == -1)
            {
                // pega nova página e bota nesse local vazio
                p->page_table[i][1] = pagina_escolhida;
                p->page_table[i][3] = ut;
                condicao = 1;
                i = 4;
            }
        }
        if (condicao == 0)
        {
            int menor_ut = p->page_table[0][3];
            int id_lru = 0;
            // verifica qual o lru
            for (i = 0; i < 4; i++)
            {
                if (menor_ut > p->page_table[i][3])
                {
                    id_lru = i;
                    menor_ut = p->page_table[i][3];
                }
            }
            // troca as paginas
            p->page_table[id_lru][1] = pagina_escolhida;
            p->page_table[id_lru][3] = ut;
        }
    }

    printf("\n");
    printf("Page Table do Processo Ativo\n");
    printf("Posicao | Pagina | Posicao na Memoria Real | Momento Referenciado\n");
    for (int k = 0; k < 4; k++)
    {
        for (int j = 0; j < 4; j++)
        {
            printf("%d | ", p->page_table[k][j]);
        }
        printf("\n");
    }
    printf("\n");
}

int getIdProcesso(int pid, processo *lista_processos, int numeroProcessos)
{
    int id;
    for (id = 0; id < numeroProcessos; id++)
    {
        if (lista_processos[id].pid == pid)
        {
            return id;
        }
    }
}

void setMemoria()
{
    for (int i = 0; i < 64; i++)
    {
        memoria[i][0] = -1;
        memoria[i][1] = -1;
    }
}

void swap_out(processo *p)
{
    int local = 0;
    for (int i = 0; i < 4; i++)
    {
        if (p->page_table[i][2] != -1)
        {
            local = p->page_table[i][2];
            memoria[local][0] = -1;
            memoria[local][1] = -1;
            p->page_table[i][2] = -1;
        }
    }
    printf("Removendo todas as paginas do processo %d da memoria real.\n", p->pid);
}

int acharVagaMemoria()
{
    for (int j = 0; j < 64; j++)
        if (memoria[j][0] == -1)
            return j;

    return -1;
}

int arranjarVaga(int pid)
{
    int flag = -1, vaga = -1;
    while (vaga < 0)
    {
        vaga = (rand() % 64);
        if (memoria[vaga][0] != pid)
            flag = 1;
    }
    return vaga;
}

void swap_in(processo *p, processo **lista_processos, int numeroProcessos)
{
    int i = 0, j = 0, pidAnterior, flag = 0;
    // vai tentar alocar todas as paginas
    for (i = 0; i < 4; i++)
    {
        // se alguma nao estiver alocada
        if (p->page_table[i][2] == -1 && p->page_table[i][1] != -1)
        {
            flag = 1;
            // tenta achar vaga vazia na memoria
            int vaga = acharVagaMemoria();
            // se nao houver vaga na memoria
            if (vaga == -1)
            {
                // procura uma vaga pra ocupar no lugar de uma pagina de outro processo
                vaga = arranjarVaga(p->pid);
                pidAnterior = memoria[vaga][0];
                processo *anterior = lista_processos[getIdProcesso(pidAnterior, lista_processos, numeroProcessos)];
                for (j = 0; j < 4; j++)
                {
                    if (memoria[vaga][1] == anterior->page_table[j][1])
                    {
                        anterior->page_table[j][2] = -1;
                        printf("A pagina %d do processo %d foi removida do espaco %d da memoria real.\n", anterior->page_table[j][1], anterior->pid, vaga);
                        j = 4;
                    }
                }
            }
            p->page_table[i][2] = vaga;
            memoria[vaga][0] = p->pid;
            memoria[vaga][1] = p->page_table[i][1];
            printf("A pagina %d do processo %d foi alocada no espaco %d da memoria real.\n", p->page_table[i][1], p->pid, vaga);
        }

        else if (p->page_table[i][2] != -1)
        {
            int pos = p->page_table[i][2];
            if (memoria[pos][2] != p->page_table[i][1])
            {
                memoria[pos][2] = p->page_table[i][1];
                printf("Espaco %d da memoria real foi atualizada com a pagina %d do processo %d.\n", pos, p->page_table[i][1], p->pid);
            }
        }
    }
    if (flag == 1)
    {
        int k;
        printf("\n");
        printf("Page Table do Processo Ativo\n");
        printf("Posicao | Pagina | Posicao na Memoria Real | Momento Referenciado\n");
        for (k = 0; k < 4; k++)
        {
            for (j = 0; j < 4; j++)
            {
                printf("%d | ", p->page_table[k][j]);
            }
            printf("\n");
        }
        printf("\n");
    }
}

//-----------------------------------------------------------------FIM NOVO-----------------------------------------------------
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
processo *verificaCPU(processo *cpu, Fila *altaPrioridade, Fila *baixaPrioridade, int tempoAtual, int *processoFinalizados)
{
    if (cpu == NULL || isTerminado(cpu, processoFinalizados) || sofreuTimeOut(cpu, baixaPrioridade))
    {
        cpu = proximoEscalonamento(altaPrioridade, baixaPrioridade);
    }
    return cpu;
}
void escalonador()
{
    int numeroProcessos, proxEspera = 0, tempoAtual = 0, processosFinalizados = 0;
    processo **areaEspera, *cpu;            // onde guardamos os processos que ainda não foram escalonados
    Fila *altaPrioridade, *baixaPrioridade; // filas existentes no sistema
    out = fopen("out.txt", "w");
    cpu = NULL;

    areaEspera = parseTabela(&numeroProcessos);

    // Criação das filas
    altaPrioridade = criar_fila(numeroProcessos);
    baixaPrioridade = criar_fila(numeroProcessos);

    // NOVO
    setMemoria();

    do
    {
        fprintf(out, "Instante %d:\n", tempoAtual);
        // Chegada de novos processos

        buscaEspera(areaEspera, altaPrioridade, tempoAtual, &proxEspera, numeroProcessos);

        // Gerenciamento da CPU
        cpu = verificaCPU(cpu, altaPrioridade, baixaPrioridade, tempoAtual, &processosFinalizados);
        if (tempoAtual % 3 == 0)
        {
            getPage(cpu, tempoAtual);
            swap_in(cpu, areaEspera, numeroProcessos);
        }
        tempoAtual++;
    } while (processosFinalizados < numeroProcessos);

    fclose(out);
}
int main(int argc, char const *argv[])
{
    escalonador();
    return 0;
}
