#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

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

typedef struct No {
    int valor;
    struct No* prox;
} No;

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

bool chegada(No* r) {
    return r == NULL;
}

void retirar(No** r) {
    No* temp = *r;
    *r = (*r)->prox;
    free(temp);
}

bool subconj(unsigned int *R, No* v) {
    return R[IDX_NACO(v->valor)] & (1 << IDX_EM_NACO(v->valor));
}

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

void avancar(No** p) {
    No* temp = *p;
    *p = (*p)->prox;
    free(temp);
}

// Função para verificar se um subconjunto define uma clique no grafo
bool verificaClique(int numVertices, unsigned int adjMatrix[MAX_ADJ_NACOS], unsigned int subset[MAX_V_NACOS]) {
    unsigned int R[MAX_ADJ_NACOS];
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
    // Limpa a matriz de adjacências
    memset(adjMatrix, 0, MAX_ADJ_NACOS * sizeof(unsigned int));

    // Calcula o número esperado de arestas
    int numArestas = (int)(densidade * numVertices * (numVertices - 1) / 2);

    // Define a semente para geração de números aleatórios
    srand(time(NULL));

    // Gera arestas aleatórias até atingir o número esperado
    int count = 0;
    while (count < numArestas) {
        int v1 = rand() % numVertices; // Vértice inicial aleatório
        int v2 = rand() % numVertices; // Vértice final aleatório

        if (v1 != v2 && !(NACO(adjMatrix,v1,v2) & (1 << IDX_EM_NACO(v2)))) {
            // Adiciona a aresta ao grafo
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
                subset[j] = rand() % numVertices; // Subconjunto aleatório de vértices
            }

            tempoTotal += medirTempoExecucao(numVertices, adjMatrix, subset, nrep);
        }

        tempos[currentIndex] = tempoTotal / nsamples;
        currentIndex++;
    }

    return tempos;
}

int main() {
    int from = 1024; // Quantidade inicial de vértices
    int to = 4096; // Quantidade final de vértices
    int by = 256; // Incremento na quantidade de vértices
    int nsamples = 10; // Quantidade de grafos por tamanho
    double dens = 0.99; // Densidade esperada dos grafos
    int nrep = 10000; // Quantidade de execuções para medição de tempo de execução
    double r = 0.1; // Fração que determina tamanho do subconjunto

    printf("Parâmetros do experimento:\nfrom: %d\nto: %d\nby: %d\nnsamples: %d\ndens: %f\nnrep: %d\nr: %f\n\n",
            from, to, by, nsamples, dens, nrep, r);

    double* tempos = estimarTempoExecucao(from, to, by, nsamples, dens, nrep, r);

    printf("Tempo médio de execução:\n");
    int currentIndex = 0;
    for (int n = from; n <= to; n += by) {
        printf("Número de vértices: %d, Tempo: %e segundos\n", n, tempos[currentIndex]);
        currentIndex++;
    }

    free(tempos);

    return 0;
}
