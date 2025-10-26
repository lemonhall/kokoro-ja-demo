# Misaki C Port - 多语言分词与G2P系统实现状态

## 📋 项目概述

本项目为 Kokoro 多语言语音合成系统的 C 语言移植版本，支持**中文、日文、英文**三种语言的分词和 G2P（Grapheme-to-Phoneme）转换。

**最近更新**：昨晚（2025-10-25）成功攻克日文分词模块，采用完整的 Viterbi 算法实现，达到可用状态！

---

## ✅ 已完成模块总览

### 1. **架构重构**（模块化）

**文件分离**：
- `misaki_tokenizer.c` - 公共基础实现（Token、TokenList、DAG）
- `misaki_tokenizer_zh.c` - 中文分词（Jieba 算法）
- `misaki_tokenizer_en.c` - 英文分词（空格分割）
- `misaki_tokenizer_ja.c` - 日文分词（Viterbi 算法）⭐ **新完成**
- `misaki_g2p_zh.c` - 中文 G2P（拼音→IPA）
- `misaki_g2p_en.c` - 英文 G2P（CMUdict）
- `misaki_g2p_ja.c` - 日文 G2P（假名→IPA，待完善）

### 2. **日文分词器**（昨晚重点突破！）

**算法**：完整 Viterbi 最优路径搜索（类似 MeCab）

**核心特性**：
- ✅ Lattice（词格）构建
- ✅ Viterbi 动态规划求最优路径
- ✅ 词性转移成本矩阵支持
- ✅ 词长奖励机制（25.0 系数）
- ✅ 未登录词单字回退

**实现亮点**：
```c
// 节点成本 = -log(频率) - 长度奖励
double node_cost = -log(freq) - (word_char_len - 1) * 25.0;

// 单字符高惩罚，避免过度切分
LatticeNode *node = misaki_lattice_add_node(..., 30.0);

// 词性转移成本
double trans_cost = misaki_get_transition_cost(from->feature, to->feature);
```

**测试结果**：
- ✅ 分词准确度显著提升
- ✅ 长词完整性保障（如「くれました」不被拆分）
- ✅ 成功处理复杂日文句子

**文件**：`src/core/misaki_tokenizer_ja.c` (238 行)

### 3. **中文分词器**（Jieba 算法）

**算法流程**：
1. 基于 Trie 树构建 DAG（有向无环图）
2. 动态规划计算最大概率路径
3. 根据路径切分生成 Token 列表

**测试状态**：
- ✅ 16 个单元测试全部通过
- ✅ 真实词典加载（86 个词汇）

**文件**：`src/core/misaki_tokenizer_zh.c` (261 行)

### 4. **中文 G2P**（汉字→拼音→IPA）

**核心函数**：
- ✅ `misaki_zh_pinyin_to_ipa()` - 拼音转 IPA（完整实现）
- ✅ `misaki_zh_g2p()` - 分词 + 拼音查询 + IPA 转换
- ⏳ `misaki_zh_tone_sandhi()` - 声调变化（TODO）
- ⏳ `misaki_zh_erhua()` - 儿化音（TODO）

**示例**：
```
输入："你好世界"
拼音：nǐ hǎo shì jiè
IPA：ni↓ xɑʊ↓ ʂi↘ tɕiɛ↘
```

**映射表**：
- 声母 21 个：b→p, zh→ʈ͡ʂ, x→ɕ 等
- 韵母 34 个：ai→aɪ, ang→ɑŋ 等
- 声调符号：→(1)、↗(2)、↓(3)、↘(4)

**文件**：`src/core/misaki_g2p_zh.c` (404 行)

---

## 🚧 待完善功能

### 高优先级（中文增强）

#### 1. 扩展中文词典

**当前状态**：86 个词汇（extracted_data/zh/dict.txt）

**目标**：扩展至 10 万+ 词汇

**数据来源**：
- Python jieba 词典：https://github.com/fxsjy/jieba/blob/master/jieba/dict.txt
- 提取格式：`词<Tab>词频<Tab>词性`

