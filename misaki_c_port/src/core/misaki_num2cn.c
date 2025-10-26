/**
 * 中文数字转换实现
 * 
 * Author: AI Assistant
 * License: MIT
 */

#include "misaki_num2cn.h"
#include "misaki_string.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <float.h>  // ⭐ 新增：用于 DBL_EPSILON

// 数字字符
static const char *DIGITS[] = {
    "零", "一", "二", "三", "四", "五", "六", "七", "八", "九"
};

// 单位（小单位）
static const char *UNITS[] = {
    "", "十", "百", "千"
};

// 大单位
static const char *BIG_UNITS[] = {
    "", "万", "亿", "兆"  // 兆 = 10^12
};

/**
 * 将 0-9999 范围内的数字转换为中文
 * 
 * @param num 数字 (0-9999)
 * @param buffer 输出缓冲区
 * @param buffer_size 缓冲区大小
 * @param skip_one 是否跳过首位的"一"（如：一千 vs 千）
 * @param use_liang 是否使用"两"
 */
static void convert_section(int num, char *buffer, int buffer_size, bool skip_one, bool use_liang) {
    if (num == 0) {
        buffer[0] = '\0';
        return;
    }
    
    char result[256] = {0};
    bool has_zero = false;
    int pos = 0;
    
    // ⭐ 安全检查：确保 buffer 足够大
    #define SAFE_APPEND(str) do { \
        int len = strlen(str); \
        if (pos + len >= 256) return; \
        strcpy(result + pos, str); \
        pos += len; \
    } while(0)
    
    // 千位
    int qian = num / 1000;
    if (qian > 0) {
        if (qian == 1 && !skip_one) {
            SAFE_APPEND(DIGITS[1]);
        } else if (qian == 2 && use_liang) {
            SAFE_APPEND("两");
        } else if (qian > 1) {
            SAFE_APPEND(DIGITS[qian]);
        }
        SAFE_APPEND(UNITS[3]);  // 千
        SAFE_APPEND(" ");  // ⭐ 单位后加空格
    }
    
    // 百位
    int bai = (num % 1000) / 100;
    if (bai > 0) {
        if (qian > 0 && bai == 0) {
            has_zero = true;
        }
        if (bai == 2 && use_liang && qian == 0) {
            SAFE_APPEND("两");
        } else {
            SAFE_APPEND(DIGITS[bai]);
        }
        SAFE_APPEND(UNITS[2]);  // 百
        SAFE_APPEND(" ");  // ⭐ 单位后加空格
        has_zero = false;
    } else if (qian > 0 && (num % 100) > 0) {
        has_zero = true;
    }
    
    // 十位
    int shi = (num % 100) / 10;
    if (shi > 0) {
        if (has_zero) {
            SAFE_APPEND(DIGITS[0]);  // 零
            SAFE_APPEND(" ");  // ⭐ 零后加空格
        }
        if (shi == 1 && num < 20 && qian == 0 && bai == 0) {
            // 10-19: "十" 而不是 "一十"
            SAFE_APPEND(UNITS[1]);
            SAFE_APPEND(" ");  // ⭐ 单位后加空格
        } else {
            if (shi == 2 && use_liang && qian == 0 && bai == 0) {
                SAFE_APPEND("两");
            } else {
                SAFE_APPEND(DIGITS[shi]);
            }
            SAFE_APPEND(UNITS[1]);  // 十
            SAFE_APPEND(" ");  // ⭐ 单位后加空格
        }
        has_zero = false;
    } else if ((qian > 0 || bai > 0) && (num % 10) > 0) {
        has_zero = true;
    }
    
    // 个位
    int ge = num % 10;
    if (ge > 0) {
        if (has_zero) {
            SAFE_APPEND(DIGITS[0]);  // 零
            SAFE_APPEND(" ");  // ⭐ 零后加空格
        }
        SAFE_APPEND(DIGITS[ge]);
        // ⭐ 注意：个位数字后不加空格，由调用者决定
    }
    
    #undef SAFE_APPEND
    
    // ⭐ 移除末尾的空格
    while (pos > 0 && result[pos - 1] == ' ') {
        result[--pos] = '\0';
    }
    
    // ⭐ 安全复制到输出缓冲区
    if (buffer_size > 0) {
        strncpy(buffer, result, buffer_size - 1);
        buffer[buffer_size - 1] = '\0';
    }
}

