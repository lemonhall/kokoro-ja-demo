/**
 * misaki_g2p.c
 * 
 * Misaki C Port - G2P Implementation
 * G2P (Grapheme-to-Phoneme) 转换实现
 * 严格按照 misaki_g2p.h 定义实现
 * 
 * License: MIT
 */

#include "misaki_g2p.h"
#include "misaki_string.h"
#include "misaki_dict.h"
#include "misaki_tokenizer.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

/* ============================================================================
 * G2P 配置
 * ========================================================================== */

G2POptions misaki_g2p_default_options(void) {
    G2POptions options = {
        .normalize_text = true,
        .remove_punctuation = false,
        .output_separators = true,
        
        .zh_tone_sandhi = true,
        .zh_erhua = true,
        .zh_neutral_tone = true,
        
        .ja_accent = false,
        .ja_long_vowel = true,
        
        .en_use_gb = false,
        .en_syllable_boundary = false
    };
    return options;
}

/* ============================================================================
 * 英文 G2P (CMUdict)
 * ========================================================================== */

char* misaki_en_g2p_word(const EnDict *dict,
                         const char *word,
                         const G2POptions *options) {
    if (!dict || !word) {
        return NULL;
    }
    
    (void)options;  // TODO: 使用选项
    
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
    // 简化实现：返回原词（音译）
    return misaki_strdup(word);
}

/* ============================================================================
 * 中文 G2P (汉字 → 拼音 → IPA)
 * ========================================================================== */

char* misaki_zh_pinyin_to_ipa(const char *pinyin) {
    if (!pinyin) {
        return NULL;
    }
    
    // TODO: 实现拼音到 IPA 的映射
    // 简化实现：暂时返回拼音本身
    return misaki_strdup(pinyin);
}

MisakiTokenList* misaki_zh_g2p(const ZhDict *dict,
                               void *tokenizer,
                               const char *text,
                               const G2POptions *options) {
    if (!dict || !tokenizer || !text) {
        return NULL;
    }
    
    // 1. 中文分词
    MisakiTokenList *tokens = misaki_zh_tokenize(tokenizer, text);
    if (!tokens) {
        return NULL;
    }
    
    // 2. 为每个 Token 查询拼音
    for (int i = 0; i < tokens->count; i++) {
        MisakiToken *token = &tokens->tokens[i];
        
        // 拼接所有字的拼音
        char pinyin_result[256] = {0};
        int pinyin_pos = 0;
        
        const char *p = token->text;
        while (*p) {
            uint32_t codepoint;
            int bytes = misaki_utf8_decode(p, &codepoint);
            if (bytes == 0) break;
            
            // 查询单字拼音
            const char **pinyins = NULL;
            int pinyin_count = 0;
            if (misaki_zh_dict_lookup(dict, codepoint, &pinyins, &pinyin_count)) {
                if (pinyin_count > 0 && pinyins[0]) {
                    // 使用第一个拼音（简化处理，完整版需要根据上下文选择）
                    int len = strlen(pinyins[0]);
                    if (pinyin_pos + len + 1 < 256) {
                        if (pinyin_pos > 0) {
                            pinyin_result[pinyin_pos++] = ' ';
                        }
                        strcpy(pinyin_result + pinyin_pos, pinyins[0]);
                        pinyin_pos += len;
                    }
                }
            }
            
            p += bytes;
        }
        
        if (pinyin_pos > 0) {
            // 转换拼音为 IPA（简化版：直接使用拼音）
            token->phonemes = misaki_strdup(pinyin_result);
        }
    }
    
    // 3. 声调变化处理（如果启用）
    if (options && options->zh_tone_sandhi) {
        misaki_zh_tone_sandhi(tokens, options);
    }
    
    // 4. 儿化音处理（如果启用）
    if (options && options->zh_erhua) {
        misaki_zh_erhua(tokens);
    }
    
    return tokens;
}

void misaki_zh_tone_sandhi(MisakiTokenList *tokens,
                           const G2POptions *options) {
    // TODO: 实现中文声调变化规则
    (void)tokens;
    (void)options;
}

void misaki_zh_erhua(MisakiTokenList *tokens) {
    // TODO: 实现儿化音处理
    (void)tokens;
}

/* ============================================================================
 * 日文 G2P (假名 → IPA)
 * ========================================================================== */

char* misaki_ja_kana_to_ipa(const char *kana) {
    if (!kana) {
        return NULL;
    }
    
    // TODO: 实现假名到 IPA 的映射（基于 OpenJTalk 规则）
    // 简化实现：返回罗马音
    return misaki_strdup(kana);
}

