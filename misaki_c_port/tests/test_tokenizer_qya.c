/**
 * @file test_tokenizer_qya.c
 * @brief Quenya Tokenizer Tests
 */

#include "misaki_tokenizer_qya.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_RED   "\x1b[31m"
#define ANSI_COLOR_RESET "\x1b[0m"

static void print_tokens(MisakiToken* tokens, int count) {
    printf("  Tokens (%d):\n", count);
    for (int i = 0; i < count; i++) {
        const char* type_str = "UNKNOWN";
        switch (tokens[i].type) {
            case TOKEN_WORD: type_str = "WORD"; break;
            case TOKEN_NUM: type_str = "NUM"; break;
            case TOKEN_PUNCT: type_str = "PUNCT"; break;
            default: break;
        }
        printf("    [%d] %s: \"%s\"\n", i, type_str, tokens[i].text);
    }
}

static void free_tokens(MisakiToken* tokens, int count) {
    for (int i = 0; i < count; i++) {
        free(tokens[i].text);
    }
    free(tokens);
}

void test_tokenize(const char* name, const char* input, int expected_count) {
    printf("\n[Test] %s\n", name);
    printf("  Input: \"%s\"\n", input);
    
    MisakiToken* tokens = NULL;
    int count = 0;
    
    int ret = misaki_tokenize_qya(input, &tokens, &count);
    
    if (ret != 0) {
        printf(ANSI_COLOR_RED "  ✗ FAIL: Tokenization failed\n" ANSI_COLOR_RESET);
        return;
    }
    
    print_tokens(tokens, count);
    
    if (count == expected_count) {
        printf(ANSI_COLOR_GREEN "  ✓ PASS (expected %d tokens)\n" ANSI_COLOR_RESET, expected_count);
    } else {
        printf(ANSI_COLOR_RED "  ✗ FAIL (expected %d, got %d)\n" ANSI_COLOR_RESET, expected_count, count);
    }
    
    free_tokens(tokens, count);
}

int main() {
    printf("=== Quenya Tokenizer Test Suite ===\n");
    
    misaki_tokenizer_qya_init();
    
    // 基础测试
    test_tokenize("Single word", "quenya", 1);
    test_tokenize("Two words", "quenya eldar", 2);
    test_tokenize("With punctuation", "quenya, eldar.", 4);  // quenya + , + eldar + .
    
    // 特殊字符测试
    test_tokenize("With ñ", "ñoldo valar", 2);
    test_tokenize("With accents", "námo mandos", 2);
    
    // 撇号测试
    test_tokenize("With apostrophe", "lúmenn' omentielvo", 2);  // lúmenn' + omentielvo
    
    // 数字测试
    test_tokenize("With numbers", "123 quenya", 2);
    
    // 完整句子测试
    test_tokenize("Complete sentence", "Elen síla lúmenn' omentielvo!", 5);  // Elen + síla + lúmenn' + omentielvo + !
    
    // 空字符串
    test_tokenize("Empty string", "", 0);
    
    // 多空格
    test_tokenize("Multiple spaces", "eldar    valar", 2);
    
    printf("\n" ANSI_COLOR_RESET "=== Tests Complete ===\n");
    
    misaki_tokenizer_qya_cleanup();
    
    return 0;
}
