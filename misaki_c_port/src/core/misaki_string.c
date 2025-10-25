/**
 * misaki_string.c
 * 
 * Misaki C Port - UTF-8 String Utilities Implementation
 * UTF-8 字符串处理实现
 * 
 * License: MIT
 */

#include "misaki_string.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ============================================================================
 * UTF-8 字符解码/编码
 * ========================================================================== */

/**
 * 解码一个 UTF-8 字符
 * 
 * 返回值：字符的字节数（1-4），失败返回 0
 */
int misaki_utf8_decode(const char *str, uint32_t *codepoint) {
    if (!str || !codepoint) {
        return 0;
    }
    
    const unsigned char *s = (const unsigned char *)str;
    
    // 1 字节（ASCII）: 0xxxxxxx
    if (s[0] <= 0x7F) {
        *codepoint = s[0];
        return 1;
    }
    
    // 2 字节：110xxxxx 10xxxxxx
    if ((s[0] & 0xE0) == 0xC0) {
        if ((s[1] & 0xC0) != 0x80) return 0;  // 无效
        *codepoint = ((s[0] & 0x1F) << 6) | (s[1] & 0x3F);
        return 2;
    }
    
    // 3 字节：1110xxxx 10xxxxxx 10xxxxxx
    if ((s[0] & 0xF0) == 0xE0) {
        if ((s[1] & 0xC0) != 0x80 || (s[2] & 0xC0) != 0x80) return 0;
        *codepoint = ((s[0] & 0x0F) << 12) | ((s[1] & 0x3F) << 6) | (s[2] & 0x3F);
        return 3;
    }
    
    // 4 字节：11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
    if ((s[0] & 0xF8) == 0xF0) {
        if ((s[1] & 0xC0) != 0x80 || (s[2] & 0xC0) != 0x80 || (s[3] & 0xC0) != 0x80) {
            return 0;
        }
        *codepoint = ((s[0] & 0x07) << 18) | ((s[1] & 0x3F) << 12) | 
                     ((s[2] & 0x3F) << 6) | (s[3] & 0x3F);
        return 4;
    }
    
    // 无效的 UTF-8 序列
    return 0;
}

/**
 * 编码一个 Unicode 码点为 UTF-8
 */
int misaki_utf8_encode(uint32_t codepoint, char *buffer) {
    if (!buffer) {
        return 0;
    }
    
    unsigned char *b = (unsigned char *)buffer;
    
    // 1 字节（ASCII）
    if (codepoint <= 0x7F) {
        b[0] = (unsigned char)codepoint;
        return 1;
    }
    
    // 2 字节
    if (codepoint <= 0x7FF) {
        b[0] = 0xC0 | (codepoint >> 6);
        b[1] = 0x80 | (codepoint & 0x3F);
        return 2;
    }
    
    // 3 字节
    if (codepoint <= 0xFFFF) {
        b[0] = 0xE0 | (codepoint >> 12);
        b[1] = 0x80 | ((codepoint >> 6) & 0x3F);
        b[2] = 0x80 | (codepoint & 0x3F);
        return 3;
    }
    
    // 4 字节
    if (codepoint <= 0x10FFFF) {
        b[0] = 0xF0 | (codepoint >> 18);
        b[1] = 0x80 | ((codepoint >> 12) & 0x3F);
        b[2] = 0x80 | ((codepoint >> 6) & 0x3F);
        b[3] = 0x80 | (codepoint & 0x3F);
        return 4;
    }
    
    // 无效的 Unicode 码点
    return 0;
}

/**
 * 获取 UTF-8 字符串的字符数（不是字节数）
 */
size_t misaki_utf8_length(const char *str) {
    if (!str) {
        return 0;
    }
    
    size_t len = 0;
    const char *p = str;
    
    while (*p) {
        uint32_t cp;
        int bytes = misaki_utf8_decode(p, &cp);
        if (bytes == 0) {
            // 无效的 UTF-8，跳过一个字节
            p++;
        } else {
            p += bytes;
        }
        len++;
    }
    
    return len;
}

/**
 * 获取 UTF-8 字符串指定位置的字符
 */
bool misaki_utf8_char_at(const char *str, size_t index, uint32_t *codepoint) {
    if (!str || !codepoint) {
        return false;
    }
    
    size_t current_index = 0;
    const char *p = str;
    
    while (*p) {
        if (current_index == index) {
            int bytes = misaki_utf8_decode(p, codepoint);
            return bytes > 0;
        }
        
        uint32_t cp;
        int bytes = misaki_utf8_decode(p, &cp);
        if (bytes == 0) {
            p++;
        } else {
            p += bytes;
        }
        current_index++;
    }
    
    return false;
}

