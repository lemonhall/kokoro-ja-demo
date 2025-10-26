/**
 * misaki_hmm.c
 * 
 * Misaki C Port - HMM Implementation
 * 中文未登录词识别实现
 * 
 * License: MIT
 */

#include "misaki_hmm.h"
#include "misaki_string.h"
#include "misaki_trie.h"
#include "misaki_tokenizer.h"  // 添加：提供 Token 函数
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

// 最小概率（log 空间）
#define MIN_PROB -3.14e100

/* ============================================================================
 * HMM 模型加载
 * ========================================================================== */

HmmModel* misaki_hmm_load(const char *file_path) {
    if (!file_path) {
        return NULL;
    }
    
    // 创建 HMM 模型
    HmmModel *model = (HmmModel*)calloc(1, sizeof(HmmModel));
    if (!model) {
        return NULL;
    }
    
    // 初始化发射概率 Trie 树
    for (int i = 0; i < HMM_STATE_COUNT; i++) {
        model->prob_emit[i] = misaki_trie_create();
    }
    
    // 提取基础路径（file_path 可能是 hmm_model.json 或 hmm_prob_emit.txt）
    char base_dir[512];
    const char *last_slash = strrchr(file_path, '/');
    if (!last_slash) {
        last_slash = strrchr(file_path, '\\');
    }
    
    if (last_slash) {
        size_t dir_len = last_slash - file_path + 1;
        memcpy(base_dir, file_path, dir_len);
        base_dir[dir_len] = '\0';
    } else {
        strcpy(base_dir, "./");
    }
    
    // 1. 加载初始概率
    char start_file[512];
    snprintf(start_file, sizeof(start_file), "%shmm_prob_start.txt", base_dir);
    
    FILE *f = fopen(start_file, "r");
    if (f) {
        char line[256];
        while (fgets(line, sizeof(line), f)) {
            char state;
            double prob;
            if (sscanf(line, "%c\t%lf", &state, &prob) == 2) {
                int idx = (state == 'B') ? HMM_STATE_B :
                         (state == 'M') ? HMM_STATE_M :
                         (state == 'E') ? HMM_STATE_E : HMM_STATE_S;
                model->prob_start[idx] = prob;
            }
        }
        fclose(f);
    } else {
        // 降级：使用默认值
        model->prob_start[HMM_STATE_B] = -0.26268660809250016;
        model->prob_start[HMM_STATE_E] = MIN_PROB;
        model->prob_start[HMM_STATE_M] = MIN_PROB;
        model->prob_start[HMM_STATE_S] = -1.4652633398537678;
    }
    
    // 2. 加载转移概率
    char trans_file[512];
    snprintf(trans_file, sizeof(trans_file), "%shmm_prob_trans.txt", base_dir);
    
    // 先初始化为最小值
    for (int i = 0; i < HMM_STATE_COUNT; i++) {
        for (int j = 0; j < HMM_STATE_COUNT; j++) {
            model->prob_trans[i][j] = MIN_PROB;
        }
    }
    
    f = fopen(trans_file, "r");
    if (f) {
        char line[256];
        while (fgets(line, sizeof(line), f)) {
            char from_state, to_state;
            double prob;
            if (sscanf(line, "%c\t%c\t%lf", &from_state, &to_state, &prob) == 3) {
                int from_idx = (from_state == 'B') ? HMM_STATE_B :
                              (from_state == 'M') ? HMM_STATE_M :
                              (from_state == 'E') ? HMM_STATE_E : HMM_STATE_S;
                int to_idx = (to_state == 'B') ? HMM_STATE_B :
                            (to_state == 'M') ? HMM_STATE_M :
                            (to_state == 'E') ? HMM_STATE_E : HMM_STATE_S;
                model->prob_trans[from_idx][to_idx] = prob;
            }
        }
        fclose(f);
    } else {
        // 降级：使用硬编码值
        model->prob_trans[HMM_STATE_B][HMM_STATE_E] = -0.510825623765990;
        model->prob_trans[HMM_STATE_B][HMM_STATE_M] = -0.916290731874155;
        model->prob_trans[HMM_STATE_E][HMM_STATE_B] = -0.5897149736854513;
        model->prob_trans[HMM_STATE_E][HMM_STATE_S] = -0.8085250474669937;
        model->prob_trans[HMM_STATE_M][HMM_STATE_E] = -0.33344856811948514;
        model->prob_trans[HMM_STATE_M][HMM_STATE_M] = -1.2603623820268226;
        model->prob_trans[HMM_STATE_S][HMM_STATE_B] = -0.7211965654669841;
        model->prob_trans[HMM_STATE_S][HMM_STATE_S] = -0.6658631448798212;
    }
    
    // 3. 加载发射概率（最大数据）
    char emit_file[512];
    snprintf(emit_file, sizeof(emit_file), "%shmm_prob_emit.txt", base_dir);
    
    f = fopen(emit_file, "r");
    if (f) {
        char line[256];
        int line_count = 0;
        while (fgets(line, sizeof(line), f)) {
            char state;
            char ch[16];  // UTF-8 字符
            double prob;
            
            // 解析格式：状态 \t 字符 \t 概率
            if (sscanf(line, "%c\t%[^\t]\t%lf", &state, ch, &prob) == 3) {
                int state_idx = (state == 'B') ? HMM_STATE_B :
                               (state == 'M') ? HMM_STATE_M :
                               (state == 'E') ? HMM_STATE_E : HMM_STATE_S;
                
                // 将概率存储到 Trie 树（使用 tag 字段存储概率）
                char prob_str[32];
                snprintf(prob_str, sizeof(prob_str), "%.10f", prob);
                misaki_trie_insert(model->prob_emit[state_idx], ch, prob, prob_str);
                line_count++;
            }
        }
        fclose(f);
        model->total_chars = line_count;
    } else {
        fprintf(stderr, "⚠️  未找到发射概率文件：%s，使用默认值\n", emit_file);
        model->total_chars = 0;
    }
    
    printf("✅ HMM 模型加载成功\n");
    printf("   - 初始概率: ✅\n");
    printf("   - 转移概率: ✅\n");
    printf("   - 发射概率: %s (%d 个字符)\n", 
           model->total_chars > 0 ? "✅" : "❌",
           model->total_chars);
    
    return model;
}

