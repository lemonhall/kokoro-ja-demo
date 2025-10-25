# Jieba 中文分词算法完整移植计划

## 📋 当前状态

### ✅ 已完成部分

1. **基础数据结构**
   - [x] Trie 树（前缀树）- `misaki_trie.c`
   - [x] DAG（有向无环图）- `misaki_tokenizer.c`
   - [x] Token / TokenList - `misaki_tokenizer.c`

2. **核心算法（简化版）**
   - [x] DAG 构建（基于 Trie 树前缀匹配）
   - [x] 动态规划最优路径选择（基于词频）
   - [x] 精确模式分词

3. **测试验证**
   - [x] 16 个单元测试全部通过
   - [x] 真实词典加载（86 个词汇）
   - [x] 实际分词效果验证

---

## 🚧 待完善功能

### 1. 词频模型优化

**当前问题**：
```c
// 当前实现：简单的 log(freq) 相加
double word_score = log(freq);
double total_score = word_score + dp[next_pos];
```

**存在的问题**：
- 单字组合的总分数可能高于多字词
- 例如："我"(1000) + "们"(500) > "我们"(1200)
- `log(1000) + log(500) = 13.12 > log(1200) = 7.09`

**改进方案**：
```c
// Python jieba 的实际实现
// route[idx] = max((log(freq) - logtotal + route[x]) for x in DAG[idx])
// 其中 logtotal 是所有词频的总和

// 或者添加词长奖励
double word_score = log(freq) + word_char_len * WORD_LENGTH_BONUS;
```

**代码位置**：`src/core/misaki_tokenizer.c:calculate_route()`

---

### 2. HMM 未登录词识别

**目标**：识别词典中不存在的词（人名、地名、新词等）

**数据结构**：
```c
typedef struct {
    double start_prob[4];      // 初始概率 [B, M, E, S]
    double trans_prob[4][4];   // 转移概率矩阵
    double emit_prob[4][65536]; // 发射概率（简化版）
} HMMModel;
```

**算法步骤**：
1. 对于未在词典中的字符序列
2. 使用 Viterbi 算法计算最优状态序列
3. 根据状态序列切分：
   - B (Begin) → 词首
   - M (Middle) → 词中
   - E (End) → 词尾
   - S (Single) → 单字成词

**实现函数**：
```c
int misaki_hmm_viterbi(const HMMModel *model,
                       const char *text,
                       HMMState *states,
                       int max_length);

MisakiTokenList* misaki_hmm_cut(const HMMModel *model, 
                                 const char *text);
```

**数据来源**：
- 可以从 jieba 的 finalseg 模块提取 HMM 参数
- 位置：`jieba/finalseg/prob_emit.py`, `prob_start.py`, `prob_trans.py`

---

### 3. 全模式分词

**目标**：返回所有可能的词组合（用于搜索引擎）

**当前状态**：
```c
MisakiTokenList* misaki_zh_tokenize_all(void *tokenizer, const char *text) {
    // TODO: 实现全模式分词
    return NULL;
}
```

**实现思路**：
```c
// 遍历 DAG，返回所有路径上的词
for (int i = 0; i < char_count; i++) {
    int next_positions[100];
    int count = misaki_dag_get_next(dag, i, next_positions, 100);
    
    for (int j = 0; j < count; j++) {
        // 提取从 i 到 next_positions[j] 的词
        // 添加到结果列表
    }
}
```

---

### 4. 搜索引擎模式

**目标**：对长词进行再切分，提高搜索召回率

**示例**：
```
输入："中国科学技术大学"
精确模式：["中国科学技术大学"]
搜索模式：["中国", "科学", "技术", "大学", "中国科学技术大学"]
```

**实现步骤**：
1. 先进行精确模式分词
2. 对长度 >= 4 的词进行再切分
3. 合并两次分词结果

---

### 5. 用户词典支持

**当前接口**：
```c
typedef struct {
    Trie *dict_trie;           // 词典 Trie 树（必需）
    bool enable_hmm;           // 是否启用 HMM
    bool enable_userdict;      // 是否启用用户词典
    Trie *user_trie;           // 用户词典 Trie 树（可选）
} ZhTokenizerConfig;
```

**实现要点**：
- 用户词典优先级高于系统词典
- 在 DAG 构建时先匹配用户词典
- 用户词典可动态加载/卸载

---

### 6. 词性标注

**目标**：为分词结果添加词性信息（名词、动词等）

**数据结构**：
```c
typedef struct {
    char *text;          // 词文本
    char *tag;           // 词性标签 ✨ 已有，需填充
    char *phonemes;      // 音素序列
    // ...
} MisakiToken;
```

**实现方式**：
- 在 Trie 树中存储词性信息
- 词典格式：`词<Tab>词频<Tab>词性`
- 分词时同时返回词性

---

## 📊 性能优化方向

### 1. 内存优化
- [ ] DAG 使用内存池分配
- [ ] Token 对象复用
- [ ] Trie 树压缩（Double-Array Trie）

### 2. 速度优化
- [ ] 预计算 log(freq) 避免重复计算
- [ ] 使用 SIMD 加速 UTF-8 解码
- [ ] 多线程分词（长文本切分）

### 3. 准确度优化
- [ ] 使用更大的词典（从 jieba 提取）
- [ ] 调整词频权重
- [ ] 添加专有名词词典

---

## 📁 文件清单

### 已实现
- `src/core/misaki_tokenizer.c` (749 行)
- `tests/test_tokenizer.c` (478 行)
- `tests/test_zh_tokenizer_real.c` (106 行)
- `extracted_data/zh/dict.txt` (86 个词汇)

### 待添加
- `data/zh/hmm_model.bin` - HMM 模型参数
- `data/zh/dict_full.txt` - 完整词典（10万+词汇）
- `data/zh/userdict.txt` - 用户自定义词典

---

## 🎯 优先级排序

### 高优先级（影响分词准确度）
1. ✅ 词频模型优化（修复单字偏好问题）
2. ⏳ HMM 未登录词识别
3. ⏳ 扩展词典（从 86 词 → 10万词）

### 中优先级（功能完善）
4. ⏳ 全模式分词
5. ⏳ 搜索引擎模式
6. ⏳ 用户词典支持

### 低优先级（锦上添花）
7. ⏳ 词性标注
8. ⏳ 性能优化

---

## 🔗 参考资源

### Python jieba 源码
- 仓库：https://github.com/fxsjy/jieba
- 核心文件：
  - `jieba/__init__.py` - 主逻辑
  - `jieba/finalseg/` - HMM 模块
  - `jieba/dict.txt` - 词典文件

### 论文与文档
- 《中文分词算法综述》
- Jieba 官方文档：https://github.com/fxsjy/jieba#%E7%AE%97%E6%B3%95

### 测试数据集
- SIGHAN Bakeoff 中文分词评测数据
- 人民日报语料库

---

## 📝 下一步行动

1. **立即修复**：词频模型（添加词长奖励）
   ```c
   double word_score = log(freq) + word_char_len * 2.0; // 词长奖励
   ```

2. **短期目标**（1-2天）：
   - 提取 jieba 完整词典（10万词）
   - 实现 HMM 未登录词识别

3. **中期目标**（1周）：
   - 完成全模式、搜索模式
   - 性能测试与优化

---

**文档创建时间**：2025-10-25  
**最后更新**：2025-10-25  
**维护者**：Misaki C Port Team
