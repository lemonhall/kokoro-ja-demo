/**
 * test_dict.c
 * 
 * 测试词典加载模块
 */

#include "misaki_dict.h"
#include "misaki_string.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

// 数据文件路径（相对于 build 目录）
#define EN_DICT_PATH "../extracted_data/en/us_dict.txt"
#define ZH_DICT_PATH "../extracted_data/zh/pinyin_dict.txt"
#define JA_VOCAB_PATH "../extracted_data/ja/vocab.txt"

// 测试英文词典加载
void test_en_dict_load() {
    printf("Testing English dictionary loading...\n");
    
    EnDict *dict = misaki_en_dict_load(EN_DICT_PATH);
    
    if (!dict) {
        printf("⚠ Warning: Could not load EN dict (file may not exist yet)\n");
        printf("  Expected path: %s\n", EN_DICT_PATH);
        printf("  Skipping EN dict tests\n");
        return;
    }
    
    assert(dict->count > 0);
    printf("  Loaded %d English words\n", dict->count);
    
    // 获取统计信息
    int total;
    double avg_word_len, avg_phoneme_len;
    misaki_en_dict_stats(dict, &total, &avg_word_len, &avg_phoneme_len);
    
    printf("  Average word length: %.2f\n", avg_word_len);
    printf("  Average phoneme length: %.2f\n", avg_phoneme_len);
    
    misaki_en_dict_free(dict);
    
    printf("✓ English dictionary loading passed\n");
}

// 测试英文词典查询
void test_en_dict_lookup() {
    printf("Testing English dictionary lookup...\n");
    
    // 创建小测试词典
    FILE *f = fopen("test_en.txt", "w");
    assert(f != NULL);
    fprintf(f, "hello\thəlˈoʊ\n");
    fprintf(f, "world\twɜːrld\n");
    fprintf(f, "test\ttɛst\n");
    fclose(f);
    
    EnDict *dict = misaki_en_dict_load("test_en.txt");
    assert(dict != NULL);
    assert(dict->count == 3);
    
    // 查询测试
    const char *phonemes = misaki_en_dict_lookup(dict, "hello");
    assert(phonemes != NULL);
    assert(strcmp(phonemes, "həlˈoʊ") == 0);
    
    // 大小写不敏感
    phonemes = misaki_en_dict_lookup(dict, "HELLO");
    assert(phonemes != NULL);
    
    phonemes = misaki_en_dict_lookup(dict, "World");
    assert(phonemes != NULL);
    
    // 未找到
    phonemes = misaki_en_dict_lookup(dict, "notfound");
    assert(phonemes == NULL);
    
    // 批量查询
    const char *words[] = {"hello", "world", "notfound"};
    const char *results[3];
    int found = misaki_en_dict_lookup_batch(dict, words, 3, results);
    assert(found == 2);  // hello 和 world 找到
    assert(results[0] != NULL);
    assert(results[1] != NULL);
    assert(results[2] == NULL);
    
    misaki_en_dict_free(dict);
    remove("test_en.txt");
    
    printf("✓ English dictionary lookup passed\n");
}

// 测试中文词典加载
void test_zh_dict_load() {
    printf("Testing Chinese dictionary loading...\n");
    
    ZhDict *dict = misaki_zh_dict_load(ZH_DICT_PATH);
    
    if (!dict) {
        printf("⚠ Warning: Could not load ZH dict (file may not exist yet)\n");
        printf("  Expected path: %s\n", ZH_DICT_PATH);
        printf("  Skipping ZH dict tests\n");
        return;
    }
    
    assert(dict->count > 0);
    printf("  Loaded %d Chinese characters\n", dict->count);
    
    // 统计信息
    int total_hanzi, total_pinyins, multi_pinyin;
    misaki_zh_dict_stats(dict, &total_hanzi, &total_pinyins, &multi_pinyin);
    
    printf("  Total hanzi: %d\n", total_hanzi);
    printf("  Total pinyins: %d\n", total_pinyins);
    printf("  Multi-pinyin hanzi: %d\n", multi_pinyin);
    
    misaki_zh_dict_free(dict);
    
    printf("✓ Chinese dictionary loading passed\n");
}