/**
 * 验证 UTF-8 字符串是否合法
 */
bool misaki_utf8_validate(const char *str) {
    if (!str) {
        return false;
    }
    
    const char *p = str;
    while (*p) {
        uint32_t cp;
        int bytes = misaki_utf8_decode(p, &cp);
        if (bytes == 0) {
            return false;  // 无效的 UTF-8 序列
        }
        p += bytes;
    }
    
    return true;
}

/* ============================================================================
 * 字符串视图 (MisakiStringView) 操作
 * ========================================================================== */

MisakiStringView misaki_sv_from_cstr(const char *str) {
    MisakiStringView sv;
    sv.data = str;
    sv.length = str ? strlen(str) : 0;
    return sv;
}

MisakiStringView misaki_sv_from_length(const char *str, size_t length) {
    MisakiStringView sv;
    sv.data = str;
    sv.length = length;
    return sv;
}

bool misaki_sv_equals(MisakiStringView a, MisakiStringView b) {
    if (a.length != b.length) {
        return false;
    }
    return memcmp(a.data, b.data, a.length) == 0;
}

bool misaki_sv_equals_cstr(MisakiStringView sv, const char *str) {
    if (!str) {
        return sv.length == 0;
    }
    
    size_t str_len = strlen(str);
    if (sv.length != str_len) {
        return false;
    }
    
    return memcmp(sv.data, str, str_len) == 0;
}

bool misaki_sv_starts_with(MisakiStringView sv, const char *prefix) {
    if (!prefix) {
        return true;
    }
    
    size_t prefix_len = strlen(prefix);
    if (sv.length < prefix_len) {
        return false;
    }
    
    return memcmp(sv.data, prefix, prefix_len) == 0;
}

bool misaki_sv_ends_with(MisakiStringView sv, const char *suffix) {
    if (!suffix) {
        return true;
    }
    
    size_t suffix_len = strlen(suffix);
    if (sv.length < suffix_len) {
        return false;
    }
    
    return memcmp(sv.data + sv.length - suffix_len, suffix, suffix_len) == 0;
}

MisakiStringView misaki_sv_trim(MisakiStringView sv) {
    MisakiStringView result = sv;
    
    // 去除开头的空白
    while (result.length > 0 && misaki_isspace(*result.data)) {
        result.data++;
        result.length--;
    }
    
    // 去除结尾的空白
    while (result.length > 0 && misaki_isspace(result.data[result.length - 1])) {
        result.length--;
    }
    
    return result;
}

int misaki_sv_split(MisakiStringView sv, char delimiter, 
                    MisakiStringView *parts, int max_parts) {
    if (!parts || max_parts <= 0) {
        return 0;
    }
    
    int count = 0;
    const char *start = sv.data;
    const char *p = sv.data;
    const char *end = sv.data + sv.length;
    
    while (p < end && count < max_parts - 1) {
        if (*p == delimiter) {
            parts[count].data = start;
            parts[count].length = p - start;
            count++;
            start = p + 1;
        }
        p++;
    }
    
    // 最后一段
    if (count < max_parts) {
        parts[count].data = start;
        parts[count].length = end - start;
        count++;
    }
    
    return count;
}

/* ============================================================================
 * 动态字符串 (MisakiString) 操作
 * ========================================================================== */

MisakiString* misaki_string_new(void) {
    MisakiString *str = (MisakiString *)malloc(sizeof(MisakiString));
    if (!str) {
        return NULL;
    }
    
    str->capacity = MISAKI_DEFAULT_CAPACITY;
    str->data = (char *)malloc(str->capacity);
    if (!str->data) {
        free(str);
        return NULL;
    }
    
    str->data[0] = '\0';
    str->length = 0;
    
    return str;
}

MisakiString* misaki_string_from_cstr(const char *str) {
    MisakiString *s = misaki_string_new();
    if (!s) {
        return NULL;
    }
    
    if (str && !misaki_string_append_cstr(s, str)) {
        misaki_string_free(s);
        return NULL;
    }
    
    return s;
}

MisakiString* misaki_string_from_sv(MisakiStringView sv) {
    MisakiString *s = misaki_string_new();
    if (!s) {
        return NULL;
    }
    
    if (!misaki_string_append_sv(s, sv)) {
        misaki_string_free(s);
        return NULL;
    }
    
    return s;
}

void misaki_string_free(MisakiString *str) {
    if (str) {
        free(str->data);
        free(str);
    }
}

void misaki_string_clear(MisakiString *str) {
    if (str) {
        str->data[0] = '\0';
        str->length = 0;
    }
}

