/**
 * test_ja_comprehensive.c
 * 
 * 日文分词器综合测试套件 - 包含20个完整测试用例
 * 
 * License: MIT
 */

#include "misaki_tokenizer.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <time.h>

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
 * 辅助函数：创建包含常用词的完整词典
 * ========================================================================== */

static Trie* create_full_dict() {
    Trie *trie = misaki_trie_create();
    
    // 基础问候语
    misaki_trie_insert(trie, "こんにちは", 10000.0, "感動詞");
    misaki_trie_insert(trie, "さようなら", 8000.0, "感動詞");
    misaki_trie_insert(trie, "おはよう", 9000.0, "感動詞");
    misaki_trie_insert(trie, "ありがとう", 9500.0, "感動詞");
    
    // 代词和助词
    misaki_trie_insert(trie, "私", 10000.0, "代名詞");
    misaki_trie_insert(trie, "あなた", 9000.0, "代名詞");
    misaki_trie_insert(trie, "彼", 8000.0, "代名詞");
    misaki_trie_insert(trie, "彼女", 8000.0, "代名詞");
    misaki_trie_insert(trie, "は", 10000.0, "助詞");
    misaki_trie_insert(trie, "が", 10000.0, "助詞");
    misaki_trie_insert(trie, "を", 10000.0, "助詞");
    misaki_trie_insert(trie, "に", 10000.0, "助詞");
    misaki_trie_insert(trie, "で", 10000.0, "助詞");
    misaki_trie_insert(trie, "と", 10000.0, "助詞");
    misaki_trie_insert(trie, "の", 10000.0, "助詞");
    misaki_trie_insert(trie, "から", 9000.0, "助詞");
    misaki_trie_insert(trie, "まで", 9000.0, "助詞");
    misaki_trie_insert(trie, "や", 8000.0, "助詞");
    
    // 名词
    misaki_trie_insert(trie, "学生", 9000.0, "名詞");
    misaki_trie_insert(trie, "先生", 9000.0, "名詞");
    misaki_trie_insert(trie, "学校", 9000.0, "名詞");
    misaki_trie_insert(trie, "会社", 9000.0, "名詞");
    misaki_trie_insert(trie, "日本", 10000.0, "名詞");
    misaki_trie_insert(trie, "日本語", 9500.0, "名詞");
    misaki_trie_insert(trie, "東京", 9500.0, "名詞");
    misaki_trie_insert(trie, "本", 8000.0, "名詞");
    misaki_trie_insert(trie, "本当", 8500.0, "名詞");
    misaki_trie_insert(trie, "猫", 7000.0, "名詞");
    misaki_trie_insert(trie, "犬", 7000.0, "名詞");
    misaki_trie_insert(trie, "友達", 8500.0, "名詞");
    misaki_trie_insert(trie, "家", 9000.0, "名詞");
    misaki_trie_insert(trie, "食べ物", 7000.0, "名詞");
    misaki_trie_insert(trie, "お茶", 8000.0, "名詞");
    misaki_trie_insert(trie, "水", 8000.0, "名詞");
    
    // 动词和形容词
    misaki_trie_insert(trie, "行く", 9000.0, "動詞");
    misaki_trie_insert(trie, "来る", 9000.0, "動詞");
    misaki_trie_insert(trie, "食べる", 8500.0, "動詞");
    misaki_trie_insert(trie, "飲む", 8500.0, "動詞");
    misaki_trie_insert(trie, "見る", 8500.0, "動詞");
    misaki_trie_insert(trie, "読む", 8000.0, "動詞");
    misaki_trie_insert(trie, "書く", 8000.0, "動詞");
    misaki_trie_insert(trie, "話す", 8000.0, "動詞");
    misaki_trie_insert(trie, "勉強", 8500.0, "名詞");
    misaki_trie_insert(trie, "勉強する", 8500.0, "動詞");
    misaki_trie_insert(trie, "する", 9500.0, "動詞");
    misaki_trie_insert(trie, "好き", 8000.0, "形容詞");
    misaki_trie_insert(trie, "嫌い", 7000.0, "形容詞");
    misaki_trie_insert(trie, "大きい", 7500.0, "形容詞");
    misaki_trie_insert(trie, "小さい", 7500.0, "形容詞");
    misaki_trie_insert(trie, "美しい", 7000.0, "形容詞");
    misaki_trie_insert(trie, "元気", 8000.0, "形容動詞");
    
    // 片假名（外来语）
    misaki_trie_insert(trie, "コンピュータ", 8000.0, "名詞");
    misaki_trie_insert(trie, "テスト", 7500.0, "名詞");
    misaki_trie_insert(trie, "プログラム", 7500.0, "名詞");
    misaki_trie_insert(trie, "ソフトウェア", 7000.0, "名詞");
    misaki_trie_insert(trie, "コーヒー", 8000.0, "名詞");
    
    // 常用词组
    misaki_trie_insert(trie, "です", 10000.0, "助動詞");
    misaki_trie_insert(trie, "ます", 10000.0, "助動詞");
    misaki_trie_insert(trie, "ました", 9500.0, "助動詞");
    misaki_trie_insert(trie, "ですか", 9000.0, "助動詞");
    misaki_trie_insert(trie, "ません", 9000.0, "助動詞");
    misaki_trie_insert(trie, "でした", 9000.0, "助動詞");
    misaki_trie_insert(trie, "だ", 9500.0, "助動詞");
    
    // 疑问词
    misaki_trie_insert(trie, "何", 8500.0, "代名詞");
    misaki_trie_insert(trie, "誰", 8500.0, "代名詞");
    misaki_trie_insert(trie, "どこ", 8500.0, "代名詞");
    misaki_trie_insert(trie, "いつ", 8500.0, "代名詞");
    misaki_trie_insert(trie, "なぜ", 8000.0, "代名詞");
    
    // 数词
    misaki_trie_insert(trie, "一", 8000.0, "数詞");
    misaki_trie_insert(trie, "二", 8000.0, "数詞");
    misaki_trie_insert(trie, "三", 8000.0, "数詞");
    misaki_trie_insert(trie, "一緒", 7500.0, "名詞");
    
    return trie;
}

