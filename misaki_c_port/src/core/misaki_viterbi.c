/**
 * misaki_viterbi.c
 * 
 * Misaki C Port - Viterbi Algorithm Implementation
 * 严格按照头文件定义实现
 * 
 * License: MIT
 */

#include "misaki_viterbi.h"
#include "misaki_string.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <float.h>

/* ============================================================================
 * Lattice 操作实现
 * ========================================================================== */

Lattice* misaki_lattice_create(int text_length) {
    if (text_length <= 0) {
        return NULL;
    }
    
    Lattice *lattice = (Lattice *)calloc(1, sizeof(Lattice));
    if (!lattice) {
        return NULL;
    }
    
    lattice->text_length = text_length;
    
    // 分配节点数组（每个位置是一个动态数组）
    lattice->nodes = (LatticeNode ***)calloc(text_length + 1, sizeof(LatticeNode **));
    lattice->node_counts = (int *)calloc(text_length + 1, sizeof(int));
    lattice->node_capacities = (int *)calloc(text_length + 1, sizeof(int));
    
    if (!lattice->nodes || !lattice->node_counts || !lattice->node_capacities) {
        free(lattice->nodes);
        free(lattice->node_counts);
        free(lattice->node_capacities);
        free(lattice);
        return NULL;
    }
    
    // 为每个位置初始化数组
    for (int i = 0; i <= text_length; i++) {
        lattice->node_capacities[i] = 10;  // 初始容量
        lattice->nodes[i] = (LatticeNode **)calloc(10, sizeof(LatticeNode *));
        if (!lattice->nodes[i]) {
            misaki_lattice_free(lattice);
            return NULL;
        }
    }
    
    // 创建 BOS 和 EOS 节点
    lattice->bos = (LatticeNode *)calloc(1, sizeof(LatticeNode));
    lattice->eos = (LatticeNode *)calloc(1, sizeof(LatticeNode));
    
    if (!lattice->bos || !lattice->eos) {
        misaki_lattice_free(lattice);
        return NULL;
    }
    
    // 初始化 BOS
    lattice->bos->pos = 0;
    lattice->bos->surface = misaki_strdup("BOS");
    lattice->bos->total_cost = 0.0;
    lattice->bos->node_cost = 0.0;
    
    // 初始化 EOS
    lattice->eos->pos = text_length;
    lattice->eos->surface = misaki_strdup("EOS");
    lattice->eos->total_cost = DBL_MAX;
    lattice->eos->node_cost = 0.0;
    
    return lattice;
}

void misaki_lattice_free(Lattice *lattice) {
    if (!lattice) {
        return;
    }
    
    // 释放所有位置的节点
    for (int i = 0; i <= lattice->text_length; i++) {
        if (lattice->nodes[i]) {
            for (int j = 0; j < lattice->node_counts[i]; j++) {
                LatticeNode *node = lattice->nodes[i][j];
                if (node) {
                    free(node->surface);
                    free(node->feature);
                    free(node->reading);
                    free(node->phonemes);
                    free(node->next);
                    free(node);
                }
            }
            free(lattice->nodes[i]);
        }
    }
    
    // 释放 BOS/EOS
    if (lattice->bos) {
        free(lattice->bos->surface);
        free(lattice->bos->next);
        free(lattice->bos);
    }
    
    if (lattice->eos) {
        free(lattice->eos->surface);
        free(lattice->eos);
    }
    
    free(lattice->nodes);
    free(lattice->node_counts);
    free(lattice->node_capacities);
    free(lattice);
}

LatticeNode* misaki_lattice_add_node(Lattice *lattice,
                                      int pos,
                                      const char *surface,
                                      const char *feature,
                                      const char *reading,
                                      double node_cost) {
    if (!lattice || pos < 0 || pos > lattice->text_length || !surface) {
        return NULL;
    }
    
    LatticeNode *node = (LatticeNode *)calloc(1, sizeof(LatticeNode));
    if (!node) {
        return NULL;
    }
    
    node->pos = pos;
    node->surface = misaki_strdup(surface);
    node->feature = feature ? misaki_strdup(feature) : NULL;
    node->reading = reading ? misaki_strdup(reading) : NULL;
    node->node_cost = node_cost;
    node->total_cost = DBL_MAX;
    node->start = pos;
    node->length = strlen(surface);
    node->next = NULL;
    node->next_count = 0;
    node->prev = NULL;
    
    // 检查是否需要扩容
    if (lattice->node_counts[pos] >= lattice->node_capacities[pos]) {
        int new_capacity = lattice->node_capacities[pos] * 2;
        LatticeNode **new_array = (LatticeNode **)realloc(
            lattice->nodes[pos], sizeof(LatticeNode *) * new_capacity);
        if (!new_array) {
            free(node->surface);
            free(node->feature);
            free(node->reading);
            free(node);
            return NULL;
        }
        lattice->nodes[pos] = new_array;
        lattice->node_capacities[pos] = new_capacity;
    }
    
    // 添加节点到数组
    lattice->nodes[pos][lattice->node_counts[pos]++] = node;
    
    return node;
}