void misaki_hmm_free(HmmModel *model) {
    if (!model) {
        return;
    }
    
    // 释放发射概率 Trie 树
    for (int i = 0; i < HMM_STATE_COUNT; i++) {
        if (model->prob_emit[i]) {
            misaki_trie_free(model->prob_emit[i]);
        }
    }
    
    free(model);
}

/* ============================================================================
 * HMM Viterbi 解码
 * ========================================================================== */

double misaki_hmm_get_emit_prob(const HmmModel *model,
                                HmmState state,
                                uint32_t codepoint) {
    if (!model || !model->prob_emit[state]) {
        return MIN_PROB;
    }
    
    // 将 Unicode 码点转为 UTF-8 字符串
    char ch[5] = {0};
    if (codepoint < 0x80) {
        ch[0] = codepoint;
    } else if (codepoint < 0x800) {
        ch[0] = 0xC0 | (codepoint >> 6);
        ch[1] = 0x80 | (codepoint & 0x3F);
    } else if (codepoint < 0x10000) {
        ch[0] = 0xE0 | (codepoint >> 12);
        ch[1] = 0x80 | ((codepoint >> 6) & 0x3F);
        ch[2] = 0x80 | (codepoint & 0x3F);
    } else {
        ch[0] = 0xF0 | (codepoint >> 18);
        ch[1] = 0x80 | ((codepoint >> 12) & 0x3F);
        ch[2] = 0x80 | ((codepoint >> 6) & 0x3F);
        ch[3] = 0x80 | (codepoint & 0x3F);
    }
    
    // 从 Trie 树中查找
    TrieMatch match;
    if (misaki_trie_match_longest(model->prob_emit[state], ch, 0, &match)) {
        // 概率存储在 frequency 字段
        return match.frequency;
    }
    
    // 如果找不到，返回默认最小发射概率
    return MIN_PROB;
}

