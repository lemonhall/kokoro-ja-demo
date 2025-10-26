/**
 * test_g2p.c
 * 
 * 测试 G2P (Grapheme-to-Phoneme) 转换
 * 
 * License: MIT
 */

#include "misaki_g2p.h"
#include "misaki_dict.h"
#include "misaki_tokenizer.h"
#include "misaki_trie.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// 测试结果统计
static int test_passed = 0;
static int test_failed = 0;

#define TEST_ASSERT(cond, msg) do { \
    if (!(cond)) { \
        printf("  ❌ FAIL: %s\n", msg); \
        test_failed++; \
        return; \
    } \
} while(0)

#define RUN_TEST(test_func) do { \
    printf("\n🧪 Running %s...\n", #test_func); \
    test_func(); \
    test_passed++; \
} while(0)

/* ============================================================================
 * 英文 G2P 测试
 * ========================================================================== */

void test_en_g2p_word(void) {
    // 加载英文词典
    const char *dict_path = "../extracted_data/en/us_dict.txt";
    EnDict *dict = misaki_en_dict_load(dict_path);
    
    if (!dict) {
        printf("  ⚠️  无法加载英文词典，跳过测试\n");
        return;
    }
    
    // 测试单词查询
    char *phonemes = misaki_en_g2p_word(dict, "hello", NULL);
    TEST_ASSERT(phonemes != NULL, "hello 应该有音素");
    printf("  hello → %s\n", phonemes);
    free(phonemes);
    
    phonemes = misaki_en_g2p_word(dict, "world", NULL);
    TEST_ASSERT(phonemes != NULL, "world 应该有音素");
    printf("  world → %s\n", phonemes);
    free(phonemes);
    
    misaki_en_dict_free(dict);
    printf("  ✅ 英文单词 G2P 成功\n");
}

void test_en_g2p_sentence(void) {
    // 加载英文词典
    const char *dict_path = "../extracted_data/en/us_dict.txt";
    EnDict *dict = misaki_en_dict_load(dict_path);
    
    if (!dict) {
        printf("  ⚠️  无法加载英文词典，跳过测试\n");
        return;
    }
    
    const char *text = "Hello world";
    MisakiTokenList *tokens = misaki_en_g2p(dict, text, NULL);
    TEST_ASSERT(tokens != NULL, "G2P 结果不应为 NULL");
    
    int count = misaki_token_list_size(tokens);
    TEST_ASSERT(count == 2, "应该有 2 个 token");
    
    // 打印结果
    misaki_g2p_print(tokens, false);
    
    misaki_token_list_free(tokens);
    misaki_en_dict_free(dict);
    printf("  ✅ 英文句子 G2P 成功\n");
}

/* ============================================================================
 * 中文 G2P 测试
 * ========================================================================== */

void test_zh_g2p(void) {
    // 加载中文词典
    const char *dict_path = "../extracted_data/zh/pinyin_dict.txt";
    ZhDict *dict = misaki_zh_dict_load(dict_path);
    
    if (!dict) {
        printf("  ⚠️  无法加载中文词典，跳过测试\n");
        return;
    }
    
    // 创建中文分词器
    const char *word_dict_path = "../extracted_data/zh/dict.txt";
    Trie *trie = misaki_trie_create();
    int word_count = misaki_trie_load_from_file(trie, word_dict_path, "word freq");
    
    if (word_count < 0) {
        printf("  ⚠️  无法加载中文词汇词典，跳过测试\n");
        misaki_zh_dict_free(dict);
        misaki_trie_free(trie);
        return;
    }
    
    ZhTokenizerConfig config = {
        .dict_trie = trie,
        .enable_hmm = false,
        .enable_userdict = false,
        .user_trie = NULL
    };
    
    void *tokenizer = misaki_zh_tokenizer_create(&config);
    TEST_ASSERT(tokenizer != NULL, "中文分词器应该创建成功");
    
    // 测试中文 G2P
    const char *text = "你好世界";
    MisakiTokenList *tokens = misaki_zh_g2p(dict, NULL, tokenizer, text, NULL);
    TEST_ASSERT(tokens != NULL, "G2P 结果不应为 NULL");
    
    printf("  原文: %s\n", text);
    misaki_g2p_print(tokens, false);
    
    // 合并音素
    char *merged = misaki_merge_phonemes(tokens, " ");
    if (merged) {
        printf("  合并音素: %s\n", merged);
        free(merged);
    }
    
    misaki_token_list_free(tokens);
    misaki_zh_tokenizer_free(tokenizer);
    misaki_trie_free(trie);
    misaki_zh_dict_free(dict);
    printf("  ✅ 中文 G2P 成功\n");
}

/* ============================================================================
 * 日文 G2P 测试
 * ========================================================================== */