bool misaki_lattice_add_edge(LatticeNode *from,
                              LatticeNode *to,
                              double edge_cost) {
    if (!from || !to) {
        return false;
    }
    
    // 扩展 next 数组
    LatticeNode **new_next = (LatticeNode **)realloc(
        from->next, sizeof(LatticeNode *) * (from->next_count + 1));
    if (!new_next) {
        return false;
    }
    
    from->next = new_next;
    from->next[from->next_count++] = to;
    
    // 更新边成本（简化：存储在 to 节点）
    to->edge_cost = edge_cost;
    
    return true;
}

int misaki_lattice_get_nodes_at(const Lattice *lattice,
                                 int pos,
                                 LatticeNode **nodes,
                                 int max_count) {
    if (!lattice || !nodes || max_count <= 0) {
        return 0;
    }
    
    if (pos < 0 || pos > lattice->text_length) {
        return 0;
    }
    
    // BOS 特殊处理
    if (pos == 0) {
        nodes[0] = lattice->bos;
        return 1;
    }
    
    // EOS 特殊处理
    if (pos == lattice->text_length) {
        nodes[0] = lattice->eos;
        return 1;
    }
    
    // 返回该位置的所有节点
    int count = lattice->node_counts[pos];
    if (count > max_count) {
        count = max_count;
    }
    
    for (int i = 0; i < count; i++) {
        nodes[i] = lattice->nodes[pos][i];
    }
    
    return count;
}

/* ============================================================================
 * Viterbi 算法实现
 * ========================================================================== */

bool misaki_viterbi_search(Lattice *lattice) {
    if (!lattice) {
        return false;
    }
    
    // 初始化 BOS
    lattice->bos->total_cost = 0.0;
    
    // 前向传播：从 BOS 开始
    // 首先处理 BOS 的后继
    for (int j = 0; j < lattice->bos->next_count; j++) {
        LatticeNode *next = lattice->bos->next[j];
        double cost = lattice->bos->total_cost + next->node_cost + next->edge_cost;
        if (cost < next->total_cost) {
            next->total_cost = cost;
            next->prev = lattice->bos;
        }
    }
    
    // 然后处理所有位置的节点
    for (int pos = 0; pos < lattice->text_length; pos++) {
        for (int i = 0; i < lattice->node_counts[pos]; i++) {
            LatticeNode *node = lattice->nodes[pos][i];
            
            // 遍历所有后继节点
            for (int j = 0; j < node->next_count; j++) {
                LatticeNode *next = node->next[j];
                double cost = node->total_cost + next->node_cost + next->edge_cost;
                
                if (cost < next->total_cost) {
                    next->total_cost = cost;
                    next->prev = node;
                }
            }
        }
    }
    
    return true;
}

int misaki_viterbi_backtrack(const Lattice *lattice,
                              LatticeNode **path,
                              int max_length) {
    if (!lattice || !path || max_length <= 0) {
        return 0;
    }
    
    int count = 0;
    LatticeNode *node = lattice->eos->prev;
    
    // 从 EOS 回溯到 BOS
    while (node && node != lattice->bos && count < max_length) {
        path[count++] = node;
        node = node->prev;
    }
    
    // 反转路径
    for (int i = 0; i < count / 2; i++) {
        LatticeNode *tmp = path[i];
        path[i] = path[count - 1 - i];
        path[count - 1 - i] = tmp;
    }
    
    return count;
}

MisakiTokenList* misaki_viterbi_extract_tokens(const Lattice *lattice) {
    if (!lattice) {
        return NULL;
    }
    
    LatticeNode *path[1000];
    int count = misaki_viterbi_backtrack(lattice, path, 1000);
    
    if (count == 0) {
        return NULL;
    }
    
    // 创建 Token 列表
    MisakiTokenList *tokens = (MisakiTokenList *)malloc(sizeof(MisakiTokenList));
    if (!tokens) {
        return NULL;
    }
    
    tokens->tokens = (MisakiToken *)calloc(count, sizeof(MisakiToken));
    if (!tokens->tokens) {
        free(tokens);
        return NULL;
    }
    
    tokens->count = count;
    
    // 填充 Token
    for (int i = 0; i < count; i++) {
        tokens->tokens[i].text = misaki_strdup(path[i]->surface);
        tokens->tokens[i].tag = path[i]->feature ? misaki_strdup(path[i]->feature) : NULL;
        tokens->tokens[i].phonemes = path[i]->phonemes ? misaki_strdup(path[i]->phonemes) : NULL;
        tokens->tokens[i].start = path[i]->pos;
        tokens->tokens[i].length = path[i]->length;
        tokens->tokens[i].score = path[i]->total_cost;
    }
    
    return tokens;
}

/* ============================================================================
 * 成本矩阵实现
 * ========================================================================== */

CostMatrix* misaki_cost_matrix_load(const char *file_path) {
    // TODO: 实现从文件加载
    (void)file_path;
    return NULL;
}

