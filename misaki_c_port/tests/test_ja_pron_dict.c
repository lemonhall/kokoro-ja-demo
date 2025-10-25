/**
 * test_ja_pron_dict.c
 * 
 * 测试日文读音词典加载
 * 
 * License: MIT
 */

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

void test_load_dict() {
    Trie *trie = misaki_trie_create();
    TEST_ASSERT(trie != NULL, "Trie 应该创建成功");
    
    // 加载词典（使用绝对路径）
    const char *dict_file = "/mnt/e/development/kokoro-ja-demo/misaki_c_port/extracted_data/ja/ja_pron_dict.tsv";
    int count = misaki_trie_load_ja_pron_dict(trie, dict_file);
    
    printf("  📖 加载了 %d 个词汇\n", count);
    TEST_ASSERT(count > 0, "应该成功加载词汇");
    
    misaki_trie_free(trie);
    printf("  ✅ 词典加载成功\n");
}

void test_lookup_with_pron() {
    Trie *trie = misaki_trie_create();
    TEST_ASSERT(trie != NULL, "Trie 应该创建成功");
    
    // 加载词典
    const char *dict_file = "/mnt/e/development/kokoro-ja-demo/misaki_c_port/extracted_data/ja/ja_pron_dict.tsv";
    int count = misaki_trie_load_ja_pron_dict(trie, dict_file);
    TEST_ASSERT(count > 0, "应该成功加载词汇");
    
    // 测试几个常用词
    struct {
        const char *word;
        const char *expected_pron;
    } test_words[] = {
        {"は", "ワ"},          // 助词 "は" 读作 "ワ"
        {"を", "オ"},          // 助词 "を" 读作 "オ"
        {"が", "ガ"},
        {"に", "ニ"},
        {"で", "デ"},
    };
    
    for (int i = 0; i < sizeof(test_words)/sizeof(test_words[0]); i++) {
        const char *pron = NULL;
        double freq = 0;
        const char *tag = NULL;
        
        bool found = misaki_trie_lookup_with_pron(
            trie, test_words[i].word, &pron, &freq, &tag);
        
        printf("  词: '%s' → 读音: '%s' (词频: %.0f, 词性: %s)\n",
               test_words[i].word,
               pron ? pron : "NULL",
               freq,
               tag ? tag : "NULL");
        
        TEST_ASSERT(found, "应该找到词汇");
        TEST_ASSERT(pron != NULL, "读音不应为 NULL");
        
        if (test_words[i].expected_pron) {
            TEST_ASSERT(strcmp(pron, test_words[i].expected_pron) == 0,
                       "读音应该匹配");
        }
    }
    
    misaki_trie_free(trie);
    printf("  ✅ 读音查询测试通过\n");
}

void test_insert_with_pron() {
    Trie *trie = misaki_trie_create();
    
    // 手动插入几个词
    TEST_ASSERT(misaki_trie_insert_with_pron(
        trie, "こんにちは", "コンニチワ", 10000, "感動詞"),
        "应该成功插入");
    
    TEST_ASSERT(misaki_trie_insert_with_pron(
        trie, "私", "ワタクシ", 15000, "代名詞"),
        "应该成功插入");
    
    // 查询
    const char *pron = NULL;
    double freq = 0;
    
    bool found = misaki_trie_lookup_with_pron(
        trie, "こんにちは", &pron, &freq, NULL);
    
    printf("  词: 'こんにちは' → 读音: '%s' (词频: %.0f)\n", 
           pron ? pron : "NULL", freq);
    
    TEST_ASSERT(found, "应该找到词汇");
    TEST_ASSERT(pron != NULL, "读音不应为 NULL");
    TEST_ASSERT(strcmp(pron, "コンニチワ") == 0, "读音应该是 'コンニチワ'");
    TEST_ASSERT(freq == 10000, "词频应该是 10000");
    
    misaki_trie_free(trie);
    printf("  ✅ 带读音插入测试通过\n");
}

int main(void) {
    printf("════════════════════════════════════════════════════════════\n");
    printf("  日文读音词典测试\n");
    printf("════════════════════════════════════════════════════════════\n");
    
    RUN_TEST(test_insert_with_pron);
    RUN_TEST(test_load_dict);
    RUN_TEST(test_lookup_with_pron);
    
    // 总结
    printf("\n════════════════════════════════════════════════════════════\n");
    printf("  测试结果:\n");
    printf("  ✅ 通过: %d\n", test_passed);
    printf("  ❌ 失败: %d\n", test_failed);
    printf("════════════════════════════════════════════════════════════\n");
    
    return test_failed > 0 ? 1 : 0;
}
