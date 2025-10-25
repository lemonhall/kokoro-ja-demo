/**
 * misaki_g2p.h
 * 
 * Misaki C Port - G2P (Grapheme-to-Phoneme) Conversion
 * 文本到音素转换（支持英文/中文/日文/韩文/越南文）
 * 
 * License: MIT
 */

#ifndef MISAKI_G2P_H
#define MISAKI_G2P_H

#include "misaki_types.h"
#include "misaki_string.h"
#include "misaki_dict.h"
#include "misaki_tokenizer.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * G2P 配置
 * ========================================================================== */

/**
 * G2P 转换选项
 */
typedef struct {
    // 通用选项
    bool normalize_text;           // 是否规范化文本（默认 true）
    bool remove_punctuation;       // 是否移除标点（默认 false）
    bool output_separators;        // 是否输出分隔符（默认 true）
    
    // 中文选项
    bool zh_tone_sandhi;           // 中文声调变化（默认 true）
    bool zh_erhua;                 // 中文儿化音（默认 true）
    bool zh_neutral_tone;          // 中文轻声处理（默认 true）
    
    // 日文选项
    bool ja_accent;                // 日文声调（默认 false）
    bool ja_long_vowel;            // 日文长音（默认 true）
    
    // 英文选项
    bool en_use_gb;                // 使用英式发音（默认 false，使用美式）
    bool en_syllable_boundary;     // 输出音节边界（默认 false）
} G2POptions;

/**
 * 获取默认 G2P 选项
 * 
 * @return 默认选项
 */
G2POptions misaki_g2p_default_options(void);

/* ============================================================================
 * 英文 G2P (CMUdict)
 * ========================================================================== */

/**
 * 英文 G2P 转换（单词）
 * 
 * @param dict 英文词典
 * @param word 英文单词
 * @param options G2P 选项（可为 NULL）
 * @return 音素字符串，失败返回 NULL（需要调用 free 释放）
 */
char* misaki_en_g2p_word(const EnDict *dict,
                         const char *word,
                         const G2POptions *options);

/**
 * 英文 G2P 转换（句子）
 * 
 * @param dict 英文词典
 * @param text 英文文本
 * @param options G2P 选项（可为 NULL）
 * @return Token 列表（每个 Token 包含音素），失败返回 NULL
 */
MisakiTokenList* misaki_en_g2p(const EnDict *dict,
                               const char *text,
                               const G2POptions *options);

/**
 * 英文未登录词处理（基于规则的音素预测）
 * 
 * @param word 英文单词
 * @return 预测的音素字符串，失败返回 NULL（需要调用 free 释放）
 */
char* misaki_en_g2p_oov(const char *word);

/* ============================================================================
 * 中文 G2P (Pinyin → IPA)
 * ========================================================================== */

/**
 * 拼音（TONE3 格式）转 IPA 音素
 * 
 * 示例: "ni3" → "ni↓"
 * 
 * @param pinyin 拼音（带数字声调，如 "ni3"）
 * @return IPA 音素，失败返回 NULL（需要调用 free 释放）
 */
char* misaki_zh_pinyin_to_ipa(const char *pinyin);

/**
 * 中文 G2P 转换（汉字 → 拼音 → IPA）
 * 
 * @param dict 中文词典
 * @param tokenizer 中文分词器
 * @param text 中文文本
 * @param options G2P 选项（可为 NULL）
 * @return Token 列表（每个 Token 包含音素），失败返回 NULL
 */
MisakiTokenList* misaki_zh_g2p(const ZhDict *dict,
                               void *tokenizer,
                               const char *text,
                               const G2POptions *options);

/**
 * 中文声调变化（Tone Sandhi）
 * 
 * 处理：三声变调、一不变调、轻声等
 * 
 * @param tokens Token 列表
 * @param options G2P 选项（可为 NULL）
 */
void misaki_zh_tone_sandhi(MisakiTokenList *tokens,
                           const G2POptions *options);

