/**
 * misaki_hmm.h
 * 
 * Misaki C Port - HMM (Hidden Markov Model) for Chinese OOV Detection
 * 中文未登录词识别（基于隐马尔可夫模型）
 * 
 * 用途：识别词典中不存在的词（人名、地名、新词等）
 * 数据来源：jieba 的 HMM 参数
 * 
 * License: MIT
 */

#ifndef MISAKI_HMM_H
#define MISAKI_HMM_H

#include "misaki_types.h"
#include "misaki_trie.h"  // 添加：完整 Trie 定义
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * HMM 状态定义
 * ========================================================================== */

// HMM 状态枚举
typedef enum {
    HMM_STATE_B = 0,  // Begin - 词的开始
    HMM_STATE_M = 1,  // Middle - 词的中间
    HMM_STATE_E = 2,  // End - 词的结束
    HMM_STATE_S = 3,  // Single - 单字词
    HMM_STATE_COUNT = 4
} HmmState;

/* ============================================================================
 * HMM 模型数据结构
 * ========================================================================== */

/**
 * HMM 模型
 * 
 * 包含三个核心概率：
 * 1. 初始概率 (prob_start): 句子开始时各状态的概率
 * 2. 转移概率 (prob_trans): 从一个状态转移到另一个状态的概率
 * 3. 发射概率 (prob_emit): 某个状态产生某个字符的概率
 */
typedef struct {
    // 初始概率: P(state) at start
    double prob_start[HMM_STATE_COUNT];
    
    // 转移概率: P(next_state | current_state)
    // 4x4 矩阵
    double prob_trans[HMM_STATE_COUNT][HMM_STATE_COUNT];
    
    // 发射概率: 使用哈希表存储 (字符 -> 概率)
    // 每个状态对应一个 Trie 树，存储字符的发射概率
    Trie *prob_emit[HMM_STATE_COUNT];
    
    // 统计信息
    int total_chars;  // 发射概率中的总字符数
} HmmModel;

/* ============================================================================
 * HMM 模型加载和释放
 * ========================================================================== */

/**
 * 从 JSON 文件加载 HMM 模型
 * 
 * @param file_path HMM 模型文件路径 (hmm_model.json)
 * @return HMM 模型对象，失败返回 NULL
 * 
 * JSON 格式示例:
 * {
 *   "prob_start": {"B": -0.26, "E": -3.14e100, "M": -3.14e100, "S": -1.46},
 *   "prob_trans": {
 *     "B": {"E": -0.51, "M": -0.92},
 *     "E": {"B": -0.57, "S": -0.87},
 *     ...
 *   },
 *   "prob_emit": {
 *     "B": {"李": -5.2, "王": -5.3, ...},
 *     ...
 *   }
 * }
 */
HmmModel* misaki_hmm_load(const char *file_path);

/**
 * 释放 HMM 模型
 * 
 * @param model HMM 模型对象
 */
void misaki_hmm_free(HmmModel *model);

/* ============================================================================
 * HMM Viterbi 解码（用于未登录词切分）
 * ========================================================================== */

/**
 * 使用 HMM Viterbi 算法对未登录文本进行分词
 * 
 * @param model HMM 模型
 * @param text UTF-8 文本（未登录词）
 * @return Token 列表，失败返回 NULL
 * 
 * 示例:
 *   输入: "李小明"
 *   HMM 预测: B(李) M(小) E(明)
 *   输出: ["李小明"] (一个词)
 * 
 *   输入: "去北京"
 *   HMM 预测: S(去) B(北) E(京)
 *   输出: ["去", "北京"]
 */
MisakiTokenList* misaki_hmm_cut(const HmmModel *model, const char *text);

/**
 * HMM Viterbi 解码（内部实现）
 * 
 * @param model HMM 模型
 * @param text UTF-8 文本
 * @param states 输出：最优状态序列（需要预分配，长度至少为字符数）
 * @return 字符数量
 */
int misaki_hmm_viterbi(const HmmModel *model, 
                       const char *text,
                       HmmState *states);

/* ============================================================================
 * 辅助函数
 * ========================================================================== */

/**
 * 将状态序列转换为分词结果
 * 
 * @param text UTF-8 文本
 * @param states 状态序列
 * @param state_count 状态数量
 * @return Token 列表
 */
MisakiTokenList* misaki_hmm_states_to_tokens(const char *text,
                                             const HmmState *states,
                                             int state_count);

/**
 * 获取发射概率
 * 
 * @param model HMM 模型
 * @param state 状态
 * @param codepoint 字符 Unicode 码点
 * @return log 概率（如果不存在返回 MIN_PROB）
 */
double misaki_hmm_get_emit_prob(const HmmModel *model,
                                HmmState state,
                                uint32_t codepoint);

#ifdef __cplusplus
}
#endif

#endif /* MISAKI_HMM_H */
