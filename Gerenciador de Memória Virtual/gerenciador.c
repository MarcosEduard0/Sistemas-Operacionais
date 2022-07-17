#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>
#define WORKING_SET_LIMIT 4
#define TOTAL_FRAMES 20
#define TOTAL_PAGINAS 30
#define NTHREADS 8

#define RED "\x1b[31m"
#define YEL "\x1b[33m"
#define GRN "\x1b[32m"
#define CYN "\x1b[36m"
#define RST "\x1b[0m"

typedef struct No_lista_swap
{
    int pid;
    struct No_lista_swap *prox;
} No_lista_swap;
typedef struct Pagina
{
    int pid;
    int page_number;
    int memoria_index;
    short em_memoria;
    struct Pagina *prox;
    // struct Pagina *anterior;
} Pagina;

typedef struct Linha_tabela
{
    short int em_memoria;
    int frame;
} Linha_tabela;

pthread_t threads[20];
pthread_mutex_t lock_memoria = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock_prox_pid = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock_numero_frames_utilizados = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock_lista_swap = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock_print = PTHREAD_MUTEX_INITIALIZER;
sem_t semaforo_swap;
int swap = 0;
int prox_pid = 1;
int numero_frames_utilizados = 0;
int prox_swap = 1;
int processos_em_espera = 0;
int disponivel_para_swap[NTHREADS];
No_lista_swap *inicio_lista_swap, *fim_lista_swap;
Pagina *memoria[TOTAL_FRAMES];

No_lista_swap *criar_no_lista_swap(int pid)
{
    No_lista_swap *novo;
    novo = malloc(sizeof(No_lista_swap *));
    novo->pid = pid;
    novo->prox = NULL;
    return novo;
}
Pagina *criar_pagina(int pid, int numero_pagina)
{
    Pagina *novo;
    novo = (Pagina *)malloc(sizeof(Pagina *));
    novo->pid = pid;
    novo->page_number = numero_pagina;
    novo->prox = NULL;
    // novo->anterior = NULL;
    novo->em_memoria = 0;
    novo->memoria_index = -1;
    return novo;
}

Pagina *achar_ultimo_no(Pagina *inicio)
{
    Pagina *atual;
    atual = inicio;
    while (atual->prox != NULL)
    {
        atual = atual->prox;
    }
    return atual;
}