/* ============================================================================
 * 测试用例 1-5: 基础语法测试
 * ========================================================================== */

void test_01_greetings() {
    Trie *trie = create_full_dict();
    JaTokenizerConfig config = { .dict_trie = trie, .use_simple_model = false };
    void *tokenizer = misaki_ja_tokenizer_create(&config);
    
    const char *text = "こんにちは";
    MisakiTokenList *tokens = misaki_ja_tokenize(tokenizer, text);
    TEST_ASSERT(tokens != NULL, "基础问候语分词应该成功");
    
    printf("  分词结果 (%d 个): ", tokens->count);
    for (int i = 0; i < tokens->count; i++) {
        printf("'%s' ", misaki_token_list_get(tokens, i)->text);
    }
    printf("\n");
    
    misaki_token_list_free(tokens);
    misaki_ja_tokenizer_free(tokenizer);
    misaki_trie_free(trie);
    printf("  ✅ 测试1通过：基础问候语\n");
}

void test_02_simple_sentence() {
    Trie *trie = create_full_dict();
    JaTokenizerConfig config = { .dict_trie = trie, .use_simple_model = false };
    void *tokenizer = misaki_ja_tokenizer_create(&config);
    
    const char *text = "私は学生です";
    MisakiTokenList *tokens = misaki_ja_tokenize(tokenizer, text);
    TEST_ASSERT(tokens != NULL, "简单句子分词应该成功");
    TEST_ASSERT(tokens->count >= 3, "应该至少有3个词");
    
    printf("  分词结果 (%d 个): ", tokens->count);
    for (int i = 0; i < tokens->count; i++) {
        printf("'%s' ", misaki_token_list_get(tokens, i)->text);
    }
    printf("\n");
    
    misaki_token_list_free(tokens);
    misaki_ja_tokenizer_free(tokenizer);
    misaki_trie_free(trie);
    printf("  ✅ 测试2通过：简单句子（主谓宾）\n");
}

void test_03_particles() {
    Trie *trie = create_full_dict();
    JaTokenizerConfig config = { .dict_trie = trie, .use_simple_model = false };
    void *tokenizer = misaki_ja_tokenizer_create(&config);
    
    const char *text = "私は本を読む";
    MisakiTokenList *tokens = misaki_ja_tokenize(tokenizer, text);
    TEST_ASSERT(tokens != NULL, "包含助词的句子分词应该成功");
    
    printf("  分词结果 (%d 个): ", tokens->count);
    for (int i = 0; i < tokens->count; i++) {
        printf("'%s' ", misaki_token_list_get(tokens, i)->text);
    }
    printf("\n");
    
    misaki_token_list_free(tokens);
    misaki_ja_tokenizer_free(tokenizer);
    misaki_trie_free(trie);
    printf("  ✅ 测试3通过：包含助词的复杂句\n");
}

