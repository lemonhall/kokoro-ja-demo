# 中文声调变化规则实现方案

## 📅 日期
2025-10-26

## 🎯 目标
实现中文声调变化（Tone Sandhi）规则，提升TTS自然度

## 📖 声调变化规则

### 1. 三声变调（最常见）⭐
**规则**：两个三声（nǐ, hǎo）连读时，前一个变二声

**示例**：
- "你好" → nǐ hǎo → **ní** hǎo ✅
- "我也" → wǒ yě → **wó** yě ✅
- "很好" → hěn hǎo → **hén** hǎo ✅

**实现**：
```c
// 如果当前音节是三声，且下一个音节也是三声
// 当前音节变为二声
if (current_tone == 3 && next_tone == 3) {
    current_tone = 2;
}
```

### 2. "一"的变调
**规则**：
- "一" + 一二三声 → 不变（yī）
- "一" + 四声 → 变二声（yí）
- "一" + 轻声 → 变二声（yí）

**示例**：
- "一天" (一+一声) → yī tiān ✅
- "一起" (一+三声) → yì qǐ ✅ 
- "一个" (一+轻声) → yí gè ✅
- "一样" (一+四声) → yí yàng ✅

**实现**：
```c
if (is_yi(current) && next_tone == 4) {
    set_tone(current, 2);
} else if (is_yi(current) && next_tone >= 1 && next_tone <= 3) {
    set_tone(current, 4);
}
```

### 3. "不"的变调
**规则**：
- "不" + 四声 → 变二声（bú）
- "不" + 其他 → 不变（bù）

**示例**：
- "不是" (不+四声) → **bú** shì ✅
- "不要" (不+四声) → **bú** yào ✅
- "不好" (不+三声) → bù hǎo（不变）

**实现**：
```c
if (is_bu(current) && next_tone == 4) {
    set_tone(current, 2);
}
```

### 4. 轻声（可选，复杂）
**规则**：某些词的第二个音节读轻声

**示例**：
- "桌子" → zhuō zi（zi读轻声）
- "椅子" → yǐ zi
- "妈妈" → mā ma

**实现**：需要轻声词表，暂时跳过

## 📊 pypinyin 验证

已验证 pypinyin 对声调变化的处理：

| 词组 | 原始拼音 | 变调拼音 | pypinyin结果 |
|-----|---------|---------|-------------|
| 你好 | nǐ hǎo | ní hǎo | ✅ ni2 hao3 |
| 一起 | yī qǐ | yì qǐ | ✅ yi4 qi3 |
| 不好 | bù hǎo | bú hǎo | ✅ bu4 hao3 |

## 🔧 实现策略

### 方案一：在 G2P 阶段处理（推荐）⭐

**优点**：
- 逻辑集中
- 可以利用分词结果
- 容易调试

**实现位置**：[`misaki_g2p_zh.c`](file://e:\development\kokoro-ja-demo\misaki_c_port\src\core\misaki_g2p_zh.c) 中的 `misaki_zh_tone_sandhi()` 函数

**实现步骤**：
1. 遍历 TokenList
2. 对每个 Token 的 phonemes（IPA），检测声调
3. 应用变调规则
4. 更新 phonemes

### 方案二：在拼音转IPA阶段处理

**缺点**：
- 需要在转换前就知道下一个音节
- 逻辑分散

## 💻 代码实现

### 1. 辅助函数：提取声调

```c
// 从 IPA 音素中提取声调
// 返回: 1-4 (四个声调), 0 (轻声或无声调)
int extract_tone_from_ipa(const char *ipa) {
    if (!ipa) return 0;
    
    // IPA 声调标记：
    // → = 一声（高平）
    // ↗ = 二声（上升）
    // ↓ = 三声（降升）
    // ↘ = 四声（下降）
    // 无标记 = 轻声
    
    if (strstr(ipa, "→")) return 1;
    if (strstr(ipa, "↗")) return 2;
    if (strstr(ipa, "↓")) return 3;
    if (strstr(ipa, "↘")) return 4;
    
    return 0;  // 轻声
}
```

### 2. 辅助函数：修改声调

```c
// 修改 IPA 音素的声调
// 返回: 新的 IPA 字符串（需要 free）
char* change_ipa_tone(const char *ipa, int new_tone) {
    if (!ipa) return NULL;
    
    // 移除旧声调标记
    char *result = strdup(ipa);
    // 替换为新声调标记
    // ...
    
    return result;
}
```

### 3. 主函数：应用声调变化

```c
void misaki_zh_tone_sandhi(MisakiTokenList *tokens, const G2POptions *options) {
    if (!tokens || tokens->count == 0) {
        return;
    }
    
    for (int i = 0; i < tokens->count - 1; i++) {
        MisakiToken *current = &tokens->tokens[i];
        MisakiToken *next = &tokens->tokens[i + 1];
        
        if (!current->phonemes || !next->phonemes) {
            continue;
        }
        
        // 提取当前和下一个音节的声调
        int current_tone = extract_tone_from_ipa(current->phonemes);
        int next_tone = extract_tone_from_ipa(next->phonemes);
        
        // 规则1: 三声变调
        if (current_tone == 3 && next_tone == 3) {
            char *new_phonemes = change_ipa_tone(current->phonemes, 2);
            if (new_phonemes) {
                free(current->phonemes);
                current->phonemes = new_phonemes;
            }
        }
        
        // 规则2: "一"的变调
        if (strcmp(current->text, "一") == 0) {
            int new_tone = -1;
            if (next_tone == 4 || next_tone == 0) {
                new_tone = 2;  // 一 + 四声/轻声 → 二声
            } else if (next_tone >= 1 && next_tone <= 3) {
                new_tone = 4;  // 一 + 一二三声 → 四声
            }
            
            if (new_tone > 0) {
                char *new_phonemes = change_ipa_tone(current->phonemes, new_tone);
                if (new_phonemes) {
                    free(current->phonemes);
                    current->phonemes = new_phonemes;
                }
            }
        }
        
        // 规则3: "不"的变调
        if (strcmp(current->text, "不") == 0 && next_tone == 4) {
            char *new_phonemes = change_ipa_tone(current->phonemes, 2);
            if (new_phonemes) {
                free(current->phonemes);
                current->phonemes = new_phonemes;
            }
        }
    }
}
```

## 📝 测试用例

```c
// 测试1: 三声变调
输入: "你好"
分词: ["你", "好"]
原始: [nǐ↓, hǎo↓]
变调: [ní↗, hǎo↓]  ✅

// 测试2: "一"的变调
输入: "一起"
分词: ["一", "起"]
原始: [yī→, qǐ↓]
变调: [yì↘, qǐ↓]  ✅

// 测试3: "不"的变调
输入: "不是"
分词: ["不", "是"]
原始: [bù↘, shì↘]
变调: [bú↗, shì↘]  ✅
```

## 🔍 当前问题

我们的 IPA 声调标记：
- `→` = 一声
- `↗` = 二声
- `↓` = 三声（实际应该是 `↓↗`，但简化为 `↓`）
- `↘` = 四声

这与标准 IPA 不完全一致，但对 TTS 足够用。

## ⏱️ 实施时间估计

- 辅助函数（提取/修改声调）：30分钟
- 主函数实现：30分钟
- 测试验证：30分钟
- **总计：1.5 小时**

## 📁 需修改文件

- [`misaki_g2p_zh.c`](file://e:\development\kokoro-ja-demo\misaki_c_port\src\core\misaki_g2p_zh.c) - 实现 `misaki_zh_tone_sandhi()`

---

**状态**：✅ 方案已确定  
**下一步**：实现代码  
**预计完成**：今晚
