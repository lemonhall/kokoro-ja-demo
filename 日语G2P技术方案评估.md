# 日语 G2P 技术方案评估：C 语言重写 vs 其他方案

## 前言

在实现 Kokoro TTS 的 Android 移植过程中，**性能**和**音质**问题都已完美解决。现在唯一的障碍是 **G2P（Grapheme-to-Phoneme，文字到音素转换）**，具体来说就是：

**如何让用户输入任意日文（包括汉字），而不只是假名？**

本文详细评估各种技术方案，重点分析**用 C 重写 MeCab + JAG2P 并封装成 JNI** 的可行性。

---

## 一、问题定义

### 1.1 当前状态

✅ **已解决**：
- 性能：RTF 0.70x（比实时快 30%）
- 音质：与 PyTorch 原版完全一致
- 假名输入：完美支持

❌ **待解决**：
- 汉字输入：不支持
- 用户体验：需要手动把汉字转成假名

### 1.2 完整 G2P 流程

要支持汉字输入，需要三个步骤：

```
汉字文本 → [分词] → [读音转换] → 假名 → [假名转音素] → 音素 → [TTS模型] → 语音
"今日は天気" → "今日/は/天気" → "きょう/は/てんき" → "kʲoː/β/a/teN/ki" → 语音 ✅
```

| 步骤 | 当前方案 | 问题 |
|------|---------|------|
| **分词** | 无 | ❌ 无法处理汉字 |
| **读音转换** | 无 | ❌ 无法查词典 |
| **假名→音素** | SimpleJapaneseG2P | ⚠️ 简化版，准确度 70% |

---

## 二、现有方案对比

### 方案 A：Python 方案（PC 端）

**技术栈**：
- MeCab（C++，通过 Python 绑定）
- UniDic（词典，~50MB）
- Misaki/JAG2P（假名→IPA）

**优点**：
- ✅ 准确度 95%+
- ✅ 成熟稳定
- ✅ 文档完善

**缺点**：
- ❌ 无法移植到 Android（C++ 扩展）
- ❌ 词典体积大（50MB）
- ❌ 依赖复杂

---

### 方案 B：Kuromoji（Java 分词器）

**技术栈**：
- Kuromoji（纯 Java 分词器）
- 自己实现假名→IPA

**优点**：
- ✅ Android 兼容
- ✅ 纯 Java，无需 JNI
- ✅ 汉字→假名准确度 95%+

**缺点**：
- ❌ 假名→IPA 需要自己实现（复杂）
- ❌ 整体准确度预估 80%
- ❌ APK 增加 5MB

---

### 方案 C：在线 API

**技术栈**：
- 调用在线 G2P 服务
- 本地只做 TTS

**优点**：
- ✅ 零开发成本
- ✅ 准确度高

**缺点**：
- ❌ 需要网络
- ❌ 依赖第三方服务
- ❌ 不符合"本地化"原则

---

### 方案 D：**C 语言重写 + JNI（本文重点）**

**技术栈**：
- 用 C 重写 MeCab 核心逻辑
- 用 C 重写 JAG2P
- 编译成 .so 库
- JNI 封装供 Kotlin 调用

**优点**：
- ✅ 完全本地化
- ✅ 性能最优（C 语言）
- ✅ 零外部依赖
- ✅ 完全可控

**缺点**：
- ⚠️ **开发工作量大**
- ⚠️ **维护成本高**
- ⚠️ **需要深入理解算法**

---

## 三、方案 D 详细评估：C 重写可行性

### 3.1 MeCab 核心分析

#### 代码规模

| 组件 | 原始 C++ 代码 | 核心逻辑 | C 重写预估 |
|------|--------------|---------|-----------|
| 词典加载 | ~2,000 行 | ~1,000 行 | ~800 行 |
| Double-Array Trie | ~3,000 行 | ~2,000 行 | ~1,500 行 |
| Viterbi 解码器 | ~2,000 行 | ~1,500 行 | ~1,200 行 |
| 其他工具 | ~8,000 行 | 可省略 | ~500 行 |
| **总计** | **~15,000 行** | **~4,500 行** | **~4,000 行** |

#### 技术难点

**1. Double-Array Trie（双数组字典树）**

这是 MeCab 最复杂的部分，用于快速前缀匹配。

**难度**：🔴🔴🔴🔴 (4/5)

**原理**：
```c
// 双数组结构
typedef struct {
    int *base;   // BASE 数组
    int *check;  // CHECK 数组
    int size;
} DoubleArrayTrie;

// 查找逻辑（简化）
int lookup(DoubleArrayTrie *trie, const char *key) {
    int s = 0;  // 初始状态
    for (int i = 0; key[i]; i++) {
        int t = trie->base[s] + key[i];
        if (trie->check[t] != s) return -1;  // 不匹配
        s = t;
    }
    return s;  // 返回状态值
}
```

