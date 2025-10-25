/**
 * misaki_trie.c
 * 
 * Misaki C Port - Trie Tree (Prefix Tree) Implementation
 * Trie 树实现（用于分词器的快速词典匹配）
 * 
 * License: MIT
 */

#include "misaki_trie.h"
#include "misaki_string.h"
#include "misaki_dict.h"  // TSVParser 定义在这里
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ============================================================================
 * Trie 节点操作
 * ========================================================================== */

static TrieNode* trie_node_create(uint32_t codepoint) {
    TrieNode *node = (TrieNode *)calloc(1, sizeof(TrieNode));
    if (!node) {
        return NULL;
    }
    
    node->codepoint = codepoint;
    node->word = NULL;
    node->frequency = 0.0;
    node->tag = NULL;
    node->children = NULL;
    node->children_count = 0;
    node->children_capacity = 0;
    node->is_word = false;
    
    return node;
}

static void trie_node_free(TrieNode *node) {
    if (!node) {
        return;
    }
    
    // 递归释放所有子节点
    for (int i = 0; i < node->children_count; i++) {
        trie_node_free(node->children[i]);
    }
    
    free(node->word);
    free(node->tag);
    free(node->children);
    free(node);
}

static TrieNode* trie_node_find_child(TrieNode *node, uint32_t codepoint) {
    if (!node) {
        return NULL;
    }
    
    // 线性查找（子节点数量通常很少）
    for (int i = 0; i < node->children_count; i++) {
        if (node->children[i]->codepoint == codepoint) {
            return node->children[i];
        }
    }
    
    return NULL;
}

static bool trie_node_add_child(TrieNode *node, TrieNode *child) {
    if (!node || !child) {
        return false;
    }
    
    // 扩容（如果需要）
    if (node->children_count >= node->children_capacity) {
        int new_capacity = node->children_capacity == 0 ? 4 : node->children_capacity * 2;
        TrieNode **new_children = (TrieNode **)realloc(
            node->children, sizeof(TrieNode *) * new_capacity);
        if (!new_children) {
            return false;
        }
        node->children = new_children;
        node->children_capacity = new_capacity;
    }
    
    node->children[node->children_count++] = child;
    return true;
}

/* ============================================================================
 * Trie 基本操作
 * ========================================================================== */

Trie* misaki_trie_create(void) {
    Trie *trie = (Trie *)malloc(sizeof(Trie));
    if (!trie) {
        return NULL;
    }
    
    trie->root = trie_node_create(0);  // 根节点码点为 0
    if (!trie->root) {
        free(trie);
        return NULL;
    }
    
    trie->word_count = 0;
    
    return trie;
}

void misaki_trie_free(Trie *trie) {
    if (!trie) {
        return;
    }
    
    trie_node_free(trie->root);
    free(trie);
}

bool misaki_trie_insert(Trie *trie, 
                        const char *word, 
                        double frequency,
                        const char *tag) {
    if (!trie || !word) {
        return false;
    }
    
    TrieNode *current = trie->root;
    const char *p = word;
    
    // 逐字符遍历
    while (*p) {
        uint32_t codepoint;
        int bytes = misaki_utf8_decode(p, &codepoint);
        if (bytes == 0) {
            return false;  // 无效的 UTF-8
        }
        
        // 查找或创建子节点
        TrieNode *child = trie_node_find_child(current, codepoint);
        if (!child) {
            child = trie_node_create(codepoint);
            if (!child || !trie_node_add_child(current, child)) {
                if (child) trie_node_free(child);
                return false;
            }
        }
        
        current = child;
        p += bytes;
    }
    
    // 标记为词尾
    if (!current->is_word) {
        trie->word_count++;
    }
    
    current->is_word = true;
    current->frequency = frequency;
    
    // 保存完整词
    if (!current->word) {
        current->word = misaki_strdup(word);
    }
    
    // 保存词性
    if (tag && !current->tag) {
        current->tag = misaki_strdup(tag);
    }
    
    return true;
}

