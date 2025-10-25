/**
 * misaki_tokenizer_ja.c
 * 
 * 日文分词器实现（基于 Trie 树贪婪匹配）
 * 
 * 简化实现：使用最长匹配算法
 * 完整版需要实现 Viterbi 算法（类似 MeCab）
 * 
 * License: MIT
 */

#include "misaki_tokenizer.h"
#include "misaki_string.h"
#include "misaki_trie.h"
#include <stdlib.h>
#include <string.h>

/* ============================================================================
 * 日文分词器结构体
 * ========================================================================== */

typedef struct {
    Trie *dict_trie;       // 词典 Trie 树
    bool use_simple_model; // 使用简化模型
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
    
    return tokenizer;
}

void misaki_ja_tokenizer_free(void *tokenizer) {
    if (tokenizer) {
        free(tokenizer);
    }
}

/* ============================================================================
 * 日文分词主函数
 * ========================================================================== */

MisakiTokenList* misaki_ja_tokenize(void *tokenizer, const char *text) {
    if (!tokenizer || !text) {
        return NULL;
    }
    
    JaTokenizer *ja = (JaTokenizer *)tokenizer;
    
    // 简化实现：使用 Trie 树贪婪匹配（类似 MeCab）
    MisakiTokenList *result = misaki_token_list_create();
    if (!result) {
        return NULL;
    }
    
    int byte_pos = 0;
    const char *p = text;
    
    while (*p) {
        // 尝试从当前位置匹配最长的词
        TrieMatch match;
        bool found = misaki_trie_match_longest(ja->dict_trie, text, byte_pos, &match);
        
        if (found && match.length > 0) {
            // 找到匹配，添加 token
            MisakiToken *token = misaki_token_create(match.word, NULL, byte_pos, match.length);
            if (token) {
                misaki_token_list_add(result, token);
                misaki_token_free(token);
            }
            
            // 移动指针
            p += match.length;
            byte_pos += match.length;
        } else {
            // 没找到，单字符切分
            uint32_t codepoint;
            int bytes = misaki_utf8_decode(p, &codepoint);
            if (bytes == 0) {
                break;
            }
            
            char single_char[8];
            memcpy(single_char, p, bytes);
            single_char[bytes] = '\0';
            
            MisakiToken *token = misaki_token_create(single_char, NULL, byte_pos, bytes);
            if (token) {
                misaki_token_list_add(result, token);
                misaki_token_free(token);
            }
            
            p += bytes;
            byte_pos += bytes;
        }
    }
    
    return result;
}