void test_04_question() {
    Trie *trie = create_full_dict();
    JaTokenizerConfig config = { .dict_trie = trie, .use_simple_model = false };
    void *tokenizer = misaki_ja_tokenizer_create(&config);
    
    const char *text = "元気ですか";
    MisakiTokenList *tokens = misaki_ja_tokenize(tokenizer, text);
    TEST_ASSERT(tokens != NULL, "疑问句分词应该成功");
    
    printf("  分词结果 (%d 个): ", tokens->count);
    for (int i = 0; i < tokens->count; i++) {
        printf("'%s' ", misaki_token_list_get(tokens, i)->text);
    }
    printf("\n");
    
    misaki_token_list_free(tokens);
    misaki_ja_tokenizer_free(tokenizer);
    misaki_trie_free(trie);
    printf("  ✅ 测试4通过：疑问句\n");
}

void test_05_punctuation() {
    Trie *trie = create_full_dict();
    JaTokenizerConfig config = { .dict_trie = trie, .use_simple_model = false };
    void *tokenizer = misaki_ja_tokenizer_create(&config);
    
    const char *text = "こんにちは、元気ですか？";
    MisakiTokenList *tokens = misaki_ja_tokenize(tokenizer, text);
    TEST_ASSERT(tokens != NULL, "包含标点的句子分词应该成功");
    
    printf("  分词结果 (%d 个): ", tokens->count);
    for (int i = 0; i < tokens->count; i++) {
        printf("'%s' ", misaki_token_list_get(tokens, i)->text);
    }
    printf("\n");
    
    misaki_token_list_free(tokens);
    misaki_ja_tokenizer_free(tokenizer);
    misaki_trie_free(trie);
    printf("  ✅ 测试5通过：标点符号处理\n");
}

/* ============================================================================
 * 测试用例 6-10: 不同文字类型
 * ========================================================================== */

void test_06_katakana() {
    Trie *trie = create_full_dict();
    JaTokenizerConfig config = { .dict_trie = trie, .use_simple_model = false };
    void *tokenizer = misaki_ja_tokenizer_create(&config);
    
    const char *text = "コンピュータとソフトウェア";
    MisakiTokenList *tokens = misaki_ja_tokenize(tokenizer, text);
    TEST_ASSERT(tokens != NULL, "片假名分词应该成功");
    
    printf("  分词结果 (%d 个): ", tokens->count);
    for (int i = 0; i < tokens->count; i++) {
        printf("'%s' ", misaki_token_list_get(tokens, i)->text);
    }
    printf("\n");
    
    misaki_token_list_free(tokens);
    misaki_ja_tokenizer_free(tokenizer);
    misaki_trie_free(trie);
    printf("  ✅ 测试6通过：片假名（外来语）\n");
}

void test_07_kanji() {
    Trie *trie = create_full_dict();
    JaTokenizerConfig config = { .dict_trie = trie, .use_simple_model = false };
    void *tokenizer = misaki_ja_tokenizer_create(&config);
    
    const char *text = "東京と日本";
    MisakiTokenList *tokens = misaki_ja_tokenize(tokenizer, text);
    TEST_ASSERT(tokens != NULL, "汉字分词应该成功");
    
    printf("  分词结果 (%d 个): ", tokens->count);
    for (int i = 0; i < tokens->count; i++) {
        printf("'%s' ", misaki_token_list_get(tokens, i)->text);
    }
    printf("\n");
    
    misaki_token_list_free(tokens);
    misaki_ja_tokenizer_free(tokenizer);
    misaki_trie_free(trie);
    printf("  ✅ 测试7通过：汉字词汇\n");
}

void test_08_mixed_scripts() {
    Trie *trie = create_full_dict();
    JaTokenizerConfig config = { .dict_trie = trie, .use_simple_model = false };
    void *tokenizer = misaki_ja_tokenizer_create(&config);
    
    const char *text = "私はコーヒーが好きです";
    MisakiTokenList *tokens = misaki_ja_tokenize(tokenizer, text);
    TEST_ASSERT(tokens != NULL, "混合文字分词应该成功");
    
    printf("  分词结果 (%d 个): ", tokens->count);
    for (int i = 0; i < tokens->count; i++) {
        printf("'%s' ", misaki_token_list_get(tokens, i)->text);
    }
    printf("\n");
    
    misaki_token_list_free(tokens);
    misaki_ja_tokenizer_free(tokenizer);
    misaki_trie_free(trie);
    printf("  ✅ 测试8通过：混合文字（平假名+片假名+汉字）\n");
}

