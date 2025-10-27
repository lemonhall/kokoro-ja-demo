#ifndef MISAKI_TOKENIZER_QYA_H
#define MISAKI_TOKENIZER_QYA_H

#include "misaki_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file misaki_tokenizer_qya.h
 * @brief Quenya (昆雅语) Tokenizer
 * 
 * 昆雅语分词器，采用基于空格和标点的简单分词策略
 */

/**
 * @brief 初始化昆雅语分词器
 * @return 成功返回0，失败返回-1
 */
int misaki_tokenizer_qya_init(void);

/**
 * @brief 清理昆雅语分词器
 */
void misaki_tokenizer_qya_cleanup(void);

/**
 * @brief 对昆雅语文本进行分词
 * @param text 输入文本（UTF-8编码）
 * @param tokens 输出token数组（调用者需释放）
 * @param token_count 输出token数量
 * @return 成功返回0，失败返回-1
 */
int misaki_tokenize_qya(const char* text, MisakiToken** tokens, int* token_count);

/**
 * @brief 检测字符是否为昆雅语标点
 * @param c 字符
 * @return 是标点返回1，否则返回0
 */
int misaki_qya_is_punctuation(char c);

/**
 * @brief 检测字符是否为昆雅语字母
 * @param c 字符（UTF-8首字节）
 * @return 是字母返回1，否则返回0
 */
int misaki_qya_is_letter(unsigned char c);

#ifdef __cplusplus
}
#endif

#endif /* MISAKI_TOKENIZER_QYA_H */
