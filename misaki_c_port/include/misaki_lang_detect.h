/**
 * misaki_lang_detect.h
 * 
 * 多语言文本自动检测模块
 * 基于多层检测策略：字符集分析 → 特征词匹配 → n-gram分析 → 分词验证
 * 
 * License: MIT
 */

#ifndef MISAKI_LANG_DETECT_H
#define MISAKI_LANG_DETECT_H

#include "misaki_types.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * 类型定义
 * ========================================================================== */

// 使用 misaki_types.h 中已定义的 MisakiLanguage 枚举
// typedef enum {
//     LANG_ENGLISH,
//     LANG_CHINESE,
//     LANG_JAPANESE,
//     LANG_KOREAN,
//     LANG_VIETNAMESE,
//     LANG_UNKNOWN
// } MisakiLanguage;

/**
 * 字符集统计信息
 */
typedef struct {
    int hiragana_count;     // 平假名数量
    int katakana_count;     // 片假名数量
    int kanji_count;        // 汉字数量
    int latin_count;        // 拉丁字母数量
    int digit_count;        // 数字数量
    int punctuation_count;  // 标点符号数量
    int hangul_count;       // 韩文字符数量
    int total_chars;        // 总字符数
} CharsetStats;

/**
 * 语言检测结果
 */
typedef struct {
    MisakiLanguage language;    // 检测到的语言
    float confidence;           // 置信度 (0.0 - 1.0)
    CharsetStats charset;       // 字符集统计
    const char *reason;         // 检测原因（调试用）
} LangDetectResult;

/**
 * 特征词权重表项
 */
typedef struct {
    const char *word;           // 特征词
    MisakiLanguage lang;        // 所属语言
    float weight;               // 权重（越高越重要）
} LangFeatureWord;

/**
 * 语言检测器配置
 */
typedef struct {
    bool enable_ngram;          // 启用n-gram分析
    bool enable_tokenization;   // 启用分词验证
    float confidence_threshold; // 置信度阈值
    
    // 外部分词器（可选）
    void *zh_tokenizer;
    void *ja_tokenizer;
} LangDetectorConfig;

/**
 * 语言检测器上下文
 */
typedef struct {
    LangDetectorConfig config;
    
    // 特征词表（运行时构建）
    LangFeatureWord *feature_words;
    int feature_word_count;
    
    // n-gram表
    const char **jp_bigrams;
    const char **zh_bigrams;
    const char **en_bigrams;
} LangDetector;

/* ============================================================================
 * API 函数
 * ========================================================================== */

/**
 * 创建语言检测器
 * 
 * @param config 配置参数（可为NULL，使用默认配置）
 * @return 检测器实例，失败返回NULL
 */
LangDetector* misaki_lang_detector_create(const LangDetectorConfig *config);

/**
 * 销毁语言检测器
 */
void misaki_lang_detector_free(LangDetector *detector);

/**
 * 检测文本语言（主接口，完整检测）
 * 
 * @param detector 检测器实例
 * @param text 输入文本（UTF-8编码）
 * @return 检测结果
 */
LangDetectResult misaki_lang_detect_full(LangDetector *detector, const char *text);

/**
 * 快速检测语言（仅基于字符集，无需创建检测器）
 * 注意：这是简化版本，与 misaki_tokenizer.h 中的 misaki_detect_language 功能类似
 * 
 * @param text 输入文本
 * @return 语言类型
 */
MisakiLanguage misaki_lang_detect_quick(const char *text);

/**
 * 获取语言名称（用于调试输出）
 */
const char* misaki_language_name(MisakiLanguage lang);

/* ============================================================================
 * 辅助函数（高级用法）
 * ========================================================================== */

/**
 * 分析字符集分布
 */
CharsetStats misaki_analyze_charset(const char *text);

/**
 * 基于特征词检测
 */
MisakiLanguage misaki_detect_by_features(const char *text);

/**
 * 基于n-gram检测
 */
MisakiLanguage misaki_detect_by_ngrams(const char *text);

/**
 * 基于分词质量检测（需要传入分词器）
 */
MisakiLanguage misaki_detect_by_tokenization(
    void *zh_tokenizer,
    void *ja_tokenizer,
    const char *text
);

/**
 * 检测是否为纯假名文本
 */
bool misaki_is_pure_kana(const char *text);

/**
 * 检测是否为纯汉字文本
 */
bool misaki_is_pure_kanji(const char *text);

/**
 * 检测是否为纯拉丁字母文本
 */
bool misaki_is_pure_latin(const char *text);

/**
 * 检测日文特征词（行政区划、助词等）
 */
bool misaki_has_japanese_features(const char *text);

#ifdef __cplusplus
}
#endif

#endif /* MISAKI_LANG_DETECT_H */