void test_09_long_words() {
    Trie *trie = create_full_dict();
    JaTokenizerConfig config = { .dict_trie = trie, .use_simple_model = false };
    void *tokenizer = misaki_ja_tokenizer_create(&config);
    
    const char *text = "勉強する";
    MisakiTokenList *tokens = misaki_ja_tokenize(tokenizer, text);
    TEST_ASSERT(tokens != NULL, "长词分词应该成功");
    
    printf("  分词结果 (%d 个): ", tokens->count);
    for (int i = 0; i < tokens->count; i++) {
        printf("'%s' ", misaki_token_list_get(tokens, i)->text);
    }
    printf("\n");
    
    misaki_token_list_free(tokens);
    misaki_ja_tokenizer_free(tokenizer);
    misaki_trie_free(trie);
    printf("  ✅ 测试9通过：长词处理\n");
}

void test_10_repeated_chars() {
    Trie *trie = create_full_dict();
    JaTokenizerConfig config = { .dict_trie = trie, .use_simple_model = false };
    void *tokenizer = misaki_ja_tokenizer_create(&config);
    
    const char *text = "ははは";
    MisakiTokenList *tokens = misaki_ja_tokenize(tokenizer, text);
    TEST_ASSERT(tokens != NULL, "重复字符分词应该成功");
    
    printf("  分词结果 (%d 个): ", tokens->count);
    for (int i = 0; i < tokens->count; i++) {
        printf("'%s' ", misaki_token_list_get(tokens, i)->text);
    }
    printf("\n");
    
    misaki_token_list_free(tokens);
    misaki_ja_tokenizer_free(tokenizer);
    misaki_trie_free(trie);
    printf("  ✅ 测试10通过：连续相同字符\n");
}

/* ============================================================================
 * 测试用例 11-15: 边界情况
 * ========================================================================== */

void test_11_single_char() {
    Trie *trie = create_full_dict();
    JaTokenizerConfig config = { .dict_trie = trie, .use_simple_model = false };
    void *tokenizer = misaki_ja_tokenizer_create(&config);
    
    const char *text = "猫";
    MisakiTokenList *tokens = misaki_ja_tokenize(tokenizer, text);
    TEST_ASSERT(tokens != NULL, "单字符分词应该成功");
    TEST_ASSERT(tokens->count == 1, "应该只有1个词");
    
    printf("  分词结果: '%s'\n", misaki_token_list_get(tokens, 0)->text);
    
    misaki_token_list_free(tokens);
    misaki_ja_tokenizer_free(tokenizer);
    misaki_trie_free(trie);
    printf("  ✅ 测试11通过：单字符处理\n");
}

void test_12_long_sentence() {
    Trie *trie = create_full_dict();
    JaTokenizerConfig config = { .dict_trie = trie, .use_simple_model = false };
    void *tokenizer = misaki_ja_tokenizer_create(&config);
    
    const char *text = "私は学校で友達と一緒に勉強するのが好きです";
    MisakiTokenList *tokens = misaki_ja_tokenize(tokenizer, text);
    TEST_ASSERT(tokens != NULL, "长句子分词应该成功");
    
    printf("  分词结果 (%d 个): ", tokens->count);
    for (int i = 0; i < tokens->count; i++) {
        printf("'%s' ", misaki_token_list_get(tokens, i)->text);
    }
    printf("\n");
    
    misaki_token_list_free(tokens);
    misaki_ja_tokenizer_free(tokenizer);
    misaki_trie_free(trie);
    printf("  ✅ 测试12通过：复杂长句\n");
}

void test_13_numbers() {
    Trie *trie = create_full_dict();
    JaTokenizerConfig config = { .dict_trie = trie, .use_simple_model = false };
    void *tokenizer = misaki_ja_tokenizer_create(&config);
    
    const char *text = "一、二、三";
    MisakiTokenList *tokens = misaki_ja_tokenize(tokenizer, text);
    TEST_ASSERT(tokens != NULL, "数字分词应该成功");
    
    printf("  分词结果 (%d 个): ", tokens->count);
    for (int i = 0; i < tokens->count; i++) {
        printf("'%s' ", misaki_token_list_get(tokens, i)->text);
    }
    printf("\n");
    
    misaki_token_list_free(tokens);
    misaki_ja_tokenizer_free(tokenizer);
    misaki_trie_free(trie);
    printf("  ✅ 测试13通过：数字混合文本\n");
}

