#include "graph.h"
#include <stdlib.h>
#include <math.h>
#include <string.h>

Graph* graph_create(uint32_t node_count) {
    Graph *g = (Graph*)calloc(1, sizeof(Graph));
    g->node_count = node_count > MAX_NODES ? MAX_NODES : node_count;
    for (uint32_t i = 0; i < g->node_count; i++) {
        g->sla_rate[i] = 0.05; // Base SLA rate decay derivative
    }
    return g;
}

void graph_add_edge(Graph *g, uint32_t src, uint32_t dst, double weight_lambda) {
    if (src >= g->node_count || dst >= g->node_count) return;
    EdgeNode *node = (EdgeNode*)malloc(sizeof(EdgeNode));
    node->target = dst;
    node->weight_lambda = weight_lambda;
    node->next = g->adj[src];
    g->adj[src] = node;
}

void graph_destroy(Graph *g) {
    if (!g) return;
    for (uint32_t i = 0; i < g->node_count; i++) {
        EdgeNode *curr = g->adj[i];
        while (curr) {
            EdgeNode *tmp = curr;
            curr = curr->next;
            free(tmp);
        }
    }
    free(g);
}

// Tarjan's SCC Traversal Algorithm
typedef struct {
    int index;
    int lowlink;
    bool on_stack;
} TarjanNode;

static void tarjan_dfs(Graph *g, uint32_t u, int *index_ptr, TarjanNode *t_nodes, 
                       uint32_t *stack, int *top, uint32_t *scc_count) {
    t_nodes[u].index = *index_ptr;
    t_nodes[u].lowlink = *index_ptr;
    (*index_ptr)++;
    
    stack[*top] = u;
    (*top)++;
    t_nodes[u].on_stack = true;

    EdgeNode *e = g->adj[u];
    while (e) {
        uint32_t v = e->target;
        if (t_nodes[v].index == -1) {
            tarjan_dfs(g, v, index_ptr, t_nodes, stack, top, scc_count);
            if (t_nodes[v].lowlink < t_nodes[u].lowlink) {
                t_nodes[u].lowlink = t_nodes[v].lowlink;
            }
        } else if (t_nodes[v].on_stack) {
            if (t_nodes[v].index < t_nodes[u].lowlink) {
                t_nodes[u].lowlink = t_nodes[v].index;
            }
        }
        e = e->next;
    }

    if (t_nodes[u].lowlink == t_nodes[u].index) {
        uint32_t count_in_scc = 0;
        uint32_t w;
        do {
            (*top)--;
            w = stack[*top];
            t_nodes[w].on_stack = false;
            count_in_scc++;
        } while (w != u);

        if (count_in_scc > 1) {
            (*scc_count)++; // Cyclic dependency loop isolated
        }
    }
}

uint32_t tarjan_scc_isolate_cycles(Graph *g) {
    TarjanNode *t_nodes = (TarjanNode*)malloc(sizeof(TarjanNode) * g->node_count);
    uint32_t *stack = (uint32_t*)malloc(sizeof(uint32_t) * g->node_count);
    int top = 0;
    int index = 0;
    uint32_t scc_count = 0;

    for (uint32_t i = 0; i < g->node_count; i++) {
        t_nodes[i].index = -1;
        t_nodes[i].lowlink = -1;
        t_nodes[i].on_stack = false;
    }

    for (uint32_t i = 0; i < g->node_count; i++) {
        if (t_nodes[i].index == -1) {
            tarjan_dfs(g, i, &index, t_nodes, stack, &top, &scc_count);
        }
    }

    free(t_nodes);
    free(stack);
    return scc_count;
}

// Compute exact mathematical degradation expression: \Lambda(G)
double compute_system_degradation_entropy(Graph *g, double time_t) {
    double total_lambda = 0.0;
    for (uint32_t v = 0; v < g->node_count; v++) {
        double edge_product = 1.0;
        EdgeNode *e = g->adj[v];
        while (e) {
            edge_product *= (1.0 - exp(-e->weight_lambda * time_t));
            e = e->next;
        }
        total_lambda += g->sla_rate[v] * edge_product;
    }
    return total_lambda;
}
