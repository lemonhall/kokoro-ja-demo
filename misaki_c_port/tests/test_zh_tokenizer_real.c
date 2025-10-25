/**
 * test_zh_tokenizer_real.c
 * 
 * 测试中文分词器（使用真实词典数据）
 * 
 * License: MIT
 */

#include "misaki_tokenizer.h"
#include "misaki_trie.h"
#include "misaki_dict.h"
#include <stdio.h>
#include <string.h>

int main(void) {
    printf("════════════════════════════════════════════════════════════\n");
    printf("  中文分词器真实测试\n");
    printf("════════════════════════════════════════════════════════════\n\n");
    
    // 从真实词典文件加载中文词典 Trie 树
    const char *dict_path = "../extracted_data/zh/dict.txt";
    printf("📖 加载中文词典: %s\n", dict_path);
    
    Trie *trie = misaki_trie_create();
    if (!trie) {
        printf("❌ Trie 树创建失败\n");
        return 1;
    }
    
    // 加载词典（格式：词<Tab>词频）
    int word_count = misaki_trie_load_from_file(trie, dict_path, "word freq");
    if (word_count < 0) {
        printf("❌ 无法加载词典文件\n");
        printf("提示：请确认文件存在：%s\n", dict_path);
        misaki_trie_free(trie);
        return 1;
    }
    
    printf("✅ 成功加载 %d 个词汇\n\n", word_count);
    
    // 创建分词器
    printf("🔧 创建中文分词器...\n");
    ZhTokenizerConfig config = {
        .dict_trie = trie,
        .enable_hmm = false,
        .enable_userdict = false,
        .user_trie = NULL
    };
    
    void *tokenizer = misaki_zh_tokenizer_create(&config);
    if (!tokenizer) {
        printf("❌ 分词器创建失败\n");
        misaki_trie_free(trie);
        return 1;
    }
    
    printf("✅ 分词器创建成功\n\n");
    
    // 测试句子
    const char *test_sentences[] = {
        "我们是中国人民",
        "北京天安门广场",
        "这个问题很重要",
        "经济发展非常快",
        NULL
    };
    
    printf("════════════════════════════════════════════════════════════\n");
    printf("  分词测试结果\n");
    printf("════════════════════════════════════════════════════════════\n\n");
    
    for (int i = 0; test_sentences[i] != NULL; i++) {
        const char *text = test_sentences[i];
        printf("📝 原句: %s\n", text);
        
        MisakiTokenList *tokens = misaki_zh_tokenize(tokenizer, text);
        if (!tokens) {
            printf("   ❌ 分词失败\n\n");
            continue;
        }
        
        int count = misaki_token_list_size(tokens);
        printf("   分词: ");
        for (int j = 0; j < count; j++) {
            MisakiToken *token = misaki_token_list_get(tokens, j);
            printf("%s", token->text);
            if (j < count - 1) {
                printf(" / ");
            }
        }
        printf(" (%d 词)\n\n", count);
        
        misaki_token_list_free(tokens);
    }
    
    // 清理
    misaki_zh_tokenizer_free(tokenizer);
    misaki_trie_free(trie);
    
    printf("════════════════════════════════════════════════════════════\n");
    printf("  测试完成\n");
    printf("════════════════════════════════════════════════════════════\n");
    
    return 0;
}
