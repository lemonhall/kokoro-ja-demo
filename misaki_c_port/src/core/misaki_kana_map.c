/**
 * misaki_kana_map.c
 * 
 * 假名→IPA 音素映射表（从 Python 的 HEPBURN 和 M2P 移植）
 * 
 * License: MIT
 */

#include "misaki_string.h"
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

/* ============================================================================
 * HEPBURN 映射表：平假名 → IPA 音素
 * ========================================================================== */

typedef struct {
    const char *kana;       // 假名（1-2个字符）
    const char *ipa;        // IPA 音素
} KanaMapping;

// 单字符平假名映射（84个基础字符）
static const KanaMapping HEPBURN_SINGLE[] = {
    // あ行
    {"ぁ", "a"}, {"あ", "a"}, {"ぃ", "i"}, {"い", "i"},
    {"ぅ", "ɯ"}, {"う", "ɯ"}, {"ぇ", "e"}, {"え", "e"},
    {"ぉ", "o"}, {"お", "o"},
    
    // か行
    {"か", "ka"}, {"が", "ɡa"}, {"き", "kʲi"}, {"ぎ", "ɡʲi"},
    {"く", "kɯ"}, {"ぐ", "ɡɯ"}, {"け", "ke"}, {"げ", "ɡe"},
    {"こ", "ko"}, {"ご", "ɡo"},
    
    // さ行
    {"さ", "sa"}, {"ざ", "ʣa"}, {"し", "ɕi"}, {"じ", "ʥi"},
    {"す", "sɨ"}, {"ず", "zɨ"}, {"せ", "se"}, {"ぜ", "ʣe"},
    {"そ", "so"}, {"ぞ", "ʣo"},
    
    // た行
    {"た", "ta"}, {"だ", "da"}, {"ち", "ʨi"}, {"ぢ", "ʥi"},
    // っ 在特殊处理中
    {"つ", "ʦɨ"}, {"づ", "zɨ"}, {"て", "te"}, {"で", "de"},
    {"と", "to"}, {"ど", "do"},
    
    // な行
    {"な", "na"}, {"に", "ɲi"}, {"ぬ", "nɯ"}, {"ね", "ne"},
    {"の", "no"},
    
    // は行
    {"は", "ha"}, {"ば", "ba"}, {"ぱ", "pa"}, {"ひ", "çi"},
    {"び", "bʲi"}, {"ぴ", "pʲi"}, {"ふ", "ɸɯ"}, {"ぶ", "bɯ"},
    {"ぷ", "pɯ"}, {"へ", "he"}, {"べ", "be"}, {"ぺ", "pe"},
    {"ほ", "ho"}, {"ぼ", "bo"}, {"ぽ", "po"},
    
    // ま行
    {"ま", "ma"}, {"み", "mʲi"}, {"む", "mɯ"}, {"め", "me"},
    {"も", "mo"},
    
    // や行
    {"ゃ", "ja"}, {"や", "ja"}, {"ゅ", "jɯ"}, {"ゆ", "jɯ"},
    {"ょ", "jo"}, {"よ", "jo"},
    
    // ら行
    {"ら", "ɾa"}, {"り", "ɾʲi"}, {"る", "ɾɯ"}, {"れ", "ɾe"},
    {"ろ", "ɾo"},
    
    // わ行
    // ⭐ 修正：「wa」使用更标准的 IPA（而不是 βa）
    {"ゎ", "wa"}, {"わ", "wa"}, {"ゐ", "i"}, {"ゑ", "e"},
    {"を", "o"},
    
    // その他
    // ん 在特殊处理中
    {"ゔ", "vɯ"}, {"ゕ", "ka"}, {"ゖ", "ke"},
    
    // 片假名扩展
    {"ヷ", "va"}, {"ヸ", "vʲi"}, {"ヹ", "ve"}, {"ヺ", "vo"},
};

