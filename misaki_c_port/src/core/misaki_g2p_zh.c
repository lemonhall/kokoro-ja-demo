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
#include "misaki_num2cn.h"  // ⭐ 新增：数字转中文
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ============================================================================
 * 拼音 → IPA 映射表（基于标准普通话）
 * 
 * 参考：
 * - https://en.wikipedia.org/wiki/Help:IPA/Mandarin
 * - Standard Chinese phonology (维基百科)
 * - pypinyin 库的实现
 * 
 * 注意：
 * 1. 汉语拼音 h → IPA /x/ （清软腭擦音，如 "hē" 喝）
 * 2. 汉语拼音 x → IPA /ɕ/ （清龈腭擦音，如 "xī" 西）
 * 3. 这两个是完全不同的音位，不要混淆！
 * ========================================================================== */

typedef struct {
    const char *pinyin;  // 拼音（不带声调）
    const char *ipa;     // IPA 音素（不带声调）
} PinyinIPAMap;

// 声母映射（21 个辅音声母 + 1 个零声母）
static const PinyinIPAMap initials_map[] = {
    {"", ""},       // 零声母（重要！用于 a, o, e, ai, ei, ao, ou 等）
    
    // 双唇音
    {"b", "p"},      // 清不送气双唇塞音
    {"p", "pʰ"},     // 清送气双唇塞音
    {"m", "m"},      // 双唇鼻音
    {"f", "f"},      // 清唇齿擦音
    
    // 舌尖中音
    {"d", "t"},      // 清不送气舌尖塞音
    {"t", "tʰ"},     // 清送气舌尖塞音
    {"n", "n"},      // 舌尖鼻音
    {"l", "l"},      // 舌尖边音
    
    // 舌根音
    {"g", "k"},      // 清不送气舌根塞音
    {"k", "kʰ"},     // 清送气舌根塞音
    {"h", "x"},      // 清软腭擦音（注意：不是 ɕ！）
    
    // 舌面音
    {"j", "tɕ"},     // 清不送气舌面塞擦音
    {"q", "tɕʰ"},    // 清送气舌面塞擦音
    {"x", "ɕ"},      // 清舌面擦音（注意：与 h 不同！）
    
    // 舌尖后音（卷舌音）
    {"zh", "ʈʂ"},    // 清不送气卷舌塞擦音
    {"ch", "ʈʂʰ"},   // 清送气卷舌塞擦音
    {"sh", "ʂ"},     // 清卷舌擦音
    {"r", "ʐ"},      // 浊卷舌擦音
    
    // 舌尖前音
    {"z", "ts"},     // 清不送气舌尖塞擦音
    {"c", "tsʰ"},    // 清送气舌尖塞擦音
    {"s", "s"},      // 清舌尖擦音
    
    {NULL, NULL}
};

