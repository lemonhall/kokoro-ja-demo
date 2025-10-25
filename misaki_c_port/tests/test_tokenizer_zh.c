/**
 * test_tokenizer_zh.c
 * 
 * 测试中文分词器
 * 
 * License: MIT
 */

#include "misaki_tokenizer.h"
#include "misaki_trie.h"
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
 * 中文分词器测试
 * ========================================================================== */

void test_zh_tokenizer_create() {
    Trie *trie = misaki_trie_create();
    TEST_ASSERT(trie != NULL, "Trie 应该创建成功");
    
    // 添加测试词汇
    misaki_trie_insert(trie, "你好", 1000.0, NULL);
    misaki_trie_insert(trie, "世界", 800.0, NULL);
    
    ZhTokenizerConfig config = {
        .dict_trie = trie,
        .enable_hmm = false,
        .enable_userdict = false,
        .user_trie = NULL
    };
    
    void *tokenizer = misaki_zh_tokenizer_create(&config);
    TEST_ASSERT(tokenizer != NULL, "中文分词器应该创建成功");
    
    misaki_zh_tokenizer_free(tokenizer);
    misaki_trie_free(trie);
    
    printf("  ✅ 中文分词器创建成功\n");
}

void test_zh_tokenize_simple() {
    // 创建词典
    Trie *trie = misaki_trie_create();
    misaki_trie_insert(trie, "你好", 1000.0, NULL);
    misaki_trie_insert(trie, "世界", 800.0, NULL);
    
    ZhTokenizerConfig config = {
        .dict_trie = trie,
        .enable_hmm = false,
        .enable_userdict = false,
        .user_trie = NULL
    };
    
    void *tokenizer = misaki_zh_tokenizer_create(&config);
    
    // 测试分词
    const char *text = "你好世界";
    MisakiTokenList *tokens = misaki_zh_tokenize(tokenizer, text);
    TEST_ASSERT(tokens != NULL, "分词结果不应为 NULL");
    
    int count = misaki_token_list_size(tokens);
    printf("  分词结果: %d 个词\n", count);
    
    for (int i = 0; i < count; i++) {
        MisakiToken *token = misaki_token_list_get(tokens, i);
        printf("    [%d] %s\n", i, token->text);
    }
    
    TEST_ASSERT(count == 2, "应该分成 2 个词");
    
    MisakiToken *token0 = misaki_token_list_get(tokens, 0);
    TEST_ASSERT(strcmp(token0->text, "你好") == 0, "第一个词应该是'你好'");
    
    MisakiToken *token1 = misaki_token_list_get(tokens, 1);
    TEST_ASSERT(strcmp(token1->text, "世界") == 0, "第二个词应该是'世界'");
    
    misaki_token_list_free(tokens);
    misaki_zh_tokenizer_free(tokenizer);
    misaki_trie_free(trie);
    
    printf("  ✅ 中文分词测试通过\n");
}

void test_zh_tokenize_with_real_dict() {
    // 从真实词典文件加载
    const char *dict_path = "../extracted_data/zh/dict.txt";
    Trie *trie = misaki_trie_create();
    
    int word_count = misaki_trie_load_from_file(trie, dict_path, "word freq");
    if (word_count < 0) {
        printf("  ⚠️  无法加载词典文件，跳过测试\n");
        misaki_trie_free(trie);
        return;
    }
    
    printf("  加载了 %d 个词汇\n", word_count);
    
    ZhTokenizerConfig config = {
        .dict_trie = trie,
        .enable_hmm = false,
        .enable_userdict = false,
        .user_trie = NULL
    };
    
    void *tokenizer = misaki_zh_tokenizer_create(&config);
    
    // 测试多个句子
    const char *texts[] = {
        "中国",
        "我们",
        "经济发展",
        NULL
    };
    
    for (int i = 0; texts[i] != NULL; i++) {
        MisakiTokenList *tokens = misaki_zh_tokenize(tokenizer, texts[i]);
        if (tokens) {
            printf("  \"%s\" →", texts[i]);
            for (int j = 0; j < tokens->count; j++) {
                printf(" [%s]", tokens->tokens[j].text);
            }
            printf("\n");
            misaki_token_list_free(tokens);
        }
    }
    
    misaki_zh_tokenizer_free(tokenizer);
    misaki_trie_free(trie);
    
    printf("  ✅ 真实词典分词测试通过\n");
}

/* ============================================================================
 * 主测试函数
 * ========================================================================== */

int main(void) {
    printf("════════════════════════════════════════════════════════════\n");
    printf("  中文分词器测试\n");
    printf("════════════════════════════════════════════════════════════\n");
    
    RUN_TEST(test_zh_tokenizer_create);
    RUN_TEST(test_zh_tokenize_simple);
    RUN_TEST(test_zh_tokenize_with_real_dict);
    
    // 总结
    printf("\n════════════════════════════════════════════════════════════\n");
    printf("  测试结果:\n");
    printf("  ✅ 通过: %d\n", test_passed);
    printf("  ❌ 失败: %d\n", test_failed);
    printf("════════════════════════════════════════════════════════════\n");
    
    return test_failed > 0 ? 1 : 0;
}
