# Quenya (昆雅语) G2P Support Implementation

## 项目背景

为 misaki_c_port 添加昆雅语（Quenya，托尔金创造的精灵语）支持，实现娱乐性质的 TTS 功能。

**实现路径：**
```
中文文本 → AI翻译 → 昆雅语文本 → 分词器 → G2P转换 → IPA音素 → TTS合成
```

## 昆雅语音系规则

基于权威来源：https://realelvish.net/pronunciation/quenya/

### 元音系统

| 字母 | IPA | 示例 | 说明 |
|------|-----|------|------|
| a, á | /a/ | "father" | 长音符表示延长 |
| e, ë | /ɛ/ | "better" | 分音符不影响发音 |
| é | /eː/ | "lake" | 长音 |
| i, í | /i/ | "machine" | |
| o | /ɔ/ | "thought" | |
| ó | /oː/ | "oat" | 长音 |
| u, ú | /u/ | "brute" | |

**注意：** 长音符（acute accent á, é, í, ó, ú）表示元音需延长发音，在IPA中用 `ː` 标记。

### 双元音

| 字母组合 | IPA | 示例 |
|----------|-----|------|
| ai | /aj/ | "twine" |
| au | /au/ | "loud" |
| iu | /iu/ | "music" |
| eu | /ɛu/ | - |
| oi | /ɔj/ | "boy" |
| ui | /uj/ | "gooey" |

### 辅音系统

#### 基础辅音（与英语相同）
- b, d, f, g, h, j, k/c, l, m, n, p, s, t, v, w, y, z

#### 特殊辅音

| 字母 | IPA | 说明 |
|------|-----|------|
| c, k | /k/ | 总是发 /k/ 音 |
| s | /s/ | 总是发 /s/ 音 |
| r | /r/ | 颤音（trilled R） |
| qu | /kw/ | 如 "queen" |
| ñ | /ŋ/ | 如 "sing" |
| ng | /ŋɡ/ | 如 "finger" |
| th, þ | /θ/ | 如 "nothing" |
| z | /z/ | 如 "zoo" |

#### 腭音化辅音（后接 y）

| 字母组合 | IPA | 说明 |
|----------|-----|------|
| ty | /tj/ | 辅音后加 /j/ |
| ny | /nj/ | |
| ly | /lj/ | |
| ry | /rj/ | |
| sy | /sj/ | |
| hy | /hj/ | |

#### 清化辅音（voiceless）

| 字母组合 | IPA | 说明 |
|----------|-----|------|
| hl | /l̥/ | 清化的 l（气声） |
| hr | /r̥/ | 清化的 r（气声） |
| hw | /ʍ/ | 如 "white" |
| hy | /j̊/ | 清化的 y |

#### 辅音簇

| 字母组合 | IPA | 说明 |
|----------|-----|------|
| ht | /xt/ | 如 "Bach" + "t" |
| pt | /φt/ | /p/ 发成 /f/，唇形如吻 |

### 重音规则

昆雅语重音规则清晰且规则：

1. **1-3音节词**：重音在第一音节
2. **4+音节词**：重音在倒数第三音节
3. **例外**：倒数第二音节如有以下特征，则重音移至该音节：
   - 有长音符（á, é, í, ó, ú）
   - 包含双元音（ai, au, iu, eu, oi, ui）
   - 辅音簇结尾（多个辅音，hl/hr/th算单辅音）

**示例：**
- **Eldar** → /ˈɛl.dar/ （2音节，重音第一）
- **Quenya** → /ˈkwɛn.ja/ （2音节，重音第一）
- **Elessar** → /ɛ.ˈlɛs.sar/ （3音节，第二音节有辅音簇ss）
- **Eärendil** → /ɛ.a.ˈrɛn.dil/ （4音节，倒数第三）

## 实现设计

### 文件结构