int misaki_hmm_viterbi(const HmmModel *model, 
                       const char *text,
                       HmmState *states) {
    if (!model || !text || !states) {
        return 0;
    }
    
    // 1. 统计字符数量
    int char_count = 0;
    uint32_t chars[256];  // 假设最多 256 个字符
    
    const char *p = text;
    while (*p && char_count < 256) {
        uint32_t codepoint;
        int bytes = misaki_utf8_decode(p, &codepoint);
        if (bytes == 0) break;
        
        chars[char_count++] = codepoint;
        p += bytes;
    }
    
    if (char_count == 0) {
        return 0;
    }
    
    // 2. 初始化 Viterbi DP 表
    double V[256][HMM_STATE_COUNT];  // V[t][s] = 时刻 t 状态 s 的最大概率
    int path[256][HMM_STATE_COUNT];  // path[t][s] = 到达 V[t][s] 的前一个状态
    
    // 初始化第一个字符
    for (int s = 0; s < HMM_STATE_COUNT; s++) {
        double emit_prob = misaki_hmm_get_emit_prob(model, s, chars[0]);
        V[0][s] = model->prob_start[s] + emit_prob;
        path[0][s] = -1;  // 第一个字符无前驱
    }
    
    // 3. 动态规划：前向传播
    for (int t = 1; t < char_count; t++) {
        for (int s = 0; s < HMM_STATE_COUNT; s++) {
            double max_prob = MIN_PROB;
            int best_prev = 0;
            
            // 找到最佳前驱状态
            for (int prev_s = 0; prev_s < HMM_STATE_COUNT; prev_s++) {
                double prob = V[t-1][prev_s] + model->prob_trans[prev_s][s];
                if (prob > max_prob) {
                    max_prob = prob;
                    best_prev = prev_s;
                }
            }
            
            double emit_prob = misaki_hmm_get_emit_prob(model, s, chars[t]);
            V[t][s] = max_prob + emit_prob;
            path[t][s] = best_prev;
        }
    }
    
    // 4. 回溯：找到最优路径
    // 找到最后一个字符的最佳状态
    double max_prob = MIN_PROB;
    int best_state = 0;
    for (int s = 0; s < HMM_STATE_COUNT; s++) {
        if (V[char_count-1][s] > max_prob) {
            max_prob = V[char_count-1][s];
            best_state = s;
        }
    }
    
    // 回溯路径
    states[char_count-1] = best_state;
    for (int t = char_count - 2; t >= 0; t--) {
        states[t] = path[t+1][states[t+1]];
    }
    
    return char_count;
}

/* ============================================================================
 * 状态序列转换为分词结果
 * ========================================================================== */

MisakiTokenList* misaki_hmm_states_to_tokens(const char *text,
                                             const HmmState *states,
                                             int state_count) {
    if (!text || !states || state_count == 0) {
        return NULL;
    }
    
    MisakiTokenList *tokens = misaki_token_list_create();
    if (!tokens) {
        return NULL;
    }
    
    // 根据状态序列切分
    // B-M-E 表示一个词，S 表示单字词
    
    int word_start = 0;
    const char *p = text;
    int char_idx = 0;
    
    while (*p && char_idx < state_count) {
        // 如果是 E 或 S，表示词结束
        if (states[char_idx] == HMM_STATE_E || states[char_idx] == HMM_STATE_S) {
            // 提取词
            const char *word_start_ptr = text;
            for (int i = 0; i < word_start && *word_start_ptr; i++) {
                uint32_t cp;
                int bytes = misaki_utf8_decode(word_start_ptr, &cp);
                word_start_ptr += bytes;
            }
            
            const char *word_end_ptr = word_start_ptr;
            for (int i = word_start; i <= char_idx && *word_end_ptr; i++) {
                uint32_t cp;
                int bytes = misaki_utf8_decode(word_end_ptr, &cp);
                word_end_ptr += bytes;
            }
            
            // 创建 token
            size_t word_len = word_end_ptr - word_start_ptr;
            char *word = (char*)malloc(word_len + 1);
            if (word) {
                memcpy(word, word_start_ptr, word_len);
                word[word_len] = '\0';
                
                // 创建 MisakiToken
                MisakiToken token = {
                    .text = word,
                    .tag = NULL,
                    .phonemes = NULL,
                    .whitespace = NULL,
                    .start = 0,
                    .length = word_len,
                    .score = 0.0
                };
                
                misaki_token_list_add(tokens, &token);  // 先 add（会复制 word）
                free(word);  // 再 free
            }
            
            word_start = char_idx + 1;
        }
        
        // 移动到下一个字符
        uint32_t cp;
        int bytes = misaki_utf8_decode(p, &cp);
        p += bytes;
        char_idx++;
    }
    
    return tokens;
}

MisakiTokenList* misaki_hmm_cut(const HmmModel *model, const char *text) {
    if (!model || !text) {
        return NULL;
    }
    
    HmmState states[256];
    int char_count = misaki_hmm_viterbi(model, text, states);
    
    if (char_count == 0) {
        return NULL;
    }
    
    return misaki_hmm_states_to_tokens(text, states, char_count);
}