void test_ja_g2p(void) {
    // 创建简单的日文词典
    Trie *trie = misaki_trie_create();
    misaki_trie_insert(trie, "こんにちは", 1.0, NULL);
    misaki_trie_insert(trie, "世界", 1.0, NULL);
    misaki_trie_insert(trie, "です", 1.0, NULL);
    
    JaTokenizerConfig config = {
        .dict_trie = trie,
        .use_simple_model = true,
        .unidic_path = NULL
    };
    
    void *tokenizer = misaki_ja_tokenizer_create(&config);
    TEST_ASSERT(tokenizer != NULL, "日文分词器应该创建成功");
    
    // 测试日文 G2P
    const char *text = "こんにちは世界";
    MisakiTokenList *tokens = misaki_ja_g2p(tokenizer, text, NULL);
    TEST_ASSERT(tokens != NULL, "G2P 结果不应为 NULL");
    
    printf("  原文: %s\n", text);
    misaki_g2p_print(tokens, false);
    
    misaki_token_list_free(tokens);
    misaki_ja_tokenizer_free(tokenizer);
    misaki_trie_free(trie);
    printf("  ✅ 日文 G2P 成功\n");
}

/* ============================================================================
 * 工具函数测试
 * ========================================================================== */

void test_merge_phonemes(void) {
    MisakiTokenList *list = misaki_token_list_create();
    
    MisakiToken token1 = {
        .text = misaki_strdup("hello"),
        .phonemes = misaki_strdup("həˈloʊ"),
        .tag = NULL,
        .start = 0,
        .length = 5,
        .score = 0.0
    };
    
    MisakiToken token2 = {
        .text = misaki_strdup("world"),
        .phonemes = misaki_strdup("wɜrld"),
        .tag = NULL,
        .start = 6,
        .length = 5,
        .score = 0.0
    };
    
    misaki_token_list_add(list, &token1);
    misaki_token_list_add(list, &token2);
    
    char *merged = misaki_merge_phonemes(list, " ");
    TEST_ASSERT(merged != NULL, "合并音素不应为 NULL");
    
    printf("  合并结果: %s\n", merged);
    TEST_ASSERT(strcmp(merged, "həˈloʊ wɜrld") == 0, "合并结果应该正确");
    
    free(merged);
    free(token1.text);
    free(token1.phonemes);
    free(token2.text);
    free(token2.phonemes);
    misaki_token_list_free(list);
    printf("  ✅ 音素合并成功\n");
}

void test_g2p_stats(void) {
    MisakiTokenList *list = misaki_token_list_create();
    
    MisakiToken token1 = {
        .text = misaki_strdup("hello"),
        .phonemes = misaki_strdup("h e l o"),
        .tag = NULL,
        .start = 0,
        .length = 5,
        .score = 0.0
    };
    
    MisakiToken token2 = {
        .text = misaki_strdup("world"),
        .phonemes = NULL,  // OOV
        .tag = NULL,
        .start = 6,
        .length = 5,
        .score = 0.0
    };
    
    misaki_token_list_add(list, &token1);
    misaki_token_list_add(list, &token2);
    
    int total_phonemes = 0;
    int oov_count = 0;
    double avg_phonemes = 0.0;
    
    misaki_g2p_stats(list, &total_phonemes, &oov_count, &avg_phonemes);
    
    printf("  总音素: %d, OOV: %d, 平均: %.2f\n", 
           total_phonemes, oov_count, avg_phonemes);
    
    TEST_ASSERT(total_phonemes == 4, "总音素应该是 4");
    TEST_ASSERT(oov_count == 1, "OOV 应该是 1");
    
    free(token1.text);
    free(token1.phonemes);
    free(token2.text);
    misaki_token_list_free(list);
    printf("  ✅ G2P 统计成功\n");
}

/* ============================================================================
 * 主测试函数
 * ========================================================================== */

int main(void) {
    printf("════════════════════════════════════════════════════════════\n");
    printf("  Misaki G2P Tests\n");
    printf("════════════════════════════════════════════════════════════\n");
    
    // 英文 G2P 测试
    RUN_TEST(test_en_g2p_word);
    RUN_TEST(test_en_g2p_sentence);
    
    // 中文 G2P 测试
    RUN_TEST(test_zh_g2p);
    
    // 日文 G2P 测试
    RUN_TEST(test_ja_g2p);
    
    // 工具函数测试
    RUN_TEST(test_merge_phonemes);
    RUN_TEST(test_g2p_stats);
    
    // 总结
    printf("\n════════════════════════════════════════════════════════════\n");
    printf("  测试结果:\n");
    printf("  ✅ 通过: %d\n", test_passed);
    printf("  ❌ 失败: %d\n", test_failed);
    printf("════════════════════════════════════════════════════════════\n");
    
    return test_failed > 0 ? 1 : 0;
}
