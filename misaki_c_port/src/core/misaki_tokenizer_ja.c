/**
 * misaki_tokenizer_ja.c
 * 
 * 日文分词器实现（基于 Viterbi 算法）
 * 
 * 完整实现：使用 Viterbi 最优路径搜索（类似 MeCab）
 * - 构建 Lattice (词格)
 * - Viterbi 动态规划求最优路径
 * - 支持成本矩阵（词性转移概率）
 * 
 * License: MIT
 */

#include "misaki_tokenizer.h"
#include "misaki_viterbi.h"
#include "misaki_string.h"
#include "misaki_trie.h"
#include "misaki_transition_rules.h"  // 添加词性转移规则
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ============================================================================
 * 日文分词器结构体
 * ========================================================================== */

typedef struct {
    Trie *dict_trie;       // 词典 Trie 树
    bool use_simple_model; // 使用简化模型（false=使用Viterbi）
    CostMatrix *cost_matrix; // 成本矩阵（可选）
} JaTokenizer;

/* ============================================================================
 * 日文分词器创建和销毁
 * ========================================================================== */

void* misaki_ja_tokenizer_create(const JaTokenizerConfig *config) {
    if (!config || !config->dict_trie) {
        return NULL;
    }
    
    JaTokenizer *tokenizer = (JaTokenizer *)calloc(1, sizeof(JaTokenizer));
    if (!tokenizer) {
        return NULL;
    }
    
    tokenizer->dict_trie = config->dict_trie;
    tokenizer->use_simple_model = config->use_simple_model;
    tokenizer->cost_matrix = NULL; // TODO: 加载成本矩阵
    
    return tokenizer;
}

void misaki_ja_tokenizer_free(void *tokenizer) {
    if (tokenizer) {
        JaTokenizer *ja = (JaTokenizer *)tokenizer;
        if (ja->cost_matrix) {
            misaki_cost_matrix_free(ja->cost_matrix);
        }
        free(tokenizer);
    }
}

/* ============================================================================
 * 日文分词主函数（纯 Viterbi 模式）
 * ========================================================================== */

/**
 * Viterbi 模式：构建 Lattice 并求最优路径
 */
