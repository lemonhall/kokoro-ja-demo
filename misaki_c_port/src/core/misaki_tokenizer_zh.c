/**
 * misaki_tokenizer_zh.c
 * 
 * 中文分词器实现（jieba 算法）
 * 
 * 算法流程：
 * 1. 基于 Trie 树构建 DAG（有向无环图）
 * 2. 动态规划计算最大概率路径
 * 3. 根据路径切分文本生成 Token 列表
 * 
 * License: MIT
 */

#include "misaki_tokenizer.h"
#include "misaki_string.h"
#include "misaki_trie.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ============================================================================
 * 中文分词器结构体
 * ========================================================================== */

typedef struct {
    Trie *dict_trie;       // 词典 Trie 树
    Trie *user_trie;       // 用户词典（可选）
    bool enable_hmm;       // 是否启用 HMM
    bool enable_userdict;  // 是否启用用户词典
} ZhTokenizer;

/* ============================================================================
 * 中文分词器创建和销毁
 * ========================================================================== */

void* misaki_zh_tokenizer_create(const ZhTokenizerConfig *config) {
    if (!config || !config->dict_trie) {
        return NULL;
    }
    
    ZhTokenizer *tokenizer = (ZhTokenizer *)calloc(1, sizeof(ZhTokenizer));
    if (!tokenizer) {
        return NULL;
    }
    
    tokenizer->dict_trie = config->dict_trie;
    tokenizer->user_trie = config->user_trie;
    tokenizer->enable_hmm = config->enable_hmm;
    tokenizer->enable_userdict = config->enable_userdict;
    
    return tokenizer;
}

void misaki_zh_tokenizer_free(void *tokenizer) {
    if (tokenizer) {
        free(tokenizer);
    }
}

/* ============================================================================
 * 动态规划算法
 * ========================================================================== */

/**
 * 动态规划计算最大概率路径
 * route[i] 表示从位置 i 开始的最优下一个位置
 * 
 * @param dag DAG 图
 * @param trie 词典 Trie 树（用于查询词频）
 * @param text 文本
 * @param route 输出：路径数组
 * @return 成功返回 true
 */
static bool calculate_route(const DAG *dag, const Trie *trie, 
                           const char *text, int *route) {
    if (!dag || !trie || !text || !route) {
        return false;
    }
    
    int n = dag->length;
    double *dp = (double *)calloc(n + 1, sizeof(double));
    if (!dp) {
        return false;
    }
    
    // 从后往前动态规划
    for (int i = n - 1; i >= 0; i--) {
        double max_score = -INFINITY;
        int best_next = i + 1;  // 默认单字
        
        // 获取从位置 i 的所有可能后继
        int next_positions[100];
        int next_count = misaki_dag_get_next(dag, i, next_positions, 100);
        
        if (next_count == 0) {
            // 没有后继，强制单字
            route[i] = i + 1;
            dp[i] = dp[i + 1];
            continue;
        }
        
        // 遍历所有可能的后继，选择最大概率
        for (int j = 0; j < next_count; j++) {
            int next_pos = next_positions[j];
            
            // 计算字节偏移
            int byte_start = 0;
            int byte_end = 0;
            
            int char_idx = 0;
            int byte_pos = 0;
            const char *p = text;
            while (*p && char_idx <= next_pos) {
                if (char_idx == i) {
                    byte_start = byte_pos;
                }
                if (char_idx == next_pos) {
                    byte_end = byte_pos;
                    break;
                }
                
                uint32_t codepoint;
                int bytes = misaki_utf8_decode(p, &codepoint);
                if (bytes == 0) break;
                
                p += bytes;
                byte_pos += bytes;
                char_idx++;
            }
            
            if (char_idx == next_pos && byte_end == 0) {
                byte_end = byte_pos;
            }
            
            // 提取词
            int word_byte_len = byte_end - byte_start;
            char word[256];
            if (word_byte_len > 0 && word_byte_len < 256) {
                memcpy(word, text + byte_start, word_byte_len);
                word[word_byte_len] = '\0';
                
                // 查询词频
                TrieMatch match;
                double freq = 0.5;  // 默认频率（单字）
                
                if (misaki_trie_match_longest(trie, text, byte_start, &match)) {
                    freq = match.frequency > 0 ? match.frequency : 1.0;
                }
                
                // 计算分数：log(freq) + dp[next_pos]
                double word_score = log(freq);
                double total_score = word_score + dp[next_pos];
                
                if (total_score > max_score) {
                    max_score = total_score;
                    best_next = next_pos;
                }
            }
        }
        
        route[i] = best_next;
        dp[i] = max_score;
    }
    
    free(dp);
    return true;
}

/* ============================================================================
 * 中文分词主函数
 * ========================================================================== */

MisakiTokenList* misaki_zh_tokenize(void *tokenizer, const char *text) {
    if (!tokenizer || !text) {
        return NULL;
    }
    
    ZhTokenizer *zh = (ZhTokenizer *)tokenizer;
    
    // 1. 构建 DAG
    DAG *dag = misaki_dag_build(text, zh->dict_trie);
    if (!dag) {
        return NULL;
    }
    
    // 2. 动态规划计算路径
    int *route = (int *)calloc(dag->length, sizeof(int));
    if (!route) {
        misaki_dag_free(dag);
        return NULL;
    }
    
    if (!calculate_route(dag, zh->dict_trie, text, route)) {
        free(route);
        misaki_dag_free(dag);
        return NULL;
    }
    
    // 3. 根据路径切分文本
    MisakiTokenList *result = misaki_token_list_create();
    if (!result) {
        free(route);
        misaki_dag_free(dag);
        return NULL;
    }
    
    int char_pos = 0;
    int byte_pos = 0;
    
    while (char_pos < dag->length - 1) {
        int next_pos = route[char_pos];
        int word_char_len = next_pos - char_pos;
        
        // 计算字节长度
        const char *p = text + byte_pos;
        int word_byte_len = 0;
        for (int i = 0; i < word_char_len; i++) {
            uint32_t codepoint;
            int bytes = misaki_utf8_decode(p, &codepoint);
            if (bytes == 0) break;
            p += bytes;
            word_byte_len += bytes;
        }
        
        // 创建 Token
        if (word_byte_len > 0) {
            char word[256];
            memcpy(word, text + byte_pos, word_byte_len);
            word[word_byte_len] = '\0';
            
            MisakiToken *token = misaki_token_create(word, NULL, byte_pos, word_byte_len);
            if (token) {
                misaki_token_list_add(result, token);
                misaki_token_free(token);
            }
        }
        
        char_pos = next_pos;
        byte_pos += word_byte_len;
    }
    
    free(route);
    misaki_dag_free(dag);
    
    return result;
}

MisakiTokenList* misaki_zh_tokenize_all(void *tokenizer, const char *text) {
    // 全模式：返回所有可能的词（暂不实现）
    (void)tokenizer;
    (void)text;
    return NULL;
}

MisakiTokenList* misaki_zh_tokenize_search(void *tokenizer, const char *text) {
    // 搜索引擎模式：对长词再切分（暂不实现）
    (void)tokenizer;
    (void)text;
    return NULL;
}