**实现步骤**：
```bash
# 1. 下载 jieba 完整词典
wget https://raw.githubusercontent.com/fxsjy/jieba/master/jieba/dict.txt

# 2. 转换为 C 版本格式
python tools/convert_jieba_dict.py dict.txt > extracted_data/zh/dict_full.txt

# 3. 更新 Trie 树加载逻辑
```

#### 2. 词频模型优化

**当前问题**：
```c
// 简单的 log(freq) 可能导致单字组合得分高于长词
double word_score = log(freq);
```

**改进方案**：
```c
// 添加词长奖励（类似日文分词器）
double word_score = log(freq) + (word_char_len - 1) * LENGTH_BONUS;

// 或使用 jieba 原版公式
// score = log(freq) - log(total_freq)
```

**代码位置**：`src/core/misaki_tokenizer_zh.c:calculate_route()`

#### 3. HMM 未登录词识别

**目标**：识别词典中不存在的词（人名、地名、新词）

**数据结构**：
```c
typedef struct {
    double start_prob[4];      // 初始概率 [B, M, E, S]
    double trans_prob[4][4];   // 转移概率矩阵
    double emit_prob[4][65536]; // 发射概率
} HMMModel;
```

**算法**：Viterbi 状态序列识别（B-M-E-S 标注）

**数据来源**：jieba 的 finalseg 模块（prob_emit.py, prob_start.py, prob_trans.py）

#### 4. 多音字上下文选择

**当前**：使用第一个拼音（简化处理）

**目标**：根据上下文选择正确读音

**示例**：
- "长城" → cháng chéng（而非 zhǎng chéng）
- "银行" → yín háng（而非 yín xíng）

**实现方式**：
- 基于词典：为常见多音字词组预设读音
- 基于规则：词性 + 上下文启发式规则
- 基于模型：训练 LSTM/Transformer 模型（高级）

#### 5. 声调变化处理

**规则**：
- **三声变调**：nǐ hǎo → ní hǎo
- **"一"变调**：一个 → yí ge
- **"不"变调**：不对 → bú duì
- **轻声**：他们 → tā men5

**实现位置**：`misaki_zh_tone_sandhi()`（已定义接口，待实现）

#### 6. 儿化音处理

**规则**：
- 玩儿 → wánr（而非 wán ér）
- 一点儿 → yìdiǎnr

**实现位置**：`misaki_zh_erhua()`（已定义接口，待实现）

---

### 中优先级

#### 7. 全模式分词

**目标**：返回所有可能的词组合（搜索引擎用途）

**示例**：
```
输入："中国科学技术大学"
精确模式：["中国科学技术大学"]
全模式：["中国", "科学", "技术", "大学", "中国科学技术大学"]
```

**实现**：遍历 DAG 所有路径

#### 8. 搜索引擎模式

**目标**：对长词再切分，提高召回率

**接口**：`misaki_zh_tokenize_search()`（已定义，待实现）

#### 9. 用户词典支持

**当前接口**：
```c
typedef struct {
    Trie *dict_trie;       // 系统词典
    Trie *user_trie;       // 用户词典 ⭐
    bool enable_userdict;  // 启用标志
} ZhTokenizerConfig;
```

**优先级**：用户词典 > 系统词典

#### 10. 文本规范化

**功能**：
- ⏳ 全角转半角
- ⏳ 繁体转简体
- ⏳ 数字转文字（123 → 一百二十三）

---

### 低优先级

11. ⏳ 词性标注（从词典提取）
12. ⏳ 性能优化（内存池、SIMD）
13. ⏳ 完善日文/英文 G2P

---

## 📊 测试覆盖率总览

| 模块 | 文件数 | 测试数 | 通过率 |
|------|--------|--------|--------|
| 公共基础 | 1 | 13 | ✅ 100% |
| 中文分词 | 1 | 3 | ✅ 100% |
| 日文分词 | 1 | 多个 | ✅ 可用 |
| 英文分词 | 1 | 2 | ✅ 100% |
| 中文 G2P | 1 | 1 | ✅ 100% |
| 英文 G2P | 1 | 2 | ✅ 100% |
| 日文 G2P | 1 | 1 | ⚠️ 待完善 |

---

## 📁 核心文件清单