static MisakiTokenList* ja_tokenize_viterbi(JaTokenizer *ja, const char *text) {
    // 1. 计算文本长度（字符数）
    int text_len = misaki_utf8_length(text);
    if (text_len == 0) {
        return NULL;
    }
    
    // 2. 创建 Lattice
    Lattice *lattice = misaki_lattice_create(text_len);
    if (!lattice) {
        return NULL;
    }
    
    // 3. 构建 Lattice：添加所有可能的节点和边
    // 存储每个位置的所有节点（用于连接边）
    LatticeNode ***nodes_by_pos = (LatticeNode ***)calloc(text_len + 1, sizeof(LatticeNode **));
    int *counts_by_pos = (int *)calloc(text_len + 1, sizeof(int));
    
    if (!nodes_by_pos || !counts_by_pos) {
        misaki_lattice_free(lattice);
        free(nodes_by_pos);
        free(counts_by_pos);
        return NULL;
    }
    
    // 为每个位置分配节点数组
    for (int i = 0; i <= text_len; i++) {
        nodes_by_pos[i] = (LatticeNode **)calloc(100, sizeof(LatticeNode *));
    }
    
    int byte_pos = 0;
    const char *p = text;
    
    for (int char_pos = 0; char_pos < text_len; char_pos++) {
        // 从当前位置查找所有可能的词
        TrieMatch matches[100];
        int match_count = misaki_trie_match_all(ja->dict_trie, text, byte_pos, matches, 100);
        
        bool has_match = false;
        for (int i = 0; i < match_count; i++) {
            TrieMatch *m = &matches[i];
            
            // 计算节点成本（使用词频的负对数）
            // 注意：频率越高，成本越低！
            // 添加长度奖励：词越长越好
            int word_char_len = misaki_utf8_length(m->word);
            double freq = m->frequency > 0 ? m->frequency : 1000.0;  // 默认频率提高
            
            // 成本 = -log(频率) - 长度奖励
            // 频率越高，成本越低；词越长，成本越低
            // 增大长度奖励，使得长词更有优势
            double node_cost = -log(freq) - (word_char_len - 1) * 10.0;  // 增加到 10.0！
            
            // 添加节点到 Lattice
            LatticeNode *node = misaki_lattice_add_node(
                lattice, char_pos, m->word, m->tag, NULL, node_cost);
            
            if (node) {
                // 计算这个词跨越的字符数
                node->length = word_char_len;
                
                // 存储节点（用于后续连接边）
                if (counts_by_pos[char_pos] < 100) {
                    nodes_by_pos[char_pos][counts_by_pos[char_pos]++] = node;
                    has_match = true;
                }
            }
        }
        
        // 如果没有匹配，添加单字符节点（未登录词、标点符号等）
        if (!has_match) {
            uint32_t codepoint;
            int bytes = misaki_utf8_decode(p, &codepoint);
            if (bytes > 0) {
                char single_char[8];
                memcpy(single_char, p, bytes);
                single_char[bytes] = '\0';
                
                // 单字符的成本较高（惩罚）
                // 提高惩罚值，使得分词器更倾向于选择长词
                LatticeNode *node = misaki_lattice_add_node(
                    lattice, char_pos, single_char, "UNK", NULL, 20.0);  // 进一步提高惩罚
                
                if (node) {
                    node->length = 1;  // 单字符
                    if (counts_by_pos[char_pos] < 100) {
                        nodes_by_pos[char_pos][counts_by_pos[char_pos]++] = node;
                    }
                }
            }
        }
        
        // 移动到下一个字符
        uint32_t codepoint;
        int bytes = misaki_utf8_decode(p, &codepoint);
        if (bytes == 0) break;
        
        p += bytes;
        byte_pos += bytes;
    }
    
    // 4. 连接边：从 BOS 到第一个位置的所有节点
    for (int i = 0; i < counts_by_pos[0]; i++) {
        misaki_lattice_add_edge(lattice->bos, nodes_by_pos[0][i], 0.0);
    }
    
    // 连接边：每个节点到其后继节点
    for (int pos = 0; pos < text_len; pos++) {
        for (int i = 0; i < counts_by_pos[pos]; i++) {
            LatticeNode *from = nodes_by_pos[pos][i];
            int next_pos = pos + from->length;
            
            if (next_pos < text_len) {
                // 连接到下一个位置的所有节点
                for (int j = 0; j < counts_by_pos[next_pos]; j++) {
                    LatticeNode *to = nodes_by_pos[next_pos][j];
                    
                    // ⭐ 添加词性转移成本！
                    double trans_cost = misaki_get_transition_cost(from->feature, to->feature);
                    
                    misaki_lattice_add_edge(from, to, trans_cost);
                }
            } else if (next_pos == text_len) {
                // 连接到 EOS
                misaki_lattice_add_edge(from, lattice->eos, 0.0);
            }
        }
    }
    
    // 5. 执行 Viterbi 算法
    bool success = misaki_viterbi_search(lattice);
    if (!success) {
        misaki_lattice_free(lattice);
        for (int i = 0; i <= text_len; i++) {
            free(nodes_by_pos[i]);
        }
        free(nodes_by_pos);
        free(counts_by_pos);
        return NULL;
    }
    
    // 6. 提取最优路径
    MisakiTokenList *result = misaki_viterbi_extract_tokens(lattice);
    
    // 7. 清理
    misaki_lattice_free(lattice);
    for (int i = 0; i <= text_len; i++) {
        free(nodes_by_pos[i]);
    }
    free(nodes_by_pos);
    free(counts_by_pos);
    
    return result;
}

MisakiTokenList* misaki_ja_tokenize(void *tokenizer, const char *text) {
    if (!tokenizer || !text) {
        return NULL;
    }
    
    JaTokenizer *ja = (JaTokenizer *)tokenizer;
    
    // 强制使用 Viterbi 模式
    return ja_tokenize_viterbi(ja, text);
}
