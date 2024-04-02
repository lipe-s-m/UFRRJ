
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

// Definições de constantes para o tamanho máximo do grafo e operações de bits
#define MAX_VERTICES 4096
#define LOG_MAX_VERTICES 12
#define TAM_NACO 32 				
#define LOG_TAM_NACO 5

#define MAX_V_NACOS (MAX_VERTICES >> LOG_TAM_NACO)
#define LOG_MAX_V_NACOS (LOG_MAX_VERTICES - LOG_TAM_NACO)
#define MAX_ADJ_NACOS (MAX_VERTICES << LOG_MAX_V_NACOS)

#define N_NACOS(n) (((n >> LOG_TAM_NACO) << LOG_TAM_NACO) == n ? (n >> LOG_TAM_NACO) : ((n >> LOG_TAM_NACO)+1))
#define IDX_VIZ(v) (v << LOG_MAX_V_NACOS)
#define IDX_NACO(v) (v >> LOG_TAM_NACO)
#define IDX_EM_NACO(v) (v - (IDX_NACO(v) << LOG_TAM_NACO))

#define NACO(a,u,v) ((a+IDX_VIZ(u))[IDX_NACO(v)])

unsigned int R[MAX_ADJ_NACOS];

// Definição de um tipo de estrutura de dados para a lista encadeada
typedef struct No {
    int valor;
    struct No* prox;
} No;

// Função para inicializar a lista encadeada com os vértices iniciais
No* partida(unsigned int *R) {
    No* n = NULL;
    for (int i = 0; i < MAX_VERTICES; i++) {
        if (R[IDX_NACO(i)] & (1 << IDX_EM_NACO(i))) {
            No* novo = (No*)malloc(sizeof(No));
            novo->valor = i;
            novo->prox = n;
            n = novo;
        }
    }
    return n;
}

// Função para verificar se a lista encadeada chegou ao fim
bool chegada(No* r) {
    return r == NULL;
}

// Função para remover um vértice da lista encadeada
void retirar(No** r) {
    No* temp = *r;
    *r = (*r)->prox;
    free(temp);
}

// Função para verificar se um vértice pertence ao subconjunto
bool subconj(unsigned int *R, No* v) {
    return R[IDX_NACO(v->valor)] & (1 << IDX_EM_NACO(v->valor));
}

// Função para obter a lista de vizinhos de um vértice
void vizinhos(unsigned int adjMatrix[MAX_ADJ_NACOS], int vertice, No** p, No** q) {
    *p = partida(adjMatrix + IDX_VIZ(vertice));
    *q = NULL;
    for (int i = 0; i < MAX_VERTICES; i++) {
        if (NACO(adjMatrix,vertice,i) & (1 << IDX_EM_NACO(i))) {
            No* novo = (No*)malloc(sizeof(No));
            novo->valor = i;
            novo->prox = *q;
            *q = novo;
        }
    }
}

// Função para avançar na lista encadeada
void avancar(No** p) {
    No* temp = *p;
    *p = (*p)->prox;
    free(temp);
}

// Função para verificar se um subconjunto define uma clique no grafo
bool verificaClique(int numVertices, unsigned int adjMatrix[MAX_ADJ_NACOS], unsigned int subset[MAX_V_NACOS]) {
    memcpy(R, adjMatrix, N_NACOS(numVertices) * sizeof(unsigned int));
    No * r = partida(R);
    while (!chegada(r)) {
        retirar(&r);
        if (!subconj(subset, r))
            return false;
        No * p, * q;
        vizinhos(adjMatrix, r->valor, &p, &q);
        while (q != NULL) {
            if (subconj(subset, q)) {
                avancar(&q);
            } else {
                break;
            }
        }
        if (q != NULL) {
            return false;
        }
    }
    return true;
}

// Função para medir o tempo de execução médio da função verificaClique
double medirTempoExecucao(int numVertices, unsigned int adjMatrix[MAX_ADJ_NACOS], unsigned int subset[MAX_V_NACOS], int repeticoes) {
    clock_t inicio, fim;

    inicio = clock();
    for (int i = 0; i < repeticoes; i++)
        verificaClique(numVertices, adjMatrix, subset);
    fim = clock();
    double tempoTotal = (double)(fim - inicio) / CLOCKS_PER_SEC;


    return tempoTotal / repeticoes;
}

// Função para gerar um grafo aleatório com base nos parâmetros especificados
void gerarGrafoAleatorio(int numVertices, double densidade, unsigned int adjMatrix[MAX_ADJ_NACOS]) {
    memset(adjMatrix, 0, MAX_ADJ_NACOS * sizeof(unsigned int));

    int numArestas = (int)(densidade * numVertices * (numVertices - 1) / 2);

    srand(time(NULL));

    int count = 0;
    while (count < numArestas) {
        int v1 = rand() % numVertices;
        int v2 = rand() % numVertices;

        if (v1 != v2 && !(NACO(adjMatrix,v1,v2) & (1 << IDX_EM_NACO(v2)))) {
            NACO(adjMatrix,v1,v2) |= (1 << IDX_EM_NACO(v2));
            NACO(adjMatrix,v2,v1) |= (1 << IDX_EM_NACO(v1));
            count++;
        }
    }
}

// Função para estimar o tempo de execução da função verificaClique para diferentes tamanhos de grafos
double* estimarTempoExecucao(int from, int to, int by, int nsamples, double dens, int nrep, double r) {
    int numVertices, subsetSize;
    double* tempos = malloc(((to - from) / by + 1) * sizeof(double));

    int currentIndex = 0;
    
    for (int n = from; n <= to; n += by) {
        numVertices = n;
        subsetSize = n * r;

        unsigned int adjMatrix[MAX_ADJ_NACOS];
        unsigned int subset[MAX_V_NACOS];

        double tempoTotal = 0.0;
        for (int i = 0; i < nsamples; i++) {
            gerarGrafoAleatorio(numVertices, dens, adjMatrix);

            for (int j = 0; j < subsetSize; j++) {
                subset[j] = rand() % numVertices;
            }
            tempoTotal += medirTempoExecucao(numVertices, adjMatrix, subset, nrep);
        }

        tempos[currentIndex] = tempoTotal / nsamples;
        currentIndex++;
    }

    return tempos;
}

int main() {
    int from = 1024;
    int to = 4096;
    int by = 256;
    int nsamples = 10;
    double dens = 0.99;
    int nrep = 10000;
    double r = 0.1;

    printf("Parametros do experimento:\nfrom: %d\nto: %d\nby: %d\nnsamples: %d\ndens: %f\nnrep: %d\nr: %f\n\n",
            from, to, by, nsamples, dens, nrep, r);
    

    double* tempos = estimarTempoExecucao(from, to, by, nsamples, dens, nrep, r);

    printf("Tempo medio de execucao:\n");
    int currentIndex = 0;
    for (int n = from; n <= to; n += by) {
        printf("Numero de vertices: %d, Tempo: %e segundos\n", n, tempos[currentIndex]);
        currentIndex++;
    }

    free(tempos);

    return 0;
}
