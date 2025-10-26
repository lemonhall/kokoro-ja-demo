/**
 * misaki_types.h
 * 
 * Misaki C Port - Core Type Definitions
 * 基于从 Python misaki 提取的数据结构设计
 * 
 * License: MIT
 * Original: https://github.com/hexgrad/misaki
 */

#ifndef MISAKI_TYPES_H
#define MISAKI_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * 错误码定义
 * ========================================================================== */

typedef enum {
    MISAKI_OK = 0,
    MISAKI_ERROR_NULL_POINTER = -1,
    MISAKI_ERROR_OUT_OF_MEMORY = -2,
    MISAKI_ERROR_FILE_NOT_FOUND = -3,
    MISAKI_ERROR_FILE_READ_ERROR = -4,
    MISAKI_ERROR_INVALID_UTF8 = -5,
    MISAKI_ERROR_INVALID_FORMAT = -6,
    MISAKI_ERROR_NOT_FOUND = -7,
    MISAKI_ERROR_UNKNOWN = -99
} MisakiError;

/* ============================================================================
 * 字符串类型（UTF-8 编码）
 * ========================================================================== */

/**
 * 字符串视图（不拥有内存）
 * 用于临时引用字符串片段，避免频繁内存分配
 */
typedef struct {
    const char *data;    // 字符串数据（UTF-8）
    size_t length;       // 字节长度（不是字符数）
} MisakiStringView;

/**
 * 动态字符串（拥有内存）
 * 用于存储需要修改的字符串
 */
typedef struct {
    char *data;          // 字符串数据（UTF-8，以 '\0' 结尾）
    size_t length;       // 字节长度（不含 '\0'）
    size_t capacity;     // 分配的容量
} MisakiString;

/* ============================================================================
 * 词典数据结构（基于提取的 TSV 数据）
 * ========================================================================== */

/**
 * 英文词典条目
 * 
 * 对应数据: en/us_dict.txt, en/gb_dict.txt
 * 格式: word<TAB>IPA_phonemes
 * 示例: "apple\tæpəl"
 */
typedef struct {
    char *word;          // 英文单词（小写，UTF-8）
    char *phonemes;      // IPA 音素序列
} EnDictEntry;

/**
 * 英文词典
 */
typedef struct {
    EnDictEntry *entries;  // 词典条目数组
    int count;             // 条目数量
    int capacity;          // 数组容量
} EnDict;

/**
 * 中文词典条目
 * 
 * 对应数据: zh/pinyin_dict.txt
 * 格式: 汉字<TAB>拼音
 * 示例: "你\tnǐ"
 * 示例: "中\tzhōng,zhòng" （多音字）
 */
typedef struct {
    uint32_t hanzi;      // Unicode 码点（单个汉字）
    char **pinyins;      // 拼音数组（带声调，如 "nǐ"）
    int pinyin_count;    // 拼音数量（多音字有多个）
} ZhDictEntry;

/**
 * 中文词典
 */
typedef struct {
    ZhDictEntry *entries;  // 词典条目数组
    int count;             // 条目数量（约 42K 汉字）
    int capacity;          // 数组容量
} ZhDict;

/**
 * 中文词组拼音词典
 * 
 * 用于解决多音字上下文选择问题
 * 数据来源: pypinyin 的 phrases_dict.json（47,111 词组）
 * 
 * 示例:
 *   词组 "长城" → 拼音 "cháng chéng"
 *   词组 "长大" → 拼音 "zhǎng dà"
 * 
 * 使用 Trie 树存储，查询效率 O(m)，m 为词长
 */
typedef struct {
    struct Trie *phrase_trie;     // 存储词组拼音的 Trie 树（前向声明）
    int count;                     // 词组数量
} ZhPhraseDict;

/**
 * 日文词汇条目
 * 
 * 对应数据: ja/vocab.txt
 * 格式: 每行一个词汇
 * 示例: "こんにちは"
 * 
 * 注意: 日文数据只有词汇列表，不含读音
 *       需要配合 MeCab/UniDic 使用
 */
typedef struct {
    char *word;          // 日文词汇（UTF-8，平假名/片假名/汉字混合）
} JaWordEntry;

/**
 * 日文词汇表
 */
typedef struct {
    JaWordEntry *entries;  // 词汇条目数组
    int count;             // 词汇数量（约 148K）
    int capacity;          // 数组容量
} JaVocab;

/* ============================================================================
 * 分词相关数据结构
 * ========================================================================== */