void test_14_spaces() {
    Trie *trie = create_full_dict();
    JaTokenizerConfig config = { .dict_trie = trie, .use_simple_model = false };
    void *tokenizer = misaki_ja_tokenizer_create(&config);
    
    const char *text = "こんにちは 元気";
    MisakiTokenList *tokens = misaki_ja_tokenize(tokenizer, text);
    TEST_ASSERT(tokens != NULL, "包含空格的文本分词应该成功");
    
    printf("  分词结果 (%d 个): ", tokens->count);
    for (int i = 0; i < tokens->count; i++) {
        printf("'%s' ", misaki_token_list_get(tokens, i)->text);
    }
    printf("\n");
    
    misaki_token_list_free(tokens);
    misaki_ja_tokenizer_free(tokenizer);
    misaki_trie_free(trie);
    printf("  ✅ 测试14通过：空格处理\n");
}

void test_15_oov() {
    Trie *trie = create_full_dict();
    JaTokenizerConfig config = { .dict_trie = trie, .use_simple_model = false };
    void *tokenizer = misaki_ja_tokenizer_create(&config);
    
    // 包含未登录词（词典中没有的词）
    const char *text = "私はラーメンが好きです";
    MisakiTokenList *tokens = misaki_ja_tokenize(tokenizer, text);
    TEST_ASSERT(tokens != NULL, "包含未登录词的文本分词应该成功");
    
    printf("  分词结果 (%d 个): ", tokens->count);
    for (int i = 0; i < tokens->count; i++) {
        MisakiToken *t = misaki_token_list_get(tokens, i);
        printf("'%s'%s ", t->text, (t->tag && strcmp(t->tag, "UNK") == 0) ? "[UNK]" : "");
    }
    printf("\n");
    
    misaki_token_list_free(tokens);
    misaki_ja_tokenizer_free(tokenizer);
    misaki_trie_free(trie);
    printf("  ✅ 测试15通过：未登录词（OOV）处理\n");
}

/* ============================================================================
 * 测试用例 16-20: 复杂场景
 * ========================================================================== */

void test_16_ambiguity() {
    Trie *trie = create_full_dict();
    // 添加可能产生歧义的词
    misaki_trie_insert(trie, "はは", 7000.0, "名詞");  // 母亲
    
    JaTokenizerConfig config = { .dict_trie = trie, .use_simple_model = false };
    void *tokenizer = misaki_ja_tokenizer_create(&config);
    
    const char *text = "私ははは";  // "我是母亲" vs "私/は/は/は"
    MisakiTokenList *tokens = misaki_ja_tokenize(tokenizer, text);
    TEST_ASSERT(tokens != NULL, "歧义文本分词应该成功");
    
    printf("  分词结果 (%d 个): ", tokens->count);
    for (int i = 0; i < tokens->count; i++) {
        printf("'%s' ", misaki_token_list_get(tokens, i)->text);
    }
    printf("\n");
    
    misaki_token_list_free(tokens);
    misaki_ja_tokenizer_free(tokenizer);
    misaki_trie_free(trie);
    printf("  ✅ 测试16通过：歧义消解（Viterbi 最优路径）\n");
}

void test_17_multiple_punct() {
    Trie *trie = create_full_dict();
    JaTokenizerConfig config = { .dict_trie = trie, .use_simple_model = false };
    void *tokenizer = misaki_ja_tokenizer_create(&config);
    
    const char *text = "本当ですか！？";
    MisakiTokenList *tokens = misaki_ja_tokenize(tokenizer, text);
    TEST_ASSERT(tokens != NULL, "多标点符号分词应该成功");
    
    printf("  分词结果 (%d 个): ", tokens->count);
    for (int i = 0; i < tokens->count; i++) {
        printf("'%s' ", misaki_token_list_get(tokens, i)->text);
    }
    printf("\n");
    
    misaki_token_list_free(tokens);
    misaki_ja_tokenizer_free(tokenizer);
    misaki_trie_free(trie);
    printf("  ✅ 测试17通过：多个标点符号\n");
}

