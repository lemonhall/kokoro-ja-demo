/**
 * test_lookup_debug.c - 调试词典查询
 */

#include "misaki_trie.h"
#include <stdio.h>

int main(void) {
    // 创建 Trie
    Trie *trie = misaki_trie_create();
    
    // 加载词典
    int count = misaki_trie_load_ja_pron_dict(trie, "../extracted_data/ja/ja_pron_dict.tsv");
    printf("加载了 %d 个词汇\n\n", count);
    
    // 测试查询
    const char *test_words[] = {
        "飲んでいる",
        "飲んで",
        "飲む",
        "飲",
        "新しい",
        "私"
    };
    
    for (int i = 0; i < 6; i++) {
        const char *word = test_words[i];
        const char *pron = NULL;
        double freq = 0;
        const char *tag = NULL;
        
        bool found = misaki_trie_lookup_with_pron(trie, word, &pron, &freq, &tag);
        
        printf("[%d] \"%s\": ", i, word);
        if (found) {
            printf("✅ 找到！读音=%s, 频率=%.0f, 词性=%s\n",
                   pron ? pron : "(无)", freq, tag ? tag : "(无)");
        } else {
            printf("❌ 未找到\n");
        }
    }
    
    // 清理
    misaki_trie_free(trie);
    
    return 0;
}
