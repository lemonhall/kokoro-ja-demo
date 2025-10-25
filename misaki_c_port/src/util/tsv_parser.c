/**
 * tsv_parser.c
 * 
 * Misaki C Port - TSV (Tab-Separated Values) Parser
 * TSV 文件解析器实现
 * 
 * License: MIT
 */

#include "misaki_dict.h"
#include "misaki_string.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ============================================================================
 * TSV Parser 内部结构
 * ========================================================================== */

struct TSVParser {
    FILE *file;                  // 文件句柄
    char *line_buffer;           // 行缓冲区
    size_t buffer_size;          // 缓冲区大小
    int line_number;             // 当前行号
    bool eof;                    // 是否到达文件末尾
};

#define TSV_INITIAL_BUFFER_SIZE 4096
#define TSV_MAX_LINE_SIZE (1024 * 1024)  // 1MB 最大行长度

/* ============================================================================
 * TSV Parser 实现
 * ========================================================================== */

TSVParser* misaki_tsv_parser_create(const char *file_path) {
    if (!file_path) {
        return NULL;
    }
    
    // 打开文件
    FILE *file = fopen(file_path, "r");
    if (!file) {
        return NULL;
    }
    
    // 创建解析器对象
    TSVParser *parser = (TSVParser *)malloc(sizeof(TSVParser));
    if (!parser) {
        fclose(file);
        return NULL;
    }
    
    // 分配行缓冲区
    parser->line_buffer = (char *)malloc(TSV_INITIAL_BUFFER_SIZE);
    if (!parser->line_buffer) {
        fclose(file);
        free(parser);
        return NULL;
    }
    
    parser->file = file;
    parser->buffer_size = TSV_INITIAL_BUFFER_SIZE;
    parser->line_number = 0;
    parser->eof = false;
    
    return parser;
}

void misaki_tsv_parser_free(TSVParser *parser) {
    if (parser) {
        if (parser->file) {
            fclose(parser->file);
        }
        free(parser->line_buffer);
        free(parser);
    }
}

int misaki_tsv_parser_next_line(TSVParser *parser, 
                                MisakiStringView *fields,
                                int max_fields) {
    if (!parser || !fields || max_fields <= 0 || parser->eof) {
        return -1;
    }
    
    // 读取一行（处理可能的长行）
    size_t pos = 0;
    int ch;
    
    while ((ch = fgetc(parser->file)) != EOF) {
        // 检查换行符
        if (ch == '\n') {
            break;
        }
        
        // 跳过 \r（Windows 换行）
        if (ch == '\r') {
            int next_ch = fgetc(parser->file);
            if (next_ch != '\n' && next_ch != EOF) {
                ungetc(next_ch, parser->file);
            }
            break;
        }
        
        // 扩展缓冲区（如果需要）
        if (pos >= parser->buffer_size - 1) {
            if (parser->buffer_size >= TSV_MAX_LINE_SIZE) {
                // 行太长，放弃
                return -1;
            }
            
            size_t new_size = parser->buffer_size * 2;
            if (new_size > TSV_MAX_LINE_SIZE) {
                new_size = TSV_MAX_LINE_SIZE;
            }
            
            char *new_buffer = (char *)realloc(parser->line_buffer, new_size);
            if (!new_buffer) {
                return -1;
            }
            
            parser->line_buffer = new_buffer;
            parser->buffer_size = new_size;
        }
        
        parser->line_buffer[pos++] = (char)ch;
    }
    
    // 检查 EOF
    if (ch == EOF) {
        if (pos == 0) {
            parser->eof = true;
            return 0;  // 文件结束
        }
    }
    
    parser->line_buffer[pos] = '\0';
    parser->line_number++;
    
    // 分割字段（按 Tab）
    int field_count = 0;
    const char *start = parser->line_buffer;
    const char *p = parser->line_buffer;
    
    while (*p && field_count < max_fields) {
        if (*p == '\t') {
            // 找到一个字段
            fields[field_count].data = start;
            fields[field_count].length = p - start;
            field_count++;
            start = p + 1;
        }
        p++;
    }
    
    // 最后一个字段
    if (field_count < max_fields) {
        fields[field_count].data = start;
        fields[field_count].length = p - start;
        field_count++;
    }
    
    return field_count;
}

int misaki_tsv_parser_line_number(const TSVParser *parser) {
    return parser ? parser->line_number : 0;
}

bool misaki_tsv_validate(const char *file_path, int expected_fields) {
    TSVParser *parser = misaki_tsv_parser_create(file_path);
    if (!parser) {
        return false;
    }
    
    MisakiStringView fields[32];  // 最多 32 个字段
    int max_check = (expected_fields > 0) ? expected_fields : 32;
    bool valid = true;
    
    while (true) {
        int field_count = misaki_tsv_parser_next_line(parser, fields, max_check);
        
        if (field_count == 0) {
            // 文件结束
            break;
        }
        
        if (field_count < 0) {
            // 错误
            valid = false;
            break;
        }
        
        // 检查字段数（如果指定了）
        if (expected_fields > 0 && field_count != expected_fields) {
            valid = false;
            break;
        }
    }
    
    misaki_tsv_parser_free(parser);
    return valid;
}
