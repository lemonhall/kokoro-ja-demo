/**
 * misaki_g2p_en.c
 * 
 * 英文 G2P 实现（基于 CMUdict）
 * 
 * License: MIT
 */

#include "misaki_g2p.h"
#include "misaki_dict.h"
#include "misaki_tokenizer.h"
#include "misaki_string.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ============================================================================
 * 英文 G2P (CMUdict)
 * ========================================================================== */

char* misaki_en_g2p_word(const EnDict *dict,
                         const char *word,
                         const G2POptions *options) {
    if (!dict || !word) {
        return NULL;
    }
    
    (void)options;  // TODO: 使用选项（英式/美式发音）
    
    // 在词典中查找单词
    const char *phonemes = misaki_en_dict_lookup(dict, word);
    if (phonemes) {
        return misaki_strdup(phonemes);
    }
    
    // 未找到，尝试 OOV 处理
    return misaki_en_g2p_oov(word);
}

MisakiTokenList* misaki_en_g2p(const EnDict *dict,
                               const char *text,
                               const G2POptions *options) {
    if (!dict || !text) {
        return NULL;
    }
    
    // 1. 英文分词（空格分割）
    MisakiTokenList *tokens = misaki_en_tokenize(text);
    if (!tokens) {
        return NULL;
    }
    
    // 2. 为每个 Token 查询音素
    for (int i = 0; i < tokens->count; i++) {
        MisakiToken *token = &tokens->tokens[i];
        
        // 查询词典
        char *phonemes = misaki_en_g2p_word(dict, token->text, options);
        if (phonemes) {
            token->phonemes = phonemes;
        }
    }
    
    return tokens;
}

char* misaki_en_g2p_oov(const char *word) {
    if (!word) {
        return NULL;
    }
    
    // TODO: 实现基于规则的音素预测
    // 可以使用 Sequitur G2P 或简单的规则映射
    // 
    // 简化实现：返回原词（音译）
    return misaki_strdup(word);
}
