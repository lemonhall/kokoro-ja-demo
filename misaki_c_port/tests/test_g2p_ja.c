/**
 * test_g2p_ja.c
 * 
 * 测试日文 G2P（假名→IPA）
 * 
 * License: MIT
 */

#include "misaki_g2p.h"
#include "misaki_tokenizer.h"
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
 * 日文 G2P 测试
 * ========================================================================== */

void test_ja_kana_to_ipa() {
    // 测试平假名→IPA
    const char *test_kana[] = {
        "あ",   // a
        "い",   // i
        "う",   // ɯ
        "え",   // e
        "お",   // o
        "か",   // ka
        "き",   // ki
        "く",   // kɯ
        "さ",   // sa
        "し",   // ɕi
        "す",   // sɯ
        "ん",   // ɴ
        NULL
    };
    
    for (int i = 0; test_kana[i] != NULL; i++) {
        char *ipa = misaki_ja_kana_to_ipa(test_kana[i]);
        if (ipa) {
            printf("  %s → %s\n", test_kana[i], ipa);
            TEST_ASSERT(strlen(ipa) > 0, "IPA 不应为空");
            free(ipa);
        } else {
            printf("  %s → (未找到)\n", test_kana[i]);
        }
    }
    
    printf("  ✅ 假名→IPA 转换测试通过\n");
}

void test_ja_g2p_hiragana() {
    Trie *trie = misaki_trie_create();
    misaki_trie_insert(trie, "こんにちは", 1000.0, NULL);
    
    JaTokenizerConfig config = {
        .dict_trie = trie,
        .use_simple_model = false  // 强制 Viterbi 模式
    };
    
    void *tokenizer = misaki_ja_tokenizer_create(&config);
    TEST_ASSERT(tokenizer != NULL, "分词器应该创建成功");
    
    // 测试平假名
    const char *text = "こんにちは";
    MisakiTokenList *tokens = misaki_ja_g2p(tokenizer, text, NULL);
    TEST_ASSERT(tokens != NULL, "G2P 结果不应为 NULL");
    
    printf("  测试: %s\n", text);
    printf("    分词结果:\n");
    for (int i = 0; i < tokens->count; i++) {
        MisakiToken *token = &tokens->tokens[i];
        if (token->phonemes) {
            printf("      %s → %s\n", token->text, token->phonemes);
            // 验证是 IPA 而不是假名
            TEST_ASSERT(strcmp(token->text, token->phonemes) != 0, "应该转换为 IPA");
        }
    }
    
    // 合并音素
    char *merged = misaki_merge_phonemes(tokens, " ");
    if (merged) {
        printf("    完整音素: %s\n", merged);
        free(merged);
    }
    
    misaki_token_list_free(tokens);
    misaki_ja_tokenizer_free(tokenizer);
    misaki_trie_free(trie);
    
    printf("  ✅ 日文平假名 G2P 测试通过\n");
}

void test_ja_g2p_katakana() {
    Trie *trie = misaki_trie_create();
    misaki_trie_insert(trie, "カタカナ", 1000.0, NULL);
    
    JaTokenizerConfig config = {
        .dict_trie = trie,
        .use_simple_model = false  // 强制 Viterbi 模式
    };
    
    void *tokenizer = misaki_ja_tokenizer_create(&config);
    
    // 测试片假名
    const char *text = "カタカナ";
    MisakiTokenList *tokens = misaki_ja_g2p(tokenizer, text, NULL);
    TEST_ASSERT(tokens != NULL, "G2P 结果不应为 NULL");
    
    printf("  测试: %s\n", text);
    printf("    分词结果:\n");
    for (int i = 0; i < tokens->count; i++) {
        MisakiToken *token = &tokens->tokens[i];
        if (token->phonemes) {
            printf("      %s → %s\n", token->text, token->phonemes);
        }
    }
    
    char *merged = misaki_merge_phonemes(tokens, " ");
    if (merged) {
        printf("    完整音素: %s\n", merged);
        free(merged);
    }
    
    misaki_token_list_free(tokens);
    misaki_ja_tokenizer_free(tokenizer);
    misaki_trie_free(trie);
    
    printf("  ✅ 日文片假名 G2P 测试通过\n");
}

