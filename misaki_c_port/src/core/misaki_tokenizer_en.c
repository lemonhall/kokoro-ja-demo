/**
 * misaki_tokenizer_en.c
 * 
 * 英文分词器实现（空格 + 标点分割）
 * 
 * License: MIT
 */

#include "misaki_tokenizer.h"
#include "misaki_string.h"
#include <stdlib.h>
#include <string.h>

/* ============================================================================
 * 辅助函数
 * ========================================================================== */

// 判断是否为空白字符
static bool is_whitespace(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

// 判断是否为标点
static bool is_punctuation(char c) {
    return (c >= 33 && c <= 47) ||   // ! " # $ % & ' ( ) * + , - . /
           (c >= 58 && c <= 64) ||   // : ; < = > ? @
           (c >= 91 && c <= 96) ||   // [ \ ] ^ _ `
           (c >= 123 && c <= 126);   // { | } ~
}

/* ============================================================================
 * 英文分词主函数
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
    
    const char *p = text;
    int token_start = 0;
    int current_pos = 0;
    bool in_token = false;
    
    while (*p) {
        char c = *p;
        
        if (is_whitespace(c)) {
            // 空白字符，结束当前 token
            if (in_token) {
                int token_len = current_pos - token_start;
                if (token_len > 0) {
                    char word[256];
                    memcpy(word, text + token_start, token_len);
                    word[token_len] = '\0';
                    
                    MisakiToken *token = misaki_token_create(word, NULL, token_start, token_len);
                    if (token) {
                        misaki_token_list_add(list, token);
                        misaki_token_free(token);
                    }
                }
                in_token = false;
            }
        } else if (is_punctuation(c)) {
            // 标点字符
            if (in_token) {
                // 结束当前 token
                int token_len = current_pos - token_start;
                if (token_len > 0) {
                    char word[256];
                    memcpy(word, text + token_start, token_len);
                    word[token_len] = '\0';
                    
                    MisakiToken *token = misaki_token_create(word, NULL, token_start, token_len);
                    if (token) {
                        misaki_token_list_add(list, token);
                        misaki_token_free(token);
                    }
                }
                in_token = false;
            }
            
            // 如果保留标点，添加为单独 token
            if (keep_punctuation) {
                char punct[2] = {c, '\0'};
                MisakiToken *token = misaki_token_create(punct, NULL, current_pos, 1);
                if (token) {
                    misaki_token_list_add(list, token);
                    misaki_token_free(token);
                }
            }
        } else {
            // 普通字符
            if (!in_token) {
                token_start = current_pos;
                in_token = true;
            }
        }
        
        p++;
        current_pos++;
    }
    
    // 处理最后一个 token
    if (in_token) {
        int token_len = current_pos - token_start;
        if (token_len > 0) {
            char word[256];
            memcpy(word, text + token_start, token_len);
            word[token_len] = '\0';
            
            MisakiToken *token = misaki_token_create(word, NULL, token_start, token_len);
            if (token) {
                misaki_token_list_add(list, token);
                misaki_token_free(token);
            }
        }
    }
    
    return list;
}
