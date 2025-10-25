/**
 * misaki_tokenizer.c
 * 
 * Misaki C Port - Tokenizer Implementation
 * 分词器实现（中文 jieba + 日文 MeCab）
 * 严格按照 misaki_tokenizer.h 定义实现
 * 
 * License: MIT
 */

#include "misaki_tokenizer.h"
#include "misaki_string.h"
#include "misaki_trie.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

/* ============================================================================
 * Token 操作实现
 * ========================================================================== */

MisakiToken* misaki_token_create(const char *text,
                                  const char *tag,
                                  int start,
                                  int length) {
    if (!text) {
        return NULL;
    }
    
    MisakiToken *token = (MisakiToken *)calloc(1, sizeof(MisakiToken));
    if (!token) {
        return NULL;
    }
    
    token->text = misaki_strdup(text);
    token->tag = tag ? misaki_strdup(tag) : NULL;
    token->start = start;
    token->length = length;
    token->score = 0.0;
    
    return token;
}

void misaki_token_free(MisakiToken *token) {
    if (!token) {
        return;
    }
    
    free(token->text);
    free(token->tag);
    free(token->phonemes);
    free(token->whitespace);
    free(token);
}

MisakiToken* misaki_token_clone(const MisakiToken *token) {
    if (!token) {
        return NULL;
    }
    
    MisakiToken *clone = misaki_token_create(token->text, token->tag,
                                              token->start, token->length);
    if (clone) {
        clone->phonemes = token->phonemes ? misaki_strdup(token->phonemes) : NULL;
        clone->whitespace = token->whitespace ? misaki_strdup(token->whitespace) : NULL;
        clone->score = token->score;
    }
    
    return clone;
}

bool misaki_token_set_phonemes(MisakiToken *token, const char *phonemes) {
    if (!token) {
        return false;
    }
    
    free(token->phonemes);
    token->phonemes = phonemes ? misaki_strdup(phonemes) : NULL;
    return true;
}

void misaki_token_set_score(MisakiToken *token, double score) {
    if (token) {
        token->score = score;
    }
}

/* ============================================================================
 * Token 列表操作实现
 * ========================================================================== */

MisakiTokenList* misaki_token_list_create(void) {
    MisakiTokenList *list = (MisakiTokenList *)malloc(sizeof(MisakiTokenList));
    if (!list) {
        return NULL;
    }
    
    list->capacity = 16;
    list->count = 0;
    list->tokens = (MisakiToken *)calloc(list->capacity, sizeof(MisakiToken));
    
    if (!list->tokens) {
        free(list);
        return NULL;
    }
    
    return list;
}

void misaki_token_list_free(MisakiTokenList *list) {
    if (!list) {
        return;
    }
    
    for (int i = 0; i < list->count; i++) {
        free(list->tokens[i].text);
        free(list->tokens[i].tag);
        free(list->tokens[i].phonemes);
        free(list->tokens[i].whitespace);
    }
    
    free(list->tokens);
    free(list);
}

bool misaki_token_list_add(MisakiTokenList *list, const MisakiToken *token) {
    if (!list || !token) {
        return false;
    }
    
    // 扩容
    if (list->count >= list->capacity) {
        int new_capacity = list->capacity * 2;
        MisakiToken *new_tokens = (MisakiToken *)realloc(
            list->tokens, sizeof(MisakiToken) * new_capacity);
        if (!new_tokens) {
            return false;
        }
        list->tokens = new_tokens;
        list->capacity = new_capacity;
    }
    
    // 复制 token
    MisakiToken *dest = &list->tokens[list->count];
    memset(dest, 0, sizeof(MisakiToken));
    
    dest->text = misaki_strdup(token->text);
    dest->tag = token->tag ? misaki_strdup(token->tag) : NULL;
    dest->phonemes = token->phonemes ? misaki_strdup(token->phonemes) : NULL;
    dest->whitespace = token->whitespace ? misaki_strdup(token->whitespace) : NULL;
    dest->start = token->start;
    dest->length = token->length;
    dest->score = token->score;
    
    list->count++;
    return true;
}

MisakiToken* misaki_token_list_get(const MisakiTokenList *list, int index) {
    if (!list || index < 0 || index >= list->count) {
        return NULL;
    }
    
    return &list->tokens[index];
}

int misaki_token_list_size(const MisakiTokenList *list) {
    return list ? list->count : 0;
}

void misaki_token_list_clear(MisakiTokenList *list) {
    if (!list) {
        return;
    }
    
    for (int i = 0; i < list->count; i++) {
        free(list->tokens[i].text);
        free(list->tokens[i].tag);
        free(list->tokens[i].phonemes);
        free(list->tokens[i].whitespace);
    }
    
    list->count = 0;
}

/* ============================================================================
 * DAG（有向无环图）操作实现
 * ========================================================================== */

