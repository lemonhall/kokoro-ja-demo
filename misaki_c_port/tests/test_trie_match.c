/**
 * test_trie_match.c - 测试 Trie 匹配功能
 */

#include "misaki_trie.h"
#include <stdio.h>

int main(void) {
    // 创建 Trie
    Trie *trie = misaki_trie_create();
    
    // 加载词典
    int count = misaki_trie_load_ja_pron_dict(trie, "../extracted_data/ja/ja_pron_dict.tsv");
    printf("加载了 %d 个词汇\n\n", count);
    
    // 测试文本
    const char *text = "飲んでいる";
    printf("测试文本: %s\n\n", text);
    
    // 从位置 0 开始匹配
    printf("从位置 0 开始匹配:\n");
    TrieMatch matches[100];
    int match_count = misaki_trie_match_all(trie, text, 0, matches, 100);
    
    printf("找到 %d 个匹配:\n", match_count);
    for (int i = 0; i < match_count; i++) {
        printf("  [%d] '%s' (freq=%.0f, tag=%s)\n",
               i, matches[i].word, matches[i].frequency, matches[i].tag ? matches[i].tag : "NULL");
    }
    
    // 清理
    misaki_trie_free(trie);
    
    return 0;
}
