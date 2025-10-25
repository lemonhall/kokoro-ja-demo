/**
 * misaki_string.h
 * 
 * Misaki C Port - UTF-8 String Utilities
 * UTF-8 字符串处理工具（零依赖，只用标准库）
 * 
 * License: MIT
 */

#ifndef MISAKI_STRING_H
#define MISAKI_STRING_H

#include "misaki_types.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * UTF-8 字符解码/编码
 * ========================================================================== */

/**
 * 解码一个 UTF-8 字符
 * 
 * @param str UTF-8 字符串指针
 * @param codepoint 输出：Unicode 码点
 * @return 字符的字节数（1-4），失败返回 0
 */
int misaki_utf8_decode(const char *str, uint32_t *codepoint);

/**
 * 编码一个 Unicode 码点为 UTF-8
 * 
 * @param codepoint Unicode 码点
 * @param buffer 输出缓冲区（至少 4 字节）
 * @return 编码的字节数（1-4），失败返回 0
 */
int misaki_utf8_encode(uint32_t codepoint, char *buffer);

/**
 * 获取 UTF-8 字符串的字符数（不是字节数）
 * 
 * @param str UTF-8 字符串
 * @return 字符数
 */
size_t misaki_utf8_length(const char *str);

/**
 * 获取 UTF-8 字符串指定位置的字符
 * 
 * @param str UTF-8 字符串
 * @param index 字符索引（不是字节索引）
 * @param codepoint 输出：Unicode 码点
 * @return 成功返回 true，失败返回 false
 */
bool misaki_utf8_char_at(const char *str, size_t index, uint32_t *codepoint);

/**
 * 验证 UTF-8 字符串是否合法
 * 
 * @param str UTF-8 字符串
 * @return 合法返回 true，非法返回 false
 */
bool misaki_utf8_validate(const char *str);

/* ============================================================================
 * 字符串视图 (MisakiStringView) 操作
 * ========================================================================== */

/**
 * 从 C 字符串创建字符串视图
 * 
 * @param str C 字符串
 * @return 字符串视图
 */
MisakiStringView misaki_sv_from_cstr(const char *str);

/**
 * 从指定长度的字符串创建视图
 * 
 * @param str 字符串指针
 * @param length 长度（字节数）
 * @return 字符串视图
 */
MisakiStringView misaki_sv_from_length(const char *str, size_t length);

/**
 * 比较两个字符串视图是否相等
 * 
 * @param a 字符串视图 A
 * @param b 字符串视图 B
 * @return 相等返回 true
 */
bool misaki_sv_equals(MisakiStringView a, MisakiStringView b);

/**
 * 比较字符串视图与 C 字符串是否相等
 * 
 * @param sv 字符串视图
 * @param str C 字符串
 * @return 相等返回 true
 */
bool misaki_sv_equals_cstr(MisakiStringView sv, const char *str);

/**
 * 字符串视图是否以指定前缀开头
 * 
 * @param sv 字符串视图
 * @param prefix 前缀
 * @return 是返回 true
 */
bool misaki_sv_starts_with(MisakiStringView sv, const char *prefix);

/**
 * 字符串视图是否以指定后缀结尾
 * 
 * @param sv 字符串视图
 * @param suffix 后缀
 * @return 是返回 true
 */
bool misaki_sv_ends_with(MisakiStringView sv, const char *suffix);

/**
 * 裁剪字符串视图（去除首尾空白）
 * 
 * @param sv 字符串视图
 * @return 裁剪后的视图
 */
MisakiStringView misaki_sv_trim(MisakiStringView sv);

/**
 * 分割字符串视图（按分隔符）
 * 
 * @param sv 字符串视图
 * @param delimiter 分隔符
 * @param parts 输出：分割结果数组
 * @param max_parts 最大分割数
 * @return 实际分割的数量
 */
int misaki_sv_split(MisakiStringView sv, char delimiter, 
                    MisakiStringView *parts, int max_parts);

