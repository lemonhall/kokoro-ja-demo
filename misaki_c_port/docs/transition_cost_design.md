# 日文分词词性转移成本矩阵 - 优化设计方案

## 📊 问题背景

### 原始 MeCab/UniDic 矩阵

- **文件**: `matrix.bin`
- **大小**: 459 MB 💀
- **结构**: 约 15,000 × 15,000 的成本矩阵
- **总元素**: ~240,000,000 个 short 值
- **问题**: 
  - 文件太大，不适合移动端
  - 加载慢，内存占用高
  - 实际使用中大部分词性组合很少出现

### 当前分词器状态

- ✅ **已实现**: Viterbi 算法 + 词频 + 长度奖励
- ✅ **效果**: 从 43 token → 27 token (改善 37%)
- ⚠️ **问题**: 
  - "久しぶり" → "久" + "しぶ" + "り" (词典缺失)
  - "会って" → "会" + "って" (动词活用未识别)
  - "食べながら" → "食" + "べ" + "ながら" (复合未识别)

---

## 🎯 优化方案

### 方案 1: 参数调优（已完成）✅

**实现**: 调整 Viterbi 算法的成本计算

```c
// 当前参数
double node_cost = -log(freq) - (word_char_len - 1) * 10.0;  // 长度奖励
double single_char_cost = 20.0;  // 单字惩罚
```

**优点**:
- ✅ 简单直接，无需额外数据
- ✅ 已证明有效

**缺点**:
- ❌ 无法处理词性搭配规则
- ❌ 对复杂语法结构效果有限

---

### 方案 2: 硬编码常用词性规则 ⭐ **推荐**

**核心思路**: 根据日语语法规则，硬编码常见的词性转移模式

#### 2.1 日语常见词性组合

| 前接词性 | 后接词性 | 成本调整 | 示例 |
|---------|---------|---------|------|
| 動詞 | 助動詞 | -5.0 | 食べ + ます |
| 動詞 | 助詞(て) | -5.0 | 会っ + て |
| 名詞 | 助詞(は/が/を/に) | -3.0 | 友達 + と |
| 形容詞 | 名詞 | -2.0 | 新しい + カフェ |
| 名詞 | 名詞 | +3.0 | 避免过度复合 |
| 接頭辞 | 名詞 | -4.0 | 新 + しい |

#### 2.2 实现代码

```c
/**
 * 获取词性转移成本
 * 基于硬编码的日语语法规则
 */
double get_transition_cost(const char *left_tag, const char *right_tag) {
    if (!left_tag || !right_tag) {
        return 0.0;
    }
    
    // 規則 1: 動詞 + 助動詞 (食べ + ます)
    if (strstr(left_tag, "動詞") && strstr(right_tag, "助動詞")) {
        return -5.0;  // 强鼓励
    }
    
    // 規則 2: 動詞 + 助詞(て形) (会っ + て)
    if (strstr(left_tag, "動詞") && strstr(right_tag, "助詞")) {
        return -4.0;
    }
    
    // 規則 3: 名詞 + 助詞 (友達 + と)
    if (strstr(left_tag, "名詞") && strstr(right_tag, "助詞")) {
        return -3.0;
    }
    
    // 規則 4: 形容詞 + 名詞 (新しい + カフェ)
    if (strstr(left_tag, "形容詞") && strstr(right_tag, "名詞")) {
        return -2.0;
    }
    
    // 規則 5: 接頭辞/接尾辞 + 名詞 (新 + しい)
    if ((strstr(left_tag, "接頭辞") || strstr(left_tag, "接尾辞")) 
        && strstr(right_tag, "名詞")) {
        return -4.0;
    }
    
    // 規則 6: 名詞 + 名詞 (避免过度复合)
    if (strstr(left_tag, "名詞") && strstr(right_tag, "名詞")) {
        return +2.0;  // 轻微惩罚
    }
    
    // 默认：无额外成本
    return 0.0;
}
```

#### 2.3 集成到 Viterbi

```c
// 在 misaki_tokenizer_ja.c 的边连接代码中
for (int i = 0; i < counts_by_pos[pos]; i++) {
    LatticeNode *from = nodes_by_pos[pos][i];
    int next_pos = pos + from->length;
    
    if (next_pos < text_len) {
        for (int j = 0; j < counts_by_pos[next_pos]; j++) {
            LatticeNode *to = nodes_by_pos[next_pos][j];
            
            // ⭐ 添加词性转移成本
            double trans_cost = get_transition_cost(from->tag, to->tag);
            
            misaki_lattice_add_edge(from, to, trans_cost);
        }
    }
}
```

**优点**:
- ✅ 实现简单（约 50 行代码）
- ✅ 无需外部数据文件
- ✅ 可以根据实测效果快速调整
- ✅ 内存占用几乎为 0

**缺点**:
- ❌ 规则数量有限
- ❌ 需要一定的日语语法知识

**预期效果**:
- "会って" → 识别为完整词
- "食べながら" → 可能改善（取决于词典）
- 27 token → 预计 20-23 token

---

### 方案 3: 提取简化词性矩阵 ⭐⭐ **备选**

**核心思路**: 只保留最常见的 100-200 个词性组合

#### 3.1 统计最常用词性

从 68,087 个词汇中统计词性分布：