// 测试中文词典查询
void test_zh_dict_lookup() {
    printf("Testing Chinese dictionary lookup...\n");
    
    // 创建测试词典
    FILE *f = fopen("test_zh.txt", "w");
    assert(f != NULL);
    fprintf(f, "你\tnǐ\n");
    fprintf(f, "好\thǎo\n");
    fprintf(f, "中\tzhōng,zhòng\n");  // 多音字
    fclose(f);
    
    ZhDict *dict = misaki_zh_dict_load("test_zh.txt");
    assert(dict != NULL);
    assert(dict->count == 3);
    
    // 查询 "你"
    uint32_t hanzi_ni = 0x4F60;  // "你" 的 Unicode
    const char **pinyins;
    int count;
    
    bool found = misaki_zh_dict_lookup(dict, hanzi_ni, &pinyins, &count);
    assert(found);
    assert(count == 1);
    assert(strcmp(pinyins[0], "nǐ") == 0);
    
    // 查询 "中" （多音字）
    uint32_t hanzi_zhong = 0x4E2D;
    found = misaki_zh_dict_lookup(dict, hanzi_zhong, &pinyins, &count);
    assert(found);
    assert(count == 2);
    assert(strcmp(pinyins[0], "zhōng") == 0);
    assert(strcmp(pinyins[1], "zhòng") == 0);
    
    // 测试 lookup_first
    const char *first_pinyin = misaki_zh_dict_lookup_first(dict, hanzi_zhong);
    assert(first_pinyin != NULL);
    assert(strcmp(first_pinyin, "zhōng") == 0);
    
    // 测试汉字判断
    assert(misaki_zh_is_hanzi(0x4F60) == true);   // 你
    assert(misaki_zh_is_hanzi(0x4E2D) == true);   // 中
    assert(misaki_zh_is_hanzi('a') == false);
    assert(misaki_zh_is_hanzi('1') == false);
    
    misaki_zh_dict_free(dict);
    remove("test_zh.txt");
    
    printf("✓ Chinese dictionary lookup passed\n");
}

// 测试日文词汇表
void test_ja_vocab() {
    printf("Testing Japanese vocabulary...\n");
    
    JaVocab *vocab = misaki_ja_vocab_load(JA_VOCAB_PATH);
    
    if (!vocab) {
        printf("⚠ Warning: Could not load JA vocab (file may not exist yet)\n");
        printf("  Expected path: %s\n", JA_VOCAB_PATH);
        printf("  Skipping JA vocab tests\n");
        return;
    }
    
    assert(vocab->count > 0);
    printf("  Loaded %d Japanese words\n", vocab->count);
    
    // 统计
    int total;
    double avg_len;
    misaki_ja_vocab_stats(vocab, &total, &avg_len);
    printf("  Average word length: %.2f bytes\n", avg_len);
    
    misaki_ja_vocab_free(vocab);
    
    printf("✓ Japanese vocabulary passed\n");
}

// 测试日文词汇查询
void test_ja_vocab_contains() {
    printf("Testing Japanese vocabulary contains...\n");
    
    // 创建测试数据
    FILE *f = fopen("test_ja.txt", "w");
    assert(f != NULL);
    fprintf(f, "こんにちは\n");
    fprintf(f, "ありがとう\n");
    fprintf(f, "さようなら\n");
    fclose(f);
    
    JaVocab *vocab = misaki_ja_vocab_load("test_ja.txt");
    assert(vocab != NULL);
    assert(vocab->count == 3);
    
    // 测试包含
    assert(misaki_ja_vocab_contains(vocab, "こんにちは") == true);
    assert(misaki_ja_vocab_contains(vocab, "ありがとう") == true);
    assert(misaki_ja_vocab_contains(vocab, "notfound") == false);
    
    misaki_ja_vocab_free(vocab);
    remove("test_ja.txt");
    
    printf("✓ Japanese vocabulary contains passed\n");
}

int main() {
    printf("==============================================\n");
    printf("Misaki Dictionary Test\n");
    printf("==============================================\n\n");
    
    // 英文词典测试
    test_en_dict_load();
    test_en_dict_lookup();
    
    // 中文词典测试
    test_zh_dict_load();
    test_zh_dict_lookup();
    
    // 日文词汇测试
    test_ja_vocab();
    test_ja_vocab_contains();
    
    printf("\n==============================================\n");
    printf("All tests passed! ✓\n");
    printf("==============================================\n");
    
    return 0;
}
