/**
 * misaki_g2p_zh.c
 * 
 * 中文 G2P 实现（汉字 → 拼音 → IPA）
 * 
 * License: MIT
 */

#include "misaki_g2p.h"
#include "misaki_dict.h"
#include "misaki_tokenizer.h"
#include "misaki_string.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ============================================================================
 * 拼音 → IPA 映射表（基于标准普通话）
 * 
 * 参考：
 * - https://en.wikipedia.org/wiki/Help:IPA/Mandarin
 * - pypinyin 库的实现
 * ========================================================================== */

typedef struct {
    const char *pinyin;  // 拼音（不带声调）
    const char *ipa;     // IPA 音素（不带声调）
} PinyinIPAMap;

// 声母映射
static const PinyinIPAMap initials_map[] = {
    {"b", "p"},
    {"p", "pʰ"},
    {"m", "m"},
    {"f", "f"},
    {"d", "t"},
    {"t", "tʰ"},
    {"n", "n"},
    {"l", "l"},
    {"g", "k"},
    {"k", "kʰ"},
    {"h", "x"},
    {"j", "tɕ"},
    {"q", "tɕʰ"},
    {"x", "ɕ"},
    {"zh", "ʈ͡ʂ"},
    {"ch", "ʈ͡ʂʰ"},
    {"sh", "ʂ"},
    {"r", "ʐ"},
    {"z", "ts"},
    {"c", "tsʰ"},
    {"s", "s"},
    {NULL, NULL}
};

// 韵母映射
static const PinyinIPAMap finals_map[] = {
    // 单韵母
    {"a", "ɑ"},
    {"o", "o"},
    {"e", "ɤ"},
    {"i", "i"},
    {"u", "u"},
    {"ü", "y"},
    {"v", "y"},  // ü 的另一种写法
    
    // 复韵母
    {"ai", "aɪ"},
    {"ei", "eɪ"},
    {"ui", "ueɪ"},
    {"ao", "ɑʊ"},
    {"ou", "oʊ"},
    {"iu", "iʊ"},
    {"ie", "iɛ"},
    {"üe", "yɛ"},
    {"ve", "yɛ"},
    {"er", "ɚ"},
    
    // 鼻韵母
    {"an", "an"},
    {"en", "ən"},
    {"in", "in"},
    {"un", "un"},
    {"ün", "yn"},
    {"vn", "yn"},
    {"ang", "ɑŋ"},
    {"eng", "əŋ"},
    {"ing", "iŋ"},
    {"ong", "ʊŋ"},
    
    // 特殊韵母
    {"ia", "iɑ"},
    {"iao", "iɑʊ"},
    {"ian", "iɛn"},
    {"iang", "iɑŋ"},
    {"iong", "iʊŋ"},
    {"ua", "uɑ"},
    {"uo", "uo"},
    {"uai", "uaɪ"},
    {"uan", "uan"},
    {"uang", "uɑŋ"},
    
    {NULL, NULL}
};

// 声调符号（上标形式）
static const char *tone_marks[] = {
    "",      // 0: 轻声
    "→",     // 1: 阴平（高平调）
    "↗",     // 2: 阳平（升调）
    "↓",     // 3: 上声（降升调）
    "↘",     // 4: 去声（降调）
    ""       // 5: 轻声（同0）
};

// 带声调符号的元音 → 基本元音 + 声调数字
// 参考：拼音声调标记规则（āáǎà ēéěè īíǐì etc.）
typedef struct {
    const char *tone_char;  // 带声调的字符（UTF-8）
    const char *base_char;  // 基本字符
    int tone;               // 声调（1-4）
} ToneCharMap;

static const ToneCharMap tone_char_map[] = {
    // a 系列
    {"ā", "a", 1}, {"á", "a", 2}, {"ǎ", "a", 3}, {"à", "a", 4},
    // e 系列
    {"ē", "e", 1}, {"é", "e", 2}, {"ě", "e", 3}, {"è", "e", 4},
    // i 系列
    {"ī", "i", 1}, {"í", "i", 2}, {"ǐ", "i", 3}, {"ì", "i", 4},
    // o 系列
    {"ō", "o", 1}, {"ó", "o", 2}, {"ǒ", "o", 3}, {"ò", "o", 4},
    // u 系列
    {"ū", "u", 1}, {"ú", "u", 2}, {"ǔ", "u", 3}, {"ù", "u", 4},
    // ü 系列
    {"ǖ", "v", 1}, {"ǘ", "v", 2}, {"ǚ", "v", 3}, {"ǜ", "v", 4},
    {NULL, NULL, 0}
};

/**
 * 提取拼音中的声调数字
 * 支持两种格式：
 * 1. 数字声调：ni3
 * 2. 符号声调：nǐ
 * 
 * @param pinyin 拼音（如 "ni3" 或 "nǐ"）
 * @param base 输出：不带声调的拼音（如 "ni"）
 * @param tone 输出：声调（1-5，0表示轻声）
 * @return 成功返回 true
 */