// 韵母映射（39 个基本韵母）
static const PinyinIPAMap finals_map[] = {
    // ===== 单韵母 (7个) =====
    {"a", "ɑ"},       // 开后不圆唇元音
    {"o", "o"},       // 半闭后圆唇元音
    {"e", "ɤ"},       // 半闭后不圆唇元音
    {"i", "i"},       // 闭前不圆唇元音（注意：zhi/chi/shi/ri 和 zi/ci/si 中的 i 需特殊处理）
    {"u", "u"},       // 闭后圆唇元音
    {"ü", "y"},       // 闭前圆唇元音
    {"v", "y"},       // ü 的另一种写法
    
    // ===== 复韵母 (13个) =====
    {"ai", "aɪ"},     // 开前复合元音
    {"ei", "eɪ"},     // 半闭前复合元音
    {"ui", "ueɪ"},    // uei 的简写（hui, dui, tui 等）
    {"ao", "ɑʊ"},     // 开后复合元音
    {"ou", "ɤʊ"},     // 半闭后复合元音（注意：不是 oʊ）
    {"iu", "iɤʊ"},    // iou 的简写（liu, niu 等）
    {"ie", "iɛ"},     // 前展复合元音
    {"üe", "yɛ"},     // 前圆复合元音
    {"ve", "yɛ"},     // üe 的另一种写法
    {"ue", "yɛ"},     // ⭐ xue, nue 等（重要！）
    {"er", "ɚ"},      // 卷舌元音（特殊韵母）
    
    // ===== 前鼻韵母 (9个) =====
    {"an", "an"},     // 开后鼻尾韵
    {"en", "ən"},     // 半开央鼻尾韵
    {"in", "in"},     // 闭前鼻尾韵
    {"un", "uən"},    // uen 的简写（gun, kun, hun 等）
    {"ün", "yn"},     // 闭前圆唇鼻尾韵
    {"vn", "yn"},     // ün 的另一种写法
    {"ian", "iɛn"},   // 前展鼻尾韵（注意：用 ɛ 而非 a）
    {"uan", "uan"},   // 后圆鼻尾韵
    {"üan", "yɛn"},   // 前圆鼻尾韵（与 ian 保持一致性）
    
    // ===== 后鼻韵母 (9个) =====
    {"ang", "ɑŋ"},    // 开后鼻尾韵
    {"eng", "ɤŋ"},    // 半闭后鼻尾韵（注意：不是 əŋ）
    {"ing", "iŋ"},    // 闭前鼻尾韵
    {"ong", "ʊŋ"},    // 半闭后圆唇鼻尾韵
    {"iang", "iɑŋ"},  // 前展后鼻韵
    {"iong", "iʊŋ"},  // 前展后圆鼻韵
    {"uang", "uɑŋ"},  // 后圆后鼻韵
    {"ueng", "uɤŋ"},  // ⭐ weng 的韵母形式（重要！）
    
    // ===== 其他复合韵母 (5个) =====
    {"ia", "iɑ"},     // 前展复合韵
    {"iao", "iɑʊ"},   // 前展后圆复合韵
    {"ua", "uɑ"},     // 后圆复合韵
    {"uo", "uo"},     // 后圆复合韵
    {"uai", "uaɪ"},   // 后圆前展复合韵
    
    // ===== 特殊空韵（卷舌/平舌音后的 i）=====
    {"_zcs_i", "ɨ"},  // ⭐ zhi/chi/shi/ri 和 zi/ci/si 中的空韵
    
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
 * 处理汉语拼音的声韵母拆分，需要特别注意：
 * 1. y-/w- 开头的拼音是零声母音节（如 yu, wu, yi, wa）
 * 2. 这些音节在拼音中写作 y-/w-，但实际是韵母的拼写变体
 * 3. 例如：yu = 零声母 + ü，而不是声母 y + 韵母 u
 * 4. ⭐ 卷舌音 zh/ch/sh/r 和平舌音 z/c/s 后的 "i" 是空韵（只发声母）
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
    
    // ⭐ 关键修复：处理 y- 开头的拼音（零声母音节）
    if (pinyin[0] == 'y') {
        // yu 系列：yu, yue, yuan, yun → ü, üe, üan, ün
        if (strcmp(pinyin, "yu") == 0) {
            strcpy(final, "ü");
            return;
        }
        if (strcmp(pinyin, "yue") == 0) {
            strcpy(final, "üe");
            return;
        }
        if (strcmp(pinyin, "yuan") == 0) {
            strcpy(final, "üan");
            return;
        }
        if (strcmp(pinyin, "yun") == 0) {
            strcpy(final, "ün");
            return;
        }
        // yi 系列：yi, ya, yao, yan, yang, ye, yong, you, yin, ying
        if (strcmp(pinyin, "yi") == 0) {
            strcpy(final, "i");
            return;
        }
        if (strcmp(pinyin, "ya") == 0) {
            strcpy(final, "ia");
            return;
        }
        if (strcmp(pinyin, "yao") == 0) {
            strcpy(final, "iao");
            return;
        }
        if (strcmp(pinyin, "yan") == 0) {
            strcpy(final, "ian");
            return;
        }
        if (strcmp(pinyin, "yang") == 0) {
            strcpy(final, "iang");
            return;
        }
        if (strcmp(pinyin, "ye") == 0) {
            strcpy(final, "ie");
            return;
        }
        if (strcmp(pinyin, "yong") == 0) {
            strcpy(final, "iong");
            return;
        }
        if (strcmp(pinyin, "you") == 0) {
            strcpy(final, "iu");
            return;
        }
        if (strcmp(pinyin, "yin") == 0) {
            strcpy(final, "in");
            return;
        }
        if (strcmp(pinyin, "ying") == 0) {
            strcpy(final, "ing");
            return;
        }
    }
    
    // ⭐ 处理 w- 开头的拼音（零声母 + u 系列韵母）
    if (pinyin[0] == 'w') {
        // wu → u
        if (strcmp(pinyin, "wu") == 0) {
            strcpy(final, "u");
            return;
        }
        // wa → ua
        if (strcmp(pinyin, "wa") == 0) {
            strcpy(final, "ua");
            return;
        }
        // wai → uai
        if (strcmp(pinyin, "wai") == 0) {
            strcpy(final, "uai");
            return;
        }
        // wan → uan
        if (strcmp(pinyin, "wan") == 0) {
            strcpy(final, "uan");
            return;
        }
        // wang → uang
        if (strcmp(pinyin, "wang") == 0) {
            strcpy(final, "uang");
            return;
        }
        // wo → uo
        if (strcmp(pinyin, "wo") == 0) {
            strcpy(final, "uo");
            return;
        }
        // wei → ui
        if (strcmp(pinyin, "wei") == 0) {
            strcpy(final, "ui");
            return;
        }
        // wen → un
        if (strcmp(pinyin, "wen") == 0) {
            strcpy(final, "un");
            return;
        }
        // weng → ueng
        if (strcmp(pinyin, "weng") == 0) {
            strcpy(final, "ueng");
            return;
        }
    }
    
    // 尝试匹配两字母声母（zh, ch, sh）
    if (strlen(pinyin) >= 2) {
        char two_char[3] = {pinyin[0], pinyin[1], '\0'};
        if (find_initial_ipa(two_char)) {
            strcpy(initial, two_char);
            strcpy(final, pinyin + 2);
            
            // ⭐ 特殊处理：zhi/chi/shi/ri 和 zi/ci/si 中的 "i" 是空韵（卷舌/平舌元音）
            // 这些音节只发声母，韵母 "i" 需要替换为特殊符号
            if (strcmp(final, "i") == 0) {
                if (strcmp(two_char, "zh") == 0 || strcmp(two_char, "ch") == 0 || strcmp(two_char, "sh") == 0) {
                    // zhi/chi/shi 的 "i" 是卷舌元音 [ɨ]
                    strcpy(final, "_zcs_i");  // 标记特殊韵母
                }
            }
            return;
        }
    }
    
    // 尝试匹配单字母声母
    char one_char[2] = {pinyin[0], '\0'};
    if (find_initial_ipa(one_char)) {
        strcpy(initial, one_char);
        strcpy(final, pinyin + 1);
        
        // ⭐ 特殊处理：ri 中的 "i" 是空韵
        if (strcmp(final, "i") == 0 && strcmp(one_char, "r") == 0) {
            strcpy(final, "_zcs_i");
        }
        // ⭐ 特殊处理：zi/ci/si 中的 "i" 是平舌元音 [ɿ]
        if (strcmp(final, "i") == 0) {
            if (strcmp(one_char, "z") == 0 || strcmp(one_char, "c") == 0 || strcmp(one_char, "s") == 0) {
                strcpy(final, "_zcs_i");
            }
        }
        return;
    }
    
    // 无声母，全部作为韵母（零声母音节）
    strcpy(final, pinyin);
    
    // ⭐ 边界情况：验证韵母是否存在于映射表中
    if (!find_final_ipa(final)) {
        // 警告：未识别的拼音（可能是拼写错误或方言）
        // 注意：这不会终止程序，只是记录警告
        // 实际应用中可能需要日志系统
        fprintf(stderr, "[G2P Warning] Unrecognized pinyin: %s (final: %s)\n", pinyin, final);
    }
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
        fprintf(stderr, "[G2P Error] No IPA mapping found for final: %s (pinyin: %s)\n", final, pinyin);
        return misaki_strdup(pinyin);
    }
    
    // ⭐ 内存安全：先计算需要的缓冲区大小
    const char *tone_mark = (tone >= 0 && tone <= 5) ? tone_marks[tone] : "";
    int needed = snprintf(NULL, 0, "%s%s%s",
                         initial_ipa ? initial_ipa : "",
                         final_ipa,
                         tone_mark);
    
    // 分配足够的内存
    char *result = (char*)malloc(needed + 1);
    if (!result) {
        return NULL;
    }
    
    // 组合：声母 + 韵母 + 声调
    snprintf(result, needed + 1, "%s%s%s",
             initial_ipa ? initial_ipa : "",
             final_ipa,
             tone_mark);
    
    return result;
}