bool misaki_string_reserve(MisakiString *str, size_t capacity) {
    if (!str) {
        return false;
    }
    
    if (capacity <= str->capacity) {
        return true;  // 已有足够容量
    }
    
    char *new_data = (char *)realloc(str->data, capacity);
    if (!new_data) {
        return false;
    }
    
    str->data = new_data;
    str->capacity = capacity;
    
    return true;
}

bool misaki_string_append_cstr(MisakiString *str, const char *append) {
    if (!str || !append) {
        return false;
    }
    
    size_t append_len = strlen(append);
    size_t new_length = str->length + append_len;
    
    // 确保容量足够
    if (new_length + 1 > str->capacity) {
        size_t new_capacity = str->capacity * 2;
        while (new_capacity < new_length + 1) {
            new_capacity *= 2;
        }
        if (!misaki_string_reserve(str, new_capacity)) {
            return false;
        }
    }
    
    memcpy(str->data + str->length, append, append_len);
    str->length = new_length;
    str->data[str->length] = '\0';
    
    return true;
}

bool misaki_string_append_sv(MisakiString *str, MisakiStringView sv) {
    if (!str) {
        return false;
    }
    
    size_t new_length = str->length + sv.length;
    
    if (new_length + 1 > str->capacity) {
        size_t new_capacity = str->capacity * 2;
        while (new_capacity < new_length + 1) {
            new_capacity *= 2;
        }
        if (!misaki_string_reserve(str, new_capacity)) {
            return false;
        }
    }
    
    memcpy(str->data + str->length, sv.data, sv.length);
    str->length = new_length;
    str->data[str->length] = '\0';
    
    return true;
}

bool misaki_string_append_char(MisakiString *str, char c) {
    if (!str) {
        return false;
    }
    
    if (str->length + 2 > str->capacity) {
        if (!misaki_string_reserve(str, str->capacity * 2)) {
            return false;
        }
    }
    
    str->data[str->length++] = c;
    str->data[str->length] = '\0';
    
    return true;
}

bool misaki_string_append_codepoint(MisakiString *str, uint32_t codepoint) {
    if (!str) {
        return false;
    }
    
    char buffer[4];
    int bytes = misaki_utf8_encode(codepoint, buffer);
    if (bytes == 0) {
        return false;
    }
    
    if (str->length + bytes + 1 > str->capacity) {
        if (!misaki_string_reserve(str, str->capacity * 2)) {
            return false;
        }
    }
    
    memcpy(str->data + str->length, buffer, bytes);
    str->length += bytes;
    str->data[str->length] = '\0';
    
    return true;
}

const char* misaki_string_cstr(const MisakiString *str) {
    return str ? str->data : NULL;
}

/* ============================================================================
 * 字符串工具函数
 * ========================================================================== */

size_t misaki_strcpy_safe(char *dest, const char *src, size_t dest_size) {
    if (!dest || !src || dest_size == 0) {
        return 0;
    }
    
    size_t i;
    for (i = 0; i < dest_size - 1 && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }
    dest[i] = '\0';
    
    return i;
}

size_t misaki_strcat_safe(char *dest, const char *src, size_t dest_size) {
    if (!dest || !src || dest_size == 0) {
        return 0;
    }
    
    size_t dest_len = strlen(dest);
    if (dest_len >= dest_size - 1) {
        return dest_len;
    }
    
    size_t i;
    for (i = 0; i < dest_size - dest_len - 1 && src[i] != '\0'; i++) {
        dest[dest_len + i] = src[i];
    }
    dest[dest_len + i] = '\0';
    
    return dest_len + i;
}

char* misaki_strdup(const char *str) {
    if (!str) {
        return NULL;
    }
    
    size_t len = strlen(str);
    char *dup = (char *)malloc(len + 1);
    if (!dup) {
        return NULL;
    }
    
    memcpy(dup, str, len + 1);
    return dup;
}

void misaki_strlower(char *str) {
    if (!str) {
        return;
    }
    
    for (char *p = str; *p; p++) {
        *p = tolower((unsigned char)*p);
    }
}

void misaki_strupper(char *str) {
    if (!str) {
        return;
    }
    
    for (char *p = str; *p; p++) {
        *p = toupper((unsigned char)*p);
    }
}

char* misaki_strtrim(char *str) {
    if (!str) {
        return NULL;
    }
    
    // 去除开头空白
    while (*str && misaki_isspace(*str)) {
        str++;
    }
    
    if (*str == '\0') {
        return str;
    }
    
    // 去除结尾空白
    char *end = str + strlen(str) - 1;
    while (end > str && misaki_isspace(*end)) {
        end--;
    }
    *(end + 1) = '\0';
    
    return str;
}

bool misaki_isspace(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v';
}

bool misaki_isdigit(char c) {
    return c >= '0' && c <= '9';
}

bool misaki_isalpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}
