/**
 * misaki_tokenizer.h
 * 
 * Misaki C Port - Tokenizer APIs
 * 分词器接口（jieba 中文分词 + MeCab 日文分词）
 * 
 * License: MIT
 */

#ifndef MISAKI_TOKENIZER_H
#define MISAKI_TOKENIZER_H

#include "misaki_types.h"
#include "misaki_string.h"
#include "misaki_trie.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Token 操作
 * ========================================================================== */

/**
 * 创建 Token
 * 
 * @param text 原始文本（UTF-8）
 * @param tag 词性标签
 * @param start 起始位置（字节偏移）
 * @param length 长度（字节数）
 * @return Token 对象，失败返回 NULL
 */
MisakiToken* misaki_token_create(const char *text,
                                  const char *tag,
                                  int start,
                                  int length);

/**
 * 释放 Token
 * 
 * @param token Token 对象
 */
void misaki_token_free(MisakiToken *token);

/**
 * 复制 Token
 * 
 * @param token Token 对象
 * @return 新 Token 对象
 */
MisakiToken* misaki_token_clone(const MisakiToken *token);

/**
 * 设置 Token 的音素
 * 
 * @param token Token 对象
 * @param phonemes 音素序列
 * @return 成功返回 true
 */
bool misaki_token_set_phonemes(MisakiToken *token, const char *phonemes);

/**
 * 设置 Token 的分数
 * 
 * @param token Token 对象
 * @param score 分数
 */
void misaki_token_set_score(MisakiToken *token, double score);

/* ============================================================================
 * Token 列表操作
 * ========================================================================== */

/**
 * 创建 Token 列表
 * 
 * @return Token 列表对象，失败返回 NULL
 */
MisakiTokenList* misaki_token_list_create(void);

/**
 * 释放 Token 列表
 * 
 * @param list Token 列表对象
 */
void misaki_token_list_free(MisakiTokenList *list);

/**
 * 添加 Token 到列表
 * 
 * @param list Token 列表对象
 * @param token Token 对象（会被复制）
 * @return 成功返回 true
 */
bool misaki_token_list_add(MisakiTokenList *list, const MisakiToken *token);

/**
 * 获取列表中的 Token
 * 
 * @param list Token 列表对象
 * @param index 索引
 * @return Token 对象，超出范围返回 NULL
 */
MisakiToken* misaki_token_list_get(const MisakiTokenList *list, int index);

/**
 * 获取列表长度
 * 
 * @param list Token 列表对象
 * @return Token 数量
 */
int misaki_token_list_size(const MisakiTokenList *list);

/**
 * 清空列表
 * 
 * @param list Token 列表对象
 */
void misaki_token_list_clear(MisakiTokenList *list);

/* ============================================================================
 * DAG（有向无环图）操作
 * 用于 jieba 分词算法
 * ========================================================================== */

/**
 * 创建 DAG
 * 
 * @param text_length 文本长度（字符数，不是字节数）
 * @return DAG 对象，失败返回 NULL
 */
DAG* misaki_dag_create(int text_length);

/**
 * 释放 DAG
 * 
 * @param dag DAG 对象
 */
void misaki_dag_free(DAG *dag);

/**
 * 添加边（从 from 到 to）
 * 
 * @param dag DAG 对象
 * @param from 起始位置
 * @param to 结束位置
 * @return 成功返回 true
 */
bool misaki_dag_add_edge(DAG *dag, int from, int to);

/**
 * 获取某个位置的所有后继位置
 * 
 * @param dag DAG 对象
 * @param position 位置
 * @param next_positions 输出：后继位置数组
 * @param max_count 最大数量
 * @return 实际后继数量
 */
int misaki_dag_get_next(const DAG *dag, 
                        int position,
                        int *next_positions,
                        int max_count);

/**
 * 构建 DAG（基于 Trie 树）
 * 
 * 从文本的每个位置开始，使用 Trie 树查找所有可能的词
 * 
 * @param text 文本（UTF-8）
 * @param trie Trie 树对象
 * @return DAG 对象，失败返回 NULL
 */
DAG* misaki_dag_build(const char *text, const Trie *trie);

/* ============================================================================
 * 中文分词器 (Jieba-like)
 * 算法: Trie + DAG + 动态规划 + HMM
 * ========================================================================== */

/**
 * 中文分词器配置
 */
typedef struct {
    Trie *dict_trie;           // 词典 Trie 树（必需）
    bool enable_hmm;           // 是否启用 HMM 未登录词识别（默认 true）
    bool enable_userdict;      // 是否启用用户词典（默认 false）
    Trie *user_trie;           // 用户词典 Trie 树（可选）
} ZhTokenizerConfig;

/**
 * 创建中文分词器
 * 
 * @param config 配置
 * @return 分词器对象，失败返回 NULL
 */
void* misaki_zh_tokenizer_create(const ZhTokenizerConfig *config);

/**
 * 释放中文分词器
 * 
 * @param tokenizer 分词器对象
 */
void misaki_zh_tokenizer_free(void *tokenizer);

/**
 * 中文分词（精确模式）
 * 
 * @param tokenizer 分词器对象
 * @param text 文本（UTF-8）
 * @return Token 列表，失败返回 NULL
 */
MisakiTokenList* misaki_zh_tokenize(void *tokenizer, const char *text);

/**
 * 中文分词（全模式，返回所有可能的词）
 * 
 * @param tokenizer 分词器对象
 * @param text 文本（UTF-8）
 * @return Token 列表，失败返回 NULL
 */