```python
# 统计词性出现频率
pos_freq = {}
with open('ja_pron_dict.tsv') as f:
    for line in f:
        parts = line.strip().split('\t')
        if len(parts) >= 4:
            pos = parts[3]  # 词性
            pos_freq[pos] = pos_freq.get(pos, 0) + 1

# 选择 top 100 词性
top_pos = sorted(pos_freq.items(), key=lambda x: -x[1])[:100]
```

#### 3.2 提取子矩阵

```python
# 从 matrix.bin 提取对应的成本
pos_to_id = {pos: i for i, (pos, _) in enumerate(top_pos)}

with open('transition_cost_100.txt', 'w') as f:
    for left_pos in top_pos:
        for right_pos in top_pos:
            left_id = pos_to_id[left_pos[0]]
            right_id = pos_to_id[right_pos[0]]
            cost = matrix[left_id][right_id]
            f.write(f"{left_pos[0]}\t{right_pos[0]}\t{cost}\n")
```

#### 3.3 数据结构

```c
// 词性 ID 映射
typedef struct {
    const char *tag;  // 词性名称
    int id;           // 内部 ID (0-99)
} TagMapping;

// 成本矩阵 (100 x 100 = 10,000 个 short)
static short transition_matrix[100][100];

// 词性名称到 ID 的映射
static TagMapping tag_map[] = {
    {"助詞", 0},
    {"助動詞", 1},
    {"名詞", 2},
    {"動詞", 3},
    // ... 共 100 个
};

// 查询成本
double get_transition_cost_from_matrix(const char *left_tag, const char *right_tag) {
    int left_id = find_tag_id(left_tag);
    int right_id = find_tag_id(right_tag);
    
    if (left_id < 0 || right_id < 0) {
        return 0.0;  // 未知词性，使用默认
    }
    
    return (double)transition_matrix[left_id][right_id];
}
```

**数据大小**:
- 100 × 100 × 2 字节 = **20 KB** ✅
- 加上词性映射表 ≈ **25 KB**

**优点**:
- ✅ 基于真实数据，覆盖面广
- ✅ 文件小，适合移动端
- ✅ 可以离线生成，运行时直接加载

**缺点**:
- ❌ 需要额外的数据文件
- ❌ 需要编写提取脚本
- ❌ 对非常见词性无效

**预期效果**:
- 27 token → 预计 18-22 token
- 更接近 MeCab 的原始效果

---

## 🎯 最终推荐方案

### **阶段 1: 立即实施方案 2** ⭐⭐⭐⭐⭐

1. **实现硬编码规则** (1小时)
   - 添加 `get_transition_cost()` 函数
   - 集成到 Viterbi 边连接逻辑
   - 编写 10-15 条核心规则

2. **测试验证** (30分钟)
   - 使用长句测试
   - 对比优化前后的 token 数
   - 调整规则权重

3. **预期效果**:
   - ✅ 代码量小（< 100 行）
   - ✅ 无外部依赖
   - ✅ 立即可用
   - ✅ 性能提升 10-20%

### **阶段 2: 可选实施方案 3** ⭐⭐⭐

如果方案 2 效果不满意，再考虑：

1. **统计分析** (1小时)
   - 分析 68K 词典的词性分布
   - 选择 top 100 词性

2. **提取矩阵** (2小时)
   - 编写 Python 脚本解析 matrix.bin
   - 提取子矩阵并导出文本格式
   - 转换为 C 数组

3. **集成加载** (1小时)
   - 修改分词器加载矩阵
   - 实现查询函数

---

## 📝 实现清单

### 方案 2: 硬编码规则

- [ ] 创建 `misaki_transition_rules.c/h`
- [ ] 实现 `get_transition_cost()` 函数
- [ ] 修改 `misaki_tokenizer_ja.c` 集成规则
- [ ] 编写测试用例
- [ ] 性能对比测试

### 方案 3: 简化矩阵（可选）

- [ ] 编写 `extract_transition_matrix.py`
- [ ] 统计词性分布，选择 top 100
- [ ] 从 matrix.bin 提取子矩阵
- [ ] 生成 C 语言数组文件
- [ ] 实现加载和查询函数
- [ ] 对比方案 2 的效果差异

---

## 🎯 预期成果

| 指标 | 当前 | 方案2 | 方案3 |
|-----|------|-------|-------|
| Token数 | 27 | 20-23 | 18-22 |
| 代码量 | - | +100行 | +300行 |
| 数据大小 | 0 | 0 | 25KB |
| 实现时间 | - | 1.5小时 | 4小时 |
| 移动端适配 | ✅ | ✅ | ✅ |

---

## 📌 总结

**推荐路线**:
1. ✅ **先实现方案 2**（硬编码规则）
   - 投入小，见效快
   - 适合快速迭代
   
2. ⏸️ **根据效果决定是否需要方案 3**
   - 如果方案 2 已满足需求 → 停止
   - 如果还需要进一步优化 → 实施方案 3

**关键原则**: 
- 🎯 **够用就好**，不过度优化
- 📱 **移动端优先**，控制数据大小
- 🚀 **快速迭代**，根据实测调整

---

## 📖 参考资料

- [MeCab 官方文档](https://taku910.github.io/mecab/)
- [UniDic 词典规范](https://clrd.ninjal.ac.jp/unidic/)
- 本项目测试结果: 43 token → 27 token (长度奖励优化)