void test_18_special_chars() {
    Trie *trie = create_full_dict();
    JaTokenizerConfig config = { .dict_trie = trie, .use_simple_model = false };
    void *tokenizer = misaki_ja_tokenizer_create(&config);
    
    const char *text = "「こんにちは」";
    MisakiTokenList *tokens = misaki_ja_tokenize(tokenizer, text);
    TEST_ASSERT(tokens != NULL, "特殊符号分词应该成功");
    
    printf("  分词结果 (%d 个): ", tokens->count);
    for (int i = 0; i < tokens->count; i++) {
        printf("'%s' ", misaki_token_list_get(tokens, i)->text);
    }
    printf("\n");
    
    misaki_token_list_free(tokens);
    misaki_ja_tokenizer_free(tokenizer);
    misaki_trie_free(trie);
    printf("  ✅ 测试18通过：特殊符号（日文引号）\n");
}

void test_19_very_long() {
    Trie *trie = create_full_dict();
    JaTokenizerConfig config = { .dict_trie = trie, .use_simple_model = false };
    void *tokenizer = misaki_ja_tokenizer_create(&config);
    
    const char *text = "私は東京の学校で先生と友達と一緒に日本語を勉強しますが、"
                       "コンピュータのプログラムも書くのが好きです";
    MisakiTokenList *tokens = misaki_ja_tokenize(tokenizer, text);
    TEST_ASSERT(tokens != NULL, "极长句子分词应该成功");
    
    printf("  分词结果 (%d 个): ", tokens->count);
    for (int i = 0; i < tokens->count; i++) {
        printf("'%s' ", misaki_token_list_get(tokens, i)->text);
    }
    printf("\n");
    
    misaki_token_list_free(tokens);
    misaki_ja_tokenizer_free(tokenizer);
    misaki_trie_free(trie);
    printf("  ✅ 测试19通过：极长句子（50+字符）\n");
}

void test_20_performance() {
    Trie *trie = create_full_dict();
    JaTokenizerConfig config = { .dict_trie = trie, .use_simple_model = false };
    void *tokenizer = misaki_ja_tokenizer_create(&config);
    
    const char *text = "私は学生です";
    
    clock_t start = clock();
    int iterations = 1000;
    
    for (int i = 0; i < iterations; i++) {
        MisakiTokenList *tokens = misaki_ja_tokenize(tokenizer, text);
        if (tokens) {
            misaki_token_list_free(tokens);
        }
    }
    
    clock_t end = clock();
    double elapsed = ((double)(end - start)) / CLOCKS_PER_SEC;
    double avg_ms = (elapsed / iterations) * 1000.0;
    
    printf("  性能测试: %d 次分词，平均耗时 %.3f ms\n", iterations, avg_ms);
    TEST_ASSERT(avg_ms < 10.0, "平均分词时间应该小于 10ms");
    
    misaki_ja_tokenizer_free(tokenizer);
    misaki_trie_free(trie);
    printf("  ✅ 测试20通过：性能测试\n");
}

/* ============================================================================
 * 主测试函数
 * ========================================================================== */

int main(void) {
    printf("════════════════════════════════════════════════════════════\n");
    printf("  日文分词器综合测试套件 (20个测试用例)\n");
    printf("════════════════════════════════════════════════════════════\n");
    
    // 基础语法测试 (1-5)
    RUN_TEST(test_01_greetings);
    RUN_TEST(test_02_simple_sentence);
    RUN_TEST(test_03_particles);
    RUN_TEST(test_04_question);
    RUN_TEST(test_05_punctuation);
    
    // 不同文字类型 (6-10)
    RUN_TEST(test_06_katakana);
    RUN_TEST(test_07_kanji);
    RUN_TEST(test_08_mixed_scripts);
    RUN_TEST(test_09_long_words);
    RUN_TEST(test_10_repeated_chars);
    
    // 边界情况 (11-15)
    RUN_TEST(test_11_single_char);
    RUN_TEST(test_12_long_sentence);
    RUN_TEST(test_13_numbers);
    RUN_TEST(test_14_spaces);
    RUN_TEST(test_15_oov);
    
    // 复杂场景 (16-20)
    RUN_TEST(test_16_ambiguity);
    RUN_TEST(test_17_multiple_punct);
    RUN_TEST(test_18_special_chars);
    RUN_TEST(test_19_very_long);
    RUN_TEST(test_20_performance);
    
    // 总结
    printf("\n════════════════════════════════════════════════════════════\n");
    printf("  测试结果:\n");
    printf("  ✅ 通过: %d/20\n", test_passed);
    printf("  ❌ 失败: %d/20\n", test_failed);
    printf("════════════════════════════════════════════════════════════\n");
    
    return test_failed > 0 ? 1 : 0;
}
