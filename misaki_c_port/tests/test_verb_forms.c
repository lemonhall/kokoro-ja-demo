#include "misaki_tokenizer.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    const char *text = (argc > 1) ? argv[1] : "彼は新聞を読みながら、コーヒーを飲んでいる。";
    
    printf("\n测试文本: %s\n\n", text);
    
    // 加载词典
    Trie *trie = misaki_trie_create();
    int count = misaki_trie_load_ja_pron_dict(trie, "../extracted_data/ja/ja_pron_dict.tsv");
    printf("加载了 %d 个词汇\n\n", count);
    
    // 配置分词器
    JaTokenizerConfig config = {
        .dict_trie = trie,
        .use_simple_model = false  // 使用 Viterbi 模式
    };
    
    void *tokenizer = misaki_ja_tokenizer_create(&config);
    if (!tokenizer) {
        fprintf(stderr, "分词器创建失败\n");
        return 1;
    }
    
    // 分词
    MisakiTokenList *result = misaki_ja_tokenize(tokenizer, text);
    if (!result) {
        fprintf(stderr, "分词失败\n");
        return 1;
    }
    
    printf("分词结果: %d 个词\n", result->count);
    for (int i = 0; i < result->count; i++) {
        MisakiToken *token = misaki_token_list_get(result, i);
        printf("  [%d] '%s'\n", i, token->text);
    }
    
    // 清理
    misaki_token_list_free(result);
    misaki_ja_tokenizer_free(tokenizer);
    misaki_trie_free(trie);
    
    return 0;
}
