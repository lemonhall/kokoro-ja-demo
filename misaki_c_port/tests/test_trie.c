/**
 * test_trie.c
 * 
 * 测试 Trie 树模块
 */

#include "misaki_trie.h"
#include "misaki_string.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

// 测试基本插入和查询
void test_basic_insert_lookup() {
    printf("Testing basic insert and lookup...\n");
    
    Trie *trie = misaki_trie_create();
    assert(trie != NULL);
    
    // 插入词语
    assert(misaki_trie_insert(trie, "hello", 10.0, "noun") == true);
    assert(misaki_trie_insert(trie, "world", 8.0, "noun") == true);
    assert(misaki_trie_insert(trie, "help", 5.0, "verb") == true);
    
    // 查询存在的词
    double freq;
    const char *tag;
    
    assert(misaki_trie_lookup(trie, "hello", &freq, &tag) == true);
    assert(freq == 10.0);
    assert(strcmp(tag, "noun") == 0);
    
    assert(misaki_trie_lookup(trie, "world", &freq, &tag) == true);
    assert(freq == 8.0);
    
    // 查询不存在的词
    assert(misaki_trie_lookup(trie, "notfound", NULL, NULL) == false);
    assert(misaki_trie_lookup(trie, "hel", NULL, NULL) == false);  // 前缀不算
    
    // 测试 contains
    assert(misaki_trie_contains(trie, "hello") == true);
    assert(misaki_trie_contains(trie, "help") == true);
    assert(misaki_trie_contains(trie, "hel") == false);
    
    misaki_trie_free(trie);
    
    printf("✓ Basic insert and lookup passed\n");
}

// 测试 UTF-8 中文支持
void test_utf8_chinese() {
    printf("Testing UTF-8 Chinese support...\n");
    
    Trie *trie = misaki_trie_create();
    assert(trie != NULL);
    
    // 插入中文词语
    assert(misaki_trie_insert(trie, "你好", 100.0, "greeting") == true);
    assert(misaki_trie_insert(trie, "世界", 90.0, "noun") == true);
    assert(misaki_trie_insert(trie, "你", 50.0, "pronoun") == true);
    
    // 查询
    double freq;
    const char *tag;
    
    assert(misaki_trie_lookup(trie, "你好", &freq, &tag) == true);
    assert(freq == 100.0);
    assert(strcmp(tag, "greeting") == 0);
    
    assert(misaki_trie_lookup(trie, "你", &freq, NULL) == true);
    assert(freq == 50.0);
    
    // "你好" 和 "你" 都存在（测试前缀重叠）
    assert(misaki_trie_contains(trie, "你好") == true);
    assert(misaki_trie_contains(trie, "你") == true);
    
    misaki_trie_free(trie);
    
    printf("✓ UTF-8 Chinese support passed\n");
}

// 测试前缀匹配（分词核心功能）
void test_prefix_matching() {
    printf("Testing prefix matching...\n");
    
    Trie *trie = misaki_trie_create();
    assert(trie != NULL);
    
    // 插入测试词
    misaki_trie_insert(trie, "中", 10.0, NULL);
    misaki_trie_insert(trie, "中国", 100.0, NULL);
    misaki_trie_insert(trie, "中国人", 80.0, NULL);
    misaki_trie_insert(trie, "国", 5.0, NULL);
    misaki_trie_insert(trie, "国人", 20.0, NULL);
    
    // 测试从 "中国人" 开始的所有匹配
    TrieMatch matches[10];
    int count = misaki_trie_match_all(trie, "中国人很好", 0, matches, 10);
    
    assert(count == 3);  // "中", "中国", "中国人"
    assert(strcmp(matches[0].word, "中") == 0);
    assert(matches[0].length == 3);  // "中" 是 3 字节
    
    assert(strcmp(matches[1].word, "中国") == 0);
    assert(matches[1].length == 6);  // "中国" 是 6 字节
    
    assert(strcmp(matches[2].word, "中国人") == 0);
    assert(matches[2].length == 9);  // "中国人" 是 9 字节
    
    // 测试最长匹配
    TrieMatch longest;
    assert(misaki_trie_match_longest(trie, "中国人很好", 0, &longest) == true);
    assert(strcmp(longest.word, "中国人") == 0);
    
    // 从位置 3 开始（"国人"）
    count = misaki_trie_match_all(trie, "中国人很好", 3, matches, 10);
    assert(count == 2);  // "国", "国人"
    assert(strcmp(matches[0].word, "国") == 0);
    assert(strcmp(matches[1].word, "国人") == 0);
    
    misaki_trie_free(trie);
    
    printf("✓ Prefix matching passed\n");
}