bool misaki_int_to_chinese(long long num, char *buffer, int buffer_size, bool use_liang) {
    if (!buffer || buffer_size <= 0) {
        return false;
    }
    
    // 特殊情况：0
    if (num == 0) {
        snprintf(buffer, buffer_size, "零");
        return true;
    }
    
    // 处理负数
    bool is_negative = false;
    if (num < 0) {
        is_negative = true;
        num = -num;
    }
    
    char result[1024] = {0};
    int pos = 0;
    
    // 添加负号
    if (is_negative) {
        strcpy(result, "负 ");  // ⭐ 添加空格
        pos = strlen(result);
    }
    
    // 分解为各个段（每4位一段）
    int sections[4] = {0};  // 个、万、亿、兆
    int section_count = 0;
    
    long long temp = num;
    while (temp > 0 && section_count < 4) {
        sections[section_count] = temp % 10000;
        temp /= 10000;
        section_count++;
    }
    
    // 从高位到低位转换
    bool need_zero = false;
    for (int i = section_count - 1; i >= 0; i--) {
        if (sections[i] == 0) {
            need_zero = true;
            continue;
        }
        
        // 添加零
        if (need_zero && pos > 0) {
            if (pos + strlen("零 ") < sizeof(result)) {  // ⭐ 零后面也加空格
                strcpy(result + pos, "零 ");
                pos += strlen("零 ");
            }
        }
        need_zero = false;
        
        // 转换当前段
        char section_str[256];
        bool skip_one = (i == section_count - 1 && sections[i] < 2000);
        convert_section(sections[i], section_str, sizeof(section_str), skip_one, use_liang);
        
        if (pos + strlen(section_str) < sizeof(result)) {
            strcpy(result + pos, section_str);
            pos += strlen(section_str);
        }
        
        // ⭐ 添加大单位（并在后面加空格）
        if (i > 0) {
            if (pos + strlen(BIG_UNITS[i]) + 1 < sizeof(result)) {
                strcpy(result + pos, BIG_UNITS[i]);
                pos += strlen(BIG_UNITS[i]);
                result[pos++] = ' ';  // 单位后面加空格
                result[pos] = '\0';
            }
        }
    }
    
    // 移除末尾的空格
    while (pos > 0 && result[pos - 1] == ' ') {
        result[--pos] = '\0';
    }
    
    snprintf(buffer, buffer_size, "%s", result);
    return true;
}

