/**
 * test_lang_detect.c
 * 
 * 语言检测模块单元测试
 */

#include "misaki_lang_detect.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// 测试用例结构
typedef struct {
    const char *text;
    MisakiLanguage expected;
    const char *description;
} TestCase;

// 测试结果统计
typedef struct {
    int total;
    int passed;
    int failed;
} TestStats;

/**
 * 运行单个测试用例
 */
bool run_test(LangDetector *detector, const TestCase *test, TestStats *stats) {
    stats->total++;
    
    printf("测试 #%d: %s\n", stats->total, test->description);
    printf("  文本: \"%s\"\n", test->text);
    printf("  期望: %s\n", misaki_language_name(test->expected));
    
    LangDetectResult result = misaki_lang_detect_full(detector, test->text);
    printf("  结果: %s (置信度: %.2f%%, 原因: %s)\n", 
           misaki_language_name(result.language),
           result.confidence * 100,
           result.reason);
    
    // 显示字符集统计
    if (result.charset.total_chars > 0) {
        printf("  字符: ");
        if (result.charset.hiragana_count > 0) {
            printf("平假名=%d ", result.charset.hiragana_count);
        }
        if (result.charset.katakana_count > 0) {
            printf("片假名=%d ", result.charset.katakana_count);
        }
        if (result.charset.kanji_count > 0) {
            printf("汉字=%d ", result.charset.kanji_count);
        }
        if (result.charset.latin_count > 0) {
            printf("拉丁=%d ", result.charset.latin_count);
        }
        printf("总计=%d\n", result.charset.total_chars);
    }
    
    bool passed = (result.language == test->expected);
    if (passed) {
        printf("  ✅ 通过\n");
        stats->passed++;
    } else {
        printf("  ❌ 失败\n");
        stats->failed++;
    }
    printf("\n");
    
    return passed;
}

/**
 * 主测试函数
 */
int main(void) {
    printf("═══════════════════════════════════════════════════════════\n");
    printf("   Misaki 语言检测模块测试\n");
    printf("═══════════════════════════════════════════════════════════\n\n");
    
    // 创建检测器
    LangDetectorConfig config = {
        .enable_ngram = true,
        .enable_tokenization = false,
        .confidence_threshold = 0.5f
    };
    LangDetector *detector = misaki_lang_detector_create(&config);
    if (!detector) {
        printf("❌ 无法创建语言检测器\n");
        return 1;
    }
    
    // 测试用例集
    TestCase tests[] = {
        // 日文测试（含假名）
        {"こんにちは", LANG_JAPANESE, "纯平假名"},
        {"カタカナ", LANG_JAPANESE, "纯片假名"},
        {"今日はいい天気です", LANG_JAPANESE, "日文常规句子"},
        {"これは本です。", LANG_JAPANESE, "简单日文句子"},
        
        // 日文测试（纯汉字，关键测试）
        {"東京都渋谷区", LANG_JAPANESE, "日文地址（纯汉字）"},
        {"大阪府", LANG_JAPANESE, "行政区划（含'府'）"},
        {"北海道", LANG_JAPANESE, "行政区划（含'道'）"},
        {"京都市", LANG_JAPANESE, "地名（含'市'）"},
        
        // 中文测试
        {"你好世界", LANG_CHINESE, "简单中文问候"},
        {"这是一本书", LANG_CHINESE, "中文句子（含'是'）"},
        {"我在中国", LANG_CHINESE, "中文句子（含'在'）"},
        {"学习了很多知识", LANG_CHINESE, "中文句子（含'了'）"},
        {"今天的天气很好", LANG_CHINESE, "中文句子（含'的'）"},
        
        // 纯汉字测试（中日区分）
        {"中国北京市", LANG_CHINESE, "中国地名（'市'在末尾）"},
        {"学习", LANG_CHINESE, "纯汉字词语（中文）"},
        
        // 英文测试
        {"Hello World", LANG_ENGLISH, "英文问候"},
        {"This is a book", LANG_ENGLISH, "英文句子"},
        {"The quick brown fox", LANG_ENGLISH, "英文常用句"},
        
        // 特殊测试
        {"", LANG_UNKNOWN, "空字符串"},
        {"a", LANG_UNKNOWN, "单个字符"},
        {"123", LANG_UNKNOWN, "纯数字"},
    };
    
    int num_tests = sizeof(tests) / sizeof(tests[0]);
    TestStats stats = {0};
    
    // 运行所有测试
    for (int i = 0; i < num_tests; i++) {
        run_test(detector, &tests[i], &stats);
    }
    
    // 测试快速检测函数
    printf("═══════════════════════════════════════════════════════════\n");
    printf("   快速检测模式测试\n");
    printf("═══════════════════════════════════════════════════════════\n\n");
    
    const char *quick_tests[] = {
        "こんにちは",
        "東京都",
        "你好",
        "Hello"
    };
    
    for (int i = 0; i < 4; i++) {
        MisakiLanguage lang = misaki_lang_detect_quick(quick_tests[i]);
        printf("文本: \"%s\" → %s\n", quick_tests[i], misaki_language_name(lang));
    }
    printf("\n");
    
    // 销毁检测器
    misaki_lang_detector_free(detector);
    
    // 打印统计
    printf("═══════════════════════════════════════════════════════════\n");
    printf("   测试统计\n");
    printf("═══════════════════════════════════════════════════════════\n");
    printf("总计: %d\n", stats.total);
    printf("通过: %d (%.1f%%)\n", stats.passed, 
           stats.total > 0 ? (stats.passed * 100.0 / stats.total) : 0);
    printf("失败: %d (%.1f%%)\n", stats.failed,
           stats.total > 0 ? (stats.failed * 100.0 / stats.total) : 0);
    printf("═══════════════════════════════════════════════════════════\n\n");
    
    return (stats.failed == 0) ? 0 : 1;
}
