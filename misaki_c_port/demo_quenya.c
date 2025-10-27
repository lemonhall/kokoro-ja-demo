/**
 * @file demo_quenya.c
 * @brief Quenya Language G2P Demo
 * 
 * 演示昆雅语（精灵语）的文本到音素转换
 */

#include "misaki_g2p_qya.h"
#include "misaki_tokenizer_qya.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
    printf("=== Quenya (精灵语) G2P Demo ===\n\n");
    
    // 初始化
    misaki_g2p_qya_init();
    misaki_tokenizer_qya_init();
    
    // 测试文本列表
    const char* test_texts[] = {
        "Quenya",
        "Eldar",
        "Valar",
        "Elen síla lúmenn' omentielvo",  // 星光照耀我们相遇之时
        "Namárië",                        // 告别
        "Arda",                           // 世界
        "Ilúvatar",                       // 至高神
        NULL
    };
    
    for (int i = 0; test_texts[i] != NULL; i++) {
        const char* text = test_texts[i];
        
        printf(">>> Input: \"%s\"\n", text);
        
        // 分词
        MisakiToken* tokens = NULL;
        int token_count = 0;
        
        if (misaki_tokenize_qya(text, &tokens, &token_count) == 0) {
            printf("  Tokens (%d): ", token_count);
            for (int j = 0; j < token_count; j++) {
                if (tokens[j].type == TOKEN_WORD) {
                    printf("[%s] ", tokens[j].text);
                }
            }
            printf("\n");
            
            // G2P 转换
            for (int j = 0; j < token_count; j++) {
                if (tokens[j].type == TOKEN_WORD) {
                    char* phonemes = NULL;
                    if (misaki_g2p_qya_convert(tokens[j].text, &phonemes) == 0) {
                        printf("    %s → IPA: /%s/\n", tokens[j].text, phonemes);
                        free(phonemes);
                    }
                }
            }
            
            // 释放内存
            for (int j = 0; j < token_count; j++) {
                free(tokens[j].text);
            }
            free(tokens);
        }
        
        printf("\n");
    }
    
    // 清理
    misaki_g2p_qya_cleanup();
    misaki_tokenizer_qya_cleanup();
    
    return 0;
}
