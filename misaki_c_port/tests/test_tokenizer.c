/**
 * test_tokenizer.c
 * 
 * 测试 misaki_tokenizer.c（Token、DAG、分词器）
 * 
 * License: MIT
 */

#include "misaki_tokenizer.h"
#include "misaki_trie.h"
#include "misaki_string.h"
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
 * Token 操作测试
 * ========================================================================== */

void test_token_create_free(void) {
    MisakiToken *token = misaki_token_create("你好", "n", 0, 6);
    TEST_ASSERT(token != NULL, "Token 应该创建成功");
    TEST_ASSERT(strcmp(token->text, "你好") == 0, "Token text 应该正确");
    TEST_ASSERT(strcmp(token->tag, "n") == 0, "Token tag 应该正确");
    TEST_ASSERT(token->start == 0, "Token start 应该为 0");
    TEST_ASSERT(token->length == 6, "Token length 应该为 6");
    TEST_ASSERT(token->score == 0.0, "Token score 初始应该为 0");
    
    misaki_token_free(token);
    printf("  ✅ Token 创建和释放成功\n");
}

void test_token_clone(void) {
    MisakiToken *token = misaki_token_create("世界", "n", 6, 6);
    misaki_token_set_phonemes(token, "shi4 jie4");
    misaki_token_set_score(token, 0.95);
    
    MisakiToken *clone = misaki_token_clone(token);
    TEST_ASSERT(clone != NULL, "Clone 应该成功");
    TEST_ASSERT(strcmp(clone->text, "世界") == 0, "Clone text 应该一致");
    TEST_ASSERT(strcmp(clone->tag, "n") == 0, "Clone tag 应该一致");
    TEST_ASSERT(strcmp(clone->phonemes, "shi4 jie4") == 0, "Clone phonemes 应该一致");
    TEST_ASSERT(clone->score == 0.95, "Clone score 应该一致");
    TEST_ASSERT(clone->start == 6, "Clone start 应该一致");
    TEST_ASSERT(clone->length == 6, "Clone length 应该一致");
    
    misaki_token_free(token);
    misaki_token_free(clone);
    printf("  ✅ Token 克隆成功\n");
}

void test_token_set_phonemes(void) {
    MisakiToken *token = misaki_token_create("中国", "ns", 0, 6);
    
    bool result = misaki_token_set_phonemes(token, "zhong1 guo2");
    TEST_ASSERT(result == true, "设置 phonemes 应该成功");
    TEST_ASSERT(strcmp(token->phonemes, "zhong1 guo2") == 0, "Phonemes 应该正确");
    
    // 更新 phonemes
    result = misaki_token_set_phonemes(token, "zhong4 guo2");
    TEST_ASSERT(result == true, "更新 phonemes 应该成功");
    TEST_ASSERT(strcmp(token->phonemes, "zhong4 guo2") == 0, "更新后的 phonemes 应该正确");
    
    misaki_token_free(token);
    printf("  ✅ Token 设置音素成功\n");
}

void test_token_set_score(void) {
    MisakiToken *token = misaki_token_create("测试", "v", 0, 6);
    
    misaki_token_set_score(token, 0.88);
    TEST_ASSERT(token->score == 0.88, "设置 score 应该成功");
    
    misaki_token_set_score(token, -1.23);
    TEST_ASSERT(token->score == -1.23, "更新 score 应该成功");
    
    misaki_token_free(token);
    printf("  ✅ Token 设置分数成功\n");
}

/* ============================================================================
 * TokenList 操作测试
 * ========================================================================== */

void test_token_list_create_free(void) {
    MisakiTokenList *list = misaki_token_list_create();
    TEST_ASSERT(list != NULL, "TokenList 应该创建成功");
    TEST_ASSERT(list->count == 0, "TokenList 初始 count 应该为 0");
    TEST_ASSERT(list->capacity >= 16, "TokenList 初始 capacity 应该 >= 16");
    
    misaki_token_list_free(list);
    printf("  ✅ TokenList 创建和释放成功\n");
}

