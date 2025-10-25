/**
 * misaki_kana_map.h
 * 
 * 假名→IPA 音素映射表
 * 
 * License: MIT
 */

#ifndef MISAKI_KANA_MAP_H
#define MISAKI_KANA_MAP_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * 查找假名的 IPA 映射
 * 
 * @param kana 假名字符串（UTF-8）
 * @param out_ipa 输出：IPA 音素（如果找到）
 * @return 匹配的字节数，0表示未找到
 */
int misaki_kana_to_ipa(const char *kana, const char **out_ipa);

/**
 * 处理特殊假名字符（促音、拨音、长音）
 * 
 * @param kana 当前假名
 * @param next_kana 下一个假名（可为 NULL）
 * @param out_ipa 输出：IPA 音素
 * @return 匹配的字节数，0表示不是特殊字符
 */
int misaki_kana_special(const char *kana, const char *next_kana, const char **out_ipa);

/**
 * 将整个假名字符串转换为 IPA
 * 
 * @param kana_str 假名字符串（UTF-8）
 * @param out_buffer 输出缓冲区
 * @param buffer_size 缓冲区大小
 * @return IPA 音素长度，-1表示错误
 */
int misaki_kana_string_to_ipa(const char *kana_str, char *out_buffer, int buffer_size);

#ifdef __cplusplus
}
#endif

#endif /* MISAKI_KANA_MAP_H */
