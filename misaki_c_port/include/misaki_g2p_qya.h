#ifndef MISAKI_G2P_QYA_H
#define MISAKI_G2P_QYA_H

#include "misaki_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file misaki_g2p_qya.h
 * @brief Quenya (昆雅语) Grapheme-to-Phoneme conversion
 * 
 * 实现托尔金创造的昆雅语（精灵语）的文本到音素转换
 * 基于标准Noldorin发音规则
 */

/**
 * @brief 初始化昆雅语G2P转换器
 * @return 成功返回0，失败返回-1
 */
int misaki_g2p_qya_init(void);

/**
 * @brief 清理昆雅语G2P转换器
 */
void misaki_g2p_qya_cleanup(void);

/**
 * @brief 将昆雅语单词转换为IPA音素
 * @param word 输入的昆雅语单词（UTF-8编码）
 * @param phonemes 输出的音素序列（调用者需释放内存）
 * @return 成功返回0，失败返回-1
 */
int misaki_g2p_qya_convert(const char* word, char** phonemes);

/**
 * @brief 将昆雅语文本转换为IPA音素序列
 * @param text 输入文本
 * @param phonemes 输出音素序列（调用者需释放内存）
 * @return 成功返回0，失败返回-1
 */
int misaki_g2p_qya_text(const char* text, char** phonemes);

/**
 * @brief 计算昆雅语单词的音节数
 * @param word 输入单词
 * @return 音节数，失败返回-1
 */
int misaki_qya_count_syllables(const char* word);

/**
 * @brief 计算昆雅语单词的重音位置
 * @param word 输入单词
 * @return 重音所在音节索引（0-based），失败返回-1
 */
int misaki_qya_calculate_stress(const char* word);

/**
 * @brief 检测是否为双元音
 * @param str 当前字符位置
 * @param diphthong 输出双元音的IPA表示
 * @return 双元音字符长度，非双元音返回0
 */
int misaki_qya_is_diphthong(const char* str, char* diphthong);

/**
 * @brief 检测是否为长元音（带长音符）
 * @param str 当前字符位置
 * @param vowel 输出长元音的IPA表示
 * @return 长元音字符长度，非长元音返回0
 */
int misaki_qya_is_long_vowel(const char* str, char* vowel);

/**
 * @brief 检测是否为辅音簇
 * @param str 当前字符位置
 * @param cluster 输出辅音簇的IPA表示
 * @return 辅音簇字符长度，非辅音簇返回0
 */
int misaki_qya_is_consonant_cluster(const char* str, char* cluster);

#ifdef __cplusplus
}
#endif

#endif /* MISAKI_G2P_QYA_H */
