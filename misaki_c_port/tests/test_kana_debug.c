/**
 * test_kana_debug.c
 * 
 * 调试假名转换问题
 */

#include "misaki_kana_map.h"
#include <stdio.h>
#include <string.h>

int main(void) {
    printf("════════════════════════════════════════════════════════════\n");
    printf("  假名转换调试\n");
    printf("════════════════════════════════════════════════════════════\n\n");
    
    // 测试几个完整的词
    const char *test_words[] = {
        "ワタクシ",     // 私的读音
        "ガクセー",     // 学生的读音（带长音）
        "デス",         // です的读音
        "コーヒー",     // 咖啡（多个长音）
        "アリガトウ",   // 谢谢
    };
    
    for (int i = 0; i < sizeof(test_words)/sizeof(test_words[0]); i++) {
        char ipa_buffer[256] = {0};
        int len = misaki_kana_string_to_ipa(test_words[i], ipa_buffer, sizeof(ipa_buffer));
        
        printf("测试 %d:\n", i+1);
        printf("  输入: '%s'\n", test_words[i]);
        printf("  输出: '%s'\n", ipa_buffer);
        printf("  长度: %d\n\n", len);
    }
    
    // 逐字符测试
    printf("\n逐字符测试 'ワタクシ':\n");
    const char *word = "ワタクシ";
    const char *p = word;
    int step = 0;
    
    while (*p) {
        const char *ipa = NULL;
        int matched = misaki_kana_to_ipa(p, &ipa);
        
        printf("  步骤 %d: 位置=%ld, 匹配=%d字节, IPA='%s'\n",
               ++step, p - word, matched, ipa ? ipa : "NULL");
        
        if (matched > 0) {
            p += matched;
        } else {
            p += 3; // 假设是3字节UTF-8
        }
    }
    
    return 0;
}
