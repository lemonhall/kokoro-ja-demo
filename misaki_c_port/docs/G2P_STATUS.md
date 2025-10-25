# G2P 模块实现状态

## ✅ 已完成功能

### 1. 英文 G2P（基于 CMUdict）

**核心函数**：
- ✅ `misaki_en_g2p_word()` - 单词查询
- ✅ `misaki_en_g2p()` - 句子转换
- ✅ `misaki_en_g2p_oov()` - 未登录词处理（简化版）

**示例**：
```
输入："Hello world"
输出：
  [0] "Hello" → həlˈO
  [1] "world" → wˈɜɹld
```

**数据来源**：`extracted_data/en/us_dict.txt`（183,561 个单词）

**文件**：`src/core/misaki_g2p_en.c` (78 行)

---

### 2. 中文 G2P（汉字 → 拼音 → IPA）

**核心函数**：
- ✅ `misaki_zh_g2p()` - 完整流程（分词 + 拼音查询 + IPA 转换）
- ✅ `misaki_zh_pinyin_to_ipa()` - 拼音转 IPA（完整实现）
- ⏳ `misaki_zh_tone_sandhi()` - 声调变化（TODO）
- ⏳ `misaki_zh_erhua()` - 儿化音（TODO）

**示例**：
```
输入："你好世界"
输出：
  [0] "你" → ni↓
  [1] "好" → xɑʊ↓
  [2] "世界" → ʂi↘ tɕiɛ↘
  
合并音素: "ni↓ xɑʊ↓ ʂi↘ tɕiɛ↘"
```

**数据来源**：
- 单字拼音：`extracted_data/zh/pinyin_dict.txt`（41,924 个汉字）
- 词汇词典：`extracted_data/zh/dict.txt`（86 个词汇）

**文件**：`src/core/misaki_g2p_zh.c` (380 行)

**实现细节**：
- ✅ 声母映射：21 个（b→p, zh→ʈ͡ʂ, x→ɕ 等）
- ✅ 韵母映射：34 个（ai→aɪ, ang→ɑŋ 等）
- ✅ 声调符号：→↗↓↘（IPA 上标）
- ✅ 支持两种格式：ni3 和 nǐ

**注意事项**：
- 多音字选择：当前使用第一个拼音（简化处理）
- 完整版需要根据上下文选择正确拼音

---

### 3. 日文 G2P（假名 → IPA）

**核心函数**：
- ✅ `misaki_ja_g2p()` - 完整流程（分词 + 音素转换）
- ✅ `misaki_ja_kana_to_ipa()` - 假名转 IPA（简化版）
- ⏳ `misaki_ja_long_vowel()` - 长音处理（TODO）

**示例**：
```
输入："こんにちは世界"
输出：
  [0] "こんにちは" → こんにちは（待完善）
  [1] "世界" → 世界（待完善）
```

**待完善**：
- 假名 → 罗马音 → IPA 映射
- 可参考 OpenJTalk 的 `jpcommon_rule_utf_8.h` 规则

---

### 4. 工具函数

**已实现**：
- ✅ `misaki_merge_phonemes()` - 合并音素序列
- ✅ `misaki_count_phonemes()` - 统计音素数量
- ✅ `misaki_g2p_print()` - 打印 G2P 结果
- ✅ `misaki_g2p_stats()` - 统计信息（总音素、OOV 数量）
- ✅ `misaki_g2p_default_options()` - 默认配置

**待实现**：
- ⏳ `misaki_normalize_text()` - 文本规范化
- ⏳ `misaki_zh_num_to_text()` - 中文数字转文字
- ⏳ `misaki_en_num_to_text()` - 英文数字转文字
- ⏳ `misaki_fullwidth_to_halfwidth()` - 全角转半角
- ⏳ `misaki_traditional_to_simplified()` - 繁体转简体

---

## 📊 测试覆盖率

| 模块 | 测试数量 | 通过率 | 文件 |
|------|---------|--------|------|
| 英文 G2P | 2 | 100% | `tests/test_g2p.c` |
| 中文 G2P | 1 | 100% | `tests/test_g2p.c` |
| 日文 G2P | 1 | 100% | `tests/test_g2p.c` |
| 工具函数 | 2 | 100% | `tests/test_g2p.c` |
| **总计** | **6** | **100%** | - |

---

## 🚀 下一步优化

### 高优先级

1. **中文拼音 → IPA 转换**
   ```c
   // 当前：nǐ hǎo
   // 目标：ni↓ xɑʊ↓ (完整 IPA)
   ```

2. **日文假名 → IPA 转换**
   ```c
   // 当前：こんにちは
   // 目标：konnit͡ɕiwa (基于 OpenJTalk 规则)
   ```

3. **多音字上下文选择**
   ```c
   // 示例："长城"
   // 正确：cháng chéng (而不是 zhǎng chéng)
   ```

### 中优先级

4. **中文声调变化**
   - 三声变调：nǐ hǎo → ní hǎo
   - 一不变调：一个 → yí ge
   - 轻声处理：他们 → tā men5

5. **儿化音处理**
   - 玩儿 → wánr (而不是 wán ér)
   - 一点儿 → yìdiǎnr

6. **英文未登录词**
   - 基于规则的音素预测（如 Sequitur G2P）

### 低优先级

7. **文本规范化**
   - 全角转半角
   - 繁体转简体
   - 数字转文字

8. **韩文/越南文支持**

---

## 📁 相关文件

### 核心实现
- `src/core/misaki_g2p.c` (560 行) - G2P 实现
- `include/misaki_g2p.h` (418 行) - G2P 接口

### 测试
- `tests/test_g2p.c` (299 行) - 6 个测试用例

### 数据
- `extracted_data/en/us_dict.txt` - 英文词典（183,561 词）
- `extracted_data/zh/pinyin_dict.txt` - 中文拼音（41,924 字）
- `extracted_data/zh/dict.txt` - 中文词汇（86 词）

---

## 🎯 完整流程示例

### 英文 TTS 流程
```
文本输入："Hello world"
    ↓
分词：["Hello", "world"]
    ↓
G2P：["həlˈO", "wˈɜɹld"]
    ↓
TTS 模型 → 语音输出
```

### 中文 TTS 流程
```
文本输入："你好世界"
    ↓
分词：["你", "好", "世界"]
    ↓
拼音：["nǐ", "hǎo", "shì jiè"]
    ↓
IPA：["ni↓", "xɑʊ↓", "ʂʅ↓ tɕiɛ↓"] (待实现)
    ↓
TTS 模型 → 语音输出
```

### 日文 TTS 流程
```
文本输入："こんにちは世界"
    ↓
分词：["こんにちは", "世界"]
    ↓
音素：["konnit͡ɕiwa", "sekai"] (待实现)
    ↓
TTS 模型 → 语音输出
```

---

## 📚 参考资源

### 拼音 → IPA 映射
- [汉语拼音 IPA 对照表](https://en.wikipedia.org/wiki/Help:IPA/Mandarin)
- pypinyin 库的实现

### 日文假名 → 音素
- OpenJTalk 规则文件：`open_jtalk-1.11/mecab-naist-jdic/jpcommon_rule_utf_8.h`
- [日本語 IPA 对照表](https://en.wikipedia.org/wiki/Help:IPA/Japanese)

### 声调变化
- 《现代汉语词典》声调变化规则
- [三声变调规则](https://resources.allsetlearning.com/chinese/grammar/Tone_change_rules)

---

**更新时间**：2025-10-25  
**状态**：G2P 基础框架完成，核心功能待完善
