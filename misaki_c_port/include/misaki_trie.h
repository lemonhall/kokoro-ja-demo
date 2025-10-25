/**
 * misaki_trie.h
 * 
 * Misaki C Port - Trie Tree (Prefix Tree)
 * Trie 树实现，用于 jieba 和 MeCab 分词算法
 * 
 * License: MIT
 */

#ifndef MISAKI_TRIE_H
#define MISAKI_TRIE_H

#include "misaki_types.h"
#include "misaki_string.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Trie 树基本操作
 * ========================================================================== */

/**
 * 创建空的 Trie 树
 * 
 * @return Trie 树对象，失败返回 NULL
 */
Trie* misaki_trie_create(void);

/**
 * 释放 Trie 树
 * 
 * @param trie Trie 树对象
 */
void misaki_trie_free(Trie *trie);

/**
 * 插入词汇到 Trie 树
 * 
 * @param trie Trie 树对象
 * @param word 词汇（UTF-8）
 * @param frequency 词频（用于路径选择）
 * @param tag 词性标签（可为 NULL）
 * @return 成功返回 true
 */
bool misaki_trie_insert(Trie *trie, 
                        const char *word, 
                        double frequency,
                        const char *tag);

/**
 * 查询词汇是否存在
 * 
 * @param trie Trie 树对象
 * @param word 词汇（UTF-8）
 * @return 存在返回 true
 */
bool misaki_trie_contains(const Trie *trie, const char *word);

/**
 * 查询词汇信息
 * 
 * @param trie Trie 树对象
 * @param word 词汇（UTF-8）
 * @param frequency 输出：词频（可为 NULL）
 * @param tag 输出：词性标签（可为 NULL）
 * @return 成功返回 true，未找到返回 false
 */
bool misaki_trie_lookup(const Trie *trie,
                        const char *word,
                        double *frequency,
                        const char **tag);

/**
 * 删除词汇
 * 
 * @param trie Trie 树对象
 * @param word 词汇（UTF-8）
 * @return 成功返回 true，未找到返回 false
 */
bool misaki_trie_remove(Trie *trie, const char *word);

/**
 * 清空 Trie 树
 * 
 * @param trie Trie 树对象
 */
void misaki_trie_clear(Trie *trie);

/* ============================================================================
 * 前缀匹配（分词核心功能）
 * ========================================================================== */

/**
 * 前缀匹配结果
 */
typedef struct {
    const char *word;        // 匹配的词汇
    int length;              // 词汇长度（字节数）
    double frequency;        // 词频
    const char *tag;         // 词性标签
} TrieMatch;

/**
 * 查找所有以指定位置开头的词汇
 * 
 * 用于 jieba 的 DAG 构建：从文本的某个位置开始，找出所有可能的词
 * 
 * @param trie Trie 树对象
 * @param text 文本（UTF-8）
 * @param start_pos 起始位置（字节偏移）
 * @param matches 输出：匹配结果数组
 * @param max_matches 最大匹配数
 * @return 实际匹配数量
 */
int misaki_trie_match_all(const Trie *trie,
                          const char *text,
                          int start_pos,
                          TrieMatch *matches,
                          int max_matches);

/**
 * 查找最长匹配的词汇
 * 
 * @param trie Trie 树对象
 * @param text 文本（UTF-8）
 * @param start_pos 起始位置（字节偏移）
 * @param match 输出：匹配结果
 * @return 成功返回 true
 */
bool misaki_trie_match_longest(const Trie *trie,
                               const char *text,
                               int start_pos,
                               TrieMatch *match);

/**
 * 贪婪匹配（尽可能匹配最长的词）
 * 
 * @param trie Trie 树对象
 * @param text 文本（UTF-8）
 * @param matches 输出：匹配结果数组
 * @param max_matches 最大匹配数
 * @return 实际匹配数量
 */
int misaki_trie_greedy_match(const Trie *trie,
                             const char *text,
                             TrieMatch *matches,
                             int max_matches);

/* ============================================================================
 * Trie 树构建工具
 * ========================================================================== */

/**
 * 从词典文件批量加载到 Trie 树
 * 
 * @param trie Trie 树对象
 * @param file_path 词典文件路径
 * @param format 文件格式：
 *               "word" - 每行一个词（词频默认 1.0）
 *               "word freq" - 每行：词 词频
 *               "word freq tag" - 每行：词 词频 词性
 * @return 成功加载的词汇数量，失败返回 -1
 */
int misaki_trie_load_from_file(Trie *trie, 
                               const char *file_path,
                               const char *format);

