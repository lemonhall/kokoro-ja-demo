/**
 * misaki_viterbi.h
 * 
 * Misaki C Port - Viterbi Algorithm
 * Viterbi 算法实现（用于日文分词、韩文分词）
 * 
 * License: MIT
 */

#ifndef MISAKI_VITERBI_H
#define MISAKI_VITERBI_H

#include "misaki_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Lattice (词格) 数据结构
 * ========================================================================== */

/**
 * Lattice Node: 词格节点
 */
typedef struct LatticeNode {
    int pos;                   // 在文本中的位置
    char *surface;             // 表层形式（原始文本）
    char *feature;             // 特征字符串（词性等）
    char *reading;             // 读音（假名/拼音）
    char *phonemes;            // 音素序列
    
    double node_cost;          // 节点成本
    double edge_cost;          // 边成本（到前驱的成本）
    double total_cost;         // 累积成本
    
    struct LatticeNode *prev;  // 前驱节点（回溯用）
    struct LatticeNode **next; // 后继节点数组
    int next_count;            // 后继数量
    
    int start;                 // 起始位置（字节偏移）
    int length;                // 长度（字节数）
} LatticeNode;

/**
 * Lattice: 词格（保存所有可能的分词路径）
 */
typedef struct Lattice {
    LatticeNode **nodes;       // 节点数组（按位置分组）
    int *node_counts;          // 每个位置的节点数
    int text_length;           // 文本长度（字符数）
    
    LatticeNode *bos;          // 起始节点 (Begin of Sentence)
    LatticeNode *eos;          // 结束节点 (End of Sentence)
} Lattice;

/* ============================================================================
 * Lattice 操作
 * ========================================================================== */

/**
 * 创建 Lattice
 * 
 * @param text_length 文本长度（字符数）
 * @return Lattice 对象，失败返回 NULL
 */
Lattice* misaki_lattice_create(int text_length);

/**
 * 释放 Lattice
 * 
 * @param lattice Lattice 对象
 */
void misaki_lattice_free(Lattice *lattice);

/**
 * 添加节点到 Lattice
 * 
 * @param lattice Lattice 对象
 * @param pos 位置
 * @param surface 表层形式
 * @param feature 特征
 * @param reading 读音
 * @param node_cost 节点成本
 * @return 创建的节点，失败返回 NULL
 */
LatticeNode* misaki_lattice_add_node(Lattice *lattice,
                                      int pos,
                                      const char *surface,
                                      const char *feature,
                                      const char *reading,
                                      double node_cost);

/**
 * 添加边（连接两个节点）
 * 
 * @param from 起始节点
 * @param to 结束节点
 * @param edge_cost 边成本
 * @return 成功返回 true
 */
bool misaki_lattice_add_edge(LatticeNode *from,
                              LatticeNode *to,
                              double edge_cost);

/**
 * 获取指定位置的所有节点
 * 
 * @param lattice Lattice 对象
 * @param pos 位置
 * @param nodes 输出：节点数组
 * @param max_count 最大数量
 * @return 实际节点数量
 */
int misaki_lattice_get_nodes_at(const Lattice *lattice,
                                 int pos,
                                 LatticeNode **nodes,
                                 int max_count);

/* ============================================================================
 * Viterbi 算法
 * ========================================================================== */

/**
 * Viterbi 算法：求最优路径
 * 
 * 使用动态规划找到成本最小的路径
 * 
 * @param lattice Lattice 对象
 * @return 成功返回 true
 */
bool misaki_viterbi_search(Lattice *lattice);

/**
 * 回溯最优路径
 * 
 * @param lattice Lattice 对象（已执行 viterbi_search）
 * @param path 输出：路径节点数组
 * @param max_length 最大长度
 * @return 实际路径长度
 */
int misaki_viterbi_backtrack(const Lattice *lattice,
                              LatticeNode **path,
                              int max_length);

/**
 * 提取 Token 列表（从最优路径）
 * 
 * @param lattice Lattice 对象（已执行 viterbi_search）
 * @return Token 列表，失败返回 NULL
 */
MisakiTokenList* misaki_viterbi_extract_tokens(const Lattice *lattice);

/* ============================================================================
 * 成本计算
 * ========================================================================== */

/**
 * 成本矩阵（用于计算边成本）
 */
typedef struct CostMatrix {
    double **trans_cost;       // 转移成本矩阵
    int pos_count;             // 词性数量
} CostMatrix;

/**
 * 加载成本矩阵
 * 
 * @param file_path 成本文件路径
 * @return 成本矩阵，失败返回 NULL
 */
CostMatrix* misaki_cost_matrix_load(const char *file_path);

/**
 * 释放成本矩阵
 * 
 * @param matrix 成本矩阵
 */
void misaki_cost_matrix_free(CostMatrix *matrix);

/**
 * 计算边成本
 * 
 * @param matrix 成本矩阵
 * @param from_pos 起始词性 ID
 * @param to_pos 结束词性 ID
 * @return 边成本
 */
double misaki_cost_matrix_get(const CostMatrix *matrix,
                               int from_pos,
                               int to_pos);

/* ============================================================================
 * N-Best 路径（可选）
 * ========================================================================== */

/**
 * N-Best 结果
 */
typedef struct NBestResult {
    LatticeNode **path;        // 路径节点数组
    int path_length;           // 路径长度
    double total_cost;         // 总成本
} NBestResult;

/**
 * 求 N-Best 路径（Top N 个最优路径）
 * 
 * @param lattice Lattice 对象
 * @param n N 值
 * @param results 输出：N-Best 结果数组
 * @return 实际结果数量
 */
int misaki_viterbi_nbest(const Lattice *lattice,
                         int n,
                         NBestResult *results);

/**
 * 释放 N-Best 结果
 * 
 * @param results N-Best 结果数组
 * @param count 结果数量
 */
void misaki_nbest_free(NBestResult *results, int count);

/* ============================================================================
 * 调试工具
 * ========================================================================== */

/**
 * 打印 Lattice 结构
 * 
 * @param lattice Lattice 对象
 */
void misaki_lattice_print(const Lattice *lattice);

/**
 * 导出 Lattice 为 DOT 格式（Graphviz）
 * 
 * @param lattice Lattice 对象
 * @param file_path 输出文件路径
 * @return 成功返回 true
 */
bool misaki_lattice_export_dot(const Lattice *lattice, const char *file_path);

/**
 * 获取 Lattice 统计信息
 * 
 * @param lattice Lattice 对象
 * @param total_nodes 输出：总节点数
 * @param total_edges 输出：总边数
 * @param avg_nodes_per_pos 输出：平均每个位置的节点数
 */
void misaki_lattice_stats(const Lattice *lattice,
                          int *total_nodes,
                          int *total_edges,
                          double *avg_nodes_per_pos);

#ifdef __cplusplus
}
#endif

#endif /* MISAKI_VITERBI_H */