DAG* misaki_dag_create(int text_length) {
    if (text_length <= 0) {
        return NULL;
    }
    
    DAG *dag = (DAG *)calloc(1, sizeof(DAG));
    if (!dag) {
        return NULL;
    }
    
    dag->length = text_length;
    dag->capacity = text_length;
    
    // 为每个位置分配节点
    dag->nodes = (DAGNode *)calloc(text_length, sizeof(DAGNode));
    if (!dag->nodes) {
        free(dag);
        return NULL;
    }
    
    // 初始化每个节点
    for (int i = 0; i < text_length; i++) {
        dag->nodes[i].capacity = 4;  // 初始容量
        dag->nodes[i].count = 0;
        dag->nodes[i].next_positions = (int *)malloc(sizeof(int) * 4);
        if (!dag->nodes[i].next_positions) {
            // 释放已分配的内存
            for (int j = 0; j < i; j++) {
                free(dag->nodes[j].next_positions);
            }
            free(dag->nodes);
            free(dag);
            return NULL;
        }
    }
    
    return dag;
}

void misaki_dag_free(DAG *dag) {
    if (!dag) {
        return;
    }
    
    if (dag->nodes) {
        for (int i = 0; i < dag->length; i++) {
            free(dag->nodes[i].next_positions);
        }
        free(dag->nodes);
    }
    
    free(dag);
}

bool misaki_dag_add_edge(DAG *dag, int from, int to) {
    if (!dag || from < 0 || from >= dag->length || to < from || to > dag->length) {
        return false;
    }
    
    DAGNode *node = &dag->nodes[from];
    
    // 检查是否已存在
    for (int i = 0; i < node->count; i++) {
        if (node->next_positions[i] == to) {
            return true;  // 已存在
        }
    }
    
    // 扩容
    if (node->count >= node->capacity) {
        int new_capacity = node->capacity * 2;
        int *new_next = (int *)realloc(node->next_positions, sizeof(int) * new_capacity);
        if (!new_next) {
            return false;
        }
        node->next_positions = new_next;
        node->capacity = new_capacity;
    }
    
    // 添加边
    node->next_positions[node->count++] = to;
    return true;
}

int misaki_dag_get_next(const DAG *dag,
                        int position,
                        int *next_positions,
                        int max_count) {
    if (!dag || position < 0 || position >= dag->length || !next_positions || max_count <= 0) {
        return 0;
    }
    
    const DAGNode *node = &dag->nodes[position];
    int count = node->count < max_count ? node->count : max_count;
    
    for (int i = 0; i < count; i++) {
        next_positions[i] = node->next_positions[i];
    }
    
    return count;
}

DAG* misaki_dag_build(const char *text, const Trie *trie) {
    if (!text || !trie) {
        return NULL;
    }
    
    // 计算字符数（不是字节数）
    int char_count = (int)misaki_utf8_length(text);
    if (char_count == 0) {
        return NULL;
    }
    
    DAG *dag = misaki_dag_create(char_count + 1);  // +1 for EOS
    if (!dag) {
        return NULL;
    }
    
    // 从每个位置开始，使用 Trie 树查找所有可能的词
    int byte_pos = 0;
    for (int char_pos = 0; char_pos < char_count; char_pos++) {
        // 使用 Trie 进行前缀匹配
        TrieMatch matches[100];
        int match_count = misaki_trie_match_all(trie, text, byte_pos, matches, 100);
        
        if (match_count > 0) {
            // 为每个匹配添加边
            for (int i = 0; i < match_count; i++) {
                // 计算匹配词的字符长度
                int word_char_len = (int)misaki_utf8_length(matches[i].word);
                int next_char_pos = char_pos + word_char_len;
                
                misaki_dag_add_edge(dag, char_pos, next_char_pos);
            }
        } else {
            // 没有匹配，添加单字边
            misaki_dag_add_edge(dag, char_pos, char_pos + 1);
        }
        
        // 移动到下一个字符
        uint32_t codepoint;
        int bytes = misaki_utf8_decode(text + byte_pos, &codepoint);
        if (bytes == 0) {
            break;
        }
        byte_pos += bytes;
    }
    
    return dag;
}

/* ============================================================================
 * 调试与统计实现
 * ========================================================================== */

void misaki_token_list_print(const MisakiTokenList *list) {
    if (!list) {
        printf("(null)\n");
        return;
    }
    
    printf("TokenList (%d tokens):\n", list->count);
    for (int i = 0; i < list->count; i++) {
        const MisakiToken *token = &list->tokens[i];
        printf("  [%d] \"%s\"", i, token->text);
        if (token->tag) {
            printf(" (%s)", token->tag);
        }
        if (token->phonemes) {
            printf(" -> %s", token->phonemes);
        }
        printf(" [%d:%d, score=%.2f]\n", token->start, token->length, token->score);
    }
}