/**
 * 从词汇数组批量插入
 * 
 * @param trie Trie 树对象
 * @param words 词汇数组
 * @param frequencies 词频数组（可为 NULL，默认 1.0）
 * @param tags 词性数组（可为 NULL）
 * @param count 词汇数量
 * @return 成功插入的数量
 */
int misaki_trie_insert_batch(Trie *trie,
                             const char **words,
                             const double *frequencies,
                             const char **tags,
                             int count);

/* ============================================================================
 * Trie 树遍历
 * ========================================================================== */

/**
 * 遍历回调函数
 * 
 * @param word 词汇
 * @param frequency 词频
 * @param tag 词性标签
 * @param user_data 用户数据
 * @return 继续遍历返回 true，停止返回 false
 */
typedef bool (*TrieTraverseCallback)(const char *word,
                                      double frequency,
                                      const char *tag,
                                      void *user_data);

/**
 * 遍历所有词汇（深度优先）
 * 
 * @param trie Trie 树对象
 * @param callback 回调函数
 * @param user_data 用户数据
 */
void misaki_trie_traverse(const Trie *trie,
                          TrieTraverseCallback callback,
                          void *user_data);

/**
 * 遍历指定前缀的所有词汇
 * 
 * @param trie Trie 树对象
 * @param prefix 前缀（UTF-8）
 * @param callback 回调函数
 * @param user_data 用户数据
 */
void misaki_trie_traverse_prefix(const Trie *trie,
                                 const char *prefix,
                                 TrieTraverseCallback callback,
                                 void *user_data);

/* ============================================================================
 * Trie 树统计与调试
 * ========================================================================== */

/**
 * 获取 Trie 树统计信息
 * 
 * @param trie Trie 树对象
 * @param total_words 输出：总词汇数
 * @param total_nodes 输出：总节点数
 * @param avg_depth 输出：平均深度
 * @param max_depth 输出：最大深度
 */
void misaki_trie_stats(const Trie *trie,
                      int *total_words,
                      int *total_nodes,
                      double *avg_depth,
                      int *max_depth);

/**
 * 打印 Trie 树结构（调试用）
 * 
 * @param trie Trie 树对象
 * @param max_depth 最大打印深度（0 表示无限制）
 */
void misaki_trie_print(const Trie *trie, int max_depth);

/**
 * 保存 Trie 树到文件
 * 
 * @param trie Trie 树对象
 * @param file_path 文件路径
 * @param format 文件格式（同 misaki_trie_load_from_file）
 * @return 成功返回 true
 */
bool misaki_trie_save_to_file(const Trie *trie,
                              const char *file_path,
                              const char *format);

/* ============================================================================
 * Trie 节点操作（高级用法）
 * ========================================================================== */

/**
 * 查找 Trie 节点
 * 
 * @param trie Trie 树对象
 * @param word 词汇（UTF-8）
 * @return 节点指针，未找到返回 NULL
 */
TrieNode* misaki_trie_find_node(const Trie *trie, const char *word);

/**
 * 获取节点的子节点数量
 * 
 * @param node Trie 节点
 * @return 子节点数量
 */
int misaki_trie_node_children_count(const TrieNode *node);

/**
 * 获取节点的所有子节点
 * 
 * @param node Trie 节点
 * @param children 输出：子节点数组
 * @param max_children 最大子节点数
 * @return 实际子节点数量
 */
int misaki_trie_node_get_children(const TrieNode *node,
                                  TrieNode **children,
                                  int max_children);

/**
 * 判断节点是否为词尾
 * 
 * @param node Trie 节点
 * @return 是返回 true
 */
bool misaki_trie_node_is_word(const TrieNode *node);

/**
 * 获取节点对应的完整词汇
 * 
 * @param node Trie 节点
 * @return 词汇字符串，不是词尾返回 NULL
 */
const char* misaki_trie_node_get_word(const TrieNode *node);

/* ============================================================================
 * 内存优化（可选）
 * ========================================================================== */

/**
 * 压缩 Trie 树（减少内存占用）
 * 
 * 合并单分支节点，减少节点数量
 * 
 * @param trie Trie 树对象
 */
void misaki_trie_compact(Trie *trie);

/**
 * 计算 Trie 树内存占用
 * 
 * @param trie Trie 树对象
 * @return 内存占用（字节）
 */
size_t misaki_trie_memory_usage(const Trie *trie);

#ifdef __cplusplus
}
#endif

#endif /* MISAKI_TRIE_H */