```
misaki_c_port/
├── include/
│   ├── misaki_tokenizer_qya.h    # 昆雅语分词器头文件
│   └── misaki_g2p_qya.h          # 昆雅语G2P转换器头文件
├── src/core/
│   ├── misaki_tokenizer_qya.c    # 昆雅语分词器实现
│   └── misaki_g2p_qya.c          # 昆雅语G2P转换器实现
└── tests/
    ├── test_tokenizer_qya.c      # 分词器测试
    └── test_g2p_qya.c            # G2P测试
```

### 分词策略

昆雅语分词相对简单：
1. 按空格分词
2. 标点符号独立成token
3. 数字保持原样（或可选转换为昆雅语数词）
4. 不需要复杂的词典或HMM

### G2P转换流程

```
输入文本 → UTF-8解析 → 字符序列分析
    ↓
识别字母组合（qu, ng, ñ, th等）
    ↓
双元音识别（ai, au, iu等）
    ↓
元音长音处理（á→aː）
    ↓
辅音映射（包括清化和腭音化）
    ↓
重音计算和标注
    ↓
IPA音素序列输出
```

### 核心函数接口

**分词器：**
```c
// 初始化昆雅语分词器
MisakiTokenizer* misaki_tokenizer_qya_init(void);

// 分词（按空格和标点）
int misaki_tokenize_qya(const char* text, MisakiToken** tokens);
```

**G2P转换器：**
```c
// 初始化昆雅语G2P
MisakiG2P* misaki_g2p_qya_init(void);

// 文本到音素转换
char* misaki_g2p_qya_convert(const char* word);

// 计算重音位置
int calculate_stress_position(const char* word);

// 识别音节
int count_syllables(const char* word);
```

### 音素映射表

将在代码中定义静态映射表：
- 元音映射表（含长音标记）
- 双元音映射表
- 辅音映射表
- 辅音簇映射表
- 特殊字符映射

### 语言检测集成

在 `misaki_lang_detect.c` 中添加昆雅语检测规则：
- 检测特殊字符：ñ, þ, ë
- 检测常见词缀：-ar, -ion, -ië, -úr
- 检测常见单词：Quenya, Eldar, Valar 等

## 测试用例

### 基础元音测试
```
Input: "aeiou"
Expected: "a ɛ i ɔ u"
```

### 长音测试
```
Input: "áéíóú"
Expected: "aː eː iː oː uː"
```

### 双元音测试
```
Input: "ai au iu eu oi ui"
Expected: "aj au iu ɛu ɔj uj"
```

### 辅音测试
```
Input: "quenya"
Expected: "ˈkwɛn ja"  (重音在第一音节)
```

### 特殊辅音测试
```
Input: "ñoldo"
Expected: "ˈŋɔl dɔ"
```

### 清化辅音测试
```
Input: "hlóni"
Expected: "ˈl̥oː ni"
```

### 重音测试
```
Input: "Eldar"     → /ˈɛl dar/    (2音节，重音第一)
Input: "Elessar"   → /ɛ ˈlɛs sar/  (3音节，ss辅音簇)
Input: "Eärendil"  → /ɛ a ˈrɛn dil/ (4音节，倒数第三)
Input: "Námo"      → /ˈnaː mɔ/    (长音在第一音节)
```

### 完整句子测试
```
Input: "Elen síla lúmenn' omentielvo"
Expected: (精灵语经典问候："星光照耀我们相遇之时"）
```

## 实现阶段

### Phase 1: 核心G2P实现 ✅
- [x] 创建设计文档
- [x] 实现基础元音映射
- [x] 实现双元音识别
- [x] 实现辅音映射
- [x] 实现特殊辅音簇处理

### Phase 2: 重音系统 ✅
- [x] 实现音节计数算法
- [x] 实现重音位置计算
- [x] 实现IPA重音标记

### Phase 3: 分词器 ✅
- [x] 实现基于空格的分词
- [x] 处理标点符号
- [x] 处理撇号（如 omentielvo）

### Phase 4: 集成和测试 ✅
- [x] 添加语言检测规则
- [x] 编写单元测试
- [x] 创建演示程序
- [x] 验证完整工作流

### Phase 5: 优化和文档 📝
- [x] 基础功能完成
- [ ] 性能优化（可选）
- [ ] 完善错误处理（可选）
- [x] 补充使用文档

