/**
 * @file test_g2p_qya.c
 * @brief Quenya G2P Tests
 */

#include "misaki_g2p_qya.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_RED   "\x1b[31m"
#define ANSI_COLOR_RESET "\x1b[0m"

static int test_count = 0;
static int pass_count = 0;

void test_case(const char* name, const char* input, const char* expected) {
    test_count++;
    printf("\n[Test %d] %s\n", test_count, name);
    printf("  Input:    \"%s\"\n", input);
    
    char* result = NULL;
    int ret = misaki_g2p_qya_convert(input, &result);
    
    if (ret != 0 || !result) {
        printf(ANSI_COLOR_RED "  ✗ FAIL: Conversion failed\n" ANSI_COLOR_RESET);
        return;
    }
    
    printf("  Output:   \"%s\"\n", result);
    printf("  Expected: \"%s\"\n", expected);
    
    if (strcmp(result, expected) == 0) {
        printf(ANSI_COLOR_GREEN "  ✓ PASS\n" ANSI_COLOR_RESET);
        pass_count++;
    } else {
        printf(ANSI_COLOR_RED "  ✗ FAIL\n" ANSI_COLOR_RESET);
    }
    
    free(result);
}

void test_syllable_count(const char* word, int expected) {
    test_count++;
    int count = misaki_qya_count_syllables(word);
    printf("\n[Test %d] Syllable count: \"%s\"\n", test_count, word);
    printf("  Expected: %d, Got: %d\n", expected, count);
    
    if (count == expected) {
        printf(ANSI_COLOR_GREEN "  ✓ PASS\n" ANSI_COLOR_RESET);
        pass_count++;
    } else {
        printf(ANSI_COLOR_RED "  ✗ FAIL\n" ANSI_COLOR_RESET);
    }
}

int main() {
    printf("=== Quenya G2P Test Suite ===\n");
    
    misaki_g2p_qya_init();
    
    // ========== 基础元音测试 ==========
    printf("\n>>> Basic Vowels\n");
    test_case("Short vowels", "aeiou", "a ɛ i ɔ u");
    
    // ========== 长元音测试 ==========
    printf("\n>>> Long Vowels\n");
    test_case("Long a", "á", "aː");
    test_case("Long e", "é", "eː");
    test_case("Long i", "í", "iː");
    test_case("Long o", "ó", "oː");
    test_case("Long u", "ú", "uː");
    
    // ========== 双元音测试 ==========
    printf("\n>>> Diphthongs\n");
    test_case("ai diphthong", "ai", "aj");
    test_case("au diphthong", "au", "au");
    test_case("iu diphthong", "iu", "iu");
    test_case("eu diphthong", "eu", "ɛu");
    test_case("oi diphthong", "oi", "ɔj");
    test_case("ui diphthong", "ui", "uj");
    
    // ========== 辅音测试 ==========
    printf("\n>>> Consonants\n");
    test_case("qu cluster", "qu", "kw");
    test_case("ng cluster", "ng", "ŋɡ");
    test_case("th digraph", "th", "θ");
    
    // ========== 特殊字符测试 ==========
    printf("\n>>> Special Characters\n");
    test_case("ñ character", "ñ", "ŋ");
    test_case("þ character", "þ", "θ");
    
    // ========== 清化辅音测试 ==========
    printf("\n>>> Voiceless Consonants\n");
    test_case("hl cluster", "hl", "l̥");
    test_case("hr cluster", "hr", "r̥");
    test_case("hw cluster", "hw", "ʍ");
    test_case("hy cluster", "hy", "j̊");
    
    // ========== 腭音化测试 ==========
    printf("\n>>> Palatalized Consonants\n");
    test_case("ty cluster", "ty", "t j");
    test_case("ny cluster", "ny", "n j");
    test_case("ly cluster", "ly", "l j");
    
    // ========== 完整单词测试 ==========
    printf("\n>>> Complete Words\n");
    test_case("Quenya", "quenya", "kw ɛ n j a");
    test_case("Eldar", "eldar", "ɛ l d a r");
    test_case("Valar", "valar", "v a l a r");
    test_case("Ñoldo", "ñoldo", "ŋ ɔ l d ɔ");
    
    // ========== 音节计数测试 ==========
    printf("\n>>> Syllable Counting\n");
    test_syllable_count("a", 1);
    test_syllable_count("ai", 1);
    test_syllable_count("eldar", 2);
    test_syllable_count("quenya", 2);  // qu-e-nya, 'ny' 是辅音簇
    test_syllable_count("eärendil", 4);
    
    // ========== 总结 ==========
    printf("\n" ANSI_COLOR_RESET "=================================\n");
    printf("Tests: %d/%d passed\n", pass_count, test_count);
    
    if (pass_count == test_count) {
        printf(ANSI_COLOR_GREEN "All tests passed! ✓\n" ANSI_COLOR_RESET);
    } else {
        printf(ANSI_COLOR_RED "%d tests failed ✗\n" ANSI_COLOR_RESET, test_count - pass_count);
    }
    
    misaki_g2p_qya_cleanup();
    
    return (pass_count == test_count) ? 0 : 1;
}