**需要实现的功能**：
- ✅ Trie 构建（可以预处理，不需要在运行时）
- ✅ 快速查找
- ✅ 前缀匹配

**开发时间**：2-3 天（如果理解原理）

---

**2. Viterbi 算法（动态规划）**

用于找到最优分词路径。

**难度**：🟡🟡🟡 (3/5)

**原理**：
```c
typedef struct {
    char *word;      // 词
    int cost;        // 代价
    int pos;         // 词性
} Token;

// Viterbi 解码（简化）
Token* viterbi(const char *text, Dictionary *dict) {
    int n = strlen(text);
    int *dp = malloc(n * sizeof(int));      // 最小代价
    int *prev = malloc(n * sizeof(int));    // 前驱节点
    
    // 动态规划
    for (int i = 0; i < n; i++) {
        dp[i] = INT_MAX;
        
        // 尝试所有可能的词
        for (int j = 0; j <= i; j++) {
            char word[256];
            strncpy(word, text + j, i - j + 1);
            
            Token *token = dict_lookup(dict, word);
            if (token) {
                int cost = (j > 0 ? dp[j-1] : 0) + token->cost;
                if (cost < dp[i]) {
                    dp[i] = cost;
                    prev[i] = j;
                }
            }
        }
    }
    
    // 回溯找路径
    return backtrack(prev, n);
}
```

**需要实现的功能**：
- ✅ 构建词图（lattice）
- ✅ 计算最优路径
- ✅ 回溯分词结果

**开发时间**：2-3 天

---

**3. 词典格式解析**

UniDic 是二进制格式，需要逆向工程。

**难度**：🟡🟡 (2/5)

**方案**：
- 不直接用 UniDic 二进制
- 把 UniDic 转成自定义的简单格式（CSV 或二进制）
- 只保留必要字段：词、读音、代价、词性

**示例**：
```c
typedef struct {
    char word[64];       // 词：今日
    char reading[64];    // 读音：キョウ
    int cost;            // 代价：5000
    int pos;             // 词性：名词
} DictEntry;

// 词典文件格式（自定义二进制）
// [词数量: 4 bytes]
// [词条1: DictEntry]
// [词条2: DictEntry]
// ...
```

**开发时间**：1-2 天

---

### 3.2 JAG2P（假名→IPA）分析

#### 代码规模

| 组件 | 原始代码 | C 重写预估 |
|------|---------|-----------|
| 假名映射表 | ~200 行 | ~200 行 |
| 拗音处理 | ~100 行 | ~150 行 |
| 长音规则 | ~50 行 | ~80 行 |
| 促音规则 | ~50 行 | ~80 行 |
| 拨音规则 | ~50 行 | ~80 行 |
| **总计** | **~450 行** | **~600 行** |

#### 技术难点

**难度**：🟢🟢 (2/5) - 相对简单

**核心逻辑**：
```c
// 假名→音素映射
typedef struct {
    char kana[8];      // 假名：きゃ
    char phoneme[16];  // 音素：kʲa
} KanaMapping;

KanaMapping kana_map[] = {
    // 单假名
    {"あ", "a"},
    {"い", "i"},
    {"う", "ɯ"},
    
    // 拗音（组合）
    {"きゃ", "kʲa"},
    {"きゅ", "kʲɯ"},
    {"きょ", "kʲo"},
    
    // ... 约 200 条
};

// 转换函数
char* kana_to_phoneme(const char *kana) {
    char *result = malloc(1024);
    int pos = 0;
    
    for (int i = 0; kana[i]; ) {
        // 1. 先尝试匹配 2 字符组合（拗音）
        char combo[8];
        strncpy(combo, kana + i, 6);  // UTF-8 最多 6 字节
        
        if (find_mapping(combo)) {
            strcat(result, find_mapping(combo));
            i += 6;
            continue;
        }
        
        // 2. 单字符映射
        char single[4];
        strncpy(single, kana + i, 3);
        strcat(result, find_mapping(single));
        i += 3;
    }
    
    return result;
}
```

**开发时间**：1-2 天

---

### 3.3 JNI 封装

**难度**：🟢 (1/5) - 简单

