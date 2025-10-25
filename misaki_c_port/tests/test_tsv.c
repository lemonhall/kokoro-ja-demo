/**
 * test_tsv.c
 * 
 * 测试 TSV Parser 模块
 */

#include "misaki_dict.h"
#include "misaki_string.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// 创建测试数据文件
void create_test_data() {
    // 简单的英文词典测试数据
    FILE *f = fopen("test_dict.tsv", "w");
    assert(f != NULL);
    fprintf(f, "hello\thəlˈoʊ\n");
    fprintf(f, "world\twɜːrld\n");
    fprintf(f, "apple\tæpəl\n");
    fprintf(f, "test\ttɛst\n");
    fclose(f);
    
    // 中文拼音测试数据
    f = fopen("test_pinyin.tsv", "w");
    assert(f != NULL);
    fprintf(f, "你\tnǐ\n");
    fprintf(f, "好\thǎo\n");
    fprintf(f, "世\tshì\n");
    fprintf(f, "界\tjiè\n");
    fprintf(f, "中\tzhōng,zhòng\n");  // 多音字
    fclose(f);
    
    // 错误格式的文件（字段数不一致）
    f = fopen("test_invalid.tsv", "w");
    assert(f != NULL);
    fprintf(f, "one\ttwo\tthree\n");  // 3 个字段
    fprintf(f, "only_one\n");         // 1 个字段
    fclose(f);
}

// 清理测试数据
void cleanup_test_data() {
    remove("test_dict.tsv");
    remove("test_pinyin.tsv");
    remove("test_invalid.tsv");
}

// 测试基本解析
void test_basic_parsing() {
    printf("Testing basic TSV parsing...\n");
    
    TSVParser *parser = misaki_tsv_parser_create("test_dict.tsv");
    assert(parser != NULL);
    
    MisakiStringView fields[10];
    int line_count = 0;
    
    while (true) {
        int field_count = misaki_tsv_parser_next_line(parser, fields, 10);
        
        if (field_count == 0) {
            break;  // 文件结束
        }
        
        assert(field_count == 2);  // 应该有 2 个字段
        line_count++;
        
        // 验证第一行
        if (line_count == 1) {
            assert(misaki_sv_equals_cstr(fields[0], "hello"));
            assert(misaki_sv_equals_cstr(fields[1], "həlˈoʊ"));
        }
    }
    
    assert(line_count == 4);  // 应该有 4 行
    assert(misaki_tsv_parser_line_number(parser) == 4);
    
    misaki_tsv_parser_free(parser);
    
    printf("✓ Basic parsing passed\n");
}

// 测试 UTF-8 支持
void test_utf8_support() {
    printf("Testing UTF-8 support...\n");
    
    TSVParser *parser = misaki_tsv_parser_create("test_pinyin.tsv");
    assert(parser != NULL);
    
    MisakiStringView fields[10];
    int field_count = misaki_tsv_parser_next_line(parser, fields, 10);
    
    assert(field_count == 2);
    
    // 验证中文字符
    assert(fields[0].length == 3);  // "你" 是 3 字节（UTF-8）
    assert(memcmp(fields[0].data, "你", 3) == 0);
    
    // 验证拼音
    assert(misaki_sv_equals_cstr(fields[1], "nǐ"));
    
    misaki_tsv_parser_free(parser);
    
    printf("✓ UTF-8 support passed\n");
}

// 测试多音字（逗号分隔）
void test_multi_values() {
    printf("Testing multi-value fields...\n");
    
    TSVParser *parser = misaki_tsv_parser_create("test_pinyin.tsv");
    assert(parser != NULL);
    
    MisakiStringView fields[10];
    
    // 跳到最后一行（"中"）
    for (int i = 0; i < 5; i++) {
        misaki_tsv_parser_next_line(parser, fields, 10);
    }
    
    // 最后一行应该是 "中\tzhōng,zhòng"
    assert(fields[0].length == 3);  // "中"
    assert(memcmp(fields[0].data, "中", 3) == 0);
    
    // 第二个字段包含逗号分隔的多个值
    char *pinyin = strndup(fields[1].data, fields[1].length);
    assert(strstr(pinyin, "zhōng") != NULL);
    assert(strstr(pinyin, "zhòng") != NULL);
    free(pinyin);
    
    misaki_tsv_parser_free(parser);
    
    printf("✓ Multi-value fields passed\n");
}