### 实现文件
- `src/core/misaki_tokenizer.c` (460 行) - 公共基础
- `src/core/misaki_tokenizer_zh.c` (261 行) - 中文分词 ⭐
- `src/core/misaki_tokenizer_ja.c` (238 行) - 日文分词 ⭐ 新完成
- `src/core/misaki_tokenizer_en.c` (117 行) - 英文分词
- `src/core/misaki_g2p_zh.c` (404 行) - 中文 G2P ⭐
- `src/core/misaki_g2p_en.c` (78 行) - 英文 G2P
- `src/core/misaki_g2p_ja.c` (105 行) - 日文 G2P

### 头文件
- `include/misaki_tokenizer.h` (10.9KB)
- `include/misaki_g2p.h` (12.3KB)
- `include/misaki_viterbi.h` (7.5KB) ⭐ 日文专用

### 测试文件
- `tests/test_tokenizer_zh.c` - 中文分词测试
- `tests/test_tokenizer_ja.c` - 日文分词测试
- `tests/test_g2p_zh.c` - 中文 G2P 测试

### 数据文件
- `extracted_data/zh/dict.txt` (86 词) ⚠️ 需扩展
- `extracted_data/zh/pinyin_dict.txt` (41,924 汉字)
- `extracted_data/ja/ja_pron_dict.tsv` (日文词典)
- `extracted_data/en/us_dict.txt` (183,561 词)

---

## 🎯 下一步行动计划（中文增强）

### 第一阶段：数据扩充（1天）
1. ✅ 下载 jieba 完整词典
2. ✅ 编写转换脚本 `generate_chinese_dict.py`
3. ✅ 生成 `extracted_data/zh/dict_full.txt`（34.9万+词）
4. ✅ 更新 Trie 加载逻辑，测试大词典性能

**实际成果**：
- 词典从 86 词 扩充至 **349,041 词**（**4,058x** 增长）
- 加载时间：2.2 秒
- 分词吐吐量：654 次/秒

### 第二阶段：算法优化（1-2天）
1. ✅ 修复词频模型（添加词长奖励，系数 15.0）
2. ⏳ 实现 HMM 未登录词识别
3. ✅ 测试分词准确率提升效果

**实际成果**：
- 词长奖励系数：3.0 → 10.0 → **15.0**（迭代调优）
- 分词效果大幅提升：
  - "我们是中国人民" → "我们 / 是 / 中国 / 人民" ✅
  - "人工智能技术正在改变世界" → "人工智能 / 技术 / 正在 / 改变 / 世界" ✅

### 第三阶段：G2P 完善（1天）
1. ✅ 实现多音字上下文选择（基于词典）
2. ✅ 实现声调变化规则
3. ✅ 实现儿化音处理

### 第四阶段：测试验证（0.5天）
1. ✅ 增加综合测试用例
2. ✅ 对比 Python 版本准确率
3. ✅ 性能基准测试

---

## 📚 参考资源

### 中文分词
- jieba 源码：https://github.com/fxsjy/jieba
- 核心算法：`jieba/__init__.py`
- HMM 模块：`jieba/finalseg/`

### 拼音与 G2P
- pypinyin：https://github.com/mozillazg/python-pinyin
- 汉语拼音 IPA 对照：https://en.wikipedia.org/wiki/Help:IPA/Mandarin

### 日文分词
- MeCab 算法：http://taku910.github.io/mecab/
- Viterbi 算法：https://en.wikipedia.org/wiki/Viterbi_algorithm

---

## 🔥 昨晚成果总结

**攻克难题**：日文分词器从简化版升级为完整 Viterbi 实现

**关键突破**：
1. ✅ Lattice 词格正确构建
2. ✅ Viterbi 最优路径搜索成功
3. ✅ 词长奖励系数调优（25.0）
4. ✅ 单字惩罚系数调优（30.0）
5. ✅ 词性转移成本矩阵集成

**效果**：分词准确度达到生产可用水平！

---

**文档创建时间**：2025-10-25  
**最后更新**：2025-10-26（大更新：架构重构 + 日文分词完成）  
**维护者**：Misaki C Port Team

---

## 今日任务：强化中文模块 🚀

重点方向：
1. 扩展中文词典至 10 万+
2. 优化中文分词算法
3. 完善中文 G2P 功能
4. 实现多音字、声调变化、儿化音处理