/**
 * 中文儿化音处理
 * 
 * 示例: "玩儿" → "war↗" （而不是 "wan↗ ar↓"）
 * 
 * @param tokens Token 列表
 */
void misaki_zh_erhua(MisakiTokenList *tokens);

/* ============================================================================
 * 日文 G2P (Kana → IPA)
 * ========================================================================== */

/**
 * 假名转 IPA 音素（基于 OpenJTalk 规则）
 * 
 * 示例: "こんにちは" → "konnit͡ɕiwa"
 * 
 * @param kana 假名（平假名或片假名）
 * @return IPA 音素，失败返回 NULL（需要调用 free 释放）
 */
char* misaki_ja_kana_to_ipa(const char *kana);

/**
 * 日文 G2P 转换（完整流程：分词 + G2P）
 * 
 * @param tokenizer 日文分词器
 * @param text 日文文本
 * @param options G2P 选项（可为 NULL）
 * @return Token 列表（每个 Token 包含音素），失败返回 NULL
 */
MisakiTokenList* misaki_ja_g2p(void *tokenizer,
                               const char *text,
                               const G2POptions *options);

/**
 * 日文长音处理
 * 
 * 示例: "コーヒー" → "koːhiː"
 * 
 * @param tokens Token 列表
 */
void misaki_ja_long_vowel(MisakiTokenList *tokens);

/* ============================================================================
 * 韩文 G2P (Hangul → IPA)
 * ========================================================================== */

/**
 * 韩文字符转 IPA 音素
 * 
 * @param hangul 韩文字符
 * @return IPA 音素，失败返回 NULL（需要调用 free 释放）
 */
char* misaki_ko_char_to_ipa(const char *hangul);

/**
 * 韩文 G2P 转换
 * 
 * @param text 韩文文本
 * @param options G2P 选项（可为 NULL）
 * @return Token 列表（每个 Token 包含音素），失败返回 NULL
 */
MisakiTokenList* misaki_ko_g2p(const char *text,
                               const G2POptions *options);

/* ============================================================================
 * 越南文 G2P
 * ========================================================================== */

/**
 * 越南文 G2P 转换
 * 
 * @param text 越南文文本
 * @param options G2P 选项（可为 NULL）
 * @return Token 列表（每个 Token 包含音素），失败返回 NULL
 */
MisakiTokenList* misaki_vi_g2p(const char *text,
                               const G2POptions *options);

/* ============================================================================
 * 通用 G2P 接口
 * ========================================================================== */

/**
 * 通用 G2P 转换（自动检测语言）
 * 
 * @param context Misaki 上下文（包含所有词典和分词器）
 * @param text 文本（UTF-8）
 * @param lang 语言类型（LANG_UNKNOWN 表示自动检测）
 * @param options G2P 选项（可为 NULL）
 * @return Token 列表（每个 Token 包含音素），失败返回 NULL
 */
MisakiTokenList* misaki_g2p(MisakiContext *context,
                            const char *text,
                            MisakiLanguage lang,
                            const G2POptions *options);

/**
 * 批量 G2P 转换
 * 
 * @param context Misaki 上下文
 * @param texts 文本数组
 * @param count 文本数量
 * @param lang 语言类型
 * @param options G2P 选项（可为 NULL）
 * @param results 输出：Token 列表数组
 * @return 成功转换的数量
 */
int misaki_g2p_batch(MisakiContext *context,
                     const char **texts,
                     int count,
                     MisakiLanguage lang,
                     const G2POptions *options,
                     MisakiTokenList **results);

/* ============================================================================
 * 音素后处理
 * ========================================================================== */

/**
 * 规范化音素序列
 * 
 * - 移除重复空格
 * - 统一分隔符
 * - 移除无效字符
 * 
 * @param phonemes 音素序列
 * @return 规范化后的音素序列（需要调用 free 释放）
 */
char* misaki_normalize_phonemes(const char *phonemes);

/**
 * 合并 Token 列表的音素为单个字符串
 * 
 * @param tokens Token 列表
 * @param separator 分隔符（如 " " 或 NULL）
 * @return 合并后的音素字符串（需要调用 free 释放）
 */