bool misaki_trie_contains(const Trie *trie, const char *word) {
    double freq;
    const char *tag;
    return misaki_trie_lookup(trie, word, &freq, &tag);
}

bool misaki_trie_lookup(const Trie *trie,
                        const char *word,
                        double *frequency,
                        const char **tag) {
    if (!trie || !word) {
        return false;
    }
    
    TrieNode *current = trie->root;
    const char *p = word;
    
    while (*p) {
        uint32_t codepoint;
        int bytes = misaki_utf8_decode(p, &codepoint);
        if (bytes == 0) {
            return false;
        }
        
        current = trie_node_find_child(current, codepoint);
        if (!current) {
            return false;  // 未找到
        }
        
        p += bytes;
    }
    
    if (!current->is_word) {
        return false;  // 不是完整词
    }
    
    if (frequency) {
        *frequency = current->frequency;
    }
    
    if (tag) {
        *tag = current->tag;
    }
    
    return true;
}

bool misaki_trie_remove(Trie *trie, const char *word) {
    // TODO: 实现删除（需要处理节点引用计数）
    // 暂时只标记为非词尾
    if (!trie || !word) {
        return false;
    }
    
    TrieNode *current = trie->root;
    const char *p = word;
    
    while (*p) {
        uint32_t codepoint;
        int bytes = misaki_utf8_decode(p, &codepoint);
        if (bytes == 0) {
            return false;
        }
        
        current = trie_node_find_child(current, codepoint);
        if (!current) {
            return false;
        }
        
        p += bytes;
    }
    
    if (current->is_word) {
        current->is_word = false;
        trie->word_count--;
        return true;
    }
    
    return false;
}

void misaki_trie_clear(Trie *trie) {
    if (!trie) {
        return;
    }
    
    trie_node_free(trie->root);
    trie->root = trie_node_create(0);
    trie->word_count = 0;
}

/* ============================================================================
 * 前缀匹配（分词核心功能）
 * ========================================================================== */

int misaki_trie_match_all(const Trie *trie,
                          const char *text,
                          int start_pos,
                          TrieMatch *matches,
                          int max_matches) {
    if (!trie || !text || !matches || max_matches <= 0) {
        return 0;
    }
    
    int match_count = 0;
    TrieNode *current = trie->root;
    const char *p = text + start_pos;
    int current_pos = start_pos;
    
    while (*p && match_count < max_matches) {
        uint32_t codepoint;
        int bytes = misaki_utf8_decode(p, &codepoint);
        if (bytes == 0) {
            break;  // 无效的 UTF-8
        }
        
        current = trie_node_find_child(current, codepoint);
        if (!current) {
            break;  // 没有匹配的前缀
        }
        
        current_pos += bytes;
        
        // 如果是词尾，记录匹配
        if (current->is_word) {
            matches[match_count].word = current->word;
            matches[match_count].length = current_pos - start_pos;
            matches[match_count].frequency = current->frequency;
            matches[match_count].tag = current->tag;
            match_count++;
        }
        
        p += bytes;
    }
    
    return match_count;
}

bool misaki_trie_match_longest(const Trie *trie,
                               const char *text,
                               int start_pos,
                               TrieMatch *match) {
    if (!trie || !text || !match) {
        return false;
    }
    
    TrieMatch matches[100];  // 临时数组
    int count = misaki_trie_match_all(trie, text, start_pos, matches, 100);
    
    if (count == 0) {
        return false;
    }
    
    // 返回最后一个（最长的）
    *match = matches[count - 1];
    return true;
}

int misaki_trie_greedy_match(const Trie *trie,
                             const char *text,
                             TrieMatch *matches,
                             int max_matches) {
    if (!trie || !text || !matches || max_matches <= 0) {
        return 0;
    }
    
    int match_count = 0;
    int pos = 0;
    int text_len = strlen(text);
    
    while (pos < text_len && match_count < max_matches) {
        TrieMatch match;
        
        // 尝试最长匹配
        if (misaki_trie_match_longest(trie, text, pos, &match)) {
            matches[match_count++] = match;
            pos += match.length;
        } else {
            // 无法匹配，跳过一个字符
            uint32_t cp;
            int bytes = misaki_utf8_decode(text + pos, &cp);
            if (bytes == 0) {
                break;
            }
            pos += bytes;
        }
    }
    
    return match_count;
}