bool misaki_float_to_chinese(double num, char *buffer, int buffer_size, bool use_liang) {
    if (!buffer || buffer_size <= 0) {
        return false;
    }
    
    // 处理负数
    bool is_negative = false;
    if (num < 0) {
        is_negative = true;
        num = -num;
    }
    
    // 分离整数和小数部分
    long long int_part = (long long)num;
    double frac_part = num - int_part;
    
    char result[1024] = {0};
    int pos = 0;
    
    // 添加负号
    if (is_negative) {
        strcpy(result, "负");
        pos = strlen(result);
    }
    
    // 转换整数部分
    char int_str[512];
    misaki_int_to_chinese(int_part, int_str, sizeof(int_str), use_liang);
    
    if (pos + strlen(int_str) < sizeof(result)) {
        strcpy(result + pos, int_str);
        pos += strlen(int_str);
    }
    
    // ⭐ 优化：使用更精确的浮点比较
    // 如果小数部分大于最小精度
    if (frac_part > DBL_EPSILON * 100) {  // 使用 DBL_EPSILON 而不是硬编码
        if (pos + 6 < sizeof(result)) {
            strcpy(result + pos, "点 ");  // ⭐ 点后面加空格
            pos += strlen("点 ");
        }
        
        // 提取小数位（最多6位）
        char frac_str[32];
        snprintf(frac_str, sizeof(frac_str), "%.6f", frac_part);
        
        // 跳过 "0."
        const char *p = strchr(frac_str, '.');
        if (p) {
            p++;  // 跳过小数点
            while (*p && *p != '0') {
                // 逐位转换
                int digit = *p - '0';
                if (digit >= 0 && digit <= 9) {
                    if (pos + strlen(DIGITS[digit]) + 1 < sizeof(result)) {
                        strcpy(result + pos, DIGITS[digit]);
                        pos += strlen(DIGITS[digit]);
                        result[pos++] = ' ';  // ⭐ 每个数字后加空格
                        result[pos] = '\0';
                    }
                }
                p++;
            }
            
            // 移除末尾的零和空格
            while (pos > 0) {
                if (result[pos - 1] == ' ') {
                    result[--pos] = '\0';
                    continue;
                }
                int zero_len = strlen("零");
                if (pos >= zero_len && strcmp(result + pos - zero_len, "零") == 0) {
                    pos -= zero_len;
                    result[pos] = '\0';
                } else {
                    break;
                }
            }
        }
    }
    
    result[pos] = '\0';
    snprintf(buffer, buffer_size, "%s", result);
    return true;
}

bool misaki_is_number(const char *str) {
    if (!str || *str == '\0') {
        return false;
    }
    
    const char *p = str;
    bool has_digit = false;
    bool has_dot = false;
    
    // 跳过开头的符号
    if (*p == '-' || *p == '+' || *p == '¥' || *p == '$') {
        p++;
    }
    
    while (*p) {
        if (isdigit(*p)) {
            has_digit = true;
            p++;
        } else if (*p == '.' && !has_dot) {
            has_dot = true;
            p++;
        } else if (*p == ',' || *p == ' ') {
            // 千位分隔符，跳过
            p++;
        } else if (*p == '%') {
            // 百分号，只能在末尾
            return has_digit && *(p + 1) == '\0';
        } else {
            return false;
        }
    }
    
    return has_digit;
}

bool misaki_num_string_to_chinese(const char *num_str, char *buffer, int buffer_size) {
    if (!num_str || !buffer || buffer_size <= 0) {
        return false;
    }
    
    // 移除千位分隔符和空格
    char cleaned[256] = {0};
    const char *p = num_str;
    int cleaned_pos = 0;
    
    bool has_percent = false;
    bool has_currency = false;
    char currency_symbol[16] = {0};
    
    // 检测货币符号
    if (*p == '¥') {
        has_currency = true;
        strcpy(currency_symbol, "元");
        p++;
    } else if (*p == '$') {
        has_currency = true;
        strcpy(currency_symbol, "美元");
        p++;
    }
    
    // 清理数字字符串
    while (*p && cleaned_pos < 255) {
        if (*p == ',' || *p == ' ') {
            p++;
            continue;
        }
        if (*p == '%') {
            has_percent = true;
            p++;
            break;
        }
        cleaned[cleaned_pos++] = *p++;
    }
    cleaned[cleaned_pos] = '\0';
    
    // 检测中文货币单位
    if (!has_currency) {
        if (strstr(num_str, "元")) {
            has_currency = true;
            strcpy(currency_symbol, "元");
        } else if (strstr(num_str, "角")) {
            has_currency = true;
            strcpy(currency_symbol, "角");
        } else if (strstr(num_str, "分")) {
            has_currency = true;
            strcpy(currency_symbol, "分");
        }
    }
    
    char result[512] = {0};
    
    // 百分比特殊处理
    if (has_percent) {
        strcpy(result, "百分之 ");  // ⭐ 后面加空格
    }
    
    // 判断是整数还是小数
    if (strchr(cleaned, '.')) {
        // 小数
        double num = atof(cleaned);
        char num_cn[256];
        misaki_float_to_chinese(num, num_cn, sizeof(num_cn), true);
        strcat(result, num_cn);
    } else {
        // 整数
        long long num = atoll(cleaned);
        char num_cn[256];
        misaki_int_to_chinese(num, num_cn, sizeof(num_cn), true);
        strcat(result, num_cn);
    }
    
    // ⭐ 添加货币单位（前面加空格）
    if (has_currency) {
        strcat(result, " ");
        strcat(result, currency_symbol);
    }
    
    snprintf(buffer, buffer_size, "%s", result);
    return true;
}

