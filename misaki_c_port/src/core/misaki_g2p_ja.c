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
    
    // 3. 长音处理（如果启用）
    if (options && options->ja_long_vowel) {
        misaki_ja_long_vowel(tokens);
    }
    
    return tokens;
}

/**
 * 日文长音处理
 * 
 * 处理日文中的长音现象，例如：
 *   - 長音符：コーヒー → koːçiː (已由 kana_map 处理)
 *   - 同元音重复：おおきい → oːkiː (TODO)
 *   - 特殊组合：えい → eː (TODO)
 * 
 * 现状：
 *   - ✅ 長音符 ー 已由 kana_map 正确映射为 IPA ː
 *   - ⚠️  同元音重复处理尚未实现
 *   - ⚠️  特殊组合规则尚未实现
 * 
 * @param tokens Token 列表
 */
void misaki_ja_long_vowel(MisakiTokenList *tokens) {
    // 注意：kana_map 已经处理了长音符 ー → ː
    // 例如：「コーヒー」已经被正确转换为「koːçiː」
    
    // TODO: 实现额外的长音规则：
    // 1. 同元音重复：おおきい → oːkiː
    // 2. 特殊组合：えい、おう 等
    
    (void)tokens;
}
