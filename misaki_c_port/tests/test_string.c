/**
 * test_string.c
 * 
 * 测试 misaki_string 模块
 */

#include "misaki_string.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// 测试 UTF-8 解码
void test_utf8_decode() {
    printf("Testing UTF-8 decode...\n");
    
    // ASCII
    uint32_t cp;
    int bytes = misaki_utf8_decode("A", &cp);
    assert(bytes == 1 && cp == 'A');
    
    // 中文（3 字节）
    bytes = misaki_utf8_decode("你", &cp);
    assert(bytes == 3 && cp == 0x4F60);
    
    // 日文（3 字节）
    bytes = misaki_utf8_decode("あ", &cp);
    assert(bytes == 3 && cp == 0x3042);
    
    // Emoji（4 字节）
    bytes = misaki_utf8_decode("😀", &cp);
    assert(bytes == 4 && cp == 0x1F600);
    
    printf("✓ UTF-8 decode passed\n");
}

// 测试 UTF-8 编码
void test_utf8_encode() {
    printf("Testing UTF-8 encode...\n");
    
    char buffer[5] = {0};
    
    // ASCII
    int bytes = misaki_utf8_encode('A', buffer);
    assert(bytes == 1 && buffer[0] == 'A');
    
    // 中文
    bytes = misaki_utf8_encode(0x4F60, buffer);  // '你'
    assert(bytes == 3);
    
    printf("✓ UTF-8 encode passed\n");
}

// 测试字符串长度
void test_utf8_length() {
    printf("Testing UTF-8 length...\n");
    
    assert(misaki_utf8_length("Hello") == 5);
    assert(misaki_utf8_length("你好") == 2);
    assert(misaki_utf8_length("Hello 世界") == 8);
    assert(misaki_utf8_length("") == 0);
    
    printf("✓ UTF-8 length passed\n");
}

// 测试字符串视图
void test_string_view() {
    printf("Testing string view...\n");
    
    MisakiStringView sv = misaki_sv_from_cstr("Hello World");
    assert(sv.length == 11);
    assert(misaki_sv_equals_cstr(sv, "Hello World"));
    assert(misaki_sv_starts_with(sv, "Hello"));
    assert(misaki_sv_ends_with(sv, "World"));
    
    // 测试 trim
    MisakiStringView sv2 = misaki_sv_from_cstr("  Hello  ");
    MisakiStringView trimmed = misaki_sv_trim(sv2);
    assert(trimmed.length == 5);
    
    printf("✓ String view passed\n");
}

// 测试动态字符串
void test_dynamic_string() {
    printf("Testing dynamic string...\n");
    
    MisakiString *s = misaki_string_new();
    assert(s != NULL);
    
    misaki_string_append_cstr(s, "Hello");
    assert(s->length == 5);
    
    misaki_string_append_cstr(s, " ");
    misaki_string_append_cstr(s, "World");
    assert(s->length == 11);
    
    const char *cstr = misaki_string_cstr(s);
    assert(strcmp(cstr, "Hello World") == 0);
    
    misaki_string_free(s);
    
    printf("✓ Dynamic string passed\n");
}

// 测试工具函数
void test_utils() {
    printf("Testing utility functions...\n");
    
    char str[100];
    
    // strcpy_safe
    misaki_strcpy_safe(str, "Hello", sizeof(str));
    assert(strcmp(str, "Hello") == 0);
    
    // strcat_safe
    misaki_strcat_safe(str, " World", sizeof(str));
    assert(strcmp(str, "Hello World") == 0);
    
    // strdup
    char *dup = misaki_strdup("Test");
    assert(strcmp(dup, "Test") == 0);
    free(dup);
    
    // strlower
    strcpy(str, "HELLO");
    misaki_strlower(str);
    assert(strcmp(str, "hello") == 0);
    
    // strupper
    strcpy(str, "world");
    misaki_strupper(str);
    assert(strcmp(str, "WORLD") == 0);
    
    // isspace/isdigit/isalpha
    assert(misaki_isspace(' '));
    assert(misaki_isdigit('5'));
    assert(misaki_isalpha('a'));
    
    printf("✓ Utility functions passed\n");
}

int main() {
    printf("==============================================\n");
    printf("Misaki String Module Test\n");
    printf("==============================================\n\n");
    
    test_utf8_decode();
    test_utf8_encode();
    test_utf8_length();
    test_string_view();
    test_dynamic_string();
    test_utils();
    
    printf("\n==============================================\n");
    printf("All tests passed! ✓\n");
    printf("==============================================\n");
    
    return 0;
}
