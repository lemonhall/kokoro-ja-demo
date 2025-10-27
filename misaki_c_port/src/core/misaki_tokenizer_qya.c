/**
 * @file misaki_tokenizer_qya.c
 * @brief Quenya (昆雅语) Tokenizer Implementation
 * 
 * 实现基于空格和标点的简单分词策略
 */

#include "misaki_tokenizer_qya.h"
#include "misaki_string.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ============ 辅助函数 ============ */

int misaki_qya_is_punctuation(char c) {
    // 常见标点符号
    return (c == '.' || c == ',' || c == '!' || c == '?' || 
            c == ';' || c == ':' || c == '\'' || c == '"' ||
            c == '(' || c == ')' || c == '[' || c == ']' ||
            c == '-' || c == '—');
}

int misaki_qya_is_letter(unsigned char c) {
    // ASCII字母
    if (isalpha(c)) return 1;
    
    // UTF-8多字节字符（昆雅语特殊字符）
    if (c >= 0xC0) return 1;
    
    return 0;
}

/**
 * @brief UTF-8字符长度
 */
static int utf8_char_len(const char* str) {
    unsigned char c = (unsigned char)*str;
    if (c < 0x80) return 1;
    if ((c & 0xE0) == 0xC0) return 2;
    if ((c & 0xF0) == 0xE0) return 3;
    if ((c & 0xF8) == 0xF0) return 4;
    return 1;
}

/* ============ API实现 ============ */

int misaki_tokenizer_qya_init(void) {
    // 当前不需要初始化资源
    return 0;
}

void misaki_tokenizer_qya_cleanup(void) {
    // 当前不需要清理资源
}

int misaki_tokenize_qya(const char* text, MisakiToken** tokens, int* token_count) {
    if (!text || !tokens || !token_count) return -1;
    
    size_t text_len = strlen(text);
    if (text_len == 0) {
        *tokens = NULL;
        *token_count = 0;
        return 0;
    }
    
    // 预分配token数组（假设平均每5个字符一个token）
    int capacity = (text_len / 5) + 10;
    MisakiToken* token_array = (MisakiToken*)malloc(sizeof(MisakiToken) * capacity);
    if (!token_array) return -1;
    
    int count = 0;
    const char* p = text;
    
    while (*p) {
        // 跳过空白字符
        while (*p && isspace((unsigned char)*p)) {
            p++;
        }
        if (!*p) break;
        
        // 扩容检查
        if (count >= capacity) {
            capacity *= 2;
            MisakiToken* new_array = (MisakiToken*)realloc(token_array, sizeof(MisakiToken) * capacity);
            if (!new_array) {
                // 释放已分配的token
                for (int i = 0; i < count; i++) {
                    free(token_array[i].text);
                }
                free(token_array);
                return -1;
            }
            token_array = new_array;
        }
        
        const char* token_start = p;
        int token_len = 0;
        
        // 判断token类型
        if (misaki_qya_is_punctuation(*p)) {
            // 标点符号
            token_len = utf8_char_len(p);
            p += token_len;
            
            token_array[count].text = (char*)malloc(token_len + 1);
            if (!token_array[count].text) {
                for (int i = 0; i < count; i++) {
                    free(token_array[i].text);
                }
                free(token_array);
                return -1;
            }
            memcpy(token_array[count].text, token_start, token_len);
            token_array[count].text[token_len] = '\0';
            token_array[count].type = TOKEN_PUNCT;
            count++;
            
        } else if (isdigit((unsigned char)*p)) {
            // 数字
            while (*p && isdigit((unsigned char)*p)) {
                p++;
                token_len++;
            }
            
            token_array[count].text = (char*)malloc(token_len + 1);
            if (!token_array[count].text) {
                for (int i = 0; i < count; i++) {
                    free(token_array[i].text);
                }
                free(token_array);
                return -1;
            }
            memcpy(token_array[count].text, token_start, token_len);
            token_array[count].text[token_len] = '\0';
            token_array[count].type = TOKEN_NUM;
            count++;
            
        } else if (misaki_qya_is_letter((unsigned char)*p)) {
            // 昆雅语单词（字母序列）
            while (*p && (misaki_qya_is_letter((unsigned char)*p) || *p == '\'')) {
                int char_len = utf8_char_len(p);
                p += char_len;
                token_len += char_len;
            }
            
            token_array[count].text = (char*)malloc(token_len + 1);
            if (!token_array[count].text) {
                for (int i = 0; i < count; i++) {
                    free(token_array[i].text);
                }
                free(token_array);
                return -1;
            }
            memcpy(token_array[count].text, token_start, token_len);
            token_array[count].text[token_len] = '\0';
            token_array[count].type = TOKEN_WORD;
            count++;
            
        } else {
            // 未知字符，跳过
            int char_len = utf8_char_len(p);
            p += char_len;
        }
    }
    
    *tokens = token_array;
    *token_count = count;
    return 0;
}