MisakiTokenList* misaki_zh_tokenize_all(void *tokenizer, const char *text);

/**
 * 中文分词（搜索引擎模式，对长词再切分）
 * 
 * @param tokenizer 分词器对象
 * @param text 文本（UTF-8）
 * @return Token 列表，失败返回 NULL
 */
MisakiTokenList* misaki_zh_tokenize_search(void *tokenizer, const char *text);

/* ============================================================================
 * 日文分词器 (MeCab-like)
 * 算法: Trie + Viterbi
 * ========================================================================== */

/**
 * 日文分词器配置
 */
typedef struct {
    Trie *dict_trie;           // 词典 Trie 树（必需）
    const char *unidic_path;   // UniDic 词典路径（可选）
    bool use_simple_model;     // 使用简化模型（默认 true）
} JaTokenizerConfig;

/**
 * 创建日文分词器
 * 
 * @param config 配置
 * @return 分词器对象，失败返回 NULL
 */
void* misaki_ja_tokenizer_create(const JaTokenizerConfig *config);

/**
 * 释放日文分词器
 * 
 * @param tokenizer 分词器对象
 */
void misaki_ja_tokenizer_free(void *tokenizer);

/**
 * 日文分词
 * 
 * @param tokenizer 分词器对象
 * @param text 文本（UTF-8）
 * @return Token 列表，失败返回 NULL
 */
MisakiTokenList* misaki_ja_tokenize(void *tokenizer, const char *text);

/* ============================================================================
 * 英文分词器（简单空格分割 + 标点处理）
 * ========================================================================== */

/**
 * 英文分词（按空格和标点分割）
 * 
 * @param text 文本（UTF-8）
 * @return Token 列表，失败返回 NULL
 */
MisakiTokenList* misaki_en_tokenize(const char *text);

/**
 * 英文分词（保留标点）
 * 
 * @param text 文本（UTF-8）
 * @param keep_punctuation 是否保留标点作为独立 Token
 * @return Token 列表，失败返回 NULL
 */
MisakiTokenList* misaki_en_tokenize_ex(const char *text, bool keep_punctuation);

/* ============================================================================
 * 通用分词器（自动检测语言）
 * ========================================================================== */

/**
 * 自动检测文本语言
 * 
 * @param text 文本（UTF-8）
 * @return 语言类型
 */
MisakiLanguage misaki_detect_language(const char *text);

/**
 * 通用分词（自动选择分词器）
 * 
 * @param context Misaki 上下文（包含所有词典）
 * @param text 文本（UTF-8）
 * @param lang 语言类型（LANG_UNKNOWN 表示自动检测）
 * @return Token 列表，失败返回 NULL
 */
MisakiTokenList* misaki_tokenize(MisakiContext *context,
                                  const char *text,
                                  MisakiLanguage lang);

/* ============================================================================
 * HMM（隐马尔可夫模型）未登录词识别
 * 用于识别词典中不存在的词（如人名、地名等）
 * ========================================================================== */

/**
 * HMM 状态
 */
typedef enum {
    HMM_STATE_BEGIN,    // 词首
    HMM_STATE_MIDDLE,   // 词中
    HMM_STATE_END,      // 词尾
    HMM_STATE_SINGLE    // 单字成词
} HMMState;

/**
 * HMM 模型
 */
typedef struct {
    double start_prob[4];      // 初始概率
    double trans_prob[4][4];   // 转移概率
    double emit_prob[4][65536]; // 发射概率（简化：只存储常用字）
} HMMModel;

/**
 * 加载 HMM 模型
 * 
 * @param file_path 模型文件路径
 * @return HMM 模型，失败返回 NULL
 */
HMMModel* misaki_hmm_load(const char *file_path);

/**
 * 释放 HMM 模型
 * 
 * @param model HMM 模型
 */
void misaki_hmm_free(HMMModel *model);

/**
 * Viterbi 算法：寻找最优状态序列
 * 
 * @param model HMM 模型
 * @param text 文本（UTF-8）
 * @param states 输出：状态序列
 * @param max_length 最大长度
 * @return 实际长度
 */
int misaki_hmm_viterbi(const HMMModel *model,
                       const char *text,
                       HMMState *states,
                       int max_length);

/**
 * 使用 HMM 切分未登录词
 * 
 * @param model HMM 模型
 * @param text 文本（UTF-8）
 * @return Token 列表，失败返回 NULL
 */
MisakiTokenList* misaki_hmm_cut(const HMMModel *model, const char *text);

/* ============================================================================
 * 调试与统计
 * ========================================================================== */

/**
 * 打印 Token 列表
 * 
 * @param list Token 列表
 */
void misaki_token_list_print(const MisakiTokenList *list);

/**
 * 打印 DAG 结构
 * 
 * @param dag DAG 对象
 * @param text 对应的文本（用于显示）
 */
void misaki_dag_print(const DAG *dag, const char *text);

/**
 * 获取分词统计信息
 * 
 * @param list Token 列表
 * @param total_tokens 输出：总 Token 数
 * @param avg_token_length 输出：平均 Token 长度
 * @param max_token_length 输出：最大 Token 长度
 */
void misaki_token_list_stats(const MisakiTokenList *list,
                             int *total_tokens,
                             double *avg_token_length,
                             int *max_token_length);

#ifdef __cplusplus
}
#endif

#endif /* MISAKI_TOKENIZER_H */
