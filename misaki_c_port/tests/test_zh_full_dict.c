/**
 * test_zh_full_dict.c
 * 
 * 测试大规模中文词典（34.9万词）加载和分词效果
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../include/misaki_tokenizer.h"
#include "../include/misaki_trie.h"

// 测试用例
const char *test_cases[] = {
    "我们是中国人民",
    "经济发展非常快",
    "中国科学技术大学是一所著名的高等学府",
    "人工智能技术正在改变世界",
    "北京天安门广场是中国的象征",
    "机器学习和深度学习是当前热门技术",
    "自然语言处理在语音合成中很重要",
    "今天天气真不错，我们去公园散步吧",
    NULL
};

int main() {
    printf("🚀 测试大规模中文词典加载\n\n");
    
    // 1. 加载词典到 Trie 树
    printf("📖 加载词典...\n");
    clock_t start = clock();
    
    const char *dict_path = "../extracted_data/zh/dict_full.txt";
    FILE *f = fopen(dict_path, "r");
    if (!f) {
        printf("❌ 无法打开词典文件: %s\n", dict_path);
        return 1;
    }
    
    Trie *trie = misaki_trie_create();
    if (!trie) {
        printf("❌ 创建 Trie 树失败\n");
        fclose(f);
        return 1;
    }
    
    int word_count = 0;
    char line[512];
    while (fgets(line, sizeof(line), f)) {
        // 移除换行符
        line[strcspn(line, "\n")] = 0;
        
        // 分割：词<Tab>词频
        char *tab = strchr(line, '\t');
        if (!tab) continue;
        
        *tab = '\0';
        char *word = line;
        int freq = atoi(tab + 1);
        
        // 插入 Trie
        if (misaki_trie_insert(trie, word, (double)freq, "n")) {
            word_count++;
        }
        
        // 每 10 万词汇打印进度
        if (word_count % 100000 == 0) {
            printf("  已加载 %d 词...\n", word_count);
        }
    }
    fclose(f);
    
    clock_t end = clock();
    double load_time = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("✅ 词典加载完成！\n");
    printf("  - 词汇数量: %d\n", word_count);
    printf("  - 加载时间: %.2f 秒\n\n", load_time);
    
    // 2. 创建分词器
    printf("🔧 创建中文分词器...\n");
    ZhTokenizerConfig config = {
        .dict_trie = trie,
        .user_trie = NULL,
        .enable_hmm = false,
        .enable_userdict = false
    };
    
    void *tokenizer = misaki_zh_tokenizer_create(&config);
    if (!tokenizer) {
        printf("❌ 创建分词器失败\n");
        misaki_trie_free(trie);
        return 1;
    }
    
    printf("✅ 分词器创建成功\n\n");
    
    // 3. 测试分词
    printf("📝 测试分词效果:\n");
    printf("𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠\n\n");
    
    for (int i = 0; test_cases[i] != NULL; i++) {
        const char *text = test_cases[i];
        
        printf("输入: %s\n", text);
        
        start = clock();
        MisakiTokenList *tokens = misaki_zh_tokenize(tokenizer, text);
        end = clock();
        double seg_time = (double)(end - start) / CLOCKS_PER_SEC * 1000;
        
        if (!tokens) {
            printf("❌ 分词失败\n\n");
            continue;
        }
        
        printf("输出: ");
        for (int j = 0; j < tokens->count; j++) {
            if (j > 0) printf(" / ");
            printf("%s", tokens->tokens[j].text);
        }
        printf("\n");
        printf("Token数: %d, 耗时: %.2f ms\n\n", tokens->count, seg_time);
        
        misaki_token_list_free(tokens);
    }
    
    // 4. 性能测试
    printf("\n⚡ 性能测试:\n");
    printf("𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠𝆠\n");
    
    const char *perf_text = "中国科学技术大学是一所著名的高等学府，位于安徽省合肥市。"
                           "学校创建于1958年，是中国科学院所属的一所重点大学。";
    
    int iterations = 1000;
    start = clock();
    for (int i = 0; i < iterations; i++) {
        MisakiTokenList *tokens = misaki_zh_tokenize(tokenizer, perf_text);
        misaki_token_list_free(tokens);
    }
    end = clock();
    double total_time = (double)(end - start) / CLOCKS_PER_SEC;
    double avg_time = total_time / iterations * 1000;
    
    printf("测试文本: %s\n", perf_text);
    printf("迭代次数: %d\n", iterations);
    printf("总耗时: %.2f 秒\n", total_time);
    printf("平均耗时: %.2f ms/次\n", avg_time);
    printf("吞吐量: %.0f 次/秒\n", iterations / total_time);
    
    // 5. 清理
    misaki_zh_tokenizer_free(tokenizer);
    misaki_trie_free(trie);
    
    printf("\n✨ 测试完成！\n");
    return 0;
}