char* misaki_merge_phonemes(const MisakiTokenList *tokens,
                            const char *separator);

/**
 * 添加音素边界标记
 * 
 * @param tokens Token 列表
 * @param word_boundary 词边界标记（如 "#"）
 * @param sentence_boundary 句子边界标记（如 "."）
 */
void misaki_add_boundaries(MisakiTokenList *tokens,
                          const char *word_boundary,
                          const char *sentence_boundary);

/* ============================================================================
 * 文本规范化（预处理）
 * ========================================================================== */

/**
 * 规范化文本（预处理）
 * 
 * - 全角转半角
 * - 繁体转简体（可选）
 * - 数字转文字
 * - 移除特殊字符
 * 
 * @param text 原始文本
 * @param lang 语言类型
 * @return 规范化后的文本（需要调用 free 释放）
 */
char* misaki_normalize_text(const char *text, MisakiLanguage lang);

/**
 * 中文数字转文字
 * 
 * 示例: "123" → "一百二十三"
 * 
 * @param text 文本
 * @return 转换后的文本（需要调用 free 释放）
 */
char* misaki_zh_num_to_text(const char *text);

/**
 * 英文数字转文字
 * 
 * 示例: "123" → "one hundred twenty three"
 * 
 * @param text 文本
 * @return 转换后的文本（需要调用 free 释放）
 */
char* misaki_en_num_to_text(const char *text);

/**
 * 全角转半角
 * 
 * @param text 文本
 * @return 转换后的文本（需要调用 free 释放）
 */
char* misaki_fullwidth_to_halfwidth(const char *text);

/**
 * 繁体转简体（中文）
 * 
 * @param text 文本
 * @return 转换后的文本（需要调用 free 释放）
 */
char* misaki_traditional_to_simplified(const char *text);

/* ============================================================================
 * IPA 音素工具
 * ========================================================================== */

/**
 * 判断字符是否为 IPA 音素
 * 
 * @param codepoint Unicode 码点
 * @return 是返回 true
 */
bool misaki_is_ipa_phoneme(uint32_t codepoint);

/**
 * 判断字符串是否为有效的 IPA 音素序列
 * 
 * @param phonemes 音素序列
 * @return 有效返回 true
 */
bool misaki_validate_phonemes(const char *phonemes);

/**
 * 统计音素数量
 * 
 * @param phonemes 音素序列
 * @return 音素数量
 */
int misaki_count_phonemes(const char *phonemes);

/**
 * 分割音素序列为单个音素数组
 * 
 * @param phonemes 音素序列
 * @param result 输出：音素数组（MisakiStringView）
 * @param max_count 最大数量
 * @return 实际音素数量
 */
int misaki_split_phonemes(const char *phonemes,
                          MisakiStringView *result,
                          int max_count);

/* ============================================================================
 * 调试与统计
 * ========================================================================== */

/**
 * 打印 G2P 结果
 * 
 * @param tokens Token 列表
 * @param show_details 是否显示详细信息（词性、分数等）
 */
void misaki_g2p_print(const MisakiTokenList *tokens, bool show_details);

/**
 * 获取 G2P 统计信息
 * 
 * @param tokens Token 列表
 * @param total_phonemes 输出：总音素数
 * @param oov_count 输出：未登录词数量
 * @param avg_phonemes_per_token 输出：平均每个 Token 的音素数
 */
void misaki_g2p_stats(const MisakiTokenList *tokens,
                     int *total_phonemes,
                     int *oov_count,
                     double *avg_phonemes_per_token);

/**
 * 比较两个 G2P 结果的相似度
 * 
 * @param tokens1 Token 列表 1
 * @param tokens2 Token 列表 2
 * @return 相似度（0.0 - 1.0）
 */
double misaki_g2p_similarity(const MisakiTokenList *tokens1,
                             const MisakiTokenList *tokens2);

#ifdef __cplusplus
}
#endif

#endif /* MISAKI_G2P_H */
