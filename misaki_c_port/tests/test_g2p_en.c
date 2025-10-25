/**
 * test_g2p_en.c
 * 
 * 测试英文 G2P（CMUdict）
 * 
 * License: MIT
 */

#include "misaki_g2p.h"
#include "misaki_dict.h"
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
 * 英文 G2P 测试
 * ========================================================================== */

void test_en_g2p_create() {
    // 加载 CMUdict
    const char *cmudict_path = "../extracted_data/en/cmudict.txt";
    EnDict *dict = misaki_en_dict_load(cmudict_path);
    
    if (!dict) {
        printf("  ⚠️  无法加载 CMUdict，跳过测试\n");
        return;
    }
    
    printf("  ✅ CMUdict 加载成功\n");
    
    misaki_en_dict_free(dict);
}

void test_en_word_to_phonemes() {
    const char *cmudict_path = "../extracted_data/en/cmudict.txt";
    EnDict *dict = misaki_en_dict_load(cmudict_path);
    
    if (!dict) {
        printf("  ⚠️  无法加载 CMUdict，跳过测试\n");
        return;
    }
    
    // 测试常见单词
    const char *test_words[] = {
        "hello",
        "world",
        "the",
        "and",
        "computer",
        NULL
    };
    
    for (int i = 0; test_words[i] != NULL; i++) {
        char *phonemes = misaki_en_g2p_word(dict, test_words[i], NULL);
        if (phonemes) {
            printf("  %s → %s\n", test_words[i], phonemes);
            TEST_ASSERT(strlen(phonemes) > 0, "音素不应为空");
            free(phonemes);
        } else {
            printf("  %s → (未找到)\n", test_words[i]);
        }
    }
    
    misaki_en_dict_free(dict);
    
    printf("  ✅ 英文单词→音素转换测试通过\n");
}

void test_en_g2p_full() {
    // 加载 CMUdict
    const char *cmudict_path = "../extracted_data/en/cmudict.txt";
    EnDict *dict = misaki_en_dict_load(cmudict_path);
    
    if (!dict) {
        printf("  ⚠️  无法加载 CMUdict，跳过测试\n");
        return;
    }
    
    // 测试完整 G2P 流程（英文不需要创建分词器）
    const char *test_texts[] = {
        "hello world",
        "the quick brown fox",
        "I love you",
        NULL
    };
    
    for (int i = 0; test_texts[i] != NULL; i++) {
        printf("  测试: %s\n", test_texts[i]);
        
        MisakiTokenList *tokens = misaki_en_g2p(dict, test_texts[i], NULL);
        TEST_ASSERT(tokens != NULL, "G2P 结果不应为 NULL");
        
        printf("    分词结果:\n");
        for (int j = 0; j < tokens->count; j++) {
            MisakiToken *token = &tokens->tokens[j];
            if (token->phonemes) {
                printf("      %s → %s\n", token->text, token->phonemes);
            } else {
                printf("      %s → (无音素)\n", token->text);
            }
        }
        
        // 合并音素
        char *merged = misaki_merge_phonemes(tokens, " ");
        if (merged) {
            printf("    完整音素: %s\n", merged);
            free(merged);
        }
        
        misaki_token_list_free(tokens);
    }
    
    misaki_en_dict_free(dict);
    
    printf("  ✅ 英文完整 G2P 测试通过\n");
}

void test_en_g2p_unknown_words() {
    const char *cmudict_path = "../extracted_data/en/cmudict.txt";
    EnDict *dict = misaki_en_dict_load(cmudict_path);
    
    if (!dict) {
        printf("  ⚠️  无法加载 CMUdict，跳过测试\n");
        return;
    }
    
    // 测试包含未知单词的文本
    const char *text = "hello xyzabc world";
    MisakiTokenList *tokens = misaki_en_g2p(dict, text, NULL);
    TEST_ASSERT(tokens != NULL, "G2P 结果不应为 NULL");
    
    printf("  分词结果:\n");
    for (int i = 0; i < tokens->count; i++) {
        MisakiToken *token = &tokens->tokens[i];
        printf("    %s → %s\n", token->text, token->phonemes ? token->phonemes : "(未知)");
    }
    
    misaki_token_list_free(tokens);
    misaki_en_dict_free(dict);
    
    printf("  ✅ 英文未知单词测试通过\n");
}

/* ============================================================================
 * 主测试函数
 * ========================================================================== */

int main(void) {
    printf("════════════════════════════════════════════════════════════\n");
    printf("  英文 G2P 测试\n");
    printf("════════════════════════════════════════════════════════════\n");
    
    RUN_TEST(test_en_g2p_create);
    RUN_TEST(test_en_word_to_phonemes);
    RUN_TEST(test_en_g2p_full);
    RUN_TEST(test_en_g2p_unknown_words);
    
    // 总结
    printf("\n════════════════════════════════════════════════════════════\n");
    printf("  测试结果:\n");
    printf("  ✅ 通过: %d\n", test_passed);
    printf("  ❌ 失败: %d\n", test_failed);
    printf("════════════════════════════════════════════════════════════\n");
    
    return test_failed > 0 ? 1 : 0;
}