void test_token_list_add_get(void) {
    MisakiTokenList *list = misaki_token_list_create();
    
    MisakiToken *token1 = misaki_token_create("我", "r", 0, 3);
    MisakiToken *token2 = misaki_token_create("爱", "v", 3, 3);
    MisakiToken *token3 = misaki_token_create("中国", "ns", 6, 6);
    
    bool result = misaki_token_list_add(list, token1);
    TEST_ASSERT(result == true, "添加 token1 应该成功");
    TEST_ASSERT(list->count == 1, "添加后 count 应该为 1");
    
    result = misaki_token_list_add(list, token2);
    TEST_ASSERT(result == true, "添加 token2 应该成功");
    
    result = misaki_token_list_add(list, token3);
    TEST_ASSERT(result == true, "添加 token3 应该成功");
    TEST_ASSERT(list->count == 3, "添加后 count 应该为 3");
    
    // 测试 get
    MisakiToken *get1 = misaki_token_list_get(list, 0);
    TEST_ASSERT(get1 != NULL, "获取 token[0] 应该成功");
    TEST_ASSERT(strcmp(get1->text, "我") == 0, "token[0].text 应该为 '我'");
    
    MisakiToken *get2 = misaki_token_list_get(list, 1);
    TEST_ASSERT(strcmp(get2->text, "爱") == 0, "token[1].text 应该为 '爱'");
    
    MisakiToken *get3 = misaki_token_list_get(list, 2);
    TEST_ASSERT(strcmp(get3->text, "中国") == 0, "token[2].text 应该为 '中国'");
    
    // 测试越界
    MisakiToken *get_invalid = misaki_token_list_get(list, 10);
    TEST_ASSERT(get_invalid == NULL, "越界访问应该返回 NULL");
    
    misaki_token_free(token1);
    misaki_token_free(token2);
    misaki_token_free(token3);
    misaki_token_list_free(list);
    printf("  ✅ TokenList 添加和获取成功\n");
}

void test_token_list_size(void) {
    MisakiTokenList *list = misaki_token_list_create();
    TEST_ASSERT(misaki_token_list_size(list) == 0, "初始 size 应该为 0");
    
    MisakiToken *token = misaki_token_create("测试", "v", 0, 6);
    misaki_token_list_add(list, token);
    TEST_ASSERT(misaki_token_list_size(list) == 1, "添加后 size 应该为 1");
    
    misaki_token_list_add(list, token);
    misaki_token_list_add(list, token);
    TEST_ASSERT(misaki_token_list_size(list) == 3, "添加 3 次后 size 应该为 3");
    
    misaki_token_free(token);
    misaki_token_list_free(list);
    printf("  ✅ TokenList size 正确\n");
}

void test_token_list_clear(void) {
    MisakiTokenList *list = misaki_token_list_create();
    
    MisakiToken *token = misaki_token_create("清空", "v", 0, 6);
    misaki_token_list_add(list, token);
    misaki_token_list_add(list, token);
    TEST_ASSERT(list->count == 2, "添加后 count 应该为 2");
    
    misaki_token_list_clear(list);
    TEST_ASSERT(list->count == 0, "清空后 count 应该为 0");
    TEST_ASSERT(misaki_token_list_size(list) == 0, "清空后 size 应该为 0");
    
    misaki_token_free(token);
    misaki_token_list_free(list);
    printf("  ✅ TokenList clear 成功\n");
}

/* ============================================================================
 * DAG 操作测试
 * ========================================================================== */

void test_dag_create_free(void) {
    DAG *dag = misaki_dag_create(10);
    TEST_ASSERT(dag != NULL, "DAG 应该创建成功");
    TEST_ASSERT(dag->length == 10, "DAG length 应该为 10");
    TEST_ASSERT(dag->nodes != NULL, "DAG nodes 应该分配内存");
    
    misaki_dag_free(dag);
    printf("  ✅ DAG 创建和释放成功\n");
}

void test_dag_add_edge(void) {
    DAG *dag = misaki_dag_create(5);
    
    bool result = misaki_dag_add_edge(dag, 0, 1);
    TEST_ASSERT(result == true, "添加边 0->1 应该成功");
    
    result = misaki_dag_add_edge(dag, 0, 2);
    TEST_ASSERT(result == true, "添加边 0->2 应该成功");
    
    result = misaki_dag_add_edge(dag, 1, 3);
    TEST_ASSERT(result == true, "添加边 1->3 应该成功");
    
    misaki_dag_free(dag);
    printf("  ✅ DAG 添加边成功\n");
}