/**
 * Token: 分词后的词单元
 * 
 * 用于所有语言的分词结果
 */
typedef struct {
    char *text;          // 原始文本（UTF-8）
    char *tag;           // 词性标签（POS tag，如 "n" = 名词）
    char *phonemes;      // 音素序列（IPA 或拼音）
    char *whitespace;    // 后续空白字符
    int start;           // 起始位置（字节偏移）
    int length;          // 长度（字节数）
    double score;        // 置信度分数（用于路径选择）
} MisakiToken;

/**
 * Token 列表
 */
typedef struct {
    MisakiToken *tokens;   // Token 数组
    int count;             // Token 数量
    int capacity;          // 数组容量
} MisakiTokenList;

/* ============================================================================
 * Trie 树（前缀树）数据结构
 * 用于快速词典匹配（jieba、MeCab 核心算法）
 * ========================================================================== */

/**
 * Trie 树节点
 */
typedef struct TrieNode {
    uint32_t codepoint;        // Unicode 码点（字符）
    char *word;                // 完整词（如果是词尾）
    char *pron;                // 读音（片假名，日文专用）
    double frequency;          // 词频（用于路径选择）
    char *tag;                 // 词性标签
    struct TrieNode **children; // 子节点数组
    int children_count;        // 子节点数量
    int children_capacity;     // 子节点容量
    bool is_word;              // 是否为词尾
} TrieNode;

/**
 * Trie 树
 */
typedef struct {
    TrieNode *root;            // 根节点
    int word_count;            // 词汇总数
} Trie;

/* ============================================================================
 * DAG（有向无环图）数据结构
 * 用于 jieba 分词算法
 * ========================================================================== */

/**
 * DAG 节点：表示从某个位置开始的所有可能分词路径
 */
typedef struct {
    int *next_positions;       // 可能的下一个位置数组
    int count;                 // 数组长度
    int capacity;              // 数组容量
} DAGNode;

/**
 * DAG 图
 */
typedef struct {
    DAGNode *nodes;            // 节点数组（索引对应字符位置）
    int length;                // 文本长度（字符数）
    int capacity;              // 数组容量
} DAG;

/* ============================================================================
 * 语言类型枚举
 * ========================================================================== */

typedef enum {
    LANG_ENGLISH,
    LANG_CHINESE,
    LANG_JAPANESE,
    LANG_KOREAN,
    LANG_VIETNAMESE,
    LANG_UNKNOWN
} MisakiLanguage;

/* ============================================================================
 * G2P 上下文
 * 用于保存分词、G2P 转换的全局状态
 * ========================================================================== */

typedef struct {
    // 词典数据
    EnDict *en_dict_us;        // 美式英语词典
    EnDict *en_dict_gb;        // 英式英语词典
    ZhDict *zh_dict;           // 中文拼音词典（单字）
    ZhPhraseDict *zh_phrase_dict;  // 中文词组拼音词典（解决多音字）
    JaVocab *ja_vocab;         // 日文词汇表
    
    // 分词数据
    Trie *zh_trie;             // 中文词典 Trie 树（jieba）
    Trie *ja_trie;             // 日文词典 Trie 树（MeCab）
    
    // 错误信息
    MisakiError last_error;    // 最后一次错误码
    char error_message[256];   // 错误消息
} MisakiContext;

/* ============================================================================
 * 内存分配器（可选，用于自定义内存管理）
 * ========================================================================== */

typedef void* (*MisakiMallocFunc)(size_t size);
typedef void  (*MisakiFreeFunc)(void *ptr);
typedef void* (*MisakiReallocFunc)(void *ptr, size_t new_size);

typedef struct {
    MisakiMallocFunc malloc_func;
    MisakiFreeFunc free_func;
    MisakiReallocFunc realloc_func;
} MisakiAllocator;

/* ============================================================================
 * 辅助宏定义
 * ========================================================================== */

// 最大路径长度
#define MISAKI_MAX_PATH 4096

// 最大词长度（字节）
#define MISAKI_MAX_WORD_LENGTH 256

// 最大音素序列长度
#define MISAKI_MAX_PHONEME_LENGTH 512

// 默认数组初始容量
#define MISAKI_DEFAULT_CAPACITY 16

// UTF-8 字符最大字节数
#define MISAKI_UTF8_MAX_BYTES 4

#ifdef __cplusplus
}
#endif

#endif /* MISAKI_TYPES_H */
