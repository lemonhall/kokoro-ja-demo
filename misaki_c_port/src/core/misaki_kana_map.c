/**
 * misaki_kana_map.c
 * 
 * 假名→IPA 音素映射表（从 Python の HEPBURN 和 M2P 移植）
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

// 単字符平假名映射（84个基础字符）
static const KanaMapping HEPBURN_SINGLE[] = {
    // あ行
    {"ぁ", "a"}, {"あ", "a"}, {"ぃ", "i"}, {"い", "i"},
    {"ぅ", "ɯ"}, {"う", "ɯ"}, {"ぇ", "e"}, {"え", "e"},
    {"ぉ", "o"}, {"お", "o"},

    // か行
    // ⭐ 修正：简化 IPA，移除不必要的腭音化标记
    {"か", "ka"}, {"が", "ɡa"}, {"き", "ki"}, {"ぎ", "ɡi"},
    {"く", "kɯ"}, {"ぐ", "ɡɯ"}, {"け", "ke"}, {"げ", "ɡe"},
    {"こ", "ko"}, {"ご", "ɡo"},

    // さ行
    // ⭐ 修正：统一使用更标准的 IPA 符号
    {"さ", "sa"}, {"ざ", "dza"}, {"し", "ɕi"}, {"じ", "dʑi"},
    {"す", "sɨ"}, {"ず", "dzɨ"}, {"せ", "se"}, {"ぜ", "dze"},
    {"そ", "so"}, {"ぞ", "dzo"},

    // た行
    // ⭐ 修正：「ち」和「ぢ」使用标准 IPA tɕ 和 dʑ
    {"た", "ta"}, {"だ", "da"}, {"ち", "tɕi"}, {"ぢ", "dʑi"},
    // っ 在特殊处理中
    {"つ", "ʦɨ"}, {"づ", "zɨ"}, {"て", "te"}, {"で", "de"},
    {"と", "to"}, {"ど", "do"},
    
    // な行
    // ⭐ 修正：简化 IPA，移除不必要的腭音化标记
    {"な", "na"}, {"に", "ni"}, {"ぬ", "nɯ"}, {"ね", "ne"},
    {"の", "no"},

    // は行
    {"は", "ha"}, {"ば", "ba"}, {"ぱ", "pa"}, {"ひ", "çi"},
    {"び", "bi"}, {"ぴ", "pi"}, {"ふ", "ɸɯ"}, {"ぶ", "bɯ"},
    {"ぷ", "pɯ"}, {"へ", "he"}, {"べ", "be"}, {"ぺ", "pe"},
    {"ほ", "ho"}, {"ぼ", "bo"}, {"ぽ", "po"},
    
    // ま行
    {"ま", "ma"}, {"み", "mi"}, {"む", "mɯ"}, {"め", "me"},
    {"も", "mo"},

    // や行
    {"ゃ", "ja"}, {"や", "ja"}, {"ゅ", "jɯ"}, {"ゆ", "jɯ"},
    {"ょ", "jo"}, {"よ", "jo"},
    
    // ら行
    {"ら", "ɾa"}, {"り", "ɾi"}, {"る", "ɾɯ"}, {"れ", "ɾe"},
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
    {"きぇ", "ke"}, {"きゃ", "ka"}, {"きゅ", "kɨ"}, {"きょ", "ko"},
    
    // ぎ系
    {"ぎゃ", "ɡa"}, {"ぎゅ", "ɡɨ"}, {"ぎょ", "ɡo"},
    
    // く系
    {"くぁ", "kᵝa"}, {"くぃ", "kᵝi"}, {"くぇ", "kᵝe"}, {"くぉ", "kᵝo"},
    
    // ぐ系
    {"ぐぁ", "ɡᵝa"}, {"ぐぃ", "ɡᵝi"}, {"ぐぇ", "ɡᵝe"}, {"ぐぉ", "ɡᵝo"},
    
    // し系
    {"しぇ", "ɕe"}, {"しゃ", "ɕa"}, {"しゅ", "ɕɨ"}, {"しょ", "ɕo"},
    
    // じ系
    {"じぇ", "ʥe"}, {"じゃ", "ʥa"}, {"じゅ", "ʥɨ"}, {"じょ", "ʥo"},
    
    // ち系
    {"ちぇ", "tɕe"}, {"ちゃ", "tɕa"}, {"ちゅ", "tɕɨ"}, {"ちょ", "tɕo"},
    
    // ぢ系
    {"ぢゃ", "ʥa"}, {"ぢゅ", "ʥɨ"}, {"ぢょ", "ʥo"},
    
    // つ系
    {"つぁ", "tsa"}, {"つぃ", "tsi"}, {"つぇ", "tse"}, {"つぉ", "tso"},

    // て系
    {"てぃ", "tʲi"}, {"てゅ", "tʲɨ"},
    
    // で系
    {"でぃ", "dʲi"}, {"でゅ", "dʲɨ"},
    
    // と系
    {"とぅ", "tɯ"},
    
    // ど系
    {"どぅ", "dɯ"},
    
    // に系
    {"にぇ", "ne"}, {"にゃ", "na"}, {"にゅ", "nɨ"}, {"にょ", "no"},
    
    // ひ系
    {"ひぇ", "çe"}, {"ひゃ", "ça"}, {"ひゅ", "çɨ"}, {"ひょ", "ço"},
    
    // び系
    {"びゃ", "ba"}, {"びゅ", "bɨ"}, {"びょ", "bo"},
    
    // ぴ系
    {"ぴゃ", "pa"}, {"ぴゅ", "pɨ"}, {"ぴょ", "po"},
    
    // ふ系
    {"ふぁ", "ɸa"}, {"ふぃ", "ɸi"}, {"ふぇ", "ɸe"}, {"ふぉ", "ɸo"},
    {"ふゅ", "ɸɨ"}, {"ふょ", "ɸo"},
    
    // み系
    {"みゃ", "ma"}, {"みゅ", "mɨ"}, {"みょ", "mo"},
    
    // り系
    {"りゃ", "ɾa"}, {"りゅ", "ɾɨ"}, {"りょ", "ɾo"},
    
    // ゔ系
    {"ゔぁ", "va"}, {"ゔぃ", "vi"}, {"ゔぇ", "ve"}, {"ゔぉ", "vo"},
    {"ゔゅ", "bɨ"}, {"ゔょ", "bo"},
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
    // 差値：0x60
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
 * 查找假名の IPA 映射
 * 
 * @param kana 假名字符串（UTF-8，支持平假名和片假名）
 * @param out_ipa 输出：IPA 音素（如果找到）
 * @return 匹配の字节数，0表示未找到
 */