bool misaki_convert_numbers_in_text(const char *text, char *output, int output_size) {
    if (!text || !output || output_size <= 0) {
        return false;
    }
    
    // ⭐ 优化：先计算需要的缓冲区大小，避免固定大小缓冲区溢出
    // 估算最大长度：每个数字可能扩展为 10 倍长度的中文
    int estimated_size = strlen(text) * 10 + 1024;  // 留有余量
    if (estimated_size > output_size) {
        estimated_size = output_size;
    }
    
    char *result = (char*)malloc(estimated_size);
    if (!result) {
        return false;
    }
    
    const char *p = text;
    int pos = 0;
    
    while (*p && pos < estimated_size - 1) {
        // ⭐ 检测特殊格式（优先级：日期 > 时间 > 电话 > IP > 身份证 > 普通数字）
        
        // 提取当前可能的数字字符串
        char candidate[256] = {0};
        int candidate_len = 0;
        const char *temp_p = p;
        
        // 提取完整的数字相关字符串
        while (*temp_p && candidate_len < 255) {
            if (isdigit(*temp_p) || *temp_p == '-' || *temp_p == '/' || *temp_p == ':' || 
                *temp_p == '.' || *temp_p == '(' || *temp_p == ')' || *temp_p == ' ' ||
                *temp_p == ',' || *temp_p == '+' || *temp_p == '%' || *temp_p == '¥' || *temp_p == '$') {
                candidate[candidate_len++] = *temp_p++;
            } else {
                break;
            }
        }
        candidate[candidate_len] = '\0';
        
        // 如果没有提取到数字相关字符，直接复制当前字符
        if (candidate_len == 0) {
            result[pos++] = *p++;
            continue;
        }
        
        // 按优先级检测并转换
        char converted[512] = {0};
        bool converted_success = false;
        
        // 1. 检测身份证号（保持原样）
        if (misaki_is_id_number(candidate)) {
            strcpy(converted, candidate);
            converted_success = true;
        }
        // 2. 检测日期
        else if (misaki_is_date(candidate)) {
            converted_success = misaki_date_to_chinese(candidate, converted, sizeof(converted));
        }
        // 3. 检测时间
        else if (misaki_is_time(candidate)) {
            converted_success = misaki_time_to_chinese(candidate, converted, sizeof(converted));
        }
        // 4. 检测电话号码
        else if (misaki_is_phone_number(candidate)) {
            converted_success = misaki_phone_to_chinese(candidate, converted, sizeof(converted));
        }
        // 5. 检测IP地址
        else if (misaki_is_ip_address(candidate)) {
            converted_success = misaki_ip_to_chinese(candidate, converted, sizeof(converted));
        }
        // 6. 普通数字
        else if (misaki_is_number(candidate)) {
            converted_success = misaki_num_string_to_chinese(candidate, converted, sizeof(converted));
        }
        
        // 如果转换成功，使用转换结果；否则保持原样
        const char *to_append = converted_success ? converted : candidate;
        int append_len = strlen(to_append);
        
        if (pos + append_len < estimated_size - 1) {
            strcpy(result + pos, to_append);
            pos += append_len;
            p = temp_p;  // 移动指针
        } else {
            // ⭐ 缓冲区不够，截断
            break;
        }
    }
    
    result[pos] = '\0';
    
    // ⭐ 安全复制到输出缓冲区
    strncpy(output, result, output_size - 1);
    output[output_size - 1] = '\0';
    
    free(result);
    return true;
}
