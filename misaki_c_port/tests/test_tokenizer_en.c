/**
 * test_tokenizer_en.c
 * 
 * 测试英文分词器
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
 * 英文分词器测试
 * ========================================================================== */

void test_en_tokenizer_simple() {
    // 英文分词器不需要 create，直接调用函数
    const char *text = "Hello world";
    MisakiTokenList *tokens = misaki_en_tokenize(text);
    TEST_ASSERT(tokens != NULL, "分词结果不应为 NULL");
    
    misaki_token_list_free(tokens);
    
    printf("  ✅ 英文分词器调用成功\n");
}

void test_en_tokenize_simple() {
    // 测试简单句子
    const char *text = "Hello world";
    MisakiTokenList *tokens = misaki_en_tokenize(text);
    TEST_ASSERT(tokens != NULL, "分词结果不应为 NULL");
    
    int count = misaki_token_list_size(tokens);
    printf("  分词结果: %d 个词\n", count);
    
    for (int i = 0; i < count; i++) {
        MisakiToken *token = misaki_token_list_get(tokens, i);
        printf("    [%d] '%s' (tag: %s)\n", i, token->text, token->tag ? token->tag : "NULL");
    }
    
    TEST_ASSERT(count >= 2, "至少应该分成 2 个词");
    
    misaki_token_list_free(tokens);
    
    printf("  ✅ 英文简单分词测试通过\n");
}

void test_en_tokenize_punctuation() {
    // 测试标点符号处理
    const char *text = "Hello, world! How are you?";
    MisakiTokenList *tokens = misaki_en_tokenize(text);
    TEST_ASSERT(tokens != NULL, "分词结果不应为 NULL");
    
    printf("  分词结果: %d 个词\n", tokens->count);
    for (int i = 0; i < tokens->count; i++) {
        MisakiToken *token = misaki_token_list_get(tokens, i);
        printf("    [%d] '%s'\n", i, token->text);
    }
    
    TEST_ASSERT(tokens->count > 0, "应该有分词结果");
    
    misaki_token_list_free(tokens);
    
    printf("  ✅ 英文标点符号测试通过\n");
}

void test_en_tokenize_numbers() {
    // 测试数字处理
    const char *text = "I have 3 apples and 2.5 oranges.";
    MisakiTokenList *tokens = misaki_en_tokenize(text);
    TEST_ASSERT(tokens != NULL, "分词结果不应为 NULL");
    
    printf("  分词结果: %d 个词\n", tokens->count);
    for (int i = 0; i < tokens->count; i++) {
        MisakiToken *token = misaki_token_list_get(tokens, i);
        printf("    [%d] '%s'\n", i, token->text);
    }
    
    TEST_ASSERT(tokens->count > 0, "应该有分词结果");
    
    misaki_token_list_free(tokens);
    
    printf("  ✅ 英文数字处理测试通过\n");
}

void test_en_tokenize_contractions() {
    // 测试缩写词处理
    const char *text = "I'm don't can't won't";
    MisakiTokenList *tokens = misaki_en_tokenize(text);
    TEST_ASSERT(tokens != NULL, "分词结果不应为 NULL");
    
    printf("  分词结果: %d 个词\n", tokens->count);
    for (int i = 0; i < tokens->count; i++) {
        MisakiToken *token = misaki_token_list_get(tokens, i);
        printf("    [%d] '%s'\n", i, token->text);
    }
    
    misaki_token_list_free(tokens);
    
    printf("  ✅ 英文缩写词测试通过\n");
}

/* ============================================================================
 * 主测试函数
 * ========================================================================== */

int main(void) {
    printf("════════════════════════════════════════════════════════════\n");
    printf("  英文分词器测试\n");
    printf("════════════════════════════════════════════════════════════\n");
    
    RUN_TEST(test_en_tokenizer_simple);
    RUN_TEST(test_en_tokenize_simple);
    RUN_TEST(test_en_tokenize_punctuation);
    RUN_TEST(test_en_tokenize_numbers);
    RUN_TEST(test_en_tokenize_contractions);
    
    // 总结
    printf("\n════════════════════════════════════════════════════════════\n");
    printf("  测试结果:\n");
    printf("  ✅ 通过: %d\n", test_passed);
    printf("  ❌ 失败: %d\n", test_failed);
    printf("════════════════════════════════════════════════════════════\n");
    
    return test_failed > 0 ? 1 : 0;
}