void *processo(void *arg)
{
    int id;
    int numero_paginas_em_memoria = 0;
    Pagina **tabela_paginas; // tabela de páginas
    Pagina *lru_lista, *atual;

    lru_lista = NULL;
    tabela_paginas = (Pagina **)malloc(TOTAL_PAGINAS * sizeof(Pagina *));
    pthread_mutex_lock(&lock_prox_pid);
    id = prox_pid;
    prox_pid++;
    pthread_mutex_unlock(&lock_prox_pid);

    // inicializa tabela de páginas
    for (size_t i = 0; i < TOTAL_PAGINAS; i++)
    {

        tabela_paginas[i] = criar_pagina(id, i);
    }
    printf("Processo %d criado\n", id);
    while (1)
    {

        pthread_mutex_lock(&lock_numero_frames_utilizados); // inicio de sessão crítica
        short todos_os_frames_em_uso = numero_frames_utilizados >= TOTAL_FRAMES ? 1 : 0;
        pthread_mutex_unlock(&lock_numero_frames_utilizados); // fim de sessão crítica
        pthread_mutex_lock(&lock_lista_swap);
        if ((disponivel_para_swap[id - 1] == -1 || disponivel_para_swap[id - 1] == 0) && todos_os_frames_em_uso)
        {

            lru_lista = NULL;
            int no_removido = inicio_lista_swap->pid;
            pthread_mutex_lock(&lock_memoria);
            for (size_t i = 0; i < TOTAL_FRAMES; i++)
            {
                if (memoria[i] != NULL && memoria[i]->pid == no_removido)
                {
                    memoria[i]->em_memoria = 0;
                    memoria[i]->memoria_index = -1;
                    memoria[i]->prox = NULL;
                    memoria[i] = NULL;
                }
            }
            pthread_mutex_unlock(&lock_memoria);
            disponivel_para_swap[no_removido - 1] = 0;
            No_lista_swap *temp;
            temp = inicio_lista_swap;
            inicio_lista_swap = temp->prox;
            free(temp);
        }
        pthread_mutex_unlock(&lock_lista_swap);

        int prox_pagina = rand() % TOTAL_PAGINAS; // próxima página pedida pelo processo

        Pagina *pagina = tabela_paginas[prox_pagina];
        pthread_mutex_lock(&lock_memoria);
        if (pagina->em_memoria)
        {
            pthread_mutex_lock(&lock_print);
            printf("PROCESSO %d: Pedindo alocação da pagina %d\n\n", id, prox_pagina);
            pthread_mutex_unlock(&lock_print);
            atual = lru_lista;
            if (atual->page_number != prox_pagina)
            {
                while (1)
                {

                    if (atual->prox->page_number == prox_pagina)
                    {
                        break;
                    }
                    atual = atual->prox;
                }
                // garante que o nó mais recente sempre esteja Pagina começo
                Pagina *temp;
                temp = atual->prox;
                atual->prox = temp->prox;
                temp->prox = lru_lista;
                lru_lista = temp;
            }
            pthread_mutex_unlock(&lock_memoria);
        }
        else
        {
            pthread_mutex_unlock(&lock_memoria);
            pthread_mutex_lock(&lock_numero_frames_utilizados);
            if (numero_paginas_em_memoria == 0)
            {
                numero_frames_utilizados += 4;
            }
            pthread_mutex_unlock(&lock_numero_frames_utilizados);

            // verifica se o o processo precisa ser posto na fila de swap
            if ((disponivel_para_swap[id - 1] == -1 || disponivel_para_swap[id - 1] == 0))
            {
                pthread_mutex_lock(&lock_lista_swap);
                if (inicio_lista_swap == NULL)
                {
                    inicio_lista_swap = criar_no_lista_swap(id);
                    fim_lista_swap = inicio_lista_swap;
                    disponivel_para_swap[id - 1] = 1;
                }
                else
                {
                    No_lista_swap *novo;
                    novo = criar_no_lista_swap(id);
                    fim_lista_swap->prox = novo;
                    disponivel_para_swap[id - 1] = 1;
                }
                pthread_mutex_unlock(&lock_lista_swap);
            }
            pthread_mutex_lock(&lock_print);
            printf("PROCESSO %d: Pedindo alocação da pagina %d\n\n", id, prox_pagina);
            pthread_mutex_unlock(&lock_print);
            pthread_mutex_lock(&lock_memoria);
            atual = lru_lista;

            if (atual == NULL)
            {
                // inicializa lista
                lru_lista = tabela_paginas[prox_pagina];
                // insere na memoria
                for (size_t i = 0; i < TOTAL_FRAMES; i++)
                {
                    if (memoria[i] == NULL)
                    {
                        memoria[i] = lru_lista;
                        numero_paginas_em_memoria = 1;
                        lru_lista->em_memoria = 1;
                        lru_lista->memoria_index = i;
                        break;
                    }
                }
            }
            else if (numero_paginas_em_memoria < 4)
            {

                for (size_t i = 0; i < TOTAL_FRAMES; i++)
                {
                    if (memoria[i] == NULL)
                    {
                        memoria[i] = tabela_paginas[prox_pagina];
                        numero_paginas_em_memoria++;
                        tabela_paginas[prox_pagina]->em_memoria = 1;
                        tabela_paginas[prox_pagina]->memoria_index = i;
                        break;
                    }
                }
                tabela_paginas[prox_pagina]->prox = lru_lista;
                lru_lista = tabela_paginas[prox_pagina];
            }
            else
            {
                while (1)
                {
                    // acha o penultimo
                    if (atual->prox->prox == NULL)
                    {
                        break;
                    }
                    atual = atual->prox;
                }
                Pagina *antigo;
                antigo = atual->prox;
                atual->prox = NULL;
                antigo->em_memoria = 0; // marca o nó a ser retirado como fora da memória
                antigo->prox = NULL;
                int index;
                if (antigo->memoria_index != -1)
                {
                    index = antigo->memoria_index;
                }
                else
                {
                    for (size_t i = 0; i < TOTAL_FRAMES; i++)
                    {
                        if (memoria[i] == NULL)
                        {
                            index = i;
                            break;
                        }
                    }
                }
                tabela_paginas[prox_pagina]->memoria_index = index;
                antigo->memoria_index = -1;
                memoria[index] = tabela_paginas[prox_pagina];
                tabela_paginas[prox_pagina]->em_memoria = 1; // marca o novo nó como em memória
                tabela_paginas[prox_pagina]->prox = lru_lista;
                lru_lista = tabela_paginas[prox_pagina];
            }
            pthread_mutex_unlock(&lock_memoria);
        }
        Pagina *aux = lru_lista;
        pthread_mutex_lock(&lock_print);
        pthread_mutex_lock(&lock_memoria);

        printf("LRU: [");
        while (aux != NULL)
        {
            printf("%d", aux->page_number);
            aux = aux->prox;
            if (aux == NULL)
                printf("]\n\n");
            else
                printf(", ");
        }

        printf("\n|\t\tMEMORIA \t\t|\n|---------------------------------------|\n");
        for (int i = 0; i < TOTAL_FRAMES; i++)
        {
            if (memoria[i] == NULL)
            {
                printf("|%d:\t\tvazio\t\t\t|\n", i);
            }
            else
            {
                printf("|%d:\t   [Pid: %d, Page: %d]\t\t|\n", i, memoria[i]->pid, memoria[i]->page_number);
            }
        }

        printf("\n\t    Tabela de Paginas: \n|---------------------------------------|\n");
        for (int i = 0; i < TOTAL_PAGINAS; i++)
            if (tabela_paginas[i]->em_memoria == 0)
                printf("|%d: \t\tLivre\t\t\t|\n", tabela_paginas[i]->page_number);
            else
                printf("|%d: \t\tEm memoria\t\t|\n", tabela_paginas[i]->page_number);
        printf("\n\n");
        pthread_mutex_unlock(&lock_memoria);

        printf("SWAP: [");
        for (int i = 0; i < NTHREADS; i++)
        {
            pthread_mutex_lock(&lock_lista_swap);
            if (disponivel_para_swap[i] == 0)
            {
                printf("processo %d, ", i + 1);
            }
            pthread_mutex_unlock(&lock_lista_swap);
        }
        printf("]\n\n");
        pthread_mutex_unlock(&lock_print);

        sleep(3);
    }
}

int main(int argc, char const *argv[])
{
    sem_init(&semaforo_swap, 0, 0);
    inicio_lista_swap = NULL;
    pthread_t *threads;
    srand(time(NULL));

    for (size_t i = 0; i < TOTAL_FRAMES; i++)
    {
        memoria[i] = NULL;
    }
    for (size_t i = 0; i < NTHREADS; i++)
    {
        disponivel_para_swap[i] = -1;
    }

    // Alocano memoria
    if ((threads = malloc(sizeof(pthread_t) * NTHREADS)) == NULL)
    {
        printf("ERRO-- malloc\n");
        return 1;
    }

    // cria as threads Processo
    for (int i = 0; i < NTHREADS; i++)
    {
        if (pthread_create(&threads[i], NULL, processo, NULL))
            exit(-1);
        sleep(3);
    }

    // Espera todas as threads completarem
    for (int i = 0; i < NTHREADS; i++)
        pthread_join(threads[i], NULL);

    free(threads);
    pthread_exit(NULL);

    return 0;
}
