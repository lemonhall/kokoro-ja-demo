/**
 * misaki_lang_detect.c
 * 
 * 多语言文本自动检测实现
 * 采用分层检测策略：快速字符集分析 → 特征词匹配 → n-gram分析 → 分词验证
 * 
 * License: MIT
 */

#include "misaki_lang_detect.h"
#include "misaki_string.h"
#include "misaki_tokenizer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ============================================================================
 * 常量定义
 * ========================================================================== */

// 日文特征词（高频助词、后缀）
static const LangFeatureWord JP_FEATURES[] = {
    // 助词（超高权重）
    {"です", LANG_JAPANESE, 10.0},
    {"ます", LANG_JAPANESE, 10.0},
    {"ました", LANG_JAPANESE, 10.0},
    {"でした", LANG_JAPANESE, 10.0},
    {"ません", LANG_JAPANESE, 9.0},
    
    // 格助词
    {"は", LANG_JAPANESE, 8.0},
    {"が", LANG_JAPANESE, 8.0},
    {"を", LANG_JAPANESE, 8.0},
    {"に", LANG_JAPANESE, 7.0},
    {"の", LANG_JAPANESE, 7.0},
    {"と", LANG_JAPANESE, 6.0},
    {"で", LANG_JAPANESE, 6.0},
    {"から", LANG_JAPANESE, 6.0},
    {"まで", LANG_JAPANESE, 6.0},
    {"より", LANG_JAPANESE, 5.0},
    
    // 接续词
    {"て", LANG_JAPANESE, 6.0},
    {"た", LANG_JAPANESE, 5.0},
    {"だ", LANG_JAPANESE, 5.0},
    {"ない", LANG_JAPANESE, 6.0},
    
    // 行政区划
    {"都", LANG_JAPANESE, 4.0},
    {"道", LANG_JAPANESE, 4.0},
    {"府", LANG_JAPANESE, 4.0},
    {"県", LANG_JAPANESE, 4.0},
    {"市", LANG_JAPANESE, 3.0},
    {"区", LANG_JAPANESE, 3.0},
    {"町", LANG_JAPANESE, 3.0},
    {"村", LANG_JAPANESE, 3.0},
    
    {NULL, LANG_UNKNOWN, 0.0}
};

// 中文特征词（高频虚词）
static const LangFeatureWord ZH_FEATURES[] = {
    // 结构助词
    {"的", LANG_CHINESE, 10.0},
    {"了", LANG_CHINESE, 8.0},
    {"着", LANG_CHINESE, 7.0},
    {"过", LANG_CHINESE, 7.0},
    
    // 连词
    {"和", LANG_CHINESE, 6.0},
    {"与", LANG_CHINESE, 5.0},
    {"或", LANG_CHINESE, 5.0},
    {"但", LANG_CHINESE, 6.0},
    {"而", LANG_CHINESE, 6.0},
    {"且", LANG_CHINESE, 5.0},
    
    // 系词/副词
    {"是", LANG_CHINESE, 9.0},
    {"在", LANG_CHINESE, 7.0},
    {"有", LANG_CHINESE, 7.0},
    {"为", LANG_CHINESE, 6.0},
    {"就", LANG_CHINESE, 6.0},
    {"都", LANG_CHINESE, 5.0},
    {"也", LANG_CHINESE, 6.0},
    {"不", LANG_CHINESE, 6.0},
    {"很", LANG_CHINESE, 5.0},
    {"更", LANG_CHINESE, 5.0},
    
    // 介词
    {"对", LANG_CHINESE, 5.0},
    {"向", LANG_CHINESE, 4.0},
    {"从", LANG_CHINESE, 5.0},
    {"到", LANG_CHINESE, 5.0},
    {"被", LANG_CHINESE, 5.0},
    {"把", LANG_CHINESE, 5.0},
    
    {NULL, LANG_UNKNOWN, 0.0}
};