void test_dag_get_next(void) {
    DAG *dag = misaki_dag_create(5);
    
    // 构建 DAG:
    // 0 -> 1, 2
    // 1 -> 3
    // 2 -> 3, 4
    misaki_dag_add_edge(dag, 0, 1);
    misaki_dag_add_edge(dag, 0, 2);
    misaki_dag_add_edge(dag, 1, 3);
    misaki_dag_add_edge(dag, 2, 3);
    misaki_dag_add_edge(dag, 2, 4);
    
    // 测试获取后继
    int next[10];
    int count = misaki_dag_get_next(dag, 0, next, 10);
    TEST_ASSERT(count == 2, "位置 0 应该有 2 个后继");
    TEST_ASSERT((next[0] == 1 && next[1] == 2) || (next[0] == 2 && next[1] == 1),
                "位置 0 的后继应该是 1 和 2");
    
    count = misaki_dag_get_next(dag, 1, next, 10);
    TEST_ASSERT(count == 1, "位置 1 应该有 1 个后继");
    TEST_ASSERT(next[0] == 3, "位置 1 的后继应该是 3");
    
    count = misaki_dag_get_next(dag, 2, next, 10);
    TEST_ASSERT(count == 2, "位置 2 应该有 2 个后继");
    
    count = misaki_dag_get_next(dag, 4, next, 10);
    TEST_ASSERT(count == 0, "位置 4 应该没有后继");
    
    misaki_dag_free(dag);
    printf("  ✅ DAG 获取后继成功\n");
}

void test_dag_build_with_trie(void) {
    // 构建简单的中文词典 Trie
    Trie *trie = misaki_trie_create();
    misaki_trie_insert(trie, "我", 1.0, NULL);
    misaki_trie_insert(trie, "爱", 1.0, NULL);
    misaki_trie_insert(trie, "中国", 1.0, NULL);
    misaki_trie_insert(trie, "中", 0.5, NULL);
    misaki_trie_insert(trie, "国", 0.5, NULL);
    
    const char *text = "我爱中国";
    DAG *dag = misaki_dag_build(text, trie);
    
    TEST_ASSERT(dag != NULL, "DAG build 应该成功");
    
    // 验证 DAG 结构
    // 字符位置: 0(我) 1(爱) 2(中) 3(国)
    // 0 -> 1 (我)
    // 1 -> 2 (爱)
    // 2 -> 3 (中) 或 2 -> 4 (中国)
    // 3 -> 4 (国)
    
    int next[10];
    int count = misaki_dag_get_next(dag, 0, next, 10);
    TEST_ASSERT(count >= 1, "位置 0 应该至少有 1 个后继");
    TEST_ASSERT(next[0] == 1, "位置 0 应该指向位置 1（词 '我'）");
    
    count = misaki_dag_get_next(dag, 1, next, 10);
    TEST_ASSERT(count >= 1, "位置 1 应该至少有 1 个后继");
    TEST_ASSERT(next[0] == 2, "位置 1 应该指向位置 2（词 '爱'）");
    
    count = misaki_dag_get_next(dag, 2, next, 10);
    TEST_ASSERT(count >= 1, "位置 2 应该至少有 1 个后继");
    // 应该有 2->3 (中) 和 2->4 (中国)
    bool has_single = false;
    bool has_double = false;
    for (int i = 0; i < count; i++) {
        if (next[i] == 3) has_single = true;
        if (next[i] == 4) has_double = true;
    }
    TEST_ASSERT(has_single || has_double, "位置 2 应该有到 3 或 4 的边");
    
    misaki_dag_free(dag);
    misaki_trie_free(trie);
    printf("  ✅ DAG build (基于 Trie) 成功\n");
}

void test_dag_build_complex(void) {
    // 测试更复杂的句子
    Trie *trie = misaki_trie_create();
    misaki_trie_insert(trie, "北京", 1.0, NULL);
    misaki_trie_insert(trie, "天安门", 1.0, NULL);
    misaki_trie_insert(trie, "天", 0.3, NULL);
    misaki_trie_insert(trie, "安", 0.2, NULL);
    misaki_trie_insert(trie, "门", 0.2, NULL);
    
    const char *text = "北京天安门";
    DAG *dag = misaki_dag_build(text, trie);
    
    TEST_ASSERT(dag != NULL, "复杂 DAG build 应该成功");
    
    // 字符位置: 0(北) 1(京) 2(天) 3(安) 4(门)
    // 0 -> 2 (北京)
    // 2 -> 3 (天) 或 2 -> 5 (天安门)
    
    int next[10];
    int count = misaki_dag_get_next(dag, 0, next, 10);
    TEST_ASSERT(count >= 1, "位置 0 应该有后继");
    TEST_ASSERT(next[0] == 2, "位置 0 应该指向位置 2（词 '北京'）");
    
    count = misaki_dag_get_next(dag, 2, next, 10);
    TEST_ASSERT(count >= 1, "位置 2 应该有后继");
    // 可能有 2->3 (天) 和 2->5 (天安门)
    
    misaki_dag_free(dag);
    misaki_trie_free(trie);
    printf("  ✅ 复杂 DAG build 成功\n");
}

