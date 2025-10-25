/**
 * misaki_transition_rules.c
 * 
 * 日文词性转移规则实现
 * 
 * 核心规则（按优先级）:
 * 1. 動詞 + 助動詞 → 强制结合（避免拆分 "なりたい"）
 * 2. 動詞 + 助詞(て/た等) → 强制结合（避免拆分 "思って"）
 * 3. 名詞 + 助詞 → 鼓励结合
 * 4. 形容詞 + 名詞 → 鼓励结合（"新しい カフェ"）
 * 5. 接頭辞/接尾辞 + 名詞 → 强制结合
 * 6. 名詞 + 名詞 → 轻微惩罚（避免过度复合，但允许真实复合词）
 * 
 * License: MIT
 */

#include "misaki_transition_rules.h"
#include <string.h>
#include <stdio.h>

/* ============================================================================
 * 辅助函数：词性判断
 * ========================================================================== */

int is_verb_tag(const char *tag) {
    return tag && strstr(tag, "動詞") != NULL;
}

int is_auxiliary_verb_tag(const char *tag) {
    return tag && strstr(tag, "助動詞") != NULL;
}

int is_noun_tag(const char *tag) {
    return tag && strstr(tag, "名詞") != NULL;
}

int is_particle_tag(const char *tag) {
    return tag && strstr(tag, "助詞") != NULL;
}

int is_adjective_tag(const char *tag) {
    return tag && strstr(tag, "形容詞") != NULL;
}

static int is_prefix_tag(const char *tag) {
    return tag && strstr(tag, "接頭辞") != NULL;
}

static int is_suffix_tag(const char *tag) {
    return tag && strstr(tag, "接尾辞") != NULL;
}

/* ============================================================================
 * 核心转移成本计算
 * ========================================================================== */

double misaki_get_transition_cost(const char *left_tag, const char *right_tag) {
    // 空标签，返回默认成本
    if (!left_tag || !right_tag) {
        return 0.0;
    }
    
    // ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
    // 规则 1: 動詞 + 助動詞 → 强制结合
    // 示例: なり + たい, 思い + ます
    // ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
    if (is_verb_tag(left_tag) && is_auxiliary_verb_tag(right_tag)) {
        return -10.0;  // 非常强的鼓励
    }
    
    // ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
    // 规则 2: 動詞 + 助詞 → 强制结合
    // 示例: 会っ + て, 思っ + て
    // ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
    if (is_verb_tag(left_tag) && is_particle_tag(right_tag)) {
        return -8.0;
    }
    
    // ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
    // 规则 3: 助動詞 + 助動詞 → 强制结合
    // 示例: い + ました (避免拆分成 "いま + し + た")
    // ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
    if (is_auxiliary_verb_tag(left_tag) && is_auxiliary_verb_tag(right_tag)) {
        return -12.0;  // 最强鼓励！
    }
    
    // ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
    // 规则 4: 助動詞 + 助詞 → 强制结合
    // 示例: まし + た
    // ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
    if (is_auxiliary_verb_tag(left_tag) && is_particle_tag(right_tag)) {
        return -9.0;
    }
    
    // ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
    // 规则 5: 名詞 + 助詞 → 鼓励结合
    // 示例: 友達 + と, 宇宙飛行士 + に
    // ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
    if (is_noun_tag(left_tag) && is_particle_tag(right_tag)) {
        return -3.0;
    }
    
    // ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
    // 规则 6: 形容詞 + 名詞 → 鼓励结合
    // 示例: 新しい + カフェ, 美味しい + ケーキ
    // ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
    if (is_adjective_tag(left_tag) && is_noun_tag(right_tag)) {
        return -4.0;
    }
    
    // ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
    // 规则 7: 接頭辞/接尾辞 + 名詞 → 轻微鼓励（降低强度）
    // 示例: 新 + しい（虽然这个例子不太对，但规则仍然有效）
    // ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
    if (is_prefix_tag(left_tag) && is_noun_tag(right_tag)) {
        return -2.0;  // 降低强度：从 -5.0 → -2.0
    }
    
    if (is_noun_tag(left_tag) && is_suffix_tag(right_tag)) {
        return -2.0;  // 降低强度：从 -5.0 → -2.0
    }
    
    // ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
    // 规则 8: 名詞 + 名詞 → 适度惩罚（增加惩罚）
    // 避免过度复合，但允许真实复合词（通过词典频率来平衡）
    // 示例: 允许 "宇宙飛行士"，但避免任意名词拼接
    // ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
    if (is_noun_tag(left_tag) && is_noun_tag(right_tag)) {
        return +3.0;  // 增加惩罚：从 +1.0 → +3.0
    }
    
    // ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
    // 规则 9: 接尾辞 + 助動詞 → 鼓励结合
    // 示例: り + たい
    // ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
    if (is_suffix_tag(left_tag) && is_auxiliary_verb_tag(right_tag)) {
        return -7.0;
    }
    
    // 默认：无特殊处理
    return 0.0;
}