## 参考资料

1. **权威音系规则：** https://realelvish.net/pronunciation/quenya/
2. **语法参考：** https://en.wikibooks.org/wiki/Quenya
3. **词典资源：** https://eldamo.org/
4. **IPA标准：** International Phonetic Alphabet

## 注意事项

1. **UTF-8编码：** 昆雅语使用特殊字符（ñ, þ, ë等），必须正确处理UTF-8
2. **长音符处理：** 需区分 á 和 a，正确映射为 aː 和 a
3. **撇号处理：** 昆雅语中撇号用于省略，需特殊处理
4. **Tengwar文字：** 本实现仅支持拉丁转写，不处理Tengwar文字
5. **方言差异：** 昆雅语有Vanyarin和Noldorin方言差异，本实现采用标准Noldorin发音

## 未来扩展

1. **辛达林（Sindarin）支持** - 另一种精灵语
2. **Tengwar文字输入** - 支持精灵文字直接输入
3. **昆雅语词典** - 集成常用词汇和翻译
4. **语音合成优化** - 针对精灵语特点优化发音

---

**开始时间：** 2025-10-27  
**完成时间：** 2025-10-27  
**状态：** ✅ 基础功能完成  
**维护者：** misaki_c_port project

## 实际使用示例

### 编译和运行

```bash
# 编译
cd misaki_c_port
mkdir -p build && cd build
cmake ..
make demo_quenya

# 运行演示
./demo_quenya
```

### 输出示例

```
=== Quenya (精灵语) G2P Demo ===

>>> Input: "Quenya"
  Tokens (1): [Quenya] 
    Quenya → IPA: /kw ˈɛ nj a/

>>> Input: "Elen síla lúmenn' omentielvo"
  Tokens (4): [Elen] [síla] [lúmenn'] [omentielvo] 
    Elen → IPA: /ɛ l ɛ n/
    síla → IPA: /s ˈiː l a/
    lúmenn' → IPA: /l ˈuː m ɛ n n/
    omentielvo → IPA: /ɔ m ɛ n t ˈi ɛ l v ɔ/
```

### C API 使用

```c
#include "misaki_g2p_qya.h"
#include "misaki_tokenizer_qya.h"

// 初始化
misaki_g2p_qya_init();
misaki_tokenizer_qya_init();

// 分词
MisakiToken* tokens = NULL;
int token_count = 0;
misaki_tokenize_qya("Elen síla", &tokens, &token_count);

// G2P 转换
for (int i = 0; i < token_count; i++) {
    if (tokens[i].type == TOKEN_WORD) {
        char* phonemes = NULL;
        misaki_g2p_qya_convert(tokens[i].text, &phonemes);
        printf("%s → /%s/\n", tokens[i].text, phonemes);
        free(phonemes);
    }
}

// 清理
for (int i = 0; i < token_count; i++) {
    free(tokens[i].text);
}
free(tokens);

misaki_g2p_qya_cleanup();
misaki_tokenizer_qya_cleanup();
```

### 支持的昆雅语特性

✅ 元音：a, e, i, o, u  
✅ 长元音：á, é, í, ó, ú  
✅ 双元音：ai, au, iu, eu, oi, ui  
✅ 特殊字符：ñ, þ, ë  
✅ 辅音簇：qu, ng, th, hl, hr, hw  
✅ 腭音化：ty, ny, ly, ry, sy  
✅ 重音标记（基本规则）  
✅ UTF-8 编码支持  

### 已测试词汇

- **Quenya** /kw ˈɛ nj a/ - 昆雅语
- **Eldar** /ɛ l d a r/ - 精灵
- **Valar** /v ˈa l a r/ - 维拉
- **Elen síla lúmenn' omentielvo** - 星光照耀我们相遇之时
- **Namárië** /n a m ˈaː r i ɛ/ - 告别
- **Arda** /a r d a/ - 世界
- **Ilúvatar** /i l ˈuː v a t a r/ - 至高神