/* ============================================================================
 * Trie 树构建工具
 * ========================================================================== */

int misaki_trie_load_from_file(Trie *trie, 
                               const char *file_path,
                               const char *format) {
    if (!trie || !file_path) {
        return -1;
    }
    
    TSVParser *parser = misaki_tsv_parser_create(file_path);
    if (!parser) {
        return -1;
    }
    
    int loaded_count = 0;
    MisakiStringView fields[10];
    
    while (true) {
        int field_count = misaki_tsv_parser_next_line(parser, fields, 10);
        
        if (field_count == 0) {
            break;
        }
        
        if (field_count < 1) {
            continue;
        }
        
        // 解析格式
        char *word = strndup(fields[0].data, fields[0].length);
        if (!word) {
            continue;
        }
        
        double frequency = 1.0;
        char *tag = NULL;
        
        if (strcmp(format, "word freq") == 0 && field_count >= 2) {
            char *freq_str = strndup(fields[1].data, fields[1].length);
            if (freq_str) {
                frequency = atof(freq_str);
                free(freq_str);
            }
        } else if (strcmp(format, "word freq tag") == 0 && field_count >= 3) {
            char *freq_str = strndup(fields[1].data, fields[1].length);
            if (freq_str) {
                frequency = atof(freq_str);
                free(freq_str);
            }
            tag = strndup(fields[2].data, fields[2].length);
        }
        
        if (misaki_trie_insert(trie, word, frequency, tag)) {
            loaded_count++;
        }
        
        free(word);
        free(tag);
    }
    
    misaki_tsv_parser_free(parser);
    return loaded_count;
}

int misaki_trie_insert_batch(Trie *trie,
                             const char **words,
                             const double *frequencies,
                             const char **tags,
                             int count) {
    if (!trie || !words || count <= 0) {
        return 0;
    }
    
    int inserted = 0;
    for (int i = 0; i < count; i++) {
        double freq = frequencies ? frequencies[i] : 1.0;
        const char *tag = tags ? tags[i] : NULL;
        
        if (misaki_trie_insert(trie, words[i], freq, tag)) {
            inserted++;
        }
    }
    
    return inserted;
}

/* ============================================================================
 * Trie 树遍历
 * ========================================================================== */

static void trie_traverse_recursive(TrieNode *node, 
                                    TrieTraverseCallback callback,
                                    void *user_data) {
    if (!node) {
        return;
    }
    
    // 如果是词尾，调用回调
    if (node->is_word) {
        if (!callback(node->word, node->frequency, node->tag, user_data)) {
            return;  // 停止遍历
        }
    }
    
    // 递归遍历子节点
    for (int i = 0; i < node->children_count; i++) {
        trie_traverse_recursive(node->children[i], callback, user_data);
    }
}

void misaki_trie_traverse(const Trie *trie,
                          TrieTraverseCallback callback,
                          void *user_data) {
    if (!trie || !callback) {
        return;
    }
    
    trie_traverse_recursive(trie->root, callback, user_data);
}

void misaki_trie_traverse_prefix(const Trie *trie,
                                 const char *prefix,
                                 TrieTraverseCallback callback,
                                 void *user_data) {
    if (!trie || !prefix || !callback) {
        return;
    }
    
    // 先找到前缀对应的节点
    TrieNode *current = trie->root;
    const char *p = prefix;
    
    while (*p) {
        uint32_t codepoint;
        int bytes = misaki_utf8_decode(p, &codepoint);
        if (bytes == 0) {
            return;
        }
        
        current = trie_node_find_child(current, codepoint);
        if (!current) {
            return;  // 前缀不存在
        }
        
        p += bytes;
    }
    
    // 从该节点开始遍历
    trie_traverse_recursive(current, callback, user_data);
}

