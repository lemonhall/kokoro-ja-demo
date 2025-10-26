/**
 * misaki_dict.h
 * 
 * Misaki C Port - Dictionary APIs
 * 词典加载、查询、管理（支持英文/中文/日文）
 * 
 * License: MIT
 */

#ifndef MISAKI_DICT_H
#define MISAKI_DICT_H

#include "misaki_types.h"
#include "misaki_string.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * 英文词典 (English Dictionary)
 * 数据来源: extracted_data/en/us_dict.txt, gb_dict.txt
 * ========================================================================== */

/**
 * 从 TSV 文件加载英文词典
 * 
 * @param file_path TSV 文件路径（格式: word<TAB>phonemes）
 * @return 词典对象，失败返回 NULL
 */
EnDict* misaki_en_dict_load(const char *file_path);

/**
 * 释放英文词典
 * 
 * @param dict 词典对象
 */
void misaki_en_dict_free(EnDict *dict);

/**
 * 查询英文单词的音素
 * 
 * @param dict 词典对象
 * @param word 英文单词（会自动转小写）
 * @return 音素字符串，未找到返回 NULL
 */
const char* misaki_en_dict_lookup(const EnDict *dict, const char *word);

/**
 * 批量查询（用于优化性能）
 * 
 * @param dict 词典对象
 * @param words 单词数组
 * @param count 单词数量
 * @param results 输出：音素数组（与 words 对应）
 * @return 成功查询的数量
 */
int misaki_en_dict_lookup_batch(const EnDict *dict, 
                                 const char **words, 
                                 int count, 
                                 const char **results);

/**
 * 获取词典统计信息
 * 
 * @param dict 词典对象
 * @param total_entries 输出：总词条数
 * @param avg_word_length 输出：平均单词长度
 * @param avg_phoneme_length 输出：平均音素长度
 */
void misaki_en_dict_stats(const EnDict *dict, 
                          int *total_entries,
                          double *avg_word_length,
                          double *avg_phoneme_length);

/* ============================================================================
 * 中文词典 (Chinese Dictionary)
 * 数据来源: extracted_data/zh/pinyin_dict.txt
 * ========================================================================== */

/**
 * 从 TSV 文件加载中文词典
 * 
 * @param file_path TSV 文件路径（格式: 汉字<TAB>拼音）
 * @return 词典对象，失败返回 NULL
 */
ZhDict* misaki_zh_dict_load(const char *file_path);

/**
 * 释放中文词典
 * 
 * @param dict 词典对象
 */
void misaki_zh_dict_free(ZhDict *dict);

/**
 * 查询汉字的拼音（支持多音字）
 * 
 * @param dict 词典对象
 * @param hanzi Unicode 码点（单个汉字）
 * @param pinyins 输出：拼音数组指针
 * @param count 输出：拼音数量
 * @return 成功返回 true，失败返回 false
 */
bool misaki_zh_dict_lookup(const ZhDict *dict, 
                           uint32_t hanzi,
                           const char ***pinyins,
                           int *count);

/**
 * 查询汉字的第一个拼音（最常用读音）
 * 
 * @param dict 词典对象
 * @param hanzi Unicode 码点
 * @return 拼音字符串，未找到返回 NULL
 */
const char* misaki_zh_dict_lookup_first(const ZhDict *dict, uint32_t hanzi);

/**
 * 判断字符是否为汉字
 * 
 * @param codepoint Unicode 码点
 * @return 是返回 true
 */
bool misaki_zh_is_hanzi(uint32_t codepoint);

/**
 * 获取词典统计信息
 * 
 * @param dict 词典对象
 * @param total_hanzi 输出：总汉字数
 * @param total_pinyins 输出：总拼音数（含多音字）
 * @param multi_pinyin_count 输出：多音字数量
 */
void misaki_zh_dict_stats(const ZhDict *dict,
                          int *total_hanzi,
                          int *total_pinyins,
                          int *multi_pinyin_count);

/* ============================================================================
 * 中文词组拼音词典 (Chinese Phrase Pinyin Dictionary)
 * 数据来源: extracted_data/zh/phrase_pinyin.txt
 * 用途：解决多音字上下文选择问题
 * ========================================================================== */

/**
 * 从文本文件加载中文词组拼音词典
 * 
 * @param file_path 文本文件路径（格式：词<Tab>拼音）
 * @return 词组词典对象，失败返回 NULL
 * 
 * 示例数据：
 *   长城<Tab>cháng chéng
 *   长大<Tab>zhǎng dà
 *   银行<Tab>yín háng
 */
ZhPhraseDict* misaki_zh_phrase_dict_load(const char *file_path);

/**
 * 释放词组词典
 * 
 * @param dict 词组词典对象
 */
void misaki_zh_phrase_dict_free(ZhPhraseDict *dict);

/**
 * 查询词组拼音
 * 
 * @param dict 词组词典
 * @param phrase 词组文本（UTF-8）
 * @param pinyins 输出：拼音字符串（空格分隔，如 "cháng chéng"）
 * @return 成功返回 true
 */