// 双字符组合（80个）
static const KanaMapping HEPBURN_DIGRAPH[] = {
    // い系
    {"いぇ", "je"},
    
    // う系
    {"うぃ", "wi"}, {"うぇ", "we"}, {"うぉ", "wo"},
    
    // き系
    {"きぇ", "kʲe"}, {"きゃ", "kʲa"}, {"きゅ", "kʲɨ"}, {"きょ", "kʲo"},
    
    // ぎ系
    {"ぎゃ", "ɡʲa"}, {"ぎゅ", "ɡʲɨ"}, {"ぎょ", "ɡʲo"},
    
    // く系
    {"くぁ", "kᵝa"}, {"くぃ", "kᵝi"}, {"くぇ", "kᵝe"}, {"くぉ", "kᵝo"},
    
    // ぐ系
    {"ぐぁ", "ɡᵝa"}, {"ぐぃ", "ɡᵝi"}, {"ぐぇ", "ɡᵝe"}, {"ぐぉ", "ɡᵝo"},
    
    // し系
    {"しぇ", "ɕe"}, {"しゃ", "ɕa"}, {"しゅ", "ɕɨ"}, {"しょ", "ɕo"},
    
    // じ系
    {"じぇ", "ʥe"}, {"じゃ", "ʥa"}, {"じゅ", "ʥɨ"}, {"じょ", "ʥo"},
    
    // ち系
    {"ちぇ", "ʨe"}, {"ちゃ", "ʨa"}, {"ちゅ", "ʨɨ"}, {"ちょ", "ʨo"},
    
    // ぢ系
    {"ぢゃ", "ʥa"}, {"ぢゅ", "ʥɨ"}, {"ぢょ", "ʥo"},
    
    // つ系
    {"つぁ", "ʦa"}, {"つぃ", "ʦʲi"}, {"つぇ", "ʦe"}, {"つぉ", "ʦo"},
    
    // て系
    {"てぃ", "tʲi"}, {"てゅ", "tʲɨ"},
    
    // で系
    {"でぃ", "dʲi"}, {"でゅ", "dʲɨ"},
    
    // と系
    {"とぅ", "tɯ"},
    
    // ど系
    {"どぅ", "dɯ"},
    
    // に系
    {"にぇ", "ɲe"}, {"にゃ", "ɲa"}, {"にゅ", "ɲɨ"}, {"にょ", "ɲo"},
    
    // ひ系
    {"ひぇ", "çe"}, {"ひゃ", "ça"}, {"ひゅ", "çɨ"}, {"ひょ", "ço"},
    
    // び系
    {"びゃ", "bʲa"}, {"びゅ", "bʲɨ"}, {"びょ", "bʲo"},
    
    // ぴ系
    {"ぴゃ", "pʲa"}, {"ぴゅ", "pʲɨ"}, {"ぴょ", "pʲo"},
    
    // ふ系
    {"ふぁ", "ɸa"}, {"ふぃ", "ɸʲi"}, {"ふぇ", "ɸe"}, {"ふぉ", "ɸo"},
    {"ふゅ", "ɸʲɨ"}, {"ふょ", "ɸʲo"},
    
    // み系
    {"みゃ", "mʲa"}, {"みゅ", "mʲɨ"}, {"みょ", "mʲo"},
    
    // り系
    {"りゃ", "ɾʲa"}, {"りゅ", "ɾʲɨ"}, {"りょ", "ɾʲo"},
    
    // ゔ系
    {"ゔぁ", "va"}, {"ゔぃ", "vʲi"}, {"ゔぇ", "ve"}, {"ゔぉ", "vo"},
    {"ゔゅ", "bʲɨ"}, {"ゔょ", "bʲo"},
};