/* ============================================================================
 * 英文分词器测试
 * ========================================================================== */

void test_en_tokenize_simple(void) {
    const char *text = "Hello world this is a test";
    MisakiTokenList *tokens = misaki_en_tokenize(text);
    
    TEST_ASSERT(tokens != NULL, "英文分词结果不应为 NULL");
    
    int count = misaki_token_list_size(tokens);
    TEST_ASSERT(count == 6, "应该分出 6 个词");
    
    MisakiToken *token0 = misaki_token_list_get(tokens, 0);
    TEST_ASSERT(strcmp(token0->text, "Hello") == 0, "第 0 个词应该是 'Hello'");
    
    MisakiToken *token1 = misaki_token_list_get(tokens, 1);
    TEST_ASSERT(strcmp(token1->text, "world") == 0, "第 1 个词应该是 'world'");
    
    misaki_token_list_free(tokens);
    printf("  ✅ 简单英文分词成功\n");
}

void test_en_tokenize_with_punctuation(void) {
    const char *text = "Hello, world! How are you?";
    
    // 不保留标点
    MisakiTokenList *tokens = misaki_en_tokenize(text);
    TEST_ASSERT(tokens != NULL, "分词结果不应为 NULL");
    
    int count = misaki_token_list_size(tokens);
    TEST_ASSERT(count == 5, "不保留标点时应该分出 5 个词");
    
    misaki_token_list_free(tokens);
    
    // 保留标点
    tokens = misaki_en_tokenize_ex(text, true);
    TEST_ASSERT(tokens != NULL, "保留标点分词结果不应为 NULL");
    
    count = misaki_token_list_size(tokens);
    TEST_ASSERT(count >= 5, "保留标点时应该至少分出 5 个 token");
    
    misaki_token_list_free(tokens);
    printf("  ✅ 英文分词（含标点）成功\n");
}

/* ============================================================================
 * 日文分词器测试
 * ========================================================================== */

void test_ja_tokenize_simple(void) {
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
    
    const char *text = "こんにちは世界";
    MisakiTokenList *tokens = misaki_ja_tokenize(tokenizer, text);
    TEST_ASSERT(tokens != NULL, "日文分词结果不应为 NULL");
    
    int count = misaki_token_list_size(tokens);
    TEST_ASSERT(count == 2, "应该分出 2 个词");
    
    MisakiToken *token0 = misaki_token_list_get(tokens, 0);
    TEST_ASSERT(strcmp(token0->text, "こんにちは") == 0, "第 0 个词应该是 'こんにちは'");
    
    MisakiToken *token1 = misaki_token_list_get(tokens, 1);
    TEST_ASSERT(strcmp(token1->text, "世界") == 0, "第 1 个词应该是 '世界'");
    
    misaki_token_list_free(tokens);
    misaki_ja_tokenizer_free(tokenizer);
    misaki_trie_free(trie);
    printf("  ✅ 简单日文分词成功\n");
}

/* ============================================================================
 * 中文分词器测试
 * ========================================================================== */

void test_zh_tokenizer_create_free(void) {
    // 创建简单的中文词典
    Trie *trie = misaki_trie_create();
    misaki_trie_insert(trie, "我", 1.0, NULL);
    misaki_trie_insert(trie, "爱", 1.0, NULL);
    misaki_trie_insert(trie, "中国", 2.0, NULL);
    
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
    printf("  ✅ 中文分词器创建和释放成功\n");
}

