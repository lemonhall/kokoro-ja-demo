#include "include/misaki_tokenizer.h"
#include "include/misaki_trie.h"
#include <stdio.h>

int main() {
    Trie *trie = misaki_trie_create();
    misaki_trie_insert(trie, "こんにちは", 1000.0, NULL);
    misaki_trie_insert(trie, "元気", 900.0, NULL);
    misaki_trie_insert(trie, "ですか", 800.0, NULL);
    
    JaTokenizerConfig config = {
        .dict_trie = trie,
        .use_simple_model = false
    };
    
    void *tokenizer = misaki_ja_tokenizer_create(&config);
    const char *text = "こんにちは、元気ですか？";
    
    printf("输入: %s\n", text);
    MisakiTokenList *tokens = misaki_ja_tokenize(tokenizer, text);
    
    if (tokens) {
        printf("成功: %d 个词\n", tokens->count);
        for (int i = 0; i < tokens->count; i++) {
            printf("  [%d] %s\n", i, tokens->tokens[i].text);
        }
    } else {
        printf("失败: NULL\n");
    }
    
    return 0;
}