/* ============================================================================
 * 动态字符串 (MisakiString) 操作
 * ========================================================================== */

/**
 * 创建空的动态字符串
 * 
 * @return 动态字符串
 */
MisakiString* misaki_string_new(void);

/**
 * 从 C 字符串创建动态字符串
 * 
 * @param str C 字符串
 * @return 动态字符串
 */
MisakiString* misaki_string_from_cstr(const char *str);

/**
 * 从字符串视图创建动态字符串
 * 
 * @param sv 字符串视图
 * @return 动态字符串
 */
MisakiString* misaki_string_from_sv(MisakiStringView sv);

/**
 * 释放动态字符串
 * 
 * @param str 动态字符串
 */
void misaki_string_free(MisakiString *str);

/**
 * 清空动态字符串
 * 
 * @param str 动态字符串
 */
void misaki_string_clear(MisakiString *str);

/**
 * 追加 C 字符串
 * 
 * @param str 动态字符串
 * @param append 要追加的 C 字符串
 * @return 成功返回 true
 */
bool misaki_string_append_cstr(MisakiString *str, const char *append);

/**
 * 追加字符串视图
 * 
 * @param str 动态字符串
 * @param sv 要追加的字符串视图
 * @return 成功返回 true
 */
bool misaki_string_append_sv(MisakiString *str, MisakiStringView sv);

/**
 * 追加单个字符
 * 
 * @param str 动态字符串
 * @param c 字符
 * @return 成功返回 true
 */
bool misaki_string_append_char(MisakiString *str, char c);

/**
 * 追加 Unicode 码点（自动编码为 UTF-8）
 * 
 * @param str 动态字符串
 * @param codepoint Unicode 码点
 * @return 成功返回 true
 */
bool misaki_string_append_codepoint(MisakiString *str, uint32_t codepoint);

/**
 * 预分配容量
 * 
 * @param str 动态字符串
 * @param capacity 需要的容量
 * @return 成功返回 true
 */
bool misaki_string_reserve(MisakiString *str, size_t capacity);

/**
 * 获取 C 字符串指针（只读）
 * 
 * @param str 动态字符串
 * @return C 字符串指针
 */
const char* misaki_string_cstr(const MisakiString *str);

/* ============================================================================
 * 字符串工具函数
 * ========================================================================== */

/**
 * 字符串复制（安全版）
 * 
 * @param dest 目标缓冲区
 * @param src 源字符串
 * @param dest_size 目标缓冲区大小
 * @return 复制的字节数
 */
size_t misaki_strcpy_safe(char *dest, const char *src, size_t dest_size);

/**
 * 字符串拼接（安全版）
 * 
 * @param dest 目标缓冲区
 * @param src 源字符串
 * @param dest_size 目标缓冲区大小
 * @return 拼接后的总长度
 */
size_t misaki_strcat_safe(char *dest, const char *src, size_t dest_size);

/**
 * 字符串复制（分配新内存）
 * 
 * @param str 源字符串
 * @return 新字符串（需要调用 free 释放）
 */
char* misaki_strdup(const char *str);

/**
 * 字符串小写转换（原地修改）
 * 
 * @param str 字符串
 */
void misaki_strlower(char *str);

/**
 * 字符串大写转换（原地修改）
 * 
 * @param str 字符串
 */
void misaki_strupper(char *str);

/**
 * 去除首尾空白（原地修改）
 * 
 * @param str 字符串
 * @return 裁剪后的字符串指针
 */
char* misaki_strtrim(char *str);

/**
 * 判断字符是否为空白
 * 
 * @param c 字符
 * @return 是返回 true
 */
bool misaki_isspace(char c);

/**
 * 判断字符是否为数字
 * 
 * @param c 字符
 * @return 是返回 true
 */
bool misaki_isdigit(char c);

/**
 * 判断字符是否为字母
 * 
 * @param c 字符
 * @return 是返回 true
 */
bool misaki_isalpha(char c);

#ifdef __cplusplus
}
#endif

#endif /* MISAKI_STRING_H */