/* ============================================================================
 * Trie 树统计与调试
 * ========================================================================== */

static void trie_stats_recursive(TrieNode *node, 
                                 int *total_nodes,
                                 int depth,
                                 double *total_depth,
                                 int *max_depth) {
    if (!node) {
        return;
    }
    
    (*total_nodes)++;
    
    if (node->is_word) {
        *total_depth += depth;
        if (depth > *max_depth) {
            *max_depth = depth;
        }
    }
    
    for (int i = 0; i < node->children_count; i++) {
        trie_stats_recursive(node->children[i], total_nodes, 
                            depth + 1, total_depth, max_depth);
    }
}

void misaki_trie_stats(const Trie *trie,
                      int *total_words,
                      int *total_nodes,
                      double *avg_depth,
                      int *max_depth) {
    if (!trie) {
        return;
    }
    
    if (total_words) {
        *total_words = trie->word_count;
    }
    
    if (total_nodes || avg_depth || max_depth) {
        int t_nodes = 0;
        int m_depth = 0;
        double t_depth = 0;
        
        trie_stats_recursive(trie->root, &t_nodes, 0, &t_depth, &m_depth);
        
        if (total_nodes) {
            *total_nodes = t_nodes;
        }
        
        if (avg_depth) {
            *avg_depth = trie->word_count > 0 ? t_depth / trie->word_count : 0;
        }
        
        if (max_depth) {
            *max_depth = m_depth;
        }
    }
}

void misaki_trie_print(const Trie *trie, int max_depth) {
    if (!trie) {
        return;
    }
    
    int total_words, total_nodes, m_depth;
    double avg_depth;
    
    misaki_trie_stats(trie, &total_words, &total_nodes, &avg_depth, &m_depth);
    
    printf("Trie Statistics:\n");
    printf("  Total words: %d\n", total_words);
    printf("  Total nodes: %d\n", total_nodes);
    printf("  Average depth: %.2f\n", avg_depth);
    printf("  Max depth: %d\n", m_depth);
}

bool misaki_trie_save_to_file(const Trie *trie,
                              const char *file_path,
                              const char *format) {
    // TODO: 实现保存功能
    (void)trie;
    (void)file_path;
    (void)format;
    return false;
}

/* ============================================================================
 * Trie 节点操作（高级用法）
 * ========================================================================== */

TrieNode* misaki_trie_find_node(const Trie *trie, const char *word) {
    if (!trie || !word) {
        return NULL;
    }
    
    TrieNode *current = trie->root;
    const char *p = word;
    
    while (*p) {
        uint32_t codepoint;
        int bytes = misaki_utf8_decode(p, &codepoint);
        if (bytes == 0) {
            return NULL;
        }
        
        current = trie_node_find_child(current, codepoint);
        if (!current) {
            return NULL;
        }
        
        p += bytes;
    }
    
    return current->is_word ? current : NULL;
}

int misaki_trie_node_children_count(const TrieNode *node) {
    return node ? node->children_count : 0;
}

int misaki_trie_node_get_children(const TrieNode *node,
                                  TrieNode **children,
                                  int max_children) {
    if (!node || !children || max_children <= 0) {
        return 0;
    }
    
    int count = node->children_count < max_children ? 
                node->children_count : max_children;
    
    for (int i = 0; i < count; i++) {
        children[i] = node->children[i];
    }
    
    return count;
}

bool misaki_trie_node_is_word(const TrieNode *node) {
    return node ? node->is_word : false;
}

const char* misaki_trie_node_get_word(const TrieNode *node) {
    return (node && node->is_word) ? node->word : NULL;
}

/* ============================================================================
 * 内存优化（可选）
 * ========================================================================== */

void misaki_trie_compact(Trie *trie) {
    // TODO: 实现 Trie 树压缩
    (void)trie;
}

size_t misaki_trie_memory_usage(const Trie *trie) {
    // TODO: 计算精确的内存占用
    if (!trie) {
        return 0;
    }
    
    // 粗略估计
    return sizeof(Trie) + trie->word_count * sizeof(TrieNode) * 10;
}
