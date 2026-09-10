#ifndef GRAPH_H
#define GRAPH_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define MAX_NODES 10000

typedef struct EdgeNode {
    uint32_t target;
    double weight_lambda;
    struct EdgeNode *next;
} EdgeNode;

typedef struct {
    EdgeNode *adj[MAX_NODES];
    double sla_rate[MAX_NODES];
    uint32_t node_count;
} Graph;

Graph* graph_create(uint32_t node_count);
void graph_add_edge(Graph *g, uint32_t src, uint32_t dst, double weight_lambda);
void graph_destroy(Graph *g);

// Tarjan's SCC and Degradation Invariance calculation \Lambda(G)
uint32_t tarjan_scc_isolate_cycles(Graph *g);
double compute_system_degradation_entropy(Graph *g, double time_t);

#endif