bool misaki_zh_phrase_dict_lookup(const ZhPhraseDict *dict,
                                  const char *phrase,
                                  const char **pinyins);

/**
 * 获取词组词典统计信息
 * 
 * @param dict 词组词典对象
 * @return 词组数量
 */
int misaki_zh_phrase_dict_count(const ZhPhraseDict *dict);

/* ============================================================================
 * 日文词汇表 (Japanese Vocabulary)
 * 数据来源: extracted_data/ja/vocab.txt
 * ========================================================================== */

/**
 * 从文本文件加载日文词汇表
 * 
 * @param file_path 文本文件路径（每行一个词）
 * @return 词汇表对象，失败返回 NULL
 */
JaVocab* misaki_ja_vocab_load(const char *file_path);

/**
 * 释放日文词汇表
 * 
 * @param vocab 词汇表对象
 */
void misaki_ja_vocab_free(JaVocab *vocab);

/**
 * 检查词汇是否存在
 * 
 * @param vocab 词汇表对象
 * @param word 日文词汇（UTF-8）
 * @return 存在返回 true
 */
bool misaki_ja_vocab_contains(const JaVocab *vocab, const char *word);

/**
 * 获取词汇表统计信息
 * 
 * @param vocab 词汇表对象
 * @param total_words 输出：总词汇数
 * @param avg_word_length 输出：平均词长（字节）
 */
void misaki_ja_vocab_stats(const JaVocab *vocab,
                           int *total_words,
                           double *avg_word_length);

/* ============================================================================
 * 通用词典工具
 * ========================================================================== */

/**
 * TSV 解析器（内部使用）
 */
typedef struct TSVParser TSVParser;

/**
 * 创建 TSV 解析器
 * 
 * @param file_path 文件路径
 * @return 解析器对象，失败返回 NULL
 */
TSVParser* misaki_tsv_parser_create(const char *file_path);

/**
 * 释放 TSV 解析器
 * 
 * @param parser 解析器对象
 */
void misaki_tsv_parser_free(TSVParser *parser);

/**
 * 读取下一行并分割为字段
 * 
 * @param parser 解析器对象
 * @param fields 输出：字段数组（字符串视图）
 * @param max_fields 最大字段数
 * @return 实际字段数，文件结束返回 0，错误返回 -1
 */
int misaki_tsv_parser_next_line(TSVParser *parser, 
                                MisakiStringView *fields,
                                int max_fields);

/**
 * 获取当前行号
 * 
 * @param parser 解析器对象
 * @return 行号（从 1 开始）
 */
int misaki_tsv_parser_line_number(const TSVParser *parser);

/**
 * 验证 TSV 文件格式
 * 
 * @param file_path 文件路径
 * @param expected_fields 期望的字段数（0 表示不检查）
 * @return 成功返回 true
 */
bool misaki_tsv_validate(const char *file_path, int expected_fields);

/* ============================================================================
 * 词典排序与二分查找（性能优化）
 * ========================================================================== */

/**
 * 对英文词典排序（按单词字典序）
 * 
 * @param dict 词典对象
 */
void misaki_en_dict_sort(EnDict *dict);

/**
 * 对中文词典排序（按 Unicode 码点）
 * 
 * @param dict 词典对象
 */
void misaki_zh_dict_sort(ZhDict *dict);

/**
 * 二分查找英文单词（需先排序）
 * 
 * @param dict 词典对象（已排序）
 * @param word 单词
 * @return 音素字符串，未找到返回 NULL
 */
const char* misaki_en_dict_binary_search(const EnDict *dict, const char *word);

/**
 * 二分查找汉字（需先排序）
 * 
 * @param dict 词典对象（已排序）
 * @param hanzi Unicode 码点
 * @param pinyins 输出：拼音数组指针
 * @param count 输出：拼音数量
 * @return 成功返回 true
 */
bool misaki_zh_dict_binary_search(const ZhDict *dict,
                                  uint32_t hanzi,
                                  const char ***pinyins,
                                  int *count);

/* ============================================================================
 * 词典保存（可选，用于缓存）
 * ========================================================================== */

/**
 * 保存英文词典为 TSV 格式
 * 
 * @param dict 词典对象
 * @param file_path 输出文件路径
 * @return 成功返回 true
 */
bool misaki_en_dict_save(const EnDict *dict, const char *file_path);

/**
 * 保存中文词典为 TSV 格式
 * 
 * @param dict 词典对象
 * @param file_path 输出文件路径
 * @return 成功返回 true
 */
bool misaki_zh_dict_save(const ZhDict *dict, const char *file_path);

/**
 * 保存日文词汇表为文本格式
 * 
 * @param vocab 词汇表对象
 * @param file_path 输出文件路径
 * @return 成功返回 true
 */
bool misaki_ja_vocab_save(const JaVocab *vocab, const char *file_path);

#ifdef __cplusplus
}
#endif

#endif /* MISAKI_DICT_H */