void misaki_dag_print(const DAG *dag, const char *text) {
    if (!dag) {
        printf("(null)\n");
        return;
    }
    
    printf("DAG (length=%d):\n", dag->length);
    for (int i = 0; i < dag->length; i++) {
        printf("  [%d] ->", i);
        for (int j = 0; j < dag->nodes[i].count; j++) {
            printf(" %d", dag->nodes[i].next_positions[j]);
        }
        printf("\n");
    }
}

void misaki_token_list_stats(const MisakiTokenList *list,
                             int *total_tokens,
                             double *avg_token_length,
                             int *max_token_length) {
    if (!list) {
        return;
    }
    
    if (total_tokens) {
        *total_tokens = list->count;
    }
    
    if (avg_token_length || max_token_length) {
        int total_len = 0;
        int max_len = 0;
        
        for (int i = 0; i < list->count; i++) {
            int len = list->tokens[i].length;
            total_len += len;
            if (len > max_len) {
                max_len = len;
            }
        }
        
        if (avg_token_length) {
            *avg_token_length = list->count > 0 ? (double)total_len / list->count : 0.0;
        }
        
        if (max_token_length) {
            *max_token_length = max_len;
        }
    }
}

/* ============================================================================
 * 中文分词器实现（jieba 算法）
 * 算法流程：
 * 1. 基于 Trie 树构建 DAG（有向无环图）
 * 2. 动态规划计算最大概率路径
 * 3. 根据路径切分文本生成 Token 列表
 * ========================================================================== */

// 中文分词器结构体
typedef struct {
    Trie *dict_trie;       // 词典 Trie 树
    Trie *user_trie;       // 用户词典（可选）
    bool enable_hmm;       // 是否启用 HMM
    bool enable_userdict;  // 是否启用用户词典
} ZhTokenizer;

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
    
    // 初始化：dp[n] = 0.0（终止位置）
    // route[i] 保存从位置 i 的最优下一个位置
    
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
            int word_len = next_pos - i;  // 词的字符长度
            
            // 计算从位置 i 到 next_pos 的词的分数
            // 提取子串
            int byte_start = 0;
            int byte_end = 0;
            
            // 计算字节偏移
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
                
                // 查询词频（使用 Trie 树）
                TrieMatch match;
                double freq = 0.5;  // 默认频率（单字）
                
                if (misaki_trie_match_longest(trie, text, byte_start, &match)) {
                    freq = match.frequency > 0 ? match.frequency : 1.0;
                }
                
                // 计算分数：log(freq) + dp[next_pos]
                // 使用对数避免下溢
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

/* ============================================================================
 * 日文分词器实现（TODO）
 * ========================================================================== */

void* misaki_ja_tokenizer_create(const JaTokenizerConfig *config) {
    (void)config;
    return NULL;
}

void misaki_ja_tokenizer_free(void *tokenizer) {
    (void)tokenizer;
}

MisakiTokenList* misaki_ja_tokenize(void *tokenizer, const char *text) {
    (void)tokenizer;
    (void)text;
    return NULL;
}

/* ============================================================================
 * 英文分词器实现（简单空格分割）
 * ========================================================================== */

MisakiTokenList* misaki_en_tokenize(const char *text) {
    return misaki_en_tokenize_ex(text, false);
}

MisakiTokenList* misaki_en_tokenize_ex(const char *text, bool keep_punctuation) {
    if (!text) {
        return NULL;
    }
    
    MisakiTokenList *list = misaki_token_list_create();
    if (!list) {
        return NULL;
    }
    
    // TODO: 实现空格和标点分割
    (void)keep_punctuation;
    
    return list;
}

/* ============================================================================
 * 通用分词器实现
 * ========================================================================== */

MisakiLanguage misaki_detect_language(const char *text) {
    // TODO: 实现语言检测
    (void)text;
    return LANG_UNKNOWN;
}

MisakiTokenList* misaki_tokenize(MisakiContext *context,
                                  const char *text,
                                  MisakiLanguage lang) {
    // TODO: 实现
    (void)context;
    (void)text;
    (void)lang;
    return NULL;
}

/* ============================================================================
 * HMM 实现（TODO）
 * ========================================================================== */

HMMModel* misaki_hmm_load(const char *file_path) {
    (void)file_path;
    return NULL;
}

void misaki_hmm_free(HMMModel *model) {
    free(model);
}

int misaki_hmm_viterbi(const HMMModel *model,
                       const char *text,
                       HMMState *states,
                       int max_length) {
    (void)model;
    (void)text;
    (void)states;
    (void)max_length;
    return 0;
}

MisakiTokenList* misaki_hmm_cut(const HMMModel *model, const char *text) {
    (void)model;
    (void)text;
    return NULL;
}