// 英文特征词
static const LangFeatureWord EN_FEATURES[] = {
    {"the", LANG_ENGLISH, 10.0},
    {"and", LANG_ENGLISH, 8.0},
    {"of", LANG_ENGLISH, 8.0},
    {"to", LANG_ENGLISH, 7.0},
    {"in", LANG_ENGLISH, 7.0},
    {"is", LANG_ENGLISH, 6.0},
    {"you", LANG_ENGLISH, 6.0},
    {"that", LANG_ENGLISH, 6.0},
    {"it", LANG_ENGLISH, 5.0},
    {"for", LANG_ENGLISH, 5.0},
    {"with", LANG_ENGLISH, 5.0},
    {"on", LANG_ENGLISH, 4.0},
    {"have", LANG_ENGLISH, 5.0},
    {"be", LANG_ENGLISH, 5.0},
    {"ing", LANG_ENGLISH, 4.0},
    
    {NULL, LANG_UNKNOWN, 0.0}
};

// 昆雅语特征词
static const LangFeatureWord QYA_FEATURES[] = {
    // 常见昆雅语词汇
    {"quenya", LANG_QUENYA, 10.0},
    {"eldar", LANG_QUENYA, 9.0},
    {"valar", LANG_QUENYA, 9.0},
    {"arda", LANG_QUENYA, 8.0},
    {"silmarillion", LANG_QUENYA, 9.0},
    
    // 常见后缀
    {"ion", LANG_QUENYA, 5.0},
    {"iel", LANG_QUENYA, 5.0},
    {"wen", LANG_QUENYA, 4.0},
    {"ndil", LANG_QUENYA, 6.0},
    
    {NULL, LANG_UNKNOWN, 0.0}
};

// 日文常见2-gram
static const char *JP_BIGRAMS[] = {
    "です", "ます", "した", "して", "こと", "もの", "よう", "たい",
    "ない", "れる", "られる", "という", "であ", "での", "には",
    "ており", "として", "について", "において", "による",
    "ている", "ていた", "ていく", "ていて", "でいる",
    NULL
};

// 中文常见2-gram（词组级别）
static const char *ZH_BIGRAMS[] = {
    "的是", "的人", "的时", "的话", "的地", "的情", "的事",
    "了一", "了解", "了解", "了吗",
    "在中", "在这", "在那", "在于", "在一",
    "有的", "有一", "有人", "有关", "有些",
    "是一", "是在", "是的", "是个", "是否",
    "而且", "而是", "而不", "但是", "可以",
    "这个", "这些", "那个", "那些", "什么",
    NULL
};

// 英文常见2-gram
static const char *EN_BIGRAMS[] = {
    "of the", "in the", "to the", "and the", "for the",
    "is a", "it is", "that is", "this is", "there is",
    "have been", "has been", "will be", "can be",
    "do not", "does not", "did not", "will not",
    NULL
};

/* ============================================================================
 * Unicode 字符分类
 * ========================================================================== */

/**
 * 判断是否为平假名
 */
static bool is_hiragana(uint32_t codepoint) {
    return (codepoint >= 0x3040 && codepoint <= 0x309F);
}

/**
 * 判断是否为片假名
 */
static bool is_katakana(uint32_t codepoint) {
    return (codepoint >= 0x30A0 && codepoint <= 0x30FF) ||
           (codepoint >= 0x31F0 && codepoint <= 0x31FF);  // 扩展片假名
}

/**
 * 判断是否为汉字（CJK统一汉字）
 */
static bool is_kanji(uint32_t codepoint) {
    return (codepoint >= 0x4E00 && codepoint <= 0x9FFF) ||   // CJK基本
           (codepoint >= 0x3400 && codepoint <= 0x4DBF) ||   // CJK扩展A
           (codepoint >= 0x20000 && codepoint <= 0x2A6DF) || // CJK扩展B
           (codepoint >= 0xF900 && codepoint <= 0xFAFF);     // CJK兼容
}

/**
 * 判断是否为拉丁字母
 */
static bool is_latin(uint32_t codepoint) {
    return (codepoint >= 'A' && codepoint <= 'Z') ||
           (codepoint >= 'a' && codepoint <= 'z');
}

/**
 * 判断是否为数字
 */
static bool is_digit(uint32_t codepoint) {
    return (codepoint >= '0' && codepoint <= '9');
}

/**
 * 判断是否为昆雅语特殊字符
 */