**示例**：
```c
// jni/g2p_native.c

#include <jni.h>
#include "mecab.h"
#include "jag2p.h"

JNIEXPORT jstring JNICALL
Java_com_lsl_kokoro_1ja_1android_NativeG2P_textToPhonemes(
    JNIEnv *env,
    jobject thiz,
    jstring text
) {
    // 1. Java String → C String
    const char *input = (*env)->GetStringUTFChars(env, text, NULL);
    
    // 2. 分词
    Token *tokens = mecab_tokenize(input);
    
    // 3. 获取读音
    char kana[1024] = "";
    for (int i = 0; tokens[i].word; i++) {
        strcat(kana, tokens[i].reading);
    }
    
    // 4. 假名→音素
    char *phonemes = kana_to_phoneme(kana);
    
    // 5. C String → Java String
    jstring result = (*env)->NewStringUTF(env, phonemes);
    
    // 6. 清理
    (*env)->ReleaseStringUTFChars(env, text, input);
    free(phonemes);
    
    return result;
}
```

**Kotlin 调用**：
```kotlin
class NativeG2P {
    external fun textToPhonemes(text: String): String
    
    companion object {
        init {
            System.loadLibrary("g2p_native")
        }
    }
}

// 使用
val g2p = NativeG2P()
val phonemes = g2p.textToPhonemes("今日は良い天気")
// 返回: "kʲoːβayoiteNki"
```

**开发时间**：1 天

---

## 四、总体工作量评估

### 4.1 开发时间

| 任务 | 难度 | 预估时间 | 备注 |
|------|------|---------|------|
| **MeCab 部分** |
| Double-Array Trie | 🔴🔴🔴🔴 | 3-4 天 | 最难的部分 |
| Viterbi 算法 | 🟡🟡🟡 | 2-3 天 | 需要理解算法 |
| 词典处理 | 🟡🟡 | 1-2 天 | 格式转换 |
| **JAG2P 部分** |
| 假名→音素映射 | 🟢🟢 | 1-2 天 | 相对简单 |
| **集成部分** |
| JNI 封装 | 🟢 | 1 天 | 标准流程 |
| 测试调优 | 🟡🟡 | 2-3 天 | 验证准确度 |
| **总计** | | **10-15 天** | 全职开发 |

### 4.2 代码量

| 组件 | 代码量 | 备注 |
|------|--------|------|
| MeCab 核心 | ~4,000 行 C | 分词 + Trie + Viterbi |
| JAG2P | ~600 行 C | 假名→IPA |
| JNI 封装 | ~300 行 C | 接口层 |
| 词典数据 | ~10MB | 预处理后的词典 |
| **总计** | **~5,000 行** | |

### 4.3 APK 体积影响

| 组件 | 大小 | 备注 |
|------|------|------|
| .so 库（arm64-v8a） | ~200 KB | C 代码编译后 |
| .so 库（armeabi-v7a） | ~150 KB | 32 位版本 |
| 词典数据 | ~10 MB | 压缩后的词典 |
| **总计** | **~10.5 MB** | |

---

## 五、风险评估

### 5.1 技术风险

| 风险 | 概率 | 影响 | 缓解措施 |
|------|------|------|---------|
| **Double-Array Trie 实现失败** | 中 | 高 | 先用简单 Trie，后续优化 |
| **准确度不达标** | 中 | 中 | 与 MeCab 对比测试 |
| **性能不理想** | 低 | 低 | C 语言性能天然优势 |
| **JNI 兼容性问题** | 低 | 中 | 标准 JNI，兼容性好 |
| **词典版权问题** | 低 | 高 | 使用开源词典（IPAdic） |

### 5.2 维护成本

**挑战**：
- ❌ C 代码难调试
- ❌ 需要维护多平台编译（arm64/arm32/x86）
- ❌ 词典更新需要重新编译
- ❌ Bug 修复成本高

**对策**：
- ✅ 完善的单元测试
- ✅ 详细的技术文档
- ✅ CI/CD 自动化编译

---

## 六、替代方案对比

### 综合对比表

| 方案 | 开发时间 | APK 增加 | 准确度 | 维护成本 | 本地化 | 推荐度 |
|------|---------|---------|--------|---------|--------|--------|
| **C 重写 + JNI** | 10-15 天 | 10 MB | 95% | 高 | ✅ 完全 | ⭐⭐⭐⭐ |
| **Kuromoji + 改进 G2P** | 2-3 天 | 5 MB | 80% | 中 | ✅ 完全 | ⭐⭐⭐⭐⭐ |
| **保持现状（仅假名）** | 0 天 | 0 MB | 70% | 低 | ✅ 完全 | ⭐⭐⭐ |
| **在线 API** | 1 天 | 0 MB | 95% | 低 | ❌ 需网络 | ⭐⭐ |

