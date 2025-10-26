/**
 * test_hmm.c
 * 
 * HMM 未登录词识别测试
 */

#include "misaki_hmm.h"
#include "misaki_string.h"  // 添加：提供 misaki_utf8_decode
#include "misaki_tokenizer.h"  // 添加：提供 misaki_token_list_free
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_states(const char *text, const HmmState *states, int count) {
    const char *state_names[] = {"B", "M", "E", "S"};
    
    printf("  状态序列: ");
    for (int i = 0; i < count; i++) {
        printf("%s ", state_names[states[i]]);
    }
    printf("\n");
    
    printf("  字符-状态: ");
    const char *p = text;
    for (int i = 0; i < count && *p; i++) {
        uint32_t codepoint;
        int bytes = misaki_utf8_decode(p, &codepoint);
        if (bytes == 0) break;
        
        char ch[5] = {0};
        memcpy(ch, p, bytes);
        printf("%s(%s) ", ch, state_names[states[i]]);
        
        p += bytes;
    }
    printf("\n");
}

void test_hmm_basic() {
    printf("\n========================================\n");
    printf("测试 1: HMM 模型加载\n");
    printf("========================================\n");
    
    HmmModel *model = misaki_hmm_load("../extracted_data/zh/hmm_model.json");
    if (!model) {
        printf("❌ 无法加载 HMM 模型\n");
        return;
    }
    
    printf("✅ HMM 模型加载成功\n");
    
    // 显示初始概率
    const char *state_names[] = {"B", "M", "E", "S"};
    printf("\n📊 初始概率:\n");
    for (int i = 0; i < HMM_STATE_COUNT; i++) {
        printf("  %s: %.6f\n", state_names[i], model->prob_start[i]);
    }
    
    // 显示转移概率
    printf("\n📊 转移概率:\n");
    for (int i = 0; i < HMM_STATE_COUNT; i++) {
        for (int j = 0; j < HMM_STATE_COUNT; j++) {
            if (model->prob_trans[i][j] > -100.0) {
                printf("  %s -> %s: %.6f\n", state_names[i], state_names[j], 
                       model->prob_trans[i][j]);
            }
        }
    }
    
    misaki_hmm_free(model);
}

void test_hmm_viterbi() {
    printf("\n========================================\n");
    printf("测试 2: HMM Viterbi 解码\n");
    printf("========================================\n");
    
    HmmModel *model = misaki_hmm_load("../extracted_data/zh/hmm_model.json");
    if (!model) {
        printf("❌ 无法加载 HMM 模型\n");
        return;
    }
    
    // 测试用例
    const char *test_cases[] = {
        "李小明",      // 人名
        "去北京",      // 地名
        "中关村",      // 地名
        "王大锤",      // 人名
        "微信支付",    // 专有名词
        "人工智能",    // 术语
        NULL
    };
    
    for (int i = 0; test_cases[i]; i++) {
        const char *text = test_cases[i];
        printf("\n测试文本: %s\n", text);
        
        HmmState states[256];
        int char_count = misaki_hmm_viterbi(model, text, states);
        
        if (char_count > 0) {
            printf("  字符数: %d\n", char_count);
            print_states(text, states, char_count);
        } else {
            printf("  ❌ 解码失败\n");
        }
    }
    
    misaki_hmm_free(model);
}

void test_hmm_cut() {
    printf("\n========================================\n");
    printf("测试 3: HMM 分词\n");
    printf("========================================\n");
    
    HmmModel *model = misaki_hmm_load("../extracted_data/zh/hmm_model.json");
    if (!model) {
        printf("❌ 无法加载 HMM 模型\n");
        return;
    }
    
    // 测试用例
    const char *test_cases[] = {
        "李小明",
        "去北京",
        "王大锤说的对",
        NULL
    };
    
    for (int i = 0; test_cases[i]; i++) {
        const char *text = test_cases[i];
        printf("\n测试文本: %s\n", text);
        
        MisakiTokenList *tokens = misaki_hmm_cut(model, text);
        if (tokens && tokens->count > 0) {
            printf("  分词结果: ");
            for (int j = 0; j < tokens->count; j++) {
                printf("[%s] ", tokens->tokens[j].text);
            }
            printf("\n");
            misaki_token_list_free(tokens);
        } else {
            printf("  ❌ 分词失败\n");
        }
    }
    
    misaki_hmm_free(model);
}

int main() {
    printf("🧪 Misaki HMM 未登录词识别测试\n");
    printf("====================================\n");
    
    test_hmm_basic();
    test_hmm_viterbi();
    test_hmm_cut();
    
    printf("\n====================================\n");
    printf("✅ 所有测试完成\n");
    
    return 0;
}