static bool is_quenya_special(uint32_t codepoint) {
    // ñ (U+00F1), þ (U+00FE), á, é, í, ó, ú, ë
    return (codepoint == 0x00F1 || codepoint == 0x00FE ||
            codepoint == 0x00E1 || codepoint == 0x00E9 ||
            codepoint == 0x00ED || codepoint == 0x00F3 ||
            codepoint == 0x00FA || codepoint == 0x00EB);
}

/**
 * 判断是否为韩文
 */
static bool is_hangul(uint32_t codepoint) {
    return (codepoint >= 0xAC00 && codepoint <= 0xD7AF) ||  // 韩文音节
           (codepoint >= 0x1100 && codepoint <= 0x11FF);    // 韩文字母
}

/**
 * 判断是否为标点符号
 */
static bool is_punctuation(uint32_t codepoint) {
    return (codepoint >= 0x2000 && codepoint <= 0x206F) ||  // 通用标点
           (codepoint >= 0x3000 && codepoint <= 0x303F) ||  // CJK标点
           (codepoint == '.' || codepoint == ',' || codepoint == '!' ||
            codepoint == '?' || codepoint == ';' || codepoint == ':');
}

/* ============================================================================
 * 字符集分析
 * ========================================================================== */

/**
 * 分析字符集分布
 */
CharsetStats misaki_analyze_charset(const char *text) {
    CharsetStats stats = {0};
    
    if (!text) return stats;
    
    const char *p = text;
    while (*p) {
        uint32_t codepoint;
        int bytes = misaki_utf8_decode(p, &codepoint);
        if (bytes == 0) break;
        
        if (is_hiragana(codepoint)) {
            stats.hiragana_count++;
        } else if (is_katakana(codepoint)) {
            stats.katakana_count++;
        } else if (is_kanji(codepoint)) {
            stats.kanji_count++;
        } else if (is_latin(codepoint)) {
            stats.latin_count++;
        } else if (is_digit(codepoint)) {
            stats.digit_count++;
        } else if (is_hangul(codepoint)) {
            stats.hangul_count++;
        } else if (is_punctuation(codepoint)) {
            stats.punctuation_count++;
        }
        
        stats.total_chars++;
        p += bytes;
    }
    
    return stats;
}

/* ============================================================================
 * 特征词检测
 * ========================================================================== */

/**
 * 检测特征词并计算得分
 */
static float calculate_feature_score(const char *text, const LangFeatureWord *features) {
    float score = 0.0f;
    int match_count = 0;
    
    for (int i = 0; features[i].word != NULL; i++) {
        const char *word = features[i].word;
        int word_len = strlen(word);
        
        // 滑动窗口查找特征词
        const char *p = text;
        while (*p) {
            if (strncmp(p, word, word_len) == 0) {
                score += features[i].weight;
                match_count++;
                p += word_len;  // 跳过已匹配部分
            } else {
                uint32_t codepoint;
                int bytes = misaki_utf8_decode(p, &codepoint);
                if (bytes == 0) break;
                p += bytes;
            }
        }
    }
    
    return score;
}

/**
 * 基于特征词检测语言
 */
MisakiLanguage misaki_detect_by_features(const char *text) {
    if (!text || strlen(text) < 2) {
        return LANG_UNKNOWN;
    }
    
    // 计算各语言特征得分
    float jp_score = calculate_feature_score(text, JP_FEATURES);
    float zh_score = calculate_feature_score(text, ZH_FEATURES);
    float en_score = calculate_feature_score(text, EN_FEATURES);
    
    // 找出最高分
    float max_score = jp_score;
    MisakiLanguage result = LANG_JAPANESE;
    
    if (zh_score > max_score) {
        max_score = zh_score;
        result = LANG_CHINESE;
    }
    if (en_score > max_score) {
        max_score = en_score;
        result = LANG_ENGLISH;
    }
    
    // 如果得分太低，返回未知
    if (max_score < 1.0f) {
        return LANG_UNKNOWN;
    }
    
    return result;
}

/* ============================================================================
 * n-gram 检测
 * ========================================================================== */

/**
 * 计算n-gram匹配分数
 */