void misaki_cost_matrix_free(CostMatrix *matrix) {
    if (!matrix) {
        return;
    }
    
    if (matrix->trans_cost) {
        for (int i = 0; i < matrix->pos_count; i++) {
            free(matrix->trans_cost[i]);
        }
        free(matrix->trans_cost);
    }
    
    free(matrix);
}

double misaki_cost_matrix_get(const CostMatrix *matrix,
                               int from_pos,
                               int to_pos) {
    if (!matrix || !matrix->trans_cost) {
        return 0.0;
    }
    
    if (from_pos < 0 || from_pos >= matrix->pos_count ||
        to_pos < 0 || to_pos >= matrix->pos_count) {
        return 0.0;
    }
    
    return matrix->trans_cost[from_pos][to_pos];
}

/* ============================================================================
 * N-Best 路径实现
 * ========================================================================== */

int misaki_viterbi_nbest(const Lattice *lattice,
                         int n,
                         NBestResult *results) {
    // TODO: 实现 N-Best 搜索
    // 简化版：只返回最优路径
    if (!lattice || n <= 0 || !results) {
        return 0;
    }
    
    results[0].path = (LatticeNode **)malloc(sizeof(LatticeNode *) * 1000);
    if (!results[0].path) {
        return 0;
    }
    
    results[0].path_length = misaki_viterbi_backtrack(lattice, results[0].path, 1000);
    results[0].total_cost = lattice->eos->total_cost;
    
    return 1;
}

void misaki_nbest_free(NBestResult *results, int count) {
    if (!results) {
        return;
    }
    
    for (int i = 0; i < count; i++) {
        free(results[i].path);
    }
}

/* ============================================================================
 * 调试工具实现
 * ========================================================================== */

void misaki_lattice_print(const Lattice *lattice) {
    if (!lattice) {
        return;
    }
    
    printf("Lattice (text_length=%d):\n", lattice->text_length);
    
    for (int pos = 0; pos <= lattice->text_length; pos++) {
        printf("  Pos %d (%d nodes):\n", pos, lattice->node_counts[pos]);
        
        LatticeNode *nodes[100];
        int count = misaki_lattice_get_nodes_at(lattice, pos, nodes, 100);
        
        for (int i = 0; i < count; i++) {
            printf("    [%d] %s (cost=%.2f)\n",
                   i, nodes[i]->surface, nodes[i]->total_cost);
        }
    }
    
    printf("  Best cost: %.2f\n", lattice->eos->total_cost);
}

bool misaki_lattice_export_dot(const Lattice *lattice, const char *file_path) {
    if (!lattice || !file_path) {
        return false;
    }
    
    FILE *f = fopen(file_path, "w");
    if (!f) {
        return false;
    }
    
    fprintf(f, "digraph Lattice {\n");
    fprintf(f, "  rankdir=LR;\n");
    fprintf(f, "  node [shape=box];\n\n");
    
    // BOS
    fprintf(f, "  BOS [label=\"BOS\"];\n");
    
    // 所有节点
    int node_id = 0;
    for (int pos = 0; pos <= lattice->text_length; pos++) {
        LatticeNode *node = lattice->nodes[pos];
        while (node) {
            fprintf(f, "  N%d [label=\"%s\\npos=%d\\ncost=%.2f\"];\n",
                    node_id++, node->surface, node->pos, node->total_cost);
            node = node->next ? node->next[0] : NULL;
        }
    }
    
    // EOS
    fprintf(f, "  EOS [label=\"EOS\\ncost=%.2f\"];\n", lattice->eos->total_cost);
    
    // 边（最优路径）
    LatticeNode *path[1000];
    int count = misaki_viterbi_backtrack(lattice, path, 1000);
    
    if (count > 0) {
        fprintf(f, "  BOS -> N%d [color=red];\n", 0);
        for (int i = 0; i < count - 1; i++) {
            fprintf(f, "  N%d -> N%d [color=red];\n", i, i + 1);
        }
        fprintf(f, "  N%d -> EOS [color=red];\n", count - 1);
    }
    
    fprintf(f, "}\n");
    fclose(f);
    
    return true;
}

void misaki_lattice_stats(const Lattice *lattice,
                          int *total_nodes,
                          int *total_edges,
                          double *avg_nodes_per_pos) {
    if (!lattice) {
        return;
    }
    
    int t_nodes = 0;
    int t_edges = 0;
    
    for (int pos = 0; pos <= lattice->text_length; pos++) {
        t_nodes += lattice->node_counts[pos];
        
        LatticeNode *node = lattice->nodes[pos];
        while (node) {
            t_edges += node->next_count;
            node = node->next ? node->next[0] : NULL;
        }
    }
    
    if (total_nodes) *total_nodes = t_nodes;
    if (total_edges) *total_edges = t_edges;
    if (avg_nodes_per_pos) {
        *avg_nodes_per_pos = lattice->text_length > 0 ? 
                            (double)t_nodes / lattice->text_length : 0.0;
    }
}
