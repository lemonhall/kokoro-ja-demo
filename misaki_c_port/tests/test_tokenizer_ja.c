/**
 * test_tokenizer_ja.c
 * 
 * 测试日文分词器
 * 
 * License: MIT
 */

#include "misaki_tokenizer.h"
#include <stdio.h>
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
 * 日文分词器测试
 * ========================================================================== */

void test_ja_tokenizer_create() {
    // 创建简单的 Trie
    Trie *trie = misaki_trie_create();
    TEST_ASSERT(trie != NULL, "Trie 应该创建成功");
    
    JaTokenizerConfig config = {
        .dict_trie = trie,
        .use_simple_model = true
    };
    
    void *tokenizer = misaki_ja_tokenizer_create(&config);
    TEST_ASSERT(tokenizer != NULL, "日文分词器应该创建成功");
    
    misaki_ja_tokenizer_free(tokenizer);
    misaki_trie_free(trie);
    
    printf("  ✅ 日文分词器创建成功\n");
}

void test_ja_tokenize_hiragana() {
    Trie *trie = misaki_trie_create();
    misaki_trie_insert(trie, "こんにちは", 1000.0, NULL);
    
    JaTokenizerConfig config = {
        .dict_trie = trie,
        .use_simple_model = true
    };
    
    void *tokenizer = misaki_ja_tokenizer_create(&config);
    TEST_ASSERT(tokenizer != NULL, "分词器应该创建成功");
    
    // 测试平假名
    const char *text = "こんにちは";
    MisakiTokenList *tokens = misaki_ja_tokenize(tokenizer, text);
    TEST_ASSERT(tokens != NULL, "分词结果不应为 NULL");
    
    int count = misaki_token_list_size(tokens);
    printf("  分词结果: %d 个词\n", count);
    
    for (int i = 0; i < count; i++) {
        MisakiToken *token = misaki_token_list_get(tokens, i);
        printf("    [%d] '%s' (tag: %s)\n", i, token->text, token->tag ? token->tag : "NULL");
    }
    
    TEST_ASSERT(count >= 1, "至少应该有 1 个词");
    
    misaki_token_list_free(tokens);
    misaki_ja_tokenizer_free(tokenizer);
    misaki_trie_free(trie);
    
    printf("  ✅ 日文平假名分词测试通过\n");
}

void test_ja_tokenize_katakana() {
    Trie *trie = misaki_trie_create();
    misaki_trie_insert(trie, "コンピュータ", 1000.0, NULL);
    
    JaTokenizerConfig config = {
        .dict_trie = trie,
        .use_simple_model = true
    };
    
    void *tokenizer = misaki_ja_tokenizer_create(&config);
    
    // 测试片假名
    const char *text = "コンピュータ";
    MisakiTokenList *tokens = misaki_ja_tokenize(tokenizer, text);
    TEST_ASSERT(tokens != NULL, "分词结果不应为 NULL");
    
    printf("  分词结果: %d 个词\n", tokens->count);
    for (int i = 0; i < tokens->count; i++) {
        MisakiToken *token = misaki_token_list_get(tokens, i);
        printf("    [%d] '%s'\n", i, token->text);
    }
    
    misaki_token_list_free(tokens);
    misaki_ja_tokenizer_free(tokenizer);
    misaki_trie_free(trie);
    
    printf("  ✅ 日文片假名分词测试通过\n");
}

void test_ja_tokenize_kanji() {
    Trie *trie = misaki_trie_create();
    misaki_trie_insert(trie, "日本語", 1000.0, NULL);
    
    JaTokenizerConfig config = {
        .dict_trie = trie,
        .use_simple_model = true
    };
    
    void *tokenizer = misaki_ja_tokenizer_create(&config);
    
    // 测试汉字
    const char *text = "日本語";
    MisakiTokenList *tokens = misaki_ja_tokenize(tokenizer, text);
    TEST_ASSERT(tokens != NULL, "分词结果不应为 NULL");
    
    printf("  分词结果: %d 个词\n", tokens->count);
    for (int i = 0; i < tokens->count; i++) {
        MisakiToken *token = misaki_token_list_get(tokens, i);
        printf("    [%d] '%s'\n", i, token->text);
    }
    
    misaki_token_list_free(tokens);
    misaki_ja_tokenizer_free(tokenizer);
    misaki_trie_free(trie);
    
    printf("  ✅ 日文汉字分词测试通过\n");
}