static float calculate_ngram_score(const char *text, const char **ngrams) {
    float score = 0.0f;
    int found = 0;
    
    for (int i = 0; ngrams[i] != NULL; i++) {
        if (strstr(text, ngrams[i]) != NULL) {
            score += 1.0f;
            found++;
        }
    }
    
    return found > 0 ? score / found : 0.0f;
}

/**
 * 基于n-gram检测语言
 */
MisakiLanguage misaki_detect_by_ngrams(const char *text) {
    if (!text || strlen(text) < 4) {
        return LANG_UNKNOWN;
    }
    
    float jp_score = calculate_ngram_score(text, JP_BIGRAMS);
    float zh_score = calculate_ngram_score(text, ZH_BIGRAMS);
    float en_score = calculate_ngram_score(text, EN_BIGRAMS);
    
    float max_score = jp_score;
    MisakiLanguage result = LANG_JAPANESE;
    
    if (zh_score > max_score) {
        max_score = zh_score;
        result = LANG_CHINESE;
    }
    if (en_score > max_score) {
        max_score = en_score;
        result = LANG_ENGLISH;
    }
    
    // n-gram需要更高的阈值
    if (max_score < 0.2f) {
        return LANG_UNKNOWN;
    }
    
    return result;
}

/* ============================================================================
 * 辅助检测函数
 * ========================================================================== */

/**
 * 检测是否为纯假名文本
 */
bool misaki_is_pure_kana(const char *text) {
    CharsetStats stats = misaki_analyze_charset(text);
    int kana_count = stats.hiragana_count + stats.katakana_count;
    return (kana_count > 0 && kana_count == stats.total_chars - stats.punctuation_count);
}

/**
 * 检测是否为纯汉字文本
 */
bool misaki_is_pure_kanji(const char *text) {
    CharsetStats stats = misaki_analyze_charset(text);
    return (stats.kanji_count > 0 && 
            stats.kanji_count == stats.total_chars - stats.punctuation_count &&
            stats.hiragana_count == 0 && 
            stats.katakana_count == 0);
}

/**
 * 检测是否为纯拉丁字母文本
 */
bool misaki_is_pure_latin(const char *text) {
    CharsetStats stats = misaki_analyze_charset(text);
    return (stats.latin_count > 0 && 
            stats.latin_count + stats.digit_count + stats.punctuation_count == stats.total_chars);
}

/**
 * 检测日文特征（行政区划、助词等）
 */
bool misaki_has_japanese_features(const char *text) {
    // 检查日文特有后缀
    const char *jp_suffixes[] = {
        "都", "道", "府", "県", "市", "区", "町", "村",
        "です", "ます", "ました", "ません",
        NULL
    };
    
    for (int i = 0; jp_suffixes[i] != NULL; i++) {
        if (strstr(text, jp_suffixes[i]) != NULL) {
            return true;
        }
    }
    
    return false;
}

/* ============================================================================
 * 快速检测（基于字符集）
 * ========================================================================== */

/**
 * 快速检测语言（仅基于字符集，无需创建检测器）
 */
MisakiLanguage misaki_lang_detect_quick(const char *text) {
    if (!text || strlen(text) < 2) {
        return LANG_UNKNOWN;
    }
    
    CharsetStats stats = misaki_analyze_charset(text);
    
    // 规刱1：有假名 → 日文
    if (stats.hiragana_count > 0 || stats.katakana_count > 0) {
        return LANG_JAPANESE;
    }
    
    // 规刱2：检测昆雅语特殊字符（ñ, þ, 长音符等）
    const unsigned char *p = (const unsigned char *)text;
    int quenya_special_count = 0;
    while (*p) {
        uint32_t codepoint = 0;
        int len = misaki_utf8_decode(p, &codepoint);
        if (len > 0) {
            if (is_quenya_special(codepoint)) {
                quenya_special_count++;
            }
            p += len;
        } else {
            p++;
        }
    }
    
    // 如果有昆雅语特殊字符，且主要是拉丁字母
    if (quenya_special_count > 0 && stats.latin_count > stats.total_chars * 0.5f) {
        return LANG_QUENYA;
    }
    
    // 规刱3：纯拉丁字母 → 英文
    if (stats.latin_count > stats.total_chars * 0.7f) {
        return LANG_ENGLISH;
    }
    
    // 规刱4：韩文
    if (stats.hangul_count > 0) {
        return LANG_KOREAN;
    }
    
    // 规刱5：纯汉字 → 需要进一步判断
    if (stats.kanji_count > 0) {
        // 检查日文特征
        if (misaki_has_japanese_features(text)) {
            return LANG_JAPANESE;
        }
        // 默认为中文
        return LANG_CHINESE;
    }
    
    return LANG_UNKNOWN;
}

