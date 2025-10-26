/**
 * 中文数字转换扩展功能
 * 特殊格式处理：电话号码、日期、时间、IP地址等
 * 
 * Author: AI Assistant
 * License: MIT
 */

#include "misaki_num2cn.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// 数字字符（用于逐位读）
static const char *DIGIT_CHARS[] = {
    "零", "一", "二", "三", "四", "五", "六", "七", "八", "九"
};

/* ============================================================================
 * 检测函数
 * ========================================================================== */

bool misaki_is_phone_number(const char *str) {
    if (!str || *str == '\0') {
        return false;
    }
    
    int digit_count = 0;
    int hyphen_count = 0;
    bool has_paren = false;
    
    const char *p = str;
    while (*p) {
        if (isdigit(*p)) {
            digit_count++;
        } else if (*p == '-') {
            hyphen_count++;
        } else if (*p == '(' || *p == ')') {
            has_paren = true;
        } else if (*p != ' ') {
            return false;  // 含有其他字符
        }
        p++;
    }
    
    // 电话号码特征：
    // 1. 包含 7-15 位数字
    // 2. 可能有连字符或括号
    return (digit_count >= 7 && digit_count <= 15) && 
           (hyphen_count > 0 || has_paren || digit_count == 11);
}

bool misaki_is_date(const char *str) {
    if (!str || *str == '\0') {
        return false;
    }
    
    // 检测格式：YYYY-MM-DD, YYYY/MM/DD, YYYY年MM月DD日
    int year = 0, month = 0, day = 0;
    
    // 尝试匹配 YYYY-MM-DD
    if (sscanf(str, "%d-%d-%d", &year, &month, &day) == 3) {
        return year >= 1900 && year <= 2100 && month >= 1 && month <= 12 && day >= 1 && day <= 31;
    }
    
    // 尝试匹配 YYYY/MM/DD
    if (sscanf(str, "%d/%d/%d", &year, &month, &day) == 3) {
        return year >= 1900 && year <= 2100 && month >= 1 && month <= 12 && day >= 1 && day <= 31;
    }
    
    // 尝试匹配中文格式（简化检测）
    if (strstr(str, "年") && strstr(str, "月") && strstr(str, "日")) {
        return true;
    }
    
    return false;
}

bool misaki_is_time(const char *str) {
    if (!str || *str == '\0') {
        return false;
    }
    
    // 检测格式：HH:MM 或 HH:MM:SS
    int hour = 0, minute = 0, second = 0;
    
    // 尝试匹配 HH:MM:SS
    if (sscanf(str, "%d:%d:%d", &hour, &minute, &second) == 3) {
        return hour >= 0 && hour <= 23 && minute >= 0 && minute <= 59 && second >= 0 && second <= 59;
    }
    
    // 尝试匹配 HH:MM
    if (sscanf(str, "%d:%d", &hour, &minute) == 2) {
        return hour >= 0 && hour <= 23 && minute >= 0 && minute <= 59;
    }
    
    return false;
}

bool misaki_is_ip_address(const char *str) {
    if (!str || *str == '\0') {
        return false;
    }
    
    // 检测 IPv4 格式：XXX.XXX.XXX.XXX
    int a = 0, b = 0, c = 0, d = 0;
    
    if (sscanf(str, "%d.%d.%d.%d", &a, &b, &c, &d) == 4) {
        return a >= 0 && a <= 255 && b >= 0 && b <= 255 && 
               c >= 0 && c <= 255 && d >= 0 && d <= 255;
    }
    
    return false;
}

bool misaki_is_id_number(const char *str) {
    if (!str || *str == '\0') {
        return false;
    }
    
    int digit_count = 0;
    const char *p = str;
    
    while (*p) {
        if (isdigit(*p)) {
            digit_count++;
        } else if (*p != '-' && *p != ' ') {
            return false;
        }
        p++;
    }
    
    // 身份证号：15位或18位
    return (digit_count == 15 || digit_count == 18);
}

/* ============================================================================
 * 转换函数
 * ========================================================================== */

bool misaki_phone_to_chinese(const char *phone, char *buffer, int buffer_size) {
    if (!phone || !buffer || buffer_size <= 0) {
        return false;
    }
    
    char result[512] = {0};
    int pos = 0;
    const char *p = phone;
    
    while (*p && pos < sizeof(result) - 20) {
        if (isdigit(*p)) {
            int digit = *p - '0';
            
            // 添加数字
            if (pos > 0 && result[pos - 1] != ' ') {
                result[pos++] = ' ';
            }
            
            int len = strlen(DIGIT_CHARS[digit]);
            strcpy(result + pos, DIGIT_CHARS[digit]);
            pos += len;
        } else if (*p == '-' || *p == '(' || *p == ')') {
            // 连字符和括号用空格替代
            if (pos > 0 && result[pos - 1] != ' ') {
                result[pos++] = ' ';
            }
        }
        p++;
    }
    
    result[pos] = '\0';
    snprintf(buffer, buffer_size, "%s", result);
    return true;
}

