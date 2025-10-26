/**
 * 中文数字转换模块
 * 
 * 功能：
 * - 阿拉伯数字转中文读法
 * - 支持整数、小数、负数
 * - 支持货币、百分比等特殊格式
 * 
 * 示例：
 * - 1234 → "一千二百三十四"
 * - 10007 → "一万零七"
 * - 3.14 → "三点一四"
 * - -5 → "负五"
 * - 50% → "百分之五十"
 * 
 * Author: AI Assistant
 * License: MIT
 */

#ifndef MISAKI_NUM2CN_H
#define MISAKI_NUM2CN_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * 将整数转换为中文读法
 * 
 * @param num 整数（支持负数）
 * @param buffer 输出缓冲区
 * @param buffer_size 缓冲区大小
 * @param use_liang 是否使用"两"（true）还是"二"（false）
 * @return 成功返回 true
 */
bool misaki_int_to_chinese(long long num, char *buffer, int buffer_size, bool use_liang);

/**
 * 将小数转换为中文读法
 * 
 * @param num 小数
 * @param buffer 输出缓冲区
 * @param buffer_size 缓冲区大小
 * @param use_liang 是否使用"两"（true）还是"二"（false）
 * @return 成功返回 true
 */
bool misaki_float_to_chinese(double num, char *buffer, int buffer_size, bool use_liang);

/**
 * 将数字字符串转换为中文读法（智能识别类型）
 * 
 * 支持格式：
 * - 整数：123, -456, 1,234,567
 * - 小数：3.14, -2.5
 * - 百分比：50%, 12.5%
 * - 货币：100元, $100, ¥200
 * 
 * @param num_str 数字字符串
 * @param buffer 输出缓冲区
 * @param buffer_size 缓冲区大小
 * @return 成功返回 true
 */
bool misaki_num_string_to_chinese(const char *num_str, char *buffer, int buffer_size);

/**
 * 检测字符串是否为数字（包括带符号、小数点、逗号等）
 * 
 * @param str 字符串
 * @return 是数字返回 true
 */
bool misaki_is_number(const char *str);

/**
 * 在文本中查找并转换所有数字为中文
 * 
 * @param text 输入文本
 * @param output 输出缓冲区
 * @param output_size 输出缓冲区大小
 * @return 成功返回 true
 */
bool misaki_convert_numbers_in_text(const char *text, char *output, int output_size);

#ifdef __cplusplus
}
#endif

#endif // MISAKI_NUM2CN_H
