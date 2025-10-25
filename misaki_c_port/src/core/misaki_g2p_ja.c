/**
 * misaki_g2p_ja.c
 * 
 * 日文 G2P 实现（假名 → IPA）
 * 
 * License: MIT
 */

#include "misaki_g2p.h"
#include "misaki_tokenizer.h"
#include "misaki_string.h"
#include <stdlib.h>
#include <string.h>

/* ============================================================================
 * 假名 → IPA 映射表
 * 
 * 参考：
 * - OpenJTalk 规则文件
 * - https://en.wikipedia.org/wiki/Help:IPA/Japanese
 * ========================================================================== */

typedef struct {
    const char *kana;  // 假名（平假名或片假名）
    const char *ipa;   // IPA 音素
} KanaIPAMap;

// 平假名映射（部分，待完善）
static const KanaIPAMap hiragana_map[] = {
    // 清音 - あ行
    {"あ", "a"},
    {"い", "i"},
    {"う", "ɯ"},
    {"え", "e"},
    {"お", "o"},
    
    // 清音 - か行
    {"か", "ka"},
    {"き", "ki"},
    {"く", "kɯ"},
    {"け", "ke"},
    {"こ", "ko"},
    
    // 清音 - さ行
    {"さ", "sa"},
    {"し", "ɕi"},
    {"す", "sɯ"},
    {"せ", "se"},
    {"そ", "so"},
    
    // 清音 - た行
    {"た", "ta"},
    {"ち", "t͡ɕi"},
    {"つ", "t͡sɯ"},
    {"て", "te"},
    {"と", "to"},
    
    // 清音 - な行
    {"な", "na"},
    {"に", "ɲi"},
    {"ぬ", "nɯ"},
    {"ね", "ne"},
    {"の", "no"},
    
    // 清音 - は行
    {"は", "ha"},
    {"ひ", "çi"},
    {"ふ", "ɸɯ"},
    {"へ", "he"},
    {"ほ", "ho"},
    
    // 清音 - ま行
    {"ま", "ma"},
    {"み", "mi"},
    {"む", "mɯ"},
    {"め", "me"},
    {"も", "mo"},
    
    // 清音 - や行
    {"や", "ja"},
    {"ゆ", "jɯ"},
    {"よ", "jo"},
    
    // 清音 - ら行
    {"ら", "ɾa"},
    {"り", "ɾi"},
    {"る", "ɾɯ"},
    {"れ", "ɾe"},
    {"ろ", "ɾo"},
    
    // 清音 - わ行
    {"わ", "wa"},
    {"を", "o"},  // 现代日语中发音为 "o"
    {"ん", "ɴ"},
    
    // 浊音 - が行
    {"が", "ɡa"},
    {"ぎ", "ɡi"},
    {"ぐ", "ɡɯ"},
    {"げ", "ɡe"},
    {"ご", "ɡo"},
    
    // 浊音 - ざ行
    {"ざ", "za"},
    {"じ", "d͡ʑi"},
    {"ず", "zɯ"},
    {"ぜ", "ze"},
    {"ぞ", "zo"},
    
    // 浊音 - だ行
    {"だ", "da"},
    {"ぢ", "d͡ʑi"},
    {"づ", "zɯ"},
    {"で", "de"},
    {"ど", "do"},
    
    // 浊音 - ば行
    {"ば", "ba"},
    {"び", "bi"},
    {"ぶ", "bɯ"},
    {"べ", "be"},
    {"ぼ", "bo"},
    
    // 半浊音 - ぱ行
    {"ぱ", "pa"},
    {"ぴ", "pi"},
    {"ぷ", "pɯ"},
    {"ぺ", "pe"},
    {"ぽ", "po"},
    
    // 拗音（部分）
    {"きゃ", "kja"},
    {"きゅ", "kjɯ"},
    {"きょ", "kjo"},
    {"しゃ", "ɕa"},
    {"しゅ", "ɕɯ"},
    {"しょ", "ɕo"},
    {"ちゃ", "t͡ɕa"},
    {"ちゅ", "t͡ɕɯ"},
    {"ちょ", "t͡ɕo"},
    
    {NULL, NULL}
};

// TODO: 片假名映射（结构相同，待添加）

/**
 * 查找假名的 IPA
 */
static const char* find_kana_ipa(const char *kana) {
    // 尝试两字符匹配（拗音）
    if (strlen(kana) >= 6) {  // UTF-8 两个假名字符
        char two_char[10] = {0};
        // 简化：假设每个假名字符是 3 字节
        memcpy(two_char, kana, 6);
        
        for (int i = 0; hiragana_map[i].kana != NULL; i++) {
            if (strcmp(hiragana_map[i].kana, two_char) == 0) {
                return hiragana_map[i].ipa;
            }
        }
    }
    
    // 单字符匹配
    char one_char[10] = {0};
    // 简化：假设假名字符是 3 字节
    memcpy(one_char, kana, 3);
    
    for (int i = 0; hiragana_map[i].kana != NULL; i++) {
        if (strcmp(hiragana_map[i].kana, one_char) == 0) {
            return hiragana_map[i].ipa;
        }
    }
    
    return NULL;
}

/* ============================================================================
 * 日文 G2P 主函数
 * ========================================================================== */

char* misaki_ja_kana_to_ipa(const char *kana) {
    if (!kana) {
        return NULL;
    }
    
    char result[512] = {0};
    int result_pos = 0;
    
    const char *p = kana;
    while (*p) {
        const char *ipa = find_kana_ipa(p);
        if (ipa) {
            int ipa_len = strlen(ipa);
            if (result_pos + ipa_len < 512) {
                strcpy(result + result_pos, ipa);
                result_pos += ipa_len;
            }
            
            // 跳过已匹配的字符（简化：3 字节）
            p += 3;
        } else {
            // 未找到映射，保留原字符（3 字节）
            if (result_pos + 3 < 512) {
                memcpy(result + result_pos, p, 3);
                result_pos += 3;
            }
            p += 3;
        }
    }
    
    return misaki_strdup(result);
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
    // 例如：コーヒー → koːhiː
    (void)tokens;
}