static bool extract_tone(const char *pinyin, char *base, int *tone) {
    if (!pinyin || !base || !tone) {
        return false;
    }
    
    int len = strlen(pinyin);
    if (len == 0) {
        return false;
    }
    
    // 检查最后一个字符是否为数字（数字声调格式）
    char last_char = pinyin[len - 1];
    if (last_char >= '0' && last_char <= '5') {
        *tone = last_char - '0';
        strncpy(base, pinyin, len - 1);
        base[len - 1] = '\0';
        return true;
    }
    
    // 检查是否有声调符号（符号声调格式）
    char result[128] = {0};
    int result_pos = 0;
    int detected_tone = 0;
    
    const char *p = pinyin;
    while (*p) {
        // 尝试匹配声调符号
        bool found = false;
        for (int i = 0; tone_char_map[i].tone_char != NULL; i++) {
            int tone_len = strlen(tone_char_map[i].tone_char);
            if (strncmp(p, tone_char_map[i].tone_char, tone_len) == 0) {
                // 找到声调符号，替换为基本字符
                strcpy(result + result_pos, tone_char_map[i].base_char);
                result_pos += strlen(tone_char_map[i].base_char);
                detected_tone = tone_char_map[i].tone;
                p += tone_len;
                found = true;
                break;
            }
        }
        
        if (!found) {
            // 普通字符，直接复制
            result[result_pos++] = *p;
            p++;
        }
    }
    result[result_pos] = '\0';
    
    strcpy(base, result);
    *tone = detected_tone;
    return true;
}

/**
 * 查找声母的 IPA
 */
static const char* find_initial_ipa(const char *initial) {
    for (int i = 0; initials_map[i].pinyin != NULL; i++) {
        if (strcmp(initials_map[i].pinyin, initial) == 0) {
            return initials_map[i].ipa;
        }
    }
    return NULL;
}

/**
 * 查找韵母的 IPA
 */
static const char* find_final_ipa(const char *final) {
    for (int i = 0; finals_map[i].pinyin != NULL; i++) {
        if (strcmp(finals_map[i].pinyin, final) == 0) {
            return finals_map[i].ipa;
        }
    }
    return NULL;
}

/**
 * 分离声母和韵母
 * 
 * @param pinyin 拼音（不带声调数字）
 * @param initial 输出：声母
 * @param final 输出：韵母
 */
static void split_initial_final(const char *pinyin, char *initial, char *final) {
    initial[0] = '\0';
    final[0] = '\0';
    
    if (!pinyin || strlen(pinyin) == 0) {
        return;
    }
    
    // 尝试匹配两字母声母（zh, ch, sh）
    if (strlen(pinyin) >= 2) {
        char two_char[3] = {pinyin[0], pinyin[1], '\0'};
        if (find_initial_ipa(two_char)) {
            strcpy(initial, two_char);
            strcpy(final, pinyin + 2);
            return;
        }
    }
    
    // 尝试匹配单字母声母
    char one_char[2] = {pinyin[0], '\0'};
    if (find_initial_ipa(one_char)) {
        strcpy(initial, one_char);
        strcpy(final, pinyin + 1);
        return;
    }
    
    // 无声母，全部作为韵母
    strcpy(final, pinyin);
}

/* ============================================================================
 * 拼音 → IPA 转换
 * ========================================================================== */

char* misaki_zh_pinyin_to_ipa(const char *pinyin) {
    if (!pinyin) {
        return NULL;
    }
    
    char base[64] = {0};
    int tone = 0;
    
    // 提取声调
    if (!extract_tone(pinyin, base, &tone)) {
        return misaki_strdup(pinyin);  // 失败，返回原拼音
    }
    
    // 分离声母和韵母
    char initial[8] = {0};
    char final[32] = {0};
    split_initial_final(base, initial, final);
    
    // 查找 IPA
    const char *initial_ipa = (initial[0] != '\0') ? find_initial_ipa(initial) : "";
    const char *final_ipa = find_final_ipa(final);
    
    if (!final_ipa) {
        // 未找到韵母映射，返回原拼音
        return misaki_strdup(pinyin);
    }
    
    // 组合：声母 + 韵母 + 声调
    char result[128];
    snprintf(result, sizeof(result), "%s%s%s",
             initial_ipa ? initial_ipa : "",
             final_ipa,
             (tone >= 0 && tone <= 5) ? tone_marks[tone] : "");
    
    return misaki_strdup(result);
}

/* ============================================================================
 * 中文 G2P 主函数
 * ========================================================================== */

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
    
    // 2. 为每个 Token 查询拼音并转换为 IPA
    for (int i = 0; i < tokens->count; i++) {
        MisakiToken *token = &tokens->tokens[i];
        
        // 跳过标点
        if (!token->text || strlen(token->text) == 0) {
            continue;
        }
        
        // 拼接所有字的 IPA
        char ipa_result[512] = {0};
        int ipa_pos = 0;
        
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
                    // 使用第一个拼音（简化处理）
                    char *ipa = misaki_zh_pinyin_to_ipa(pinyins[0]);
                    if (ipa) {
                        int len = strlen(ipa);
                        if (ipa_pos + len + 1 < 512) {
                            if (ipa_pos > 0) {
                                ipa_result[ipa_pos++] = ' ';
                            }
                            strcpy(ipa_result + ipa_pos, ipa);
                            ipa_pos += len;
                        }
                        free(ipa);
                    }
                }
            }
            
            p += bytes;
        }
        
        if (ipa_pos > 0) {
            token->phonemes = misaki_strdup(ipa_result);
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

/* ============================================================================
 * 声调变化和儿化音
 * ========================================================================== */

void misaki_zh_tone_sandhi(MisakiTokenList *tokens,
                           const G2POptions *options) {
    // TODO: 实现声调变化规则
    // - 三声变调：nǐ hǎo → ní hǎo
    // - 一不变调：一个 → yí ge
    (void)tokens;
    (void)options;
}

void misaki_zh_erhua(MisakiTokenList *tokens) {
    // TODO: 实现儿化音处理
    // - 玩儿 → wánr
    (void)tokens;
}