// 测试贪婪匹配（模拟分词）
void test_greedy_matching() {
    printf("Testing greedy matching...\n");
    
    Trie *trie = misaki_trie_create();
    assert(trie != NULL);
    
    // 构建简单词典
    misaki_trie_insert(trie, "我", 50.0, NULL);
    misaki_trie_insert(trie, "爱", 40.0, NULL);
    misaki_trie_insert(trie, "北京", 100.0, NULL);
    misaki_trie_insert(trie, "天安门", 90.0, NULL);
    
    // 贪婪匹配 "我爱北京天安门"
    TrieMatch matches[20];
    int count = misaki_trie_greedy_match(trie, "我爱北京天安门", matches, 20);
    
    assert(count == 4);
    assert(strcmp(matches[0].word, "我") == 0);
    assert(strcmp(matches[1].word, "爱") == 0);
    assert(strcmp(matches[2].word, "北京") == 0);
    assert(strcmp(matches[3].word, "天安门") == 0);
    
    misaki_trie_free(trie);
    
    printf("✓ Greedy matching passed\n");
}

// 测试批量插入
void test_batch_insert() {
    printf("Testing batch insert...\n");
    
    Trie *trie = misaki_trie_create();
    assert(trie != NULL);
    
    const char *words[] = {"apple", "banana", "cherry", "date"};
    double freqs[] = {10.0, 20.0, 15.0, 5.0};
    const char *tags[] = {"fruit", "fruit", "fruit", "fruit"};
    
    int inserted = misaki_trie_insert_batch(trie, words, freqs, tags, 4);
    assert(inserted == 4);
    
    assert(misaki_trie_contains(trie, "apple") == true);
    assert(misaki_trie_contains(trie, "banana") == true);
    assert(misaki_trie_contains(trie, "cherry") == true);
    assert(misaki_trie_contains(trie, "date") == true);
    
    double freq;
    misaki_trie_lookup(trie, "banana", &freq, NULL);
    assert(freq == 20.0);
    
    misaki_trie_free(trie);
    
    printf("✓ Batch insert passed\n");
}

// 测试统计信息
void test_statistics() {
    printf("Testing statistics...\n");
    
    Trie *trie = misaki_trie_create();
    assert(trie != NULL);
    
    // 插入一些词
    misaki_trie_insert(trie, "a", 1.0, NULL);
    misaki_trie_insert(trie, "ab", 1.0, NULL);
    misaki_trie_insert(trie, "abc", 1.0, NULL);
    misaki_trie_insert(trie, "b", 1.0, NULL);
    misaki_trie_insert(trie, "bc", 1.0, NULL);
    
    int total_words, total_nodes, max_depth;
    double avg_depth;
    
    misaki_trie_stats(trie, &total_words, &total_nodes, &avg_depth, &max_depth);
    
    assert(total_words == 5);
    assert(total_nodes > 0);  // 至少有根节点
    assert(max_depth == 3);   // "abc" 的深度
    
    printf("  Total words: %d\n", total_words);
    printf("  Total nodes: %d\n", total_nodes);
    printf("  Average depth: %.2f\n", avg_depth);
    printf("  Max depth: %d\n", max_depth);
    
    misaki_trie_free(trie);
    
    printf("✓ Statistics passed\n");
}

// 测试删除
void test_remove() {
    printf("Testing remove...\n");
    
    Trie *trie = misaki_trie_create();
    assert(trie != NULL);
    
    misaki_trie_insert(trie, "hello", 10.0, NULL);
    misaki_trie_insert(trie, "world", 8.0, NULL);
    
    assert(misaki_trie_contains(trie, "hello") == true);
    assert(misaki_trie_contains(trie, "world") == true);
    
    // 删除 "hello"
    assert(misaki_trie_remove(trie, "hello") == true);
    assert(misaki_trie_contains(trie, "hello") == false);
    assert(misaki_trie_contains(trie, "world") == true);  // "world" 仍存在
    
    // 删除不存在的词
    assert(misaki_trie_remove(trie, "notfound") == false);
    
    misaki_trie_free(trie);
    
    printf("✓ Remove passed\n");
}

// 遍历回调
bool print_word_callback(const char *word, double frequency, const char *tag, void *user_data) {
    int *count = (int *)user_data;
    (*count)++;
    // printf("  [%d] %s (freq=%.1f, tag=%s)\n", *count, word, frequency, tag ? tag : "NULL");
    return true;  // 继续遍历
}

// 测试遍历
void test_traverse() {
    printf("Testing traverse...\n");
    
    Trie *trie = misaki_trie_create();
    assert(trie != NULL);
    
    misaki_trie_insert(trie, "apple", 10.0, "fruit");
    misaki_trie_insert(trie, "apricot", 5.0, "fruit");
    misaki_trie_insert(trie, "banana", 8.0, "fruit");
    
    int count = 0;
    misaki_trie_traverse(trie, print_word_callback, &count);
    assert(count == 3);
    
    // 测试前缀遍历
    count = 0;
    misaki_trie_traverse_prefix(trie, "ap", print_word_callback, &count);
    assert(count == 2);  // "apple" 和 "apricot"
    
    misaki_trie_free(trie);
    
    printf("✓ Traverse passed\n");
}

int main() {
    printf("==============================================\n");
    printf("Misaki Trie Test\n");
    printf("==============================================\n\n");
    
    test_basic_insert_lookup();
    test_utf8_chinese();
    test_prefix_matching();
    test_greedy_matching();
    test_batch_insert();
    test_statistics();
    test_remove();
    test_traverse();
    
    printf("\n==============================================\n");
    printf("All tests passed! ✓\n");
    printf("==============================================\n");
    
    return 0;
}