// 测试文件验证
void test_validation() {
    printf("Testing file validation...\n");
    
    // 有效文件（2 个字段）
    assert(misaki_tsv_validate("test_dict.tsv", 2) == true);
    assert(misaki_tsv_validate("test_pinyin.tsv", 2) == true);
    
    // 无效文件（字段数不一致）
    assert(misaki_tsv_validate("test_invalid.tsv", 2) == false);
    
    // 不存在的文件
    assert(misaki_tsv_validate("nonexistent.tsv", 2) == false);
    
    printf("✓ File validation passed\n");
}

// 测试空行和边界情况
void test_edge_cases() {
    printf("Testing edge cases...\n");
    
    // 创建包含空行的文件
    FILE *f = fopen("test_empty.tsv", "w");
    assert(f != NULL);
    fprintf(f, "a\tb\n");
    fprintf(f, "\n");  // 空行
    fprintf(f, "c\td\n");
    fclose(f);
    
    TSVParser *parser = misaki_tsv_parser_create("test_empty.tsv");
    assert(parser != NULL);
    
    MisakiStringView fields[10];
    int line_count = 0;
    
    while (true) {
        int field_count = misaki_tsv_parser_next_line(parser, fields, 10);
        if (field_count == 0) break;
        line_count++;
    }
    
    assert(line_count == 3);  // 包括空行
    
    misaki_tsv_parser_free(parser);
    remove("test_empty.tsv");
    
    printf("✓ Edge cases passed\n");
}

// 测试长行处理
void test_long_lines() {
    printf("Testing long line handling...\n");
    
    // 创建包含长行的文件
    FILE *f = fopen("test_long.tsv", "w");
    assert(f != NULL);
    
    // 写一个很长的行（超过初始缓冲区）
    fprintf(f, "word\t");
    for (int i = 0; i < 5000; i++) {
        fprintf(f, "a");
    }
    fprintf(f, "\n");
    fclose(f);
    
    TSVParser *parser = misaki_tsv_parser_create("test_long.tsv");
    assert(parser != NULL);
    
    MisakiStringView fields[10];
    int field_count = misaki_tsv_parser_next_line(parser, fields, 10);
    
    assert(field_count == 2);
    assert(misaki_sv_equals_cstr(fields[0], "word"));
    assert(fields[1].length == 5000);  // 第二个字段应该是 5000 个 'a'
    
    misaki_tsv_parser_free(parser);
    remove("test_long.tsv");
    
    printf("✓ Long line handling passed\n");
}

// 测试字段分割
void test_field_splitting() {
    printf("Testing field splitting...\n");
    
    // 创建多字段文件
    FILE *f = fopen("test_multi.tsv", "w");
    assert(f != NULL);
    fprintf(f, "field1\tfield2\tfield3\tfield4\n");
    fclose(f);
    
    TSVParser *parser = misaki_tsv_parser_create("test_multi.tsv");
    assert(parser != NULL);
    
    MisakiStringView fields[10];
    int field_count = misaki_tsv_parser_next_line(parser, fields, 10);
    
    assert(field_count == 4);
    assert(misaki_sv_equals_cstr(fields[0], "field1"));
    assert(misaki_sv_equals_cstr(fields[1], "field2"));
    assert(misaki_sv_equals_cstr(fields[2], "field3"));
    assert(misaki_sv_equals_cstr(fields[3], "field4"));
    
    misaki_tsv_parser_free(parser);
    remove("test_multi.tsv");
    
    printf("✓ Field splitting passed\n");
}

int main() {
    printf("==============================================\n");
    printf("Misaki TSV Parser Test\n");
    printf("==============================================\n\n");
    
    // 创建测试数据
    create_test_data();
    
    // 运行测试
    test_basic_parsing();
    test_utf8_support();
    test_multi_values();
    test_validation();
    test_edge_cases();
    test_long_lines();
    test_field_splitting();
    
    // 清理测试数据
    cleanup_test_data();
    
    printf("\n==============================================\n");
    printf("All tests passed! ✓\n");
    printf("==============================================\n");
    
    return 0;
}
