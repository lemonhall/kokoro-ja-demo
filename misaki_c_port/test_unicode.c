#include <stdio.h>
#include <string.h>

int main() {
    // 测试各种Unicode字符
    const char* test1 = "ʈʂ";  // U+0288 U+0282 (当前C版本用的，两个字符)
    const char* test2 = "\xea\xad\xa7";  // U+AB67 UTF-8编码（单字符）
    const char* test3 = "ꭧ";  // U+AB67 直接写入（如果编译器支持）
    
    printf("Test1 (ʈʂ 两字符): %s (bytes=%zu)\n", test1, strlen(test1));
    printf("Test2 (UTF-8转义): %s (bytes=%zu)\n", test2, strlen(test2));
    printf("Test3 (直接Unicode): %s (bytes=%zu)\n", test3, strlen(test3));
    
    // 测试组合
    const char* combo1 = "ʈʂʰ";  // 当前C版本：3个Unicode字符
    const char* combo2 = "\xea\xad\xa7\xca\xb0";  // U+AB67 + U+02B0
    const char* combo3 = "ꭧʰ";  // 直接写入
    
    printf("\nCombo1 (ʈʂʰ): %s (bytes=%zu)\n", combo1, strlen(combo1));
    printf("Combo2 (UTF-8转义): %s (bytes=%zu)\n", combo2, strlen(combo2));
    printf("Combo3 (直接Unicode): %s (bytes=%zu)\n", combo3, strlen(combo3));
    
    return 0;
}