MisakiTokenList* misaki_ja_g2p(void *tokenizer,
                               const char *text,
                               const G2POptions *options) {
    if (!tokenizer || !text) {
        return NULL;
    }
    
    // 1. 日文分词
    MisakiTokenList *tokens = misaki_ja_tokenize(tokenizer, text);
    if (!tokens) {
        return NULL;
    }
    
    // 2. 为每个 Token 转换假名为音素
    for (int i = 0; i < tokens->count; i++) {
        MisakiToken *token = &tokens->tokens[i];
        
        char *phonemes = misaki_ja_kana_to_ipa(token->text);
        if (phonemes) {
            token->phonemes = phonemes;
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
    (void)tokens;
}

/* ============================================================================
 * 韩文 G2P
 * ========================================================================== */

char* misaki_ko_char_to_ipa(const char *hangul) {
    // TODO: 实现韩文音素转换
    (void)hangul;
    return NULL;
}

MisakiTokenList* misaki_ko_g2p(const char *text,
                               const G2POptions *options) {
    // TODO: 实现韩文 G2P
    (void)text;
    (void)options;
    return NULL;
}

/* ============================================================================
 * 越南文 G2P
 * ========================================================================== */

MisakiTokenList* misaki_vi_g2p(const char *text,
                               const G2POptions *options) {
    // TODO: 实现越南文 G2P
    (void)text;
    (void)options;
    return NULL;
}

/* ============================================================================
 * 通用 G2P 接口
 * ========================================================================== */

MisakiTokenList* misaki_g2p(MisakiContext *context,
                            const char *text,
                            MisakiLanguage lang,
                            const G2POptions *options) {
    if (!context || !text) {
        return NULL;
    }
    
    // 如果未指定语言，自动检测
    if (lang == LANG_UNKNOWN) {
        lang = misaki_detect_language(text);
    }
    
    // 根据语言调用相应的 G2P 函数
    switch (lang) {
        case LANG_ENGLISH:
            return misaki_en_g2p(context->en_dict_us, text, options);
            
        case LANG_CHINESE:
            // TODO: 需要先创建中文分词器
            return NULL;
            
        case LANG_JAPANESE:
            // TODO: 需要先创建日文分词器
            return NULL;
            
        default:
            return NULL;
    }
}

int misaki_g2p_batch(MisakiContext *context,
                     const char **texts,
                     int count,
                     MisakiLanguage lang,
                     const G2POptions *options,
                     MisakiTokenList **results) {
    if (!context || !texts || !results || count <= 0) {
        return 0;
    }
    
    int success_count = 0;
    for (int i = 0; i < count; i++) {
        results[i] = misaki_g2p(context, texts[i], lang, options);
        if (results[i]) {
            success_count++;
        }
    }
    
    return success_count;
}

/* ============================================================================
 * 音素后处理
 * ========================================================================== */

char* misaki_normalize_phonemes(const char *phonemes) {
    if (!phonemes) {
        return NULL;
    }
    
    // TODO: 实现音素规范化
    return misaki_strdup(phonemes);
}

char* misaki_merge_phonemes(const MisakiTokenList *tokens,
                            const char *separator) {
    if (!tokens) {
        return NULL;
    }
    
    if (!separator) {
        separator = " ";
    }
    
    // 计算总长度
    int total_len = 0;
    for (int i = 0; i < tokens->count; i++) {
        if (tokens->tokens[i].phonemes) {
            total_len += strlen(tokens->tokens[i].phonemes);
            if (i < tokens->count - 1) {
                total_len += strlen(separator);
            }
        }
    }
    
    if (total_len == 0) {
        return misaki_strdup("");
    }
    
    // 拼接音素
    char *result = (char *)malloc(total_len + 1);
    if (!result) {
        return NULL;
    }
    
    int pos = 0;
    for (int i = 0; i < tokens->count; i++) {
        if (tokens->tokens[i].phonemes) {
            int len = strlen(tokens->tokens[i].phonemes);
            memcpy(result + pos, tokens->tokens[i].phonemes, len);
            pos += len;
            
            if (i < tokens->count - 1) {
                int sep_len = strlen(separator);
                memcpy(result + pos, separator, sep_len);
                pos += sep_len;
            }
        }
    }
    result[pos] = '\0';
    
    return result;
}

void misaki_add_boundaries(MisakiTokenList *tokens,
                          const char *word_boundary,
                          const char *sentence_boundary) {
    // TODO: 实现边界标记添加
    (void)tokens;
    (void)word_boundary;
    (void)sentence_boundary;
}

/* ============================================================================
 * 文本规范化
 * ========================================================================== */

char* misaki_normalize_text(const char *text, MisakiLanguage lang) {
    // TODO: 实现文本规范化
    (void)lang;
    return text ? misaki_strdup(text) : NULL;
}

char* misaki_zh_num_to_text(const char *text) {
    // TODO: 实现中文数字转文字
    return text ? misaki_strdup(text) : NULL;
}

char* misaki_en_num_to_text(const char *text) {
    // TODO: 实现英文数字转文字
    return text ? misaki_strdup(text) : NULL;
}

char* misaki_fullwidth_to_halfwidth(const char *text) {
    // TODO: 实现全角转半角
    return text ? misaki_strdup(text) : NULL;
}

char* misaki_traditional_to_simplified(const char *text) {
    // TODO: 实现繁体转简体
    return text ? misaki_strdup(text) : NULL;
}

/* ============================================================================
 * IPA 音素工具
 * ========================================================================== */

bool misaki_is_ipa_phoneme(uint32_t codepoint) {
    // IPA 音素 Unicode 范围
    return (codepoint >= 0x0250 && codepoint <= 0x02AF) ||  // IPA Extensions
           (codepoint >= 0x1D00 && codepoint <= 0x1D7F) ||  // Phonetic Extensions
           (codepoint >= 0x1D80 && codepoint <= 0x1DBF);    // Phonetic Extensions Supplement
}

bool misaki_validate_phonemes(const char *phonemes) {
    if (!phonemes) {
        return false;
    }
    
    // TODO: 实现完整的音素验证
    return strlen(phonemes) > 0;
}

int misaki_count_phonemes(const char *phonemes) {
    if (!phonemes) {
        return 0;
    }
    
    // 简化：按空格分割计数
    int count = 0;
    const char *p = phonemes;
    bool in_phoneme = false;
    
    while (*p) {
        if (*p == ' ' || *p == '\t') {
            in_phoneme = false;
        } else {
            if (!in_phoneme) {
                count++;
                in_phoneme = true;
            }
        }
        p++;
    }
    
    return count;
}

int misaki_split_phonemes(const char *phonemes,
                          MisakiStringView *result,
                          int max_count) {
    if (!phonemes || !result || max_count <= 0) {
        return 0;
    }
    
    // 按空格分割
    return misaki_sv_split(misaki_sv_from_cstr(phonemes), ' ', result, max_count);
}

/* ============================================================================
 * 调试与统计
 * ========================================================================== */

void misaki_g2p_print(const MisakiTokenList *tokens, bool show_details) {
    if (!tokens) {
        printf("(null)\n");
        return;
    }
    
    printf("G2P Result (%d tokens):\n", tokens->count);
    for (int i = 0; i < tokens->count; i++) {
        const MisakiToken *token = &tokens->tokens[i];
        printf("  [%d] \"%s\"", i, token->text);
        
        if (token->phonemes) {
            printf(" → %s", token->phonemes);
        }
        
        if (show_details) {
            if (token->tag) {
                printf(" (%s)", token->tag);
            }
            printf(" [score=%.2f]", token->score);
        }
        
        printf("\n");
    }
}

void misaki_g2p_stats(const MisakiTokenList *tokens,
                     int *total_phonemes,
                     int *oov_count,
                     double *avg_phonemes_per_token) {
    if (!tokens) {
        return;
    }
    
    int t_phonemes = 0;
    int t_oov = 0;
    
    for (int i = 0; i < tokens->count; i++) {
        if (tokens->tokens[i].phonemes) {
            t_phonemes += misaki_count_phonemes(tokens->tokens[i].phonemes);
        } else {
            t_oov++;
        }
    }
    
    if (total_phonemes) {
        *total_phonemes = t_phonemes;
    }
    
    if (oov_count) {
        *oov_count = t_oov;
    }
    
    if (avg_phonemes_per_token) {
        *avg_phonemes_per_token = tokens->count > 0 ? 
            (double)t_phonemes / tokens->count : 0.0;
    }
}

double misaki_g2p_similarity(const MisakiTokenList *tokens1,
                             const MisakiTokenList *tokens2) {
    // TODO: 实现音素序列相似度计算（如编辑距离）
    (void)tokens1;
    (void)tokens2;
    return 0.0;
}