bool misaki_date_to_chinese(const char *date, char *buffer, int buffer_size) {
    if (!date || !buffer || buffer_size <= 0) {
        return false;
    }
    
    int year = 0, month = 0, day = 0;
    char separator = '-';
    
    // 尝试解析日期
    if (sscanf(date, "%d-%d-%d", &year, &month, &day) == 3) {
        separator = '-';
    } else if (sscanf(date, "%d/%d/%d", &year, &month, &day) == 3) {
        separator = '/';
    } else {
        return false;
    }
    
    char result[256] = {0};
    int pos = 0;
    
    // 年份：逐位读（如 2024 → 二零二四）
    char year_str[16];
    snprintf(year_str, sizeof(year_str), "%d", year);
    for (int i = 0; year_str[i]; i++) {
        if (i > 0) {
            result[pos++] = ' ';
        }
        int digit = year_str[i] - '0';
        int len = strlen(DIGIT_CHARS[digit]);
        strcpy(result + pos, DIGIT_CHARS[digit]);
        pos += len;
    }
    
    strcat(result, "年 ");
    pos = strlen(result);
    
    // 月份：自然读法（如 1月 → 一月，12月 → 十二月）
    if (month == 1) {
        strcat(result, "一月 ");
    } else if (month == 2) {
        strcat(result, "二月 ");
    } else if (month == 3) {
        strcat(result, "三月 ");
    } else if (month == 4) {
        strcat(result, "四月 ");
    } else if (month == 5) {
        strcat(result, "五月 ");
    } else if (month == 6) {
        strcat(result, "六月 ");
    } else if (month == 7) {
        strcat(result, "七月 ");
    } else if (month == 8) {
        strcat(result, "八月 ");
    } else if (month == 9) {
        strcat(result, "九月 ");
    } else if (month == 10) {
        strcat(result, "十月 ");
    } else if (month == 11) {
        strcat(result, "十一月 ");
    } else if (month == 12) {
        strcat(result, "十二月 ");
    }
    pos = strlen(result);
    
    // 日期：自然读法（如 1日 → 一日，15日 → 十五日）
    if (day < 10) {
        strcat(result, DIGIT_CHARS[day]);
        strcat(result, "日");
    } else if (day == 10) {
        strcat(result, "十日");
    } else if (day < 20) {
        strcat(result, "十");
        strcat(result, DIGIT_CHARS[day % 10]);
        strcat(result, "日");
    } else if (day == 20) {
        strcat(result, "二十日");
    } else if (day < 30) {
        strcat(result, "二十");
        strcat(result, DIGIT_CHARS[day % 10]);
        strcat(result, "日");
    } else if (day == 30) {
        strcat(result, "三十日");
    } else {
        strcat(result, "三十");
        strcat(result, DIGIT_CHARS[day % 10]);
        strcat(result, "日");
    }
    
    snprintf(buffer, buffer_size, "%s", result);
    return true;
}

bool misaki_time_to_chinese(const char *time, char *buffer, int buffer_size) {
    if (!time || !buffer || buffer_size <= 0) {
        return false;
    }
    
    int hour = 0, minute = 0, second = -1;
    
    // 尝试解析时间
    if (sscanf(time, "%d:%d:%d", &hour, &minute, &second) < 2) {
        return false;
    }
    
    char result[256] = {0};
    
    // 小时
    if (hour == 0) {
        strcat(result, "零点 ");
    } else if (hour < 10) {
        strcat(result, DIGIT_CHARS[hour]);
        strcat(result, "点 ");
    } else if (hour == 10) {
        strcat(result, "十点 ");
    } else if (hour < 20) {
        strcat(result, "十");
        strcat(result, DIGIT_CHARS[hour % 10]);
        strcat(result, "点 ");
    } else {
        strcat(result, DIGIT_CHARS[hour / 10]);
        strcat(result, "十");
        if (hour % 10 > 0) {
            strcat(result, DIGIT_CHARS[hour % 10]);
        }
        strcat(result, "点 ");
    }
    
    // 分钟
    if (minute == 0) {
        strcat(result, "零分");
    } else if (minute < 10) {
        strcat(result, "零");
        strcat(result, DIGIT_CHARS[minute]);
        strcat(result, "分");
    } else if (minute == 10) {
        strcat(result, "十分");
    } else if (minute < 20) {
        strcat(result, "十");
        strcat(result, DIGIT_CHARS[minute % 10]);
        strcat(result, "分");
    } else {
        strcat(result, DIGIT_CHARS[minute / 10]);
        strcat(result, "十");
        if (minute % 10 > 0) {
            strcat(result, DIGIT_CHARS[minute % 10]);
        }
        strcat(result, "分");
    }
    
    // 秒（如果有）
    if (second >= 0) {
        strcat(result, " ");
        if (second == 0) {
            strcat(result, "零秒");
        } else if (second < 10) {
            strcat(result, "零");
            strcat(result, DIGIT_CHARS[second]);
            strcat(result, "秒");
        } else if (second == 10) {
            strcat(result, "十秒");
        } else if (second < 20) {
            strcat(result, "十");
            strcat(result, DIGIT_CHARS[second % 10]);
            strcat(result, "秒");
        } else {
            strcat(result, DIGIT_CHARS[second / 10]);
            strcat(result, "十");
            if (second % 10 > 0) {
                strcat(result, DIGIT_CHARS[second % 10]);
            }
            strcat(result, "秒");
        }
    }
    
    snprintf(buffer, buffer_size, "%s", result);
    return true;
}

bool misaki_ip_to_chinese(const char *ip, char *buffer, int buffer_size) {
    if (!ip || !buffer || buffer_size <= 0) {
        return false;
    }
    
    char result[512] = {0};
    int pos = 0;
    const char *p = ip;
    
    while (*p && pos < sizeof(result) - 20) {
        if (isdigit(*p)) {
            int digit = *p - '0';
            
            // 添加数字
            if (pos > 0 && result[pos - 1] != ' ') {
                result[pos++] = ' ';
            }
            
            int len = strlen(DIGIT_CHARS[digit]);
            strcpy(result + pos, DIGIT_CHARS[digit]);
            pos += len;
        } else if (*p == '.') {
            // 点
            if (pos > 0 && result[pos - 1] != ' ') {
                result[pos++] = ' ';
            }
            strcpy(result + pos, "点 ");
            pos += strlen("点 ");
        }
        p++;
    }
    
    result[pos] = '\0';
    snprintf(buffer, buffer_size, "%s", result);
    return true;
}