/* ============================================================================
 * 完整检测（多层策略）
 * ========================================================================== */

/**
 * 创建语言检测器
 */
LangDetector* misaki_lang_detector_create(const LangDetectorConfig *config) {
    LangDetector *detector = (LangDetector*)calloc(1, sizeof(LangDetector));
    if (!detector) return NULL;
    
    // 设置默认配置
    if (config) {
        detector->config = *config;
    } else {
        detector->config.enable_ngram = true;
        detector->config.enable_tokenization = false;
        detector->config.confidence_threshold = 0.5f;
    }
    
    // 设置n-gram表
    detector->jp_bigrams = JP_BIGRAMS;
    detector->zh_bigrams = ZH_BIGRAMS;
    detector->en_bigrams = EN_BIGRAMS;
    
    return detector;
}

/**
 * 销毁语言检测器
 */
void misaki_lang_detector_free(LangDetector *detector) {
    if (detector) {
        free(detector);
    }
}

/**
 * 检测文本语言（主接口）
 */
LangDetectResult misaki_lang_detect_full(LangDetector *detector, const char *text) {
    LangDetectResult result = {0};
    result.language = LANG_UNKNOWN;
    result.confidence = 0.0f;
    result.reason = "未检测";
    
    if (!text || strlen(text) < 2) {
        result.reason = "文本过短";
        return result;
    }
    
    // 第1层：字符集分析
    result.charset = misaki_analyze_charset(text);
    
    // 如果有假名，直接判定为日文
    if (result.charset.hiragana_count > 0 || result.charset.katakana_count > 0) {
        result.language = LANG_JAPANESE;
        result.confidence = 0.95f;
        result.reason = "含有假名字符";
        return result;
    }
    
    // 如果纯拉丁字母，判定为英文
    float latin_ratio = (float)result.charset.latin_count / result.charset.total_chars;
    if (latin_ratio > 0.7f) {
        result.language = LANG_ENGLISH;
        result.confidence = latin_ratio;
        result.reason = "拉丁字母占比高";
        return result;
    }
    
    // 第2层：特征词检测
    MisakiLanguage feature_lang = misaki_detect_by_features(text);
    if (feature_lang != LANG_UNKNOWN) {
        result.language = feature_lang;
        result.confidence = 0.75f;
        result.reason = "特征词匹配";
        return result;
    }
    
    // 第3层：n-gram分析
    if (detector && detector->config.enable_ngram) {
        MisakiLanguage ngram_lang = misaki_detect_by_ngrams(text);
        if (ngram_lang != LANG_UNKNOWN) {
            result.language = ngram_lang;
            result.confidence = 0.65f;
            result.reason = "n-gram模式匹配";
            return result;
        }
    }
    
    // 第4层：回退策略（基于汉字）
    if (result.charset.kanji_count > 0) {
        // 检查日文特征
        if (misaki_has_japanese_features(text)) {
            result.language = LANG_JAPANESE;
            result.confidence = 0.6f;
            result.reason = "含日文特征词";
        } else {
            result.language = LANG_CHINESE;
            result.confidence = 0.55f;
            result.reason = "汉字默认中文";
        }
        return result;
    }
    
    result.reason = "无法识别";
    return result;
}

/**
 * 获取语言名称
 */
const char* misaki_language_name(MisakiLanguage lang) {
    switch (lang) {
        case LANG_JAPANESE:   return "日语";
        case LANG_CHINESE:    return "中文";
        case LANG_ENGLISH:    return "英文";
        case LANG_VIETNAMESE: return "越南语";
        case LANG_KOREAN:     return "韩语";
        case LANG_QUENYA:     return "昆雅语";
        default:              return "未知";
    }
}
