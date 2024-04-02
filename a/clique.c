#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#define MAX_VERTICES 4096

// Definição do tipo No
typedef struct No {
    int valor;
    struct No* prox;
} No;

// Função partida
No* partida(int v, bool c[MAX_VERTICES]) {
    No* n = NULL;
    for (int i = 0; i < v; i++) {
        if (c[i]) {
            No* novo = (No*)malloc(sizeof(No));
            novo->valor = i;
            novo->prox = n;
            n = novo;
        }
    }
    return n;
}

// Função chegada
No* chegada() {
    return NULL;
}

// Função retirar
No* retirar(bool c[MAX_VERTICES], No* x) {
    c[x->valor] = false;
    No* proximo = x->prox;
    free(x);
    return proximo;
}

// Função subconj
bool subconj(No* x, bool c[MAX_VERTICES]) {
    return c[x->valor];
}

// Função vizinhos
void vizinhos(int n, bool m[MAX_VERTICES][MAX_VERTICES], int vertice, No** p, No** q) {
    *p = partida(n, m[vertice]);
    *q = chegada();
    for (int i = 0; i < n; i++) {
        if (m[vertice][i]) {
            No* novo = (No*)malloc(sizeof(No));
            novo->valor = i;
            novo->prox = *q;
            *q = novo;
        }
    }
}

// Função avancar
No* avancar(No* x) {
    No* proximo = x->prox;
    free(x);
    return proximo;
}

// Função verificaClique
bool verificaClique(int n, bool m[MAX_VERTICES][MAX_VERTICES], bool c[MAX_VERTICES]) {
    No *p, *q;
    for (int i = 0; i < n; i++) {
        vizinhos(n, m, i, &p, &q);
        if (p == NULL) {
            return true;
        }
        while (p != NULL) {
            p = retirar(c, p);
            No* aux = chegada();
            while (q != NULL) {
                if (subconj(q, c) && m[p->valor][q->valor]) {
                    aux = q;
                    q = q->prox;
                    free(aux);
                } else {
                    break;
                }
            }
            if (q == NULL) {
                return true;
            }
        }
    }
    return false;
}

// Função medirTempoExecucao
double medirTempoExecucao(int repeticoes, int n, bool m[MAX_VERTICES][MAX_VERTICES], bool c[MAX_VERTICES]) {
    clock_t inicio, fim;
    double tempo_total = 0;
    for (int i = 0; i < repeticoes; i++) {
        inicio = clock();
        verificaClique(n, m, c);
        fim = clock();
        tempo_total += ((double)(fim - inicio)) / CLOCKS_PER_SEC;
    }
    return tempo_total / repeticoes;
}

// Função gerarGrafoAleatorio
void gerarGrafoAleatorio(int n, double densidade, bool m[MAX_VERTICES][MAX_VERTICES]) {
    srand(time(NULL));
    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            if ((double)rand() / RAND_MAX <= densidade) {
                m[i][j] = m[j][i] = true;
            }
        }
    }
}

// Função estimarTempoExecucao
void estimarTempoExecucao(int from_v, int to_v, int by, int nsamples, double dens, int nrep, double r) {
    double resultados[(to_v - from_v) / by + 1];
    for (int vertices = from_v, i = 0; vertices <= to_v; vertices += by, i++) {
        double total_tempo = 0;
        bool m[MAX_VERTICES][MAX_VERTICES];
        for (int j = 0; j < nsamples; j++) {
            gerarGrafoAleatorio(vertices, dens, m);
            bool c[MAX_VERTICES];
            int subconj_size = vertices * r;
            for (int k = 0; k < vertices; k++) {
                c[k] = (rand() % vertices) < subconj_size;
            }
            total_tempo += medirTempoExecucao(nrep, vertices, m, c);
        }
        resultados[i] = total_tempo / nsamples;
    }

    printf("Resultados:\n");
    for (int i = 0; i < (to_v - from_v) / by + 1; i++) {
        printf("%d vertices: %lf segundos\n", from_v + i * by, resultados[i]);
    }
}

int main() {
    int from_v = 1024;
    int to_v = 4096;
    int by = 256;
    int nsamples = 10;
    double densidades[] = {0.1, 0.5, 0.6, 0.9, 0.99};
    int nrep = 10000;
    double r = 0.1;

    for (int i = 0; i < sizeof(densidades) / sizeof(densidades[0]); i++) {
        printf("Densidade: %lf\n", densidades[i]);
        estimarTempoExecucao(from_v, to_v, by, nsamples, densidades[i], nrep, r);
    }

    return 0;
}