// 标点符号映射
static const KanaMapping PUNCT_MAPPING[] = {
    {"。", "."}, {"、", ","}, {"？", "?"}, {"！", "!"},
    {"「", "\""}, {"」", "\""}, {"『", "\""}, {"』", "\""},
    {"：", ":"}, {"；", ";"}, {"（", "("}, {"）", ")"},
    {"《", "("}, {"》", ")"}, {"【", "["}, {"】", "]"},
    {"・", " "}, {"，", ","}, {"～", "-"}, {"〜", "-"},
    {"—", "-"}, {"«", "\""}, {"»", "\""},
};

#define HEPBURN_SINGLE_COUNT (sizeof(HEPBURN_SINGLE) / sizeof(HEPBURN_SINGLE[0]))
#define HEPBURN_DIGRAPH_COUNT (sizeof(HEPBURN_DIGRAPH) / sizeof(HEPBURN_DIGRAPH[0]))
#define PUNCT_COUNT (sizeof(PUNCT_MAPPING) / sizeof(PUNCT_MAPPING[0]))

// 片假名→平假名映射（基础字符）
static const char* kata_to_hira(const char *kata) {
    static char result[16];
    
    // 获取第一个 UTF-8 字符
    uint32_t codepoint;
    int bytes = misaki_utf8_decode(kata, &codepoint);
    if (bytes == 0) return NULL;
    
    // 片假名范围：0x30A1-0x30FF
    // 平假名范围：0x3041-0x309F
    // 差值：0x60
    if (codepoint >= 0x30A1 && codepoint <= 0x30F6) {
        uint32_t hira_codepoint = codepoint - 0x60;
        misaki_utf8_encode(hira_codepoint, result);
        return result;
    }
    
    return NULL;
}

/* ============================================================================
 * 查找函数
 * ========================================================================== */

/**
 * 查找假名的 IPA 映射
 * 
 * @param kana 假名字符串（UTF-8，支持平假名和片假名）
 * @param out_ipa 输出：IPA 音素（如果找到）
 * @return 匹配的字节数，0表示未找到
 */
int misaki_kana_to_ipa(const char *kana, const char **out_ipa) {
    if (!kana || !out_ipa) {
        return 0;
    }
    
    // 尝试片假名→平假名转换
    const char *hira = kata_to_hira(kana);
    const char *search_kana = hira ? hira : kana;
    
    // 1. 尝试双字符匹配
    for (size_t i = 0; i < HEPBURN_DIGRAPH_COUNT; i++) {
        size_t len = strlen(HEPBURN_DIGRAPH[i].kana);
        if (strncmp(search_kana, HEPBURN_DIGRAPH[i].kana, len) == 0) {
            *out_ipa = HEPBURN_DIGRAPH[i].ipa;
            // 返回原始 kana 的字节数（单字符 = 3字节）
            return hira ? 3 : (int)len;  // 修复：返回单个字符的字节数
        }
    }
    
    // 2. 尝试单字符匹配
    for (size_t i = 0; i < HEPBURN_SINGLE_COUNT; i++) {
        size_t len = strlen(HEPBURN_SINGLE[i].kana);
        if (strncmp(search_kana, HEPBURN_SINGLE[i].kana, len) == 0) {
            *out_ipa = HEPBURN_SINGLE[i].ipa;
            return hira ? 3 : (int)len;  // 修复：返回单个字符的字节数
        }
    }
    
    // 3. 尝试标点符号匹配
    for (size_t i = 0; i < PUNCT_COUNT; i++) {
        size_t len = strlen(PUNCT_MAPPING[i].kana);
        if (strncmp(kana, PUNCT_MAPPING[i].kana, len) == 0) {
            *out_ipa = PUNCT_MAPPING[i].ipa;
            return (int)len;
        }
    }
    
    return 0;
}

/**
 * 处理特殊假名字符
 * 
 * @param kana 当前假名
 * @param next_kana 下一个假名（可为 NULL）
 * @param out_ipa 输出：IPA 音素
 * @return 匹配的字节数
 */
