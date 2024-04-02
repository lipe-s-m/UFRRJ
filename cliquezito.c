#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>


#define MAX_VERTICES 4096
#define LOG_MAX_VERTICES 12
#define TAM_NACO 32 				// tipo do NACO: unsigned int; tamanho do NACO: sizeof(unsigned int) = 4
#define LOG_TAM_NACO 5

#define MAX_V_NACOS (MAX_VERTICES >> LOG_TAM_NACO)
#define LOG_MAX_V_NACOS (LOG_MAX_VERTICES - LOG_TAM_NACO)
#define MAX_ADJ_NACOS (MAX_VERTICES << LOG_MAX_V_NACOS)

#define N_NACOS(n) (((n >> LOG_TAM_NACO) << LOG_TAM_NACO) == n ? (n >> LOG_TAM_NACO) : ((n >> LOG_TAM_NACO)+1))
#define IDX_VIZ(v) (v << LOG_MAX_V_NACOS)
#define IDX_NACO(v) (v >> LOG_TAM_NACO)
#define IDX_EM_NACO(v) (v - (IDX_NACO(v) << LOG_TAM_NACO))

#define NACO(a,u,v) ((a+IDX_VIZ(u))[IDX_NACO(v)])

unsigned int adjMatrix[MAX_ADJ_NACOS];
unsigned int subset[MAX_V_NACOS];
unsigned int R[MAX_ADJ_NACOS];

//Definição estrutura No
typedef struct No {
    unsigned int ponteiro;
    struct No *proximo;
} No;
//Função inicializar o No (partida)
No* partida(unsigned int *R) {
    No* novoNo = (No*)malloc(sizeof(No));
    novoNo->ponteiro = *R;
    novoNo->proximo = NULL;
    return novoNo;
}
//Função chegada
bool chegada(No* r) {
    return r == NULL;
}
//Função retirar
void retirar(No** r) {
    if (*r != NULL) {
        No* temp = *r;
        *r = (*r)->proximo;
        free(temp);
    }
}
//Função subconj
bool subconj(unsigned int R[MAX_ADJ_NACOS], unsigned int subset[MAX_V_NACOS]) {
    for (int i = 0; i < MAX_V_NACOS; i++) {
        if ((R[i] & subset[i]) != subset[i]) {
            return false; // O subconjunto não é uma clique
        }
    }
    return true; // O subconjunto é uma clique
}
//Função vizinhos
void vizinhos(unsigned int adjMatrix[MAX_ADJ_NACOS], int vertex, unsigned int* neighbors) {
    int idx = IDX_VIZ(vertex);
    unsigned int* row = adjMatrix + idx;
    
    for (int i = 0; i < MAX_V_NACOS; i++) {
        neighbors[i] = row[i];
    }
}
//Função avancar
No* avancar(No* r) {
    return r->proximo;
}

// Função para verificar se um subconjunto define uma clique no grafo
bool verificaClique(int numVertices, unsigned int adjMatrix[MAX_ADJ_NACOS], unsigned int subset[MAX_V_NACOS]) {
    memcpy(R, adjMatrix, N_NACOS(numVertices));
    No * r = partida(&R);
    while (!chegada(r)) {
    	retirar(&r);
    	if (!subconj(R, subset))
    		return false;
    	r = avancar(r);
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
	int numNacos = N_NACOS(numVertices);
	for (int i = 0; i < numVertices; i++) {
		int idx_i = IDX_VIZ(i);
		for (int j = 0; j < numNacos; j++) {
			(adjMatrix+idx_i)[j] = 0;
		}
	}

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

    printf("Estimar tempo medio de execucao\nfrom: %d\nto: %d\nby: %d\nnsamples: %d\ndens: %f\nnrep: %d\nr: %f\n",
    		from, to, by, nsamples, dens, nrep, r);
    fflush(NULL);

    int currentIndex = 0;
    for (int n = from; n <= to; n += by) {
        numVertices = n;
        subsetSize = n * r;

    	int numNacosSubset = N_NACOS(subsetSize);

        double tempoTotal = 0.0;

        for (int i = 0; i < nsamples; i++) {
            gerarGrafoAleatorio(numVertices, dens, adjMatrix);

            for (int j = 0; j < numNacosSubset; j++) {
            	subset[j] = 0;
            }
            for (int j = 0; j < subsetSize; j++) {
                int vertex = rand() % numVertices; // Escolhe um vértice aleatório
                subset[IDX_NACO(vertex)] |= (1 << IDX_EM_NACO(vertex)); // Adiciona o vértice ao subconjunto
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