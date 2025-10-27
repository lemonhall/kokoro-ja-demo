/**
 * @file misaki_g2p_qya.c
 * @brief Quenya (昆雅语) G2P Implementation
 * 
 * 实现托尔金创造的昆雅语的G2P转换
 * 基于 https://realelvish.net/pronunciation/quenya/
 */

#include "misaki_g2p_qya.h"
#include "misaki_string.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ============ 元音映射表 ============ */

typedef struct {
    const char* grapheme;  // 书写形式
    const char* phoneme;   // IPA音素
} VowelMapping;

// 长元音（带长音符）
static const VowelMapping LONG_VOWELS[] = {
    {"á", "aː"},
    {"é", "eː"},
    {"í", "iː"},
    {"ó", "oː"},
    {"ú", "uː"},
    {NULL, NULL}
};

// 短元音
static const VowelMapping SHORT_VOWELS[] = {
    {"a", "a"},
    {"e", "ɛ"},
    {"ë", "ɛ"},  // 分音符不影响发音
    {"i", "i"},
    {"o", "ɔ"},
    {"u", "u"},
    {NULL, NULL}
};

/* ============ 双元音映射表 ============ */

static const VowelMapping DIPHTHONGS[] = {
    {"ai", "aj"},
    {"au", "au"},
    {"iu", "iu"},
    {"eu", "ɛu"},
    {"oi", "ɔj"},
    {"ui", "uj"},
    {NULL, NULL}
};

/* ============ 辅音映射表 ============ */

typedef struct {
    const char* grapheme;
    const char* phoneme;
} ConsonantMapping;

// 辅音簇和特殊辅音（按长度降序排列，优先匹配长的）
static const ConsonantMapping CONSONANT_CLUSTERS[] = {
    // 清化辅音
    {"hl", "l̥"},
    {"hr", "r̥"},
    {"hw", "ʍ"},
    {"hy", "j̊"},
    
    // 辅音簇
    {"ht", "xt"},
    {"pt", "φt"},
    
    // 腭音化
    {"ty", "tj"},
    {"ny", "nj"},
    {"ly", "lj"},
    {"ry", "rj"},
    {"sy", "sj"},
    
    // 特殊组合
    {"qu", "kw"},
    {"ng", "ŋɡ"},
    {"th", "θ"},
    
    {NULL, NULL}
};

// 单辅音
static const ConsonantMapping SINGLE_CONSONANTS[] = {
    {"ñ", "ŋ"},
    {"þ", "θ"},
    {"r", "r"},
    {"z", "z"},
    {"c", "k"},
    {"k", "k"},
    {"s", "s"},
    {"b", "b"},
    {"d", "d"},
    {"f", "f"},
    {"g", "ɡ"},
    {"h", "h"},
    {"j", "j"},
    {"l", "l"},
    {"m", "m"},
    {"n", "n"},
    {"p", "p"},
    {"t", "t"},
    {"v", "v"},
    {"w", "w"},
    {"y", "j"},
    {NULL, NULL}
};

/* ============ 辅助函数 ============ */

/**
 * @brief 检查字符串是否以指定前缀开始（不区分大小写）
 */
static int starts_with_ci(const char* str, const char* prefix) {
    while (*prefix) {
        if (tolower((unsigned char)*str) != tolower((unsigned char)*prefix)) {
            return 0;
        }
        str++;
        prefix++;
    }
    return 1;
}

/**
 * @brief 检测是否为元音字符（包括带音标的）
 */