int misaki_kana_special(const char *kana, const char *next_kana, const char **out_ipa) {
    // 促音 ッ/っ → ʔ
    if (strcmp(kana, "っ") == 0 || strcmp(kana, "ッ") == 0) {
        *out_ipa = "ʔ";
        return (int)strlen(kana);
    }
    
    // 拨音 ン/ん → 根据后续字符决定
    if (strcmp(kana, "ん") == 0 || strcmp(kana, "ン") == 0) {
        if (next_kana) {
            // 简化处理：根据下一个音的首字母
            const char *next_ipa = NULL;
            misaki_kana_to_ipa(next_kana, &next_ipa);
            
            if (next_ipa) {
                // m before m,p,b
                if (next_ipa[0] == 'm' || next_ipa[0] == 'p' || next_ipa[0] == 'b') {
                    *out_ipa = "m";
                    return (int)strlen(kana);
                }
                // ŋ before k,g
                else if (next_ipa[0] == 'k' || next_ipa[0] == 'g') {
                    *out_ipa = "ŋ";
                    return (int)strlen(kana);
                }
                // ɲ before ɲ,ʨ,ʥ
                else if ((unsigned char)next_ipa[0] > 0x7F) { // 多字节 UTF-8
                    if (strncmp(next_ipa, "ʨ", 3) == 0 || strncmp(next_ipa, "ʥ", 3) == 0 ||
                        strncmp(next_ipa, "ɲ", 3) == 0) {
                        *out_ipa = "ɲ";
                        return (int)strlen(kana);
                    }
                }
                // n before n,t,d,r,z
                else if (next_ipa[0] == 'n' || next_ipa[0] == 't' || 
                         next_ipa[0] == 'd' || next_ipa[0] == 'r' || next_ipa[0] == 'z') {
                    *out_ipa = "n";
                    return (int)strlen(kana);
                }
            }
        }
        // 默认：ɴ
        *out_ipa = "ɴ";
        return (int)strlen(kana);
    }
    
    // 长音符 ー → ː
    if (strcmp(kana, "ー") == 0) {
        *out_ipa = "ː";
        return (int)strlen(kana);
    }
    
    return 0;
}

/**
 * 将整个假名字符串转换为 IPA
 * 
 * @param kana_str 假名字符串（UTF-8）
 * @param out_buffer 输出缓冲区
 * @param buffer_size 缓冲区大小
 * @return IPA 音素长度，-1表示错误
 */
int misaki_kana_string_to_ipa(const char *kana_str, char *out_buffer, int buffer_size) {
    if (!kana_str || !out_buffer || buffer_size <= 0) {
        return -1;
    }
    
    int pos = 0;
    const char *p = kana_str;
    
    while (*p && pos < buffer_size - 1) {
        // 获取下一个字符位置
        const char *next_p = p;
        uint32_t codepoint;
        int bytes = misaki_utf8_decode(p, &codepoint);
        if (bytes > 0) {
            next_p += bytes;
        }
        
        const char *ipa = NULL;
        int matched = 0;
        
        // 1. 尝试特殊字符处理
        char current_char[16] = {0};
        strncpy(current_char, p, bytes);
        
        char next_char[16] = {0};
        if (*next_p) {
            int next_bytes = misaki_utf8_decode(next_p, &codepoint);
            if (next_bytes > 0) {
                strncpy(next_char, next_p, next_bytes);
            }
        }
        
        matched = misaki_kana_special(current_char, *next_char ? next_char : NULL, &ipa);
        
        // 2. 如果不是特殊字符，使用普通映射
        if (matched == 0) {
            matched = misaki_kana_to_ipa(p, &ipa);
        }
        
        // 3. 如果找到映射，复制到输出
        if (matched > 0 && ipa) {
            int ipa_len = strlen(ipa);
            if (pos + ipa_len < buffer_size) {
                strcpy(out_buffer + pos, ipa);
                pos += ipa_len;
            }
            p += matched;
        } else {
            // 未找到映射，跳过这个字符
            p += bytes;
        }
    }
    
    out_buffer[pos] = '\0';
    return pos;
}