/* ============================================================================
 * 中文 G2P 主函数
 * ========================================================================== */

/**
 * 将词组拼音（空格分隔）转换为 IPA
 * 
 * @param phrase_pinyin 拼音字符串（如 "cháng chéng"）
 * @return IPA 字符串（需要 free）
 */
static char* convert_phrase_pinyin_to_ipa(const char *phrase_pinyin) {
    if (!phrase_pinyin) {
        return NULL;
    }
    
    char result[512] = {0};
    int result_pos = 0;
    
    // 复制一份用于 strtok
    char *copy = misaki_strdup(phrase_pinyin);
    if (!copy) {
        return NULL;
    }
    
    // 按空格分割拼音
    char *token = strtok(copy, " ");
    while (token) {
        // 转换单个拼音为 IPA
        char *ipa = misaki_zh_pinyin_to_ipa(token);
        if (ipa) {
            int len = strlen(ipa);
            if (result_pos + len + 1 < 512) {
                if (result_pos > 0) {
                    result[result_pos++] = ' ';
                }
                strcpy(result + result_pos, ipa);
                result_pos += len;
            }
            free(ipa);
        }
        
        token = strtok(NULL, " ");
    }
    
    free(copy);
    return result_pos > 0 ? misaki_strdup(result) : NULL;
}