static int is_vowel_char(const char* str) {
    // 检查长元音
    for (int i = 0; LONG_VOWELS[i].grapheme; i++) {
        if (starts_with_ci(str, LONG_VOWELS[i].grapheme)) {
            return 1;
        }
    }
    // 检查短元音
    for (int i = 0; SHORT_VOWELS[i].grapheme; i++) {
        if (starts_with_ci(str, SHORT_VOWELS[i].grapheme)) {
            return 1;
        }
    }
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

int misaki_g2p_qya_init(void) {
    // 当前不需要初始化资源
    return 0;
}

void misaki_g2p_qya_cleanup(void) {
    // 当前不需要清理资源
}

int misaki_qya_is_diphthong(const char* str, char* diphthong) {
    for (int i = 0; DIPHTHONGS[i].grapheme; i++) {
        int len = strlen(DIPHTHONGS[i].grapheme);
        if (starts_with_ci(str, DIPHTHONGS[i].grapheme)) {
            if (diphthong) {
                strcpy(diphthong, DIPHTHONGS[i].phoneme);
            }
            return len;
        }
    }
    return 0;
}

int misaki_qya_is_long_vowel(const char* str, char* vowel) {
    for (int i = 0; LONG_VOWELS[i].grapheme; i++) {
        int len = strlen(LONG_VOWELS[i].grapheme);
        if (starts_with_ci(str, LONG_VOWELS[i].grapheme)) {
            if (vowel) {
                strcpy(vowel, LONG_VOWELS[i].phoneme);
            }
            return len;
        }
    }
    return 0;
}

int misaki_qya_is_consonant_cluster(const char* str, char* cluster) {
    for (int i = 0; CONSONANT_CLUSTERS[i].grapheme; i++) {
        int len = strlen(CONSONANT_CLUSTERS[i].grapheme);
        if (starts_with_ci(str, CONSONANT_CLUSTERS[i].grapheme)) {
            if (cluster) {
                strcpy(cluster, CONSONANT_CLUSTERS[i].phoneme);
            }
            return len;
        }
    }
    return 0;
}

int misaki_qya_count_syllables(const char* word) {
    if (!word) return 0;
    
    int syllable_count = 0;
    const char* p = word;
    
    while (*p) {
        // 跳过非字母字符
        if (!isalpha((unsigned char)*p) && (*p & 0x80) == 0) {
            p++;
            continue;
        }
        
        // 检查双元音
        char diphthong[16];
        int len = misaki_qya_is_diphthong(p, diphthong);
        if (len > 0) {
            syllable_count++;
            p += len;
            continue;
        }
        
        // 检查长元音
        len = misaki_qya_is_long_vowel(p, NULL);
        if (len > 0) {
            syllable_count++;
            p += len;
            continue;
        }
        
        // 检查短元音
        int is_vowel = 0;
        for (int i = 0; SHORT_VOWELS[i].grapheme; i++) {
            if (starts_with_ci(p, SHORT_VOWELS[i].grapheme)) {
                syllable_count++;
                p += strlen(SHORT_VOWELS[i].grapheme);
                is_vowel = 1;
                break;
            }
        }
        
        if (!is_vowel) {
            // 跳过辅音（包括 y）
            int char_len = utf8_char_len(p);
            p += char_len;
        }
    }
    
    return syllable_count;
}

int misaki_qya_calculate_stress(const char* word) {
    int syllable_count = misaki_qya_count_syllables(word);
    
    if (syllable_count <= 0) return -1;
    if (syllable_count <= 3) return 0;  // 1-3音节，重音在第一音节
    
    // 4+音节，默认倒数第三音节（索引为 count-3）
    int stress_pos = syllable_count - 3;
    
    // 检查倒数第二音节是否有特殊特征
    // TODO: 实现更精确的重音规则检测
    // - 长音符
    // - 双元音
    // - 辅音簇结尾
    
    return stress_pos;
}

int misaki_g2p_qya_convert(const char* word, char** phonemes) {
    if (!word || !phonemes) return -1;
    
    // 分配足够的缓冲区（每个字符最多4字节IPA）
    size_t max_len = strlen(word) * 4 + 64;
    char* result = (char*)malloc(max_len);
    if (!result) return -1;
    
    result[0] = '\0';
    const char* p = word;
    int syllable_index = 0;
    int stress_pos = misaki_qya_calculate_stress(word);
    
    while (*p) {
        // 跳过空格和标点
        if (isspace((unsigned char)*p) || ispunct((unsigned char)*p)) {
            p++;
            continue;
        }
        
        int matched = 0;
        
        // 1. 检查双元音
        char phoneme[16];
        int len = misaki_qya_is_diphthong(p, phoneme);
        if (len > 0) {
            // 添加重音标记
            if (syllable_index == stress_pos && strlen(result) > 0) {
                strcat(result, "ˈ");
            }
            strcat(result, phoneme);
            strcat(result, " ");
            p += len;
            syllable_index++;
            matched = 1;
            continue;
        }
        
        // 2. 检查长元音
        len = misaki_qya_is_long_vowel(p, phoneme);
        if (len > 0) {
            if (syllable_index == stress_pos && strlen(result) > 0) {
                strcat(result, "ˈ");
            }
            strcat(result, phoneme);
            strcat(result, " ");
            p += len;
            syllable_index++;
            matched = 1;
            continue;
        }
        
        // 3. 检查短元音
        for (int i = 0; SHORT_VOWELS[i].grapheme && !matched; i++) {
            if (starts_with_ci(p, SHORT_VOWELS[i].grapheme)) {
                if (syllable_index == stress_pos && strlen(result) > 0) {
                    strcat(result, "ˈ");
                }
                strcat(result, SHORT_VOWELS[i].phoneme);
                strcat(result, " ");
                p += strlen(SHORT_VOWELS[i].grapheme);
                syllable_index++;
                matched = 1;
                break;
            }
        }
        if (matched) continue;
        
        // 4. 检查辅音簇
        len = misaki_qya_is_consonant_cluster(p, phoneme);
        if (len > 0) {
            strcat(result, phoneme);
            strcat(result, " ");
            p += len;
            matched = 1;
            continue;
        }
        
        // 5. 检查单辅音
        for (int i = 0; SINGLE_CONSONANTS[i].grapheme && !matched; i++) {
            if (starts_with_ci(p, SINGLE_CONSONANTS[i].grapheme)) {
                strcat(result, SINGLE_CONSONANTS[i].phoneme);
                strcat(result, " ");
                p += strlen(SINGLE_CONSONANTS[i].grapheme);
                matched = 1;
                break;
            }
        }
        
        if (!matched) {
            // 未识别字符，跳过
            int char_len = utf8_char_len(p);
            p += char_len;
        }
    }
    
    // 移除尾部空格
    size_t result_len = strlen(result);
    if (result_len > 0 && result[result_len - 1] == ' ') {
        result[result_len - 1] = '\0';
    }
    
    *phonemes = result;
    return 0;
}

int misaki_g2p_qya_text(const char* text, char** phonemes) {
    if (!text || !phonemes) return -1;
    
    // 简单实现：将整个文本作为一个单词处理
    // TODO: 改进为逐词处理
    return misaki_g2p_qya_convert(text, phonemes);
}
