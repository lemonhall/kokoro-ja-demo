/**
 * misaki_context.h
 * 
 * Misaki C Port - Context Management
 * Misaki 上下文管理（初始化、配置、资源管理）
 * 
 * License: MIT
 */

#ifndef MISAKI_CONTEXT_H
#define MISAKI_CONTEXT_H

#include "misaki_types.h"
#include "misaki_dict.h"
#include "misaki_trie.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * 上下文初始化配置
 * ========================================================================== */

/**
 * Misaki 初始化配置
 */
typedef struct {
    // 数据目录
    const char *data_dir;          // 数据根目录（extracted_data/）
    
    // 启用的语言
    bool enable_english;           // 英文支持（默认 true）
    bool enable_chinese;           // 中文支持（默认 false）
    bool enable_japanese;          // 日文支持（默认 false）
    bool enable_korean;            // 韩文支持（默认 false）
    bool enable_vietnamese;        // 越南文支持（默认 false）
    
    // 英文配置
    bool en_load_us_dict;          // 加载美式词典（默认 true）
    bool en_load_gb_dict;          // 加载英式词典（默认 false）
    
    // 中文配置
    const char *zh_dict_path;      // 中文词典路径（默认 NULL，自动查找）
    const char *zh_jieba_dict_path; // jieba 词典路径（默认 NULL）
    bool zh_enable_hmm;            // 启用 HMM（默认 true）
    
    // 日文配置
    const char *ja_vocab_path;     // 日文词汇表路径（默认 NULL）
    const char *ja_unidic_path;    // UniDic 路径（默认 NULL）
    
    // 内存分配器（可选）
    MisakiAllocator *allocator;    // 自定义内存分配器（默认 NULL）
    
    // 日志配置
    bool enable_logging;           // 启用日志（默认 false）
    int log_level;                 // 日志级别（0-4）
} MisakiConfig;

/**
 * 获取默认配置
 * 
 * @return 默认配置
 */
MisakiConfig misaki_default_config(void);

/* ============================================================================
 * 上下文生命周期
 * ========================================================================== */

/**
 * 创建 Misaki 上下文
 * 
 * @param config 初始化配置
 * @return 上下文对象，失败返回 NULL
 */
MisakiContext* misaki_context_create(const MisakiConfig *config);

/**
 * 释放 Misaki 上下文
 * 
 * @param context 上下文对象
 */
void misaki_context_free(MisakiContext *context);

/**
 * 重新加载词典
 * 
 * @param context 上下文对象
 * @param lang 语言类型
 * @return 成功返回 true
 */
bool misaki_context_reload_dict(MisakiContext *context, MisakiLanguage lang);

/**
 * 清空缓存
 * 
 * @param context 上下文对象
 */
void misaki_context_clear_cache(MisakiContext *context);

/* ============================================================================
 * 上下文查询
 * ========================================================================== */

/**
 * 检查语言是否已启用
 * 
 * @param context 上下文对象
 * @param lang 语言类型
 * @return 已启用返回 true
 */
bool misaki_context_is_enabled(const MisakiContext *context, MisakiLanguage lang);

/**
 * 获取英文词典
 * 
 * @param context 上下文对象
 * @param use_gb 是否使用英式词典（默认美式）
 * @return 词典对象，未加载返回 NULL
 */
EnDict* misaki_context_get_en_dict(const MisakiContext *context, bool use_gb);

/**
 * 获取中文词典
 * 
 * @param context 上下文对象
 * @return 词典对象，未加载返回 NULL
 */
ZhDict* misaki_context_get_zh_dict(const MisakiContext *context);

/**
 * 获取日文词汇表
 * 
 * @param context 上下文对象
 * @return 词汇表对象，未加载返回 NULL
 */
JaVocab* misaki_context_get_ja_vocab(const MisakiContext *context);

/**
 * 获取中文 Trie 树
 * 
 * @param context 上下文对象
 * @return Trie 树对象，未加载返回 NULL
 */
Trie* misaki_context_get_zh_trie(const MisakiContext *context);

/**
 * 获取日文 Trie 树
 * 
 * @param context 上下文对象
 * @return Trie 树对象，未加载返回 NULL
 */
Trie* misaki_context_get_ja_trie(const MisakiContext *context);

/* ============================================================================
 * 错误处理
 * ========================================================================== */

/**
 * 获取最后一次错误码
 * 
 * @param context 上下文对象
 * @return 错误码
 */
MisakiError misaki_context_get_error(const MisakiContext *context);

/**
 * 获取最后一次错误消息
 * 
 * @param context 上下文对象
 * @return 错误消息字符串
 */
const char* misaki_context_get_error_message(const MisakiContext *context);

/**
 * 清除错误状态
 * 
 * @param context 上下文对象
 */
void misaki_context_clear_error(MisakiContext *context);

/**
 * 设置错误状态
 * 
 * @param context 上下文对象
 * @param error 错误码
 * @param message 错误消息（可为 NULL）
 */
void misaki_context_set_error(MisakiContext *context,
                              MisakiError error,
                              const char *message);

/* ============================================================================
 * 用户词典管理
 * ========================================================================== */

/**
 * 添加用户词（中文）
 * 
 * @param context 上下文对象
 * @param word 词汇
 * @param frequency 词频（默认 1000.0）
 * @param tag 词性（可为 NULL）
 * @return 成功返回 true
 */