int misaki_kana_to_ipa(const char *kana, const char **out_ipa) {
    if (!kana || !out_ipa) {
        return 0;
    }
    
    // 尝試片假名→平假名変換
    const char *hira = kata_to_hira(kana);
    const char *search_kana = hira ? hira : kana;
    
    // 1. 尝試双字符マッチ
    for (size_t i = 0; i < HEPBURN_DIGRAPH_COUNT; i++) {
        size_t len = strlen(HEPBURN_DIGRAPH[i].kana);
        if (strncmp(search_kana, HEPBURN_DIGRAPH[i].kana, len) == 0) {
            *out_ipa = HEPBURN_DIGRAPH[i].ipa;
            // 返却元の kana の字节数（単字符 = 3字節）
            return hira ? 3 : (int)len;  // 修正：単個文字の字节数を返却
        }
    }
    
    // 2. 尝試単字符マッチ
    for (size_t i = 0; i < HEPBURN_SINGLE_COUNT; i++) {
        size_t len = strlen(HEPBURN_SINGLE[i].kana);
        if (strncmp(search_kana, HEPBURN_SINGLE[i].kana, len) == 0) {
            *out_ipa = HEPBURN_SINGLE[i].ipa;
            return hira ? 3 : (int)len;  // 修正：単個文字の字节数を返却
        }
    }
    
    // 3. 尝試標点符号マッチ
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
 * 特殊假名文字の処理
 * 
 * @param kana 現在の假名
 * @param next_kana 次の假名（可为 NULL）
 * @param out_ipa 出力：IPA 音素
 * @return 匹配の字节数
 */
int misaki_kana_special(const char *kana, const char *next_kana, const char **out_ipa) {
    // 促音 ッ/っ → ʔ
    if (strcmp(kana, "っ") == 0 || strcmp(kana, "ッ") == 0) {
        *out_ipa = "ʔ";
        return (int)strlen(kana);
    }
    
    // 拨音 ン/ん → 次の音の先頭文字によって決定
    if (strcmp(kana, "ん") == 0 || strcmp(kana, "ン") == 0) {
        if (next_kana) {
            // 簡易処理：次の音の先頭文字
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
                // ɲ before ɲ,tɕ,dʑ,ɕ (修正：更新为新的 IPA 符号)
                else if ((unsigned char)next_ipa[0] > 0x7F) { // 多文字 UTF-8
                    if (strncmp(next_ipa, "tɕ", 4) == 0 || strncmp(next_ipa, "dʑ", 4) == 0 ||
                        strncmp(next_ipa, "ɲ", 3) == 0 || strncmp(next_ipa, "ɕ", 3) == 0) {
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
        // デフォルト：ɴ
        *out_ipa = "ɴ";
        return (int)strlen(kana);
    }
    
    // 長音符 ー → ː
    if (strcmp(kana, "ー") == 0) {
        *out_ipa = "ː";
        return (int)strlen(kana);
    }
    
    return 0;
}

/**
 * 偽名文字列を IPA に変換
 * 
 * @param kana_str 偽名文字列（UTF-8）
 * @param out_buffer 出力バッファ
 * @param buffer_size バッファサイズ
 * @return IPA 音素長さ，-1表示エラー
 */
int misaki_kana_string_to_ipa(const char *kana_str, char *out_buffer, int buffer_size) {
    if (!kana_str || !out_buffer || buffer_size <= 0) {
        return -1;
    }
    
    int pos = 0;
    const char *p = kana_str;
    char prev_vowel = '\0';  // ⭐ 记录前一个元音，用于长音检测
    
    while (*p && pos < buffer_size - 1) {
        // 次の文字位置を取得
        const char *next_p = p;
        uint32_t codepoint;
        int bytes = misaki_utf8_decode(p, &codepoint);
        if (bytes > 0) {
            next_p += bytes;
        }
        
        const char *ipa = NULL;
        int matched = 0;
        
        // 1. 特殊文字の処理を試みる
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
        
        // 2. 特殊文字でない場合は通常のマッピングを使用
        if (matched == 0) {
            matched = misaki_kana_to_ipa(p, &ipa);
        }
        
        // 3. ⭐ 长音检测：「う」在 o 段后面时转换为长音
        if (matched > 0 && ipa) {
            // 检测是否是 "う" 或 "ウ"
            if ((strcmp(current_char, "う") == 0 || strcmp(current_char, "ウ") == 0) && prev_vowel == 'o') {
                // 不输出 "ɯ"，而是输出长音符 "ː"
                if (pos + 3 < buffer_size) {  // ⭐ 修正：UTF-8 编码的 ː 需要 3 字节
                    // 直接输出 UTF-8 编码的长音符
                    out_buffer[pos++] = (char)0xCB;
                    out_buffer[pos++] = (char)0x90;
                }
                p += matched;
                // prev_vowel 保持为 'o'，因为长音后还是 o 段
            }
            // 检测是否是 "い" 或 "イ" 在 e 段后面
            else if ((strcmp(current_char, "い") == 0 || strcmp(current_char, "イ") == 0) && prev_vowel == 'e') {
                // 不输出 "i"，而是输出长音符 "ː"
                if (pos + 3 < buffer_size) {  // ⭐ 修正：UTF-8 编码
                    out_buffer[pos++] = (char)0xCB;
                    out_buffer[pos++] = (char)0x90;
                }
                p += matched;
                // prev_vowel 保持为 'e'
            }
            else {
                // 正常输出 IPA
                int ipa_len = strlen(ipa);
                if (pos + ipa_len < buffer_size) {
                    strcpy(out_buffer + pos, ipa);
                    pos += ipa_len;
                    
                    // ⭐ 更新 prev_vowel：提取最后一个字符作为元音
                    if (ipa_len > 0) {
                        char last_char = ipa[ipa_len - 1];
                        if (last_char == 'a' || last_char == 'e' || last_char == 'i' || 
                            last_char == 'o' || last_char == 'ɨ') {
                            prev_vowel = last_char;
                        } else {
                            prev_vowel = '\0';  // 不是元音结尾
                        }
                    }
                }
                p += matched;
            }
        } else {
            // マッピングが見つからなかった場合はこの文字をスキップ
            p += bytes;
            prev_vowel = '\0';
        }
    }
    
    out_buffer[pos] = '\0';
    return pos;
}
