#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
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

unsigned int R[MAX_ADJ_NACOS];


typedef struct No {
    unsigned int valor;
    struct No* proximo;
} No;

No* partida() {
    No* inicio = (No*)malloc(sizeof(No));
    inicio->valor = 0;
    inicio->proximo = NULL;
    return inicio;
}

bool chegada(No* no) {
    return (no == NULL);
}

void retirar(No* no) {
    No* temp = no->proximo;
    no->proximo = temp->proximo;
    free(temp);
}

bool subconj(unsigned int R[], unsigned int vizinhos) {
    return ((R[vizinhos / 32] >> (vizinhos % 32)) & 1);
}

unsigned int vizinhos(unsigned int adjMatrix[], unsigned int alvo) {
    return adjMatrix[alvo];
}

No* avancar(No* no) {
    No* proximo = no->proximo;
    if (proximo == NULL) return NULL;
    return proximo;
}

bool verificaClique(int numVertices, unsigned int adjMatrix[MAX_ADJ_NACOS]) {
    memcpy(R, adjMatrix, N_NACOS(numVertices));
    No * r = partida();
    while (!chegada(r)) {
        retirar(r);
        if (!subconj(R, vizinhos(adjMatrix, r->valor)))
            return false;
        r = avancar(r);
    }
    return true;
}

double medirTempoExecucao(int numVertices, unsigned int adjMatrix[MAX_ADJ_NACOS], int repeticoes) {
    clock_t inicio, fim;

    inicio = clock();
    for (int i = 0; i < repeticoes; i++)
        verificaClique(numVertices, adjMatrix);
    fim = clock();
    double tempoTotal = (double)(fim - inicio) / CLOCKS_PER_SEC;

    return tempoTotal / repeticoes;
}

void gerarGrafoAleatorio(int numVertices, double densidade, unsigned int adjMatrix[MAX_ADJ_NACOS]) {
    int numNacos = N_NACOS(numVertices);
    for (int i = 0; i < numVertices; i++) {
        int idx_i = IDX_VIZ(i);
        for (int j = 0; j < numNacos; j++) {
            (adjMatrix+idx_i)[j] = 0;
        }
    }

    int numArestas = (int)(densidade * numVertices * (numVertices - 1) / 2);
    srand(time(NULL));

    int count = 0;
    while (count < numArestas) {
        int v1 = rand() % numVertices;
        int v2 = rand() % numVertices;

        if (v1 != v2 && !(adjMatrix[IDX_VIZ(v1) + IDX_NACO(v2)] & (1 << IDX_EM_NACO(v2)))) {
            adjMatrix[IDX_VIZ(v1) + IDX_NACO(v2)] |= (1 << IDX_EM_NACO(v2));
            adjMatrix[IDX_VIZ(v2) + IDX_NACO(v1)] |= (1 << IDX_EM_NACO(v1));
            count++;
        }
    }
}

double* estimarTempoExecucao(int from, int to, int by, int nsamples, double dens, int nrep, double r) {
    int numVertices;
    double* tempos = malloc(((to - from) / by + 1) * sizeof(double));

    int currentIndex = 0;
    for (int n = from; n <= to; n += by) {
        numVertices = n;

        unsigned int adjMatrix[MAX_ADJ_NACOS];

        double tempoTotal = 0.0;

        for (int i = 0; i < nsamples; i++) {
            gerarGrafoAleatorio(numVertices, dens, adjMatrix);

            tempoTotal += medirTempoExecucao(numVertices, adjMatrix, nrep);
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

    printf("Parametros do experimento:\nMAX_VERTICES: %d\nLOG_MAX_VERTICES: %d\nTAM_NACO: %d\nLOG_TAM_NACO: %d\nMAX_V_NACOS: %d\nLOG_MAX_V_NACOS: %d\nMAX_ADJ_NACOS: %d\n\n",
            MAX_VERTICES,
            LOG_MAX_VERTICES,
            TAM_NACO,
            LOG_TAM_NACO,
            MAX_V_NACOS,
            LOG_MAX_V_NACOS,
            MAX_ADJ_NACOS);

    

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