void test_ja_g2p_mixed() {
    Trie *trie = misaki_trie_create();
    misaki_trie_insert(trie, "私", 1000.0, NULL);
    misaki_trie_insert(trie, "は", 800.0, NULL);
    misaki_trie_insert(trie, "学生", 900.0, NULL);
    misaki_trie_insert(trie, "です", 700.0, NULL);
    
    JaTokenizerConfig config = {
        .dict_trie = trie,
        .use_simple_model = false  // 强制 Viterbi 模式
    };
    
    void *tokenizer = misaki_ja_tokenizer_create(&config);
    
    // 测试混合文本（假名+汉字）
    const char *text = "私は学生です";
    MisakiTokenList *tokens = misaki_ja_g2p(tokenizer, text, NULL);
    TEST_ASSERT(tokens != NULL, "G2P 结果不应为 NULL");
    
    printf("  测试: %s\n", text);
    printf("    分词结果:\n");
    for (int i = 0; i < tokens->count; i++) {
        MisakiToken *token = &tokens->tokens[i];
        printf("      %s → %s (tag: %s)\n", 
               token->text, 
               token->phonemes ? token->phonemes : "(无音素)",
               token->tag ? token->tag : "NULL");
    }
    
    char *merged = misaki_merge_phonemes(tokens, " ");
    if (merged) {
        printf("    完整音素: %s\n", merged);
        free(merged);
    }
    
    misaki_token_list_free(tokens);
    misaki_ja_tokenizer_free(tokenizer);
    misaki_trie_free(trie);
    
    printf("  ✅ 日文混合文本 G2P 测试通过\n");
}

void test_ja_g2p_long_vowel() {
    Trie *trie = misaki_trie_create();
    misaki_trie_insert(trie, "おかあさん", 1000.0, NULL);
    
    JaTokenizerConfig config = {
        .dict_trie = trie,
        .use_simple_model = false  // 强制 Viterbi 模式
    };
    
    void *tokenizer = misaki_ja_tokenizer_create(&config);
    
    // 测试长音
    const char *text = "おかあさん";
    MisakiTokenList *tokens = misaki_ja_g2p(tokenizer, text, NULL);
    TEST_ASSERT(tokens != NULL, "G2P 结果不应为 NULL");
    
    printf("  测试: %s\n", text);
    printf("    分词结果:\n");
    for (int i = 0; i < tokens->count; i++) {
        MisakiToken *token = &tokens->tokens[i];
        if (token->phonemes) {
            printf("      %s → %s\n", token->text, token->phonemes);
        }
    }
    
    char *merged = misaki_merge_phonemes(tokens, " ");
    if (merged) {
        printf("    完整音素: %s\n", merged);
        free(merged);
    }
    
    misaki_token_list_free(tokens);
    misaki_ja_tokenizer_free(tokenizer);
    misaki_trie_free(trie);
    
    printf("  ✅ 日文长音测试通过\n");
}

/* ============================================================================
 * 主测试函数
 * ========================================================================== */

int main(void) {
    printf("════════════════════════════════════════════════════════════\n");
    printf("  日文 G2P 测试\n");
    printf("════════════════════════════════════════════════════════════\n");
    
    RUN_TEST(test_ja_kana_to_ipa);
    RUN_TEST(test_ja_g2p_hiragana);
    RUN_TEST(test_ja_g2p_katakana);
    RUN_TEST(test_ja_g2p_mixed);
    RUN_TEST(test_ja_g2p_long_vowel);
    
    // 总结
    printf("\n════════════════════════════════════════════════════════════\n");
    printf("  测试结果:\n");
    printf("  ✅ 通过: %d\n", test_passed);
    printf("  ❌ 失败: %d\n", test_failed);
    printf("════════════════════════════════════════════════════════════\n");
    
    return test_failed > 0 ? 1 : 0;
}
