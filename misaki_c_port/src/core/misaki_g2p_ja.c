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
 * 架构说明：
 *   1. dict_trie：词典 Trie 树，用于查询日文词汇的假名读音（pron 字段）
 *   2. tokenizer：日文分词器（内部也包含 Trie，但用于分词，不是查询读音）
 *   3. 两者配合：分词器负责切分，dict_trie 负责提供读音
 * 
 * 处理流程：
 *   文本 → 分词（Viterbi）→ 词典查询读音 → 假名→IPA 转换
 * 
 * 降级策略：
 *   - 优先：从 dict_trie 查询读音（支持汉字→假名）
 *   - 降级：直接转换 token 文本（仅支持纯假名）
 *   - 兜底：保留原文 + 警告
 * 
 * @param dict_trie 词典 Trie 树（用于查询读音，必须包含 pron 字段）
 * @param tokenizer 分词器（用于文本切分）
 * @param text 输入文本（UTF-8）
 * @param options G2P 选项（可为 NULL）
 * @return Token 列表，每个 token 包含原文、读音、音素，失败返回 NULL
 * 
 * @note 内存管理：返回的 MisakiTokenList 需要调用 misaki_token_list_free 释放
 */
MisakiTokenList* misaki_ja_g2p(const Trie *dict_trie,
                               void *tokenizer,
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
    for (int i = 0; i < tokens->count; i++) {
        MisakiToken *token = &tokens->tokens[i];
        
        // ⭐ 优先：从词典查询读音（假名）
        const char *pron = NULL;
        if (dict_trie && misaki_trie_lookup_with_pron(dict_trie, token->text, &pron, NULL, NULL)) {
            if (pron && strlen(pron) > 0) {
                // 将假名读音转换为 IPA
                char *phonemes = misaki_ja_kana_to_ipa(pron);
                if (phonemes) {
                    // ⭐ 安全：释放旧的 phonemes（如果存在且不是 text）
                    if (token->phonemes && token->phonemes != token->text) {
                        free(token->phonemes);
                    }
                    token->phonemes = phonemes;
                    continue;  // 成功转换，处理下一个 token
                }
            }
        }
        
        // 降级：尝试直接将文本转换为 IPA（适用于纯假名文本）
        char *phonemes = misaki_ja_kana_to_ipa(token->text);
        if (phonemes) {
            // ⭐ 安全：释放旧的 phonemes（如果存在且不是 text）
            if (token->phonemes && token->phonemes != token->text) {
                free(token->phonemes);
            }
            token->phonemes = phonemes;
        } else {
            // 无法转换的情况（未登录词、汉字等）
            // 保留原文作为后备
            if (!token->phonemes) {
                // ⭐ 安全：使用 strdup 而不是直接引用
                token->phonemes = misaki_strdup(token->text);
            }
            fprintf(stderr, "[G2P Warning] Cannot convert to IPA: %s\n", token->text);
        }
    }
    
    // 3. 长音处理（⭐ 默认启用，因为这是日文核心特性）
    bool enable_long_vowel = true;
    if (options) {
        // 如果提供了 options，尊重其设置
        enable_long_vowel = options->ja_long_vowel;
    }
    
    if (enable_long_vowel) {
        misaki_ja_long_vowel(tokens);
    }
    
    return tokens;
}

/**
 * 日文长音处理
 * 
 * 处理日文中的长音现象，例如：
 *   - 长音符：コーヒー → koːçiː (已由 kana_map 处理)
 *   - 同元音重复：おおきい → oːkiː
 *   - 特殊组合：えい → eː, おう → oː (⭐ 重要！)
 * 
 * @param tokens Token 列表
 */
void misaki_ja_long_vowel(MisakiTokenList *tokens) {
    if (!tokens) {
        return;
    }
    
    // ⭐ 实现核心长音规则：处理日文长音现象
    for (int i = 0; i < tokens->count; i++) {
        MisakiToken *token = &tokens->tokens[i];
        if (!token->phonemes) {
            continue;
        }
        
        char *phonemes = token->phonemes;
        int len = strlen(phonemes);
        char result[1024] = {0};
        int pos = 0;
        
        for (int j = 0; j < len; ) {
            // ⭐ 检测 "oɯ" → "oː" (如 とう、そう、こう)
            if (j + 2 <= len && phonemes[j] == 'o' && phonemes[j+1] == 'ɯ') {
                result[pos++] = 'o';
                result[pos++] = 'ː';
                j += 2;
            }
            // ⭐ 检测 "ei" → "eː" (如 けい、せい、めい)
            else if (j + 2 <= len && phonemes[j] == 'e' && phonemes[j+1] == 'i') {
                result[pos++] = 'e';
                result[pos++] = 'ː';
                j += 2;
            }
            // ⭐ 检测 "aɯ" → "aː" (如 おう、かう)
            else if (j + 2 <= len && phonemes[j] == 'a' && phonemes[j+1] == 'ɯ') {
                result[pos++] = 'a';
                result[pos++] = 'ː';
                j += 2;
            }
            // 其他字符直接复制
            else {
                result[pos++] = phonemes[j++];
            }
        }
        
        result[pos] = '\0';
        
        // 如果有修改，更新 phonemes
        if (strcmp(result, phonemes) != 0) {
            free(token->phonemes);
            token->phonemes = misaki_strdup(result);
        }
    }
}
