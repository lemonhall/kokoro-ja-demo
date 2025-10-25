/**
 * misaki.h
 * 
 * Misaki C Port - Main Header
 * Misaki C 语言版本主头文件
 * 
 * 这是一个从 Python misaki 移植的 C 语言实现，提供多语言 G2P 功能
 * 
 * 支持语言:
 * - 英文 (English): CMUdict
 * - 中文 (Chinese): jieba + pypinyin
 * - 日文 (Japanese): MeCab + OpenJTalk
 * - 韩文 (Korean)
 * - 越南文 (Vietnamese)
 * 
 * License: MIT
 * Original: https://github.com/hexgrad/misaki
 * C Port: https://github.com/lemonhall/kokoro-ja-demo/tree/main/misaki_c_port
 * 
 * 使用示例:
 * 
 * ```c
 * // 1. 初始化上下文
 * MisakiConfig config = misaki_default_config();
 * config.data_dir = "extracted_data";
 * config.enable_english = true;
 * config.enable_chinese = true;
 * 
 * MisakiContext *ctx = misaki_context_create(&config);
 * if (!ctx) {
 *     fprintf(stderr, "Failed to create context\n");
 *     return -1;
 * }
 * 
 * // 2. G2P 转换
 * MisakiTokenList *tokens = misaki_g2p(ctx, "Hello 世界", LANG_UNKNOWN, NULL);
 * if (tokens) {
 *     char *phonemes = misaki_merge_phonemes(tokens, " ");
 *     printf("Phonemes: %s\n", phonemes);
 *     free(phonemes);
 *     misaki_token_list_free(tokens);
 * }
 * 
 * // 3. 清理
 * misaki_context_free(ctx);
 * ```
 */

#ifndef MISAKI_H
#define MISAKI_H

/* 版本信息 */
#define MISAKI_VERSION_MAJOR 1
#define MISAKI_VERSION_MINOR 0
#define MISAKI_VERSION_PATCH 0
#define MISAKI_VERSION_STRING "1.0.0"

/* 包含所有公共头文件 */
#include "misaki_types.h"
#include "misaki_string.h"
#include "misaki_dict.h"
#include "misaki_trie.h"
#include "misaki_viterbi.h"
#include "misaki_cache.h"
#include "misaki_tokenizer.h"
#include "misaki_g2p.h"
#include "misaki_context.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * 版本信息
 * ========================================================================== */

/**
 * 获取版本字符串
 * 
 * @return 版本字符串（如 "1.0.0"）
 */
const char* misaki_version(void);

/**
 * 获取版本号
 * 
 * @param major 输出：主版本号
 * @param minor 输出：次版本号
 * @param patch 输出：补丁版本号
 */
void misaki_version_number(int *major, int *minor, int *patch);

/* ============================================================================
 * 便捷 API（简化使用）
 * ========================================================================== */

/**
 * 快速 G2P 转换（自动初始化和清理）
 * 
 * 注意：每次调用都会创建和销毁上下文，性能较低
 *      如需高性能，请使用 misaki_context_create() + misaki_g2p()
 * 
 * @param text 文本（UTF-8）
 * @param lang 语言类型（LANG_UNKNOWN 表示自动检测）
 * @param data_dir 数据目录（NULL 表示使用默认路径）
 * @return 音素字符串，失败返回 NULL（需要调用 free 释放）
 */
char* misaki_quick_g2p(const char *text,
                       MisakiLanguage lang,
                       const char *data_dir);

/**
 * 快速英文 G2P
 * 
 * @param text 英文文本
 * @return 音素字符串，失败返回 NULL（需要调用 free 释放）
 */
char* misaki_quick_en_g2p(const char *text);

/**
 * 快速中文 G2P
 * 
 * @param text 中文文本
 * @return 音素字符串，失败返回 NULL（需要调用 free 释放）
 */
char* misaki_quick_zh_g2p(const char *text);

/**
 * 快速日文 G2P
 * 
 * @param text 日文文本
 * @return 音素字符串，失败返回 NULL（需要调用 free 释放）
 */
char* misaki_quick_ja_g2p(const char *text);

/* ============================================================================
 * 全局初始化/清理（可选）
 * ========================================================================== */

/**
 * 全局初始化
 * 
 * 用于设置全局配置（如内存分配器、日志系统等）
 * 非线程安全，应在程序启动时调用一次
 * 
 * @param allocator 自定义内存分配器（可为 NULL）
 * @return 成功返回 true
 */
bool misaki_init(MisakiAllocator *allocator);

/**
 * 全局清理
 * 
 * 释放全局资源
 * 非线程安全，应在程序退出前调用一次
 */
void misaki_cleanup(void);

/* ============================================================================
 * 错误处理（全局）
 * ========================================================================== */

/**
 * 获取最后一次错误码（线程局部）
 * 
 * @return 错误码
 */
MisakiError misaki_get_last_error(void);

/**
 * 获取最后一次错误消息（线程局部）
 * 
 * @return 错误消息字符串
 */
const char* misaki_get_last_error_message(void);

/**
 * 清除错误状态（线程局部）
 */
void misaki_clear_last_error(void);

/* ============================================================================
 * 工具函数
 * ========================================================================== */

/**
 * 检查数据目录是否有效
 * 
 * @param data_dir 数据目录路径
 * @return 有效返回 true
 */
bool misaki_check_data_dir(const char *data_dir);

/**
 * 获取默认数据目录
 * 
 * 按以下顺序查找:
 * 1. 环境变量 MISAKI_DATA_DIR
 * 2. 当前目录的 extracted_data/
 * 3. 可执行文件目录的 extracted_data/
 * 
 * @return 数据目录路径，未找到返回 NULL
 */
const char* misaki_get_default_data_dir(void);

/**
 * 打印系统信息
 */
void misaki_print_system_info(void);

/* ============================================================================
 * 编译时配置检测
 * ========================================================================== */

/**
 * 检查功能是否已编译
 */
bool misaki_has_english_support(void);
bool misaki_has_chinese_support(void);
bool misaki_has_japanese_support(void);
bool misaki_has_korean_support(void);
bool misaki_has_vietnamese_support(void);
bool misaki_has_hmm_support(void);

#ifdef __cplusplus
}
#endif

#endif /* MISAKI_H */
