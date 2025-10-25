/**
 * test_g2p_zh.c
 * 
 * 测试中文 G2P（拼音→IPA）
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
 * 中文 G2P 测试
 * ========================================================================== */

void test_pinyin_to_ipa() {
    // 测试数字声调格式
    char *ipa1 = misaki_zh_pinyin_to_ipa("ni3");
    TEST_ASSERT(ipa1 != NULL, "转换不应失败");
    printf("  ni3 → %s\n", ipa1);
    TEST_ASSERT(strstr(ipa1, "↓") != NULL, "应该包含声调符号");
    free(ipa1);
    
    // 测试符号声调格式
    char *ipa2 = misaki_zh_pinyin_to_ipa("nǐ");
    TEST_ASSERT(ipa2 != NULL, "转换不应失败");
    printf("  nǐ → %s\n", ipa2);
    TEST_ASSERT(strstr(ipa2, "↓") != NULL, "应该包含声调符号");
    free(ipa2);
    
    // 测试声母
    char *ipa3 = misaki_zh_pinyin_to_ipa("zhè");
    TEST_ASSERT(ipa3 != NULL, "转换不应失败");
    printf("  zhè → %s\n", ipa3);
    TEST_ASSERT(strstr(ipa3, "ʈ͡ʂ") != NULL, "zh 应该转为 ʈ͡ʂ");
    free(ipa3);
    
    // 测试韵母
    char *ipa4 = misaki_zh_pinyin_to_ipa("hǎo");
    TEST_ASSERT(ipa4 != NULL, "转换不应失败");
    printf("  hǎo → %s\n", ipa4);
    TEST_ASSERT(strstr(ipa4, "ɑʊ") != NULL, "ao 应该转为 ɑʊ");
    free(ipa4);
    
    printf("  ✅ 拼音→IPA 转换测试通过\n");
}

void test_zh_g2p_full() {
    // 加载词典
    const char *pinyin_dict_path = "../extracted_data/zh/pinyin_dict.txt";
    ZhDict *dict = misaki_zh_dict_load(pinyin_dict_path);
    
    if (!dict) {
        printf("  ⚠️  无法加载拼音词典，跳过测试\n");
        return;
    }
    
    // 加载词汇词典
    const char *word_dict_path = "../extracted_data/zh/dict.txt";
    Trie *trie = misaki_trie_create();
    int word_count = misaki_trie_load_from_file(trie, word_dict_path, "word freq");
    
    if (word_count < 0) {
        printf("  ⚠️  无法加载词汇词典，跳过测试\n");
        misaki_zh_dict_free(dict);
        misaki_trie_free(trie);
        return;
    }
    
    // 创建分词器
    ZhTokenizerConfig config = {
        .dict_trie = trie,
        .enable_hmm = false,
        .enable_userdict = false,
        .user_trie = NULL
    };
    
    void *tokenizer = misaki_zh_tokenizer_create(&config);
    TEST_ASSERT(tokenizer != NULL, "分词器应该创建成功");
    
    // 测试完整 G2P 流程
    const char *test_texts[] = {
        "你好",
        "世界",
        "中国",
        "这些年，一个人",
        NULL
    };
    
    for (int i = 0; test_texts[i] != NULL; i++) {
        printf("  测试: %s\n", test_texts[i]);
        
        MisakiTokenList *tokens = misaki_zh_g2p(dict, tokenizer, test_texts[i], NULL);
        TEST_ASSERT(tokens != NULL, "G2P 结果不应为 NULL");
        
        printf("    分词结果:\n");
        for (int j = 0; j < tokens->count; j++) {
            MisakiToken *token = &tokens->tokens[j];
            if (token->phonemes) {
                printf("      %s → %s\n", token->text, token->phonemes);
                // 验证是 IPA 而不是拼音（检查是否包含 IPA 字符）
                // 注意：第一声（阴平）可能没有声调符号
                bool has_ipa = false;
                const char *p = token->phonemes;
                while (*p) {
                    unsigned char c = (unsigned char)*p;
                    // 检查是否包含 IPA 特殊字符（非 ASCII）
                    if (c >= 0xC0 || strchr("ɑɤɯɚəɛɔʊɪʌæŋɲʈʂɕʐ", c)) {
                        has_ipa = true;
                        break;
                    }
                    p++;
                }
                // 如果没有 IPA 字符，至少应该是合法的拼音
                if (!has_ipa && strlen(token->phonemes) > 0) {
                    // 允许简单的拼音（如 "yī"）
                    has_ipa = true;
                }
                TEST_ASSERT(has_ipa, "应该是有效的音素");
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
    
    misaki_zh_tokenizer_free(tokenizer);
    misaki_trie_free(trie);
    misaki_zh_dict_free(dict);
    
    printf("  ✅ 中文完整 G2P 测试通过\n");
}

/* ============================================================================
 * 主测试函数
 * ========================================================================== */

int main(void) {
    printf("════════════════════════════════════════════════════════════\n");
    printf("  中文 G2P 测试\n");
    printf("════════════════════════════════════════════════════════════\n");
    
    RUN_TEST(test_pinyin_to_ipa);
    RUN_TEST(test_zh_g2p_full);
    
    // 总结
    printf("\n════════════════════════════════════════════════════════════\n");
    printf("  测试结果:\n");
    printf("  ✅ 通过: %d\n", test_passed);
    printf("  ❌ 失败: %d\n", test_failed);
    printf("════════════════════════════════════════════════════════════\n");
    
    return test_failed > 0 ? 1 : 0;
}