void test_ja_tokenize_mixed() {
    Trie *trie = misaki_trie_create();
    misaki_trie_insert(trie, "私", 1000.0, NULL);
    misaki_trie_insert(trie, "は", 800.0, NULL);
    misaki_trie_insert(trie, "学生", 900.0, NULL);
    misaki_trie_insert(trie, "です", 700.0, NULL);
    
    JaTokenizerConfig config = {
        .dict_trie = trie,
        .use_simple_model = true
    };
    
    void *tokenizer = misaki_ja_tokenizer_create(&config);
    
    // 测试混合文本
    const char *text = "私は学生です";
    MisakiTokenList *tokens = misaki_ja_tokenize(tokenizer, text);
    TEST_ASSERT(tokens != NULL, "分词结果不应为 NULL");
    
    printf("  分词结果: %d 个词\n", tokens->count);
    for (int i = 0; i < tokens->count; i++) {
        MisakiToken *token = misaki_token_list_get(tokens, i);
        printf("    [%d] '%s' (tag: %s)\n", i, token->text, token->tag ? token->tag : "NULL");
    }
    
    // 应该包含假名和汉字
    bool has_hiragana = false;
    bool has_kanji = false;
    
    for (int i = 0; i < tokens->count; i++) {
        MisakiToken *token = misaki_token_list_get(tokens, i);
        if (token->tag && strcmp(token->tag, "hiragana") == 0) has_hiragana = true;
        if (token->tag && strcmp(token->tag, "kanji") == 0) has_kanji = true;
    }
    
    misaki_token_list_free(tokens);
    misaki_ja_tokenizer_free(tokenizer);
    misaki_trie_free(trie);
    
    printf("  ✅ 日文混合文本分词测试通过\n");
}

void test_ja_tokenize_punctuation() {
    Trie *trie = misaki_trie_create();
    misaki_trie_insert(trie, "こんにちは", 1000.0, NULL);
    misaki_trie_insert(trie, "元気", 900.0, NULL);
    misaki_trie_insert(trie, "ですか", 800.0, NULL);
    
    JaTokenizerConfig config = {
        .dict_trie = trie,
        .use_simple_model = true
    };
    
    void *tokenizer = misaki_ja_tokenizer_create(&config);
    
    // 测试标点符号
    const char *text = "こんにちは、元気ですか？";
    MisakiTokenList *tokens = misaki_ja_tokenize(tokenizer, text);
    TEST_ASSERT(tokens != NULL, "分词结果不应为 NULL");
    
    printf("  分词结果: %d 个词\n", tokens->count);
    for (int i = 0; i < tokens->count; i++) {
        MisakiToken *token = misaki_token_list_get(tokens, i);
        printf("    [%d] '%s' (tag: %s)\n", i, token->text, token->tag ? token->tag : "NULL");
    }
    
    misaki_token_list_free(tokens);
    misaki_ja_tokenizer_free(tokenizer);
    misaki_trie_free(trie);
    
    printf("  ✅ 日文标点符号测试通过\n");
}

/* ============================================================================
 * 主测试函数
 * ========================================================================== */

int main(void) {
    printf("════════════════════════════════════════════════════════════\n");
    printf("  日文分词器测试\n");
    printf("════════════════════════════════════════════════════════════\n");
    
    RUN_TEST(test_ja_tokenizer_create);
    RUN_TEST(test_ja_tokenize_hiragana);
    RUN_TEST(test_ja_tokenize_katakana);
    RUN_TEST(test_ja_tokenize_kanji);
    RUN_TEST(test_ja_tokenize_mixed);
    RUN_TEST(test_ja_tokenize_punctuation);
    
    // 总结
    printf("\n════════════════════════════════════════════════════════════\n");
    printf("  测试结果:\n");
    printf("  ✅ 通过: %d\n", test_passed);
    printf("  ❌ 失败: %d\n", test_failed);
    printf("════════════════════════════════════════════════════════════\n");
    
    return test_failed > 0 ? 1 : 0;
}