void test_zh_tokenize_simple(void) {
    // 构建中文词典
    Trie *trie = misaki_trie_create();
    misaki_trie_insert(trie, "我", 1.0, NULL);
    misaki_trie_insert(trie, "爱", 1.0, NULL);
    misaki_trie_insert(trie, "中国", 3.0, NULL);  // 更高频率
    misaki_trie_insert(trie, "中", 0.5, NULL);
    misaki_trie_insert(trie, "国", 0.5, NULL);
    
    ZhTokenizerConfig config = {
        .dict_trie = trie,
        .enable_hmm = false,
        .enable_userdict = false,
        .user_trie = NULL
    };
    
    void *tokenizer = misaki_zh_tokenizer_create(&config);
    TEST_ASSERT(tokenizer != NULL, "分词器创建成功");
    
    // 测试分词
    const char *text = "我爱中国";
    MisakiTokenList *tokens = misaki_zh_tokenize(tokenizer, text);
    TEST_ASSERT(tokens != NULL, "分词结果不应为 NULL");
    
    int token_count = misaki_token_list_size(tokens);
    TEST_ASSERT(token_count == 3, "应该分出 3 个词");
    
    // 验证第一个词
    MisakiToken *token0 = misaki_token_list_get(tokens, 0);
    TEST_ASSERT(token0 != NULL, "第 0 个 token 不应为 NULL");
    TEST_ASSERT(strcmp(token0->text, "我") == 0, "第 0 个词应该是 '我'");
    
    // 验证第二个词
    MisakiToken *token1 = misaki_token_list_get(tokens, 1);
    TEST_ASSERT(strcmp(token1->text, "爱") == 0, "第 1 个词应该是 '爱'");
    
    // 验证第三个词
    MisakiToken *token2 = misaki_token_list_get(tokens, 2);
    TEST_ASSERT(strcmp(token2->text, "中国") == 0, "第 2 个词应该是 '中国'（而不是 '中' + '国'）");
    
    misaki_token_list_free(tokens);
    misaki_zh_tokenizer_free(tokenizer);
    misaki_trie_free(trie);
    printf("  ✅ 简单中文分词成功\n");
}

void test_zh_tokenize_complex(void) {
    // 构建更复杂的词典
    Trie *trie = misaki_trie_create();
    misaki_trie_insert(trie, "北京", 5.0, NULL);
    misaki_trie_insert(trie, "天安门", 4.0, NULL);
    misaki_trie_insert(trie, "天", 1.0, NULL);
    misaki_trie_insert(trie, "安", 0.5, NULL);
    misaki_trie_insert(trie, "门", 0.5, NULL);
    misaki_trie_insert(trie, "广场", 3.0, NULL);
    
    ZhTokenizerConfig config = {
        .dict_trie = trie,
        .enable_hmm = false,
        .enable_userdict = false,
        .user_trie = NULL
    };
    
    void *tokenizer = misaki_zh_tokenizer_create(&config);
    
    const char *text = "北京天安门";
    MisakiTokenList *tokens = misaki_zh_tokenize(tokenizer, text);
    TEST_ASSERT(tokens != NULL, "分词结果不应为 NULL");
    
    int token_count = misaki_token_list_size(tokens);
    printf("  分词结果数量: %d\n", token_count);
    
    // 打印分词结果（调试用）
    for (int i = 0; i < token_count; i++) {
        MisakiToken *token = misaki_token_list_get(tokens, i);
        printf("    [%d] %s\n", i, token->text);
    }
    
    TEST_ASSERT(token_count >= 2, "至少应该分出 2 个词");
    
    misaki_token_list_free(tokens);
    misaki_zh_tokenizer_free(tokenizer);
    misaki_trie_free(trie);
    printf("  ✅ 复杂中文分词成功\n");
}

/* ============================================================================
 * 主测试函数
 * ========================================================================== */

int main(void) {
    printf("════════════════════════════════════════════════════════════\n");
    printf("  Misaki Tokenizer Tests\n");
    printf("════════════════════════════════════════════════════════════\n");
    
    // Token 操作测试
    RUN_TEST(test_token_create_free);
    RUN_TEST(test_token_clone);
    RUN_TEST(test_token_set_phonemes);
    RUN_TEST(test_token_set_score);
    
    // TokenList 操作测试
    RUN_TEST(test_token_list_create_free);
    RUN_TEST(test_token_list_add_get);
    RUN_TEST(test_token_list_size);
    RUN_TEST(test_token_list_clear);
    
    // DAG 操作测试
    RUN_TEST(test_dag_create_free);
    RUN_TEST(test_dag_add_edge);
    RUN_TEST(test_dag_get_next);
    RUN_TEST(test_dag_build_with_trie);
    RUN_TEST(test_dag_build_complex);
    
    // 中文分词器测试
    RUN_TEST(test_zh_tokenizer_create_free);
    RUN_TEST(test_zh_tokenize_simple);
    RUN_TEST(test_zh_tokenize_complex);
    
    // 英文分词器测试
    RUN_TEST(test_en_tokenize_simple);
    RUN_TEST(test_en_tokenize_with_punctuation);
    
    // 日文分词器测试
    RUN_TEST(test_ja_tokenize_simple);
    
    // 总结
    printf("\n════════════════════════════════════════════════════════════\n");
    printf("  测试结果:\n");
    printf("  ✅ 通过: %d\n", test_passed);
    printf("  ❌ 失败: %d\n", test_failed);
    printf("════════════════════════════════════════════════════════════\n");
    
    return test_failed > 0 ? 1 : 0;
}