---

## 七、推荐方案

### 🎯 短期方案（1-2 周）：**Kuromoji + 改进 SimpleG2P**

**理由**：
1. ✅ 开发时间短（2-3 天）
2. ✅ 准确度可接受（80%）
3. ✅ 完全本地化
4. ✅ 易于维护
5. ✅ APK 增加可控（5MB）

**实施步骤**：
1. 集成 Kuromoji（1 天）
2. 改进 SimpleJapaneseG2P：
   - 添加拗音处理（きゃ、しゅ等）
   - 添加长音规则（ー、おう→oː）
   - 添加促音/拨音规则
3. 测试调优（1 天）

**预期效果**：
- 用户可以输入汉字 ✅
- 准确度 80%（日常使用够了）
- 保持轻量级

---

### 🚀 长期方案（1-2 月）：**C 重写 + JNI**

**理由**：
1. ✅ 性能最优
2. ✅ 准确度最高（95%）
3. ✅ 零外部依赖
4. ✅ 完全可控

**实施步骤**：
1. **阶段一**（1 周）：
   - 实现 MeCab 核心（分词 + Trie）
   - 简化版词典（只保留高频词）
   
2. **阶段二**（1 周）：
   - 实现 JAG2P
   - JNI 封装
   - 基础测试
   
3. **阶段三**（2 周）：
   - 优化性能
   - 完善词典
   - 充分测试

**预期效果**：
- 完全自主实现 ✅
- 性能最优 ✅
- 准确度接近 MeCab ✅
- 可以写一篇牛逼的技术博客 🎉

---

## 八、决策建议

### 如果你的目标是...

#### 1️⃣ **快速验证技术可行性**
→ 选择：**保持现状（仅假名）**
- 0 开发成本
- 在 README 里诚实说明限制
- 重点展示性能和音质的成功

#### 2️⃣ **提供可用的产品**
→ 选择：**Kuromoji + 改进 G2P**
- 2-3 天开发时间
- 准确度 80%（日常够用）
- 用户体验显著提升

#### 3️⃣ **追求技术极致/学习目的**
→ 选择：**C 重写 + JNI**
- 10-15 天开发时间
- 准确度 95%
- 完全掌握核心技术
- **建议作为第二阶段项目**

---

## 九、我的个人建议

**阶段性推进**：

**现在（0 天）**：
- 保持现状，发布 v1.0
- 明确说明"仅支持假名"
- 重点宣传性能和音质的成功 ✅

**1-2 周后（如果有需求）**：
- 集成 Kuromoji
- 改进 G2P
- 发布 v1.1，支持汉字输入

**1-2 月后（如果真想做）**：
- C 重写 MeCab + JAG2P
- 发布 v2.0，完全自主实现
- 写一篇深度技术博客："从零实现日语分词器"

---

## 十、总结

### ✅ C 重写方案是可行的

**技术上**：
- 代码量 ~5,000 行（可控）
- 核心算法都有成熟实现可参考
- JNI 封装是标准流程

**资源上**：
- 需要 10-15 天全职开发
- 需要对算法有深入理解
- 需要持续维护

### 🎯 但不一定是最优选择

**对于技术验证项目**：
- Kuromoji 方案更务实（2-3 天 vs 10-15 天）
- 准确度差距不大（80% vs 95%）
- 维护成本更低

**对于学习目的**：
- C 重写是绝佳的学习机会
- 深入理解分词算法
- 可以写出高质量技术文章

---

## 附录：参考资料

### MeCab 相关
- **源码**: https://github.com/taku910/mecab
- **算法论文**: "Applying Conditional Random Fields to Japanese Morphological Analysis"
- **Double-Array Trie**: "An Efficient Implementation of Trie Structures"

### JAG2P 相关
- **Misaki 项目**: https://github.com/hexgrad/kokoro (内置 JAG2P)
- **日语音韵学**: Japanese Phonology (Wikipedia)

### JNI 相关
- **官方文档**: https://docs.oracle.com/javase/8/docs/technotes/guides/jni/
- **NDK 指南**: https://developer.android.com/ndk/guides

### 词典
- **IPAdic**: https://osdn.net/projects/ipadic/
- **UniDic**: https://clrd.ninjal.ac.jp/unidic/

---

**文档版本**: 1.0  
**最后更新**: 2025-10-25  
**作者**: Kokoro-Android-Demo 项目组  

---

**结论**：日语 G2P 确实恶心，但技术上完全可解。建议先用 Kuromoji 快速落地，后续有时间再考虑 C 重写。**重要的是项目已经在性能和音质上取得了巨大成功！** 🎉