MisakiTokenList* misaki_zh_g2p(const ZhDict *dict,
                               const ZhPhraseDict *phrase_dict,
                               void *tokenizer,
                               const char *text,
                               const G2POptions *options) {
    if (!dict || !tokenizer || !text) {
        return NULL;
    }
    
    // ⭐ 0. 数字预处理：将所有数字转换为中文读法
    char processed_text[4096];
    if (!misaki_convert_numbers_in_text(text, processed_text, sizeof(processed_text))) {
        // 转换失败，使用原文本
        strncpy(processed_text, text, sizeof(processed_text) - 1);
        processed_text[sizeof(processed_text) - 1] = '\0';
    }
    
    // 1. 中文分词（使用处理后的文本）
    MisakiTokenList *tokens = misaki_zh_tokenize(tokenizer, processed_text);
    if (!tokens) {
        return NULL;
    }
    
    // 2. 为每个 Token 查询拼音并转换为 IPA
    // ⭐ 优化：优先使用词组拼音（解决多音字问题）
    for (int i = 0; i < tokens->count; i++) {
        MisakiToken *token = &tokens->tokens[i];
        
        // 跳过标点
        if (!token->text || strlen(token->text) == 0) {
            continue;
        }
        
        // ⭐ 优先查询词组拼音词典
        const char *phrase_pinyin = NULL;
        if (phrase_dict && 
            misaki_zh_phrase_dict_lookup(phrase_dict, token->text, &phrase_pinyin)) {
            // 找到词组拼音，直接转换为 IPA
            char *ipa = convert_phrase_pinyin_to_ipa(phrase_pinyin);
            if (ipa) {
                token->phonemes = ipa;
                continue;  // 处理下一个 token
            }
        }
        
        // 降级：逐字查询单字拼音
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

/**
 * 从 IPA 音素中提取声调
 * 
 * @param ipa IPA 音素字符串
 * @return 1-4 (四个声调), 0 (轻声或无声调)
 */
static int extract_tone_from_ipa(const char *ipa) {
    if (!ipa) {
        return 0;
    }
    
    // IPA 声调标记：
    // → = 一声（高平）
    // ↗ = 二声（上升）
    // ↓ = 三声（降升）
    // ↘ = 四声（下降）
    
    if (strstr(ipa, "→")) return 1;
    if (strstr(ipa, "↗")) return 2;
    if (strstr(ipa, "↓")) return 3;
    if (strstr(ipa, "↘")) return 4;
    
    return 0;  // 轻声
}

/**
 * 修改 IPA 音素的声调
 * 
 * @param ipa 原 IPA 字符串
 * @param new_tone 新声调 (1-4)
 * @return 新的 IPA 字符串（需要 free）
 */
static char* change_ipa_tone(const char *ipa, int new_tone) {
    if (!ipa || new_tone < 1 || new_tone > 4) {
        return NULL;
    }
    
    // 声调标记
    const char *tone_marks[] = {"", "→", "↗", "↓", "↘"};
    const char *old_marks[] = {"→", "↗", "↓", "↘"};
    
    // 复制原字符串
    char result[256];
    strncpy(result, ipa, sizeof(result) - 1);
    result[sizeof(result) - 1] = '\0';
    
    // 移除所有旧声调标记
    for (int i = 0; i < 4; i++) {
        char *pos = strstr(result, old_marks[i]);
        if (pos) {
            // 移除该标记（UTF-8，3字节）
            memmove(pos, pos + 3, strlen(pos + 3) + 1);
            break;
        }
    }
    
    // 添加新声调标记（在字符串末尾）
    strncat(result, tone_marks[new_tone], sizeof(result) - strlen(result) - 1);
    
    return misaki_strdup(result);
}

void misaki_zh_tone_sandhi(MisakiTokenList *tokens,
                           const G2POptions *options) {
    if (!tokens || tokens->count == 0) {
        return;
    }
    
    // 遍历所有 token（除最后一个）
    for (int i = 0; i < tokens->count - 1; i++) {
        MisakiToken *current = &tokens->tokens[i];
        MisakiToken *next = &tokens->tokens[i + 1];
        
        if (!current->text || !current->phonemes || !next->phonemes) {
            continue;
        }
        
        // 提取当前和下一个音节的声调
        // 注：如果 phonemes 包含多个音节（用空格分隔），只处理最后一个
        char *current_last = strrchr(current->phonemes, ' ');
        char *next_first = strchr(next->phonemes, ' ');
        
        const char *current_ipa = current_last ? current_last + 1 : current->phonemes;
        char next_ipa_buf[128];
        if (next_first) {
            size_t len = next_first - next->phonemes;
            strncpy(next_ipa_buf, next->phonemes, len);
            next_ipa_buf[len] = '\0';
        } else {
            strncpy(next_ipa_buf, next->phonemes, sizeof(next_ipa_buf) - 1);
        }
        const char *next_ipa = next_ipa_buf;
        
        int current_tone = extract_tone_from_ipa(current_ipa);
        int next_tone = extract_tone_from_ipa(next_ipa);
        
        bool changed = false;
        int new_tone = -1;
        
        // 规则1: 三声变调（三声 + 三声 → 二声 + 三声）
        if (current_tone == 3 && next_tone == 3) {
            new_tone = 2;
            changed = true;
        }
        
        // 规则2: "一"的变调
        if (strcmp(current->text, "一") == 0) {
            if (next_tone == 4 || next_tone == 0) {
                new_tone = 2;  // 一 + 四声/轻声 → 二声
                changed = true;
            } else if (next_tone >= 1 && next_tone <= 3) {
                new_tone = 4;  // 一 + 一二三声 → 四声
                changed = true;
            }
        }
        
        // 规则3: "不"的变调
        if (strcmp(current->text, "不") == 0 && next_tone == 4) {
            new_tone = 2;  // 不 + 四声 → 二声
            changed = true;
        }
        
        // 应用变调
        if (changed && new_tone > 0) {
            // 如果 phonemes 只有一个音节，直接替换
            if (!current_last) {
                char *new_phonemes = change_ipa_tone(current->phonemes, new_tone);
                if (new_phonemes) {
                    free(current->phonemes);
                    current->phonemes = new_phonemes;
                }
            } else {
                // 多个音节，只替换最后一个
                size_t prefix_len = current_last - current->phonemes + 1;
                char *new_last = change_ipa_tone(current_ipa, new_tone);
                if (new_last) {
                    char new_phonemes[512];
                    strncpy(new_phonemes, current->phonemes, prefix_len);
                    new_phonemes[prefix_len] = '\0';
                    strncat(new_phonemes, new_last, sizeof(new_phonemes) - prefix_len - 1);
                    
                    free(current->phonemes);
                    current->phonemes = misaki_strdup(new_phonemes);
                    free(new_last);
                }
            }
        }
    }
}

void misaki_zh_erhua(MisakiTokenList *tokens) {
    // TODO: 实现儿化音处理
    // - 玩儿 → wánr
    (void)tokens;
}