bool misaki_context_add_zh_word(MisakiContext *context,
                                 const char *word,
                                 double frequency,
                                 const char *tag);

/**
 * 移除用户词（中文）
 * 
 * @param context 上下文对象
 * @param word 词汇
 * @return 成功返回 true
 */
bool misaki_context_remove_zh_word(MisakiContext *context, const char *word);

/**
 * 批量加载用户词典
 * 
 * @param context 上下文对象
 * @param file_path 词典文件路径
 * @param lang 语言类型
 * @return 成功加载的词汇数量，失败返回 -1
 */
int misaki_context_load_user_dict(MisakiContext *context,
                                   const char *file_path,
                                   MisakiLanguage lang);

/**
 * 保存用户词典
 * 
 * @param context 上下文对象
 * @param file_path 输出文件路径
 * @param lang 语言类型
 * @return 成功返回 true
 */
bool misaki_context_save_user_dict(MisakiContext *context,
                                    const char *file_path,
                                    MisakiLanguage lang);

/* ============================================================================
 * 统计信息
 * ========================================================================== */

/**
 * 上下文统计信息
 */
typedef struct {
    // 词典统计
    int en_dict_us_count;          // 美式英语词典条目数
    int en_dict_gb_count;          // 英式英语词典条目数
    int zh_dict_count;             // 中文词典条目数
    int ja_vocab_count;            // 日文词汇数
    
    // Trie 树统计
    int zh_trie_words;             // 中文 Trie 树词汇数
    int zh_trie_nodes;             // 中文 Trie 树节点数
    int ja_trie_words;             // 日文 Trie 树词汇数
    int ja_trie_nodes;             // 日文 Trie 树节点数
    
    // 内存统计
    size_t total_memory;           // 总内存占用（字节）
    size_t dict_memory;            // 词典内存
    size_t trie_memory;            // Trie 树内存
} MisakiStats;

/**
 * 获取统计信息
 * 
 * @param context 上下文对象
 * @param stats 输出：统计信息
 */
void misaki_context_get_stats(const MisakiContext *context, MisakiStats *stats);

/**
 * 打印统计信息
 * 
 * @param context 上下文对象
 */
void misaki_context_print_stats(const MisakiContext *context);

/* ============================================================================
 * 配置管理
 * ========================================================================== */

/**
 * 保存配置到文件
 * 
 * @param config 配置对象
 * @param file_path 文件路径
 * @return 成功返回 true
 */
bool misaki_config_save(const MisakiConfig *config, const char *file_path);

/**
 * 从文件加载配置
 * 
 * @param file_path 文件路径
 * @return 配置对象，失败返回默认配置
 */
MisakiConfig misaki_config_load(const char *file_path);

/* ============================================================================
 * 日志系统
 * ========================================================================== */

/**
 * 日志级别
 */
typedef enum {
    MISAKI_LOG_DEBUG = 0,
    MISAKI_LOG_INFO = 1,
    MISAKI_LOG_WARNING = 2,
    MISAKI_LOG_ERROR = 3,
    MISAKI_LOG_FATAL = 4
} MisakiLogLevel;

/**
 * 日志回调函数
 * 
 * @param level 日志级别
 * @param message 日志消息
 * @param user_data 用户数据
 */
typedef void (*MisakiLogCallback)(MisakiLogLevel level,
                                   const char *message,
                                   void *user_data);

/**
 * 设置日志回调
 * 
 * @param context 上下文对象
 * @param callback 回调函数
 * @param user_data 用户数据
 */
void misaki_context_set_log_callback(MisakiContext *context,
                                     MisakiLogCallback callback,
                                     void *user_data);

/**
 * 设置日志级别
 * 
 * @param context 上下文对象
 * @param level 日志级别
 */
void misaki_context_set_log_level(MisakiContext *context, MisakiLogLevel level);

/**
 * 写日志
 * 
 * @param context 上下文对象
 * @param level 日志级别
 * @param format 格式化字符串
 * @param ... 参数
 */
void misaki_log(MisakiContext *context,
                MisakiLogLevel level,
                const char *format,
                ...);

/* ============================================================================
 * 性能分析（可选）
 * ========================================================================== */

/**
 * 性能计数器
 */
typedef struct {
    int tokenize_count;            // 分词调用次数
    int g2p_count;                 // G2P 调用次数
    double tokenize_total_ms;      // 分词总耗时（毫秒）
    double g2p_total_ms;           // G2P 总耗时（毫秒）
    double tokenize_avg_ms;        // 分词平均耗时
    double g2p_avg_ms;             // G2P 平均耗时
} MisakiPerf;

/**
 * 启用性能分析
 * 
 * @param context 上下文对象
 * @param enable 是否启用
 */
void misaki_context_enable_profiling(MisakiContext *context, bool enable);

/**
 * 获取性能统计
 * 
 * @param context 上下文对象
 * @param perf 输出：性能统计
 */
void misaki_context_get_perf(const MisakiContext *context, MisakiPerf *perf);

/**
 * 重置性能计数器
 * 
 * @param context 上下文对象
 */
void misaki_context_reset_perf(MisakiContext *context);

/**
 * 打印性能报告
 * 
 * @param context 上下文对象
 */
void misaki_context_print_perf(const MisakiContext *context);

#ifdef __cplusplus
}
#endif

#endif /* MISAKI_CONTEXT_H */
