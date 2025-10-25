/**
 * misaki_transition_rules.h
 * 
 * 日文词性转移规则（硬编码）
 * 用于改善分词质量，避免破坏动词活用和复合名词
 * 
 * License: MIT
 */

#ifndef MISAKI_TRANSITION_RULES_H
#define MISAKI_TRANSITION_RULES_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * 获取词性转移成本
 * 
 * 基于日语语法规则的硬编码转移成本
 * 返回值：负数表示鼓励这种组合，正数表示惩罚
 * 
 * @param left_tag 前接词的词性（如 "動詞", "名詞" 等）
 * @param right_tag 后接词的词性
 * @return 转移成本（-10.0 到 +5.0）
 */
double misaki_get_transition_cost(const char *left_tag, const char *right_tag);

/**
 * 检查是否为动词相关词性
 */
int is_verb_tag(const char *tag);

/**
 * 检查是否为助动词
 */
int is_auxiliary_verb_tag(const char *tag);

/**
 * 检查是否为名词
 */
int is_noun_tag(const char *tag);

/**
 * 检查是否为助词
 */
int is_particle_tag(const char *tag);

/**
 * 检查是否为形容词
 */
int is_adjective_tag(const char *tag);

#ifdef __cplusplus
}
#endif

#endif /* MISAKI_TRANSITION_RULES_H */
