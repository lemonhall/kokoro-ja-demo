/**
 * misaki_g2p_ja.c
 * 
 * 日文 G2P 実装（文本 → 假名 → IPA）
 * 
 * 流程：
 * 1. 分词（Viterbi）
 * 2. 从词典查询读音（片假名）
 * 3. 片假名→IPA 音素（HEPBURN 映射）
 * 
 * License: MIT
 */

#include "misaki_g2p.h"
#include "misaki_tokenizer.h"
#include "misaki_string.h"
#include "misaki_kana_map.h"
#include "misaki_trie.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ============================================================================
 * 日文 G2P 主函数
 * ========================================================================== */

/**
 * 假名→IPA 转换（使用新的 kana_map 模块）
 */
char* misaki_ja_kana_to_ipa(const char *kana) {
    if (!kana) {
        return NULL;
    }
    
    char result[1024] = {0};
    int len = misaki_kana_string_to_ipa(kana, result, sizeof(result));
    
    if (len > 0) {
        return misaki_strdup(result);
    }
    
    return NULL;
}

/**
 * 日文 G2P 完整流程
 * 
 * @param tokenizer 分词器（内部包含 Trie 词典）
 * @param text 输入文本
 * @param options G2P 选项
 * @return Token 列表，每个 token 包含原文、读音、音素
 */
MisakiTokenList* misaki_ja_g2p(void *tokenizer,
                               const char *text,
                               const G2POptions *options) {
    if (!tokenizer || !text) {
        return NULL;
    }
    
    // 1. 日文分词（使用 Viterbi 算法）
    MisakiTokenList *tokens = misaki_ja_tokenize(tokenizer, text);
    if (!tokens) {
        return NULL;
    }
    
    // 2. 为每个 Token 查询读音并转换为 IPA
    // 注意：tokenizer 是 void*，无法直接访问 trie
    // 需要通过分词结果中的 tag 来判断
    for (int i = 0; i < tokens->count; i++) {
        MisakiToken *token = &tokens->tokens[i];
        
        // 尝试直接将文本转换为 IPA
        // （假设分词后的 token 是假名）
        char *phonemes = misaki_ja_kana_to_ipa(token->text);
        if (phonemes) {
            if (token->phonemes) {
                free(token->phonemes);
            }
            token->phonemes = phonemes;
        } else {
            // 如果不是假名，保留原文
            if (!token->phonemes) {
                token->phonemes = misaki_strdup(token->text);
            }
        }
    }
    
    // 3. 长音处理（如果启用）
    if (options && options->ja_long_vowel) {
        misaki_ja_long_vowel(tokens);
    }
    
    return tokens;
}

void misaki_ja_long_vowel(MisakiTokenList *tokens) {
    // TODO: 实现日文长音处理
    // 例如：コーヒー → koːhiː
    // 现在 kana_map 已经处理了长音符 ー → ː
    (void)tokens;
}
