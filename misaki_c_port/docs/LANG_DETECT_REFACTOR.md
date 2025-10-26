# 语言检测模块重构报告

## 📋 概述

将语言检测功能从 `main.c` 拆分为独立的 C 模块 `misaki_lang_detect`，实现更强大、更准确的多语言文本检测。

## 🎯 解决的问题

### 原有问题
1. **误判问题**：纯汉字日文文本（如"東京都渋谷区"）被误判为中文
2. **检测逻辑简单**：仅基于字符集统计，缺乏语义分析
3. **特征词覆盖不足**：只检查少数几个日文行政区划后缀
4. **缺乏置信度信息**：无法评估检测结果的可靠性
5. **代码耦合**：语言检测逻辑与 main.c 混在一起

### 改进方案

**✅ 采用分层检测策略**：
- 第1层：**字符集快速分析**（有假名→日文，纯拉丁→英文）
- 第2层：**特征词匹配**（高频虚词、助词、后缀）
- 第3层：**n-gram 模式分析**（常见词组搭配）
- 第4层：**回退策略**（汉字默认中文，但检查日文特征词）

## 📁 新增文件

### 1. `include/misaki_lang_detect.h` (193行)
**功能**：API接口定义

**核心类型**：
```c
// 检测结果
typedef struct {
    MisakiLanguage language;    // 检测到的语言
    float confidence;           // 置信度 (0.0-1.0)
    CharsetStats charset;       // 字符集统计
    const char *reason;         // 检测原因
} LangDetectResult;

// 检测器配置
typedef struct {
    bool enable_ngram;
    bool enable_tokenization;
    float confidence_threshold;
    void *zh_tokenizer;
    void *ja_tokenizer;
} LangDetectorConfig;
```

**主要API**：
```c
// 完整检测（带置信度和详细信息）
LangDetectResult misaki_lang_detect_full(LangDetector *detector, const char *text);

// 快速检测（仅基于字符集）
MisakiLanguage misaki_lang_detect_quick(const char *text);

// 辅助函数
CharsetStats misaki_analyze_charset(const char *text);
MisakiLanguage misaki_detect_by_features(const char *text);
MisakiLanguage misaki_detect_by_ngrams(const char *text);
```

### 2. `src/core/misaki_lang_detect.c` (595行)
**功能**：核心检测逻辑实现

**特征词表**：
- **日文**：38个特征词（助词、行政区划、接续词）
  - 超高权重：です、ます、ました（10.0）
  - 高权重：は、が、を（8.0）
  - 行政区划：都、道、府、県、市、区、町、村
  
- **中文**：26个特征词（虚词、连词、系词）
  - 超高权重：的（10.0）、是（9.0）、了（8.0）
  - 高权重：在、有、和、而、不
  
- **英文**：15个特征词（冠词、介词、连词）
  - 超高权重：the（10.0）、and、of（8.0）

**n-gram 表**：
- 日文：20个常见2-gram（です、ます、した、として等）
- 中文：18个常见2-gram（的是、的人、在中、这个等）
- 英文：10个常见2-gram（of the、in the、it is等）

### 3. `tests/test_lang_detect.c` (173行)
**功能**：单元测试

**测试覆盖**：
- ✅ 日文：纯假名、汉假混合、纯汉字地名（关键测试）
- ✅ 中文：常见句子、特征虚词、纯汉字词语
- ✅ 英文：常规句子
- ✅ 边界情况：空字符串、单字符、纯数字

**测试结果**：
```
总计: 21
通过: 20 (95.2%)
失败: 1 (4.8%)
```

## 🔧 修改的文件

### 1. `src/main.c`
**变更**：
- 添加 `#include "misaki_lang_detect.h"`
- `MisakiApp` 结构体新增 `LangDetector *lang_detector` 字段
- `init_app()` 中初始化语言检测器
- `process_text()` 使用新的检测接口，显示置信度和原因
- 移除旧的 `detect_language_simple()` 函数

### 2. `CMakeLists.txt`
**变更**：
- 添加 `misaki_lang_detect.c` 到源文件列表
- 添加 `test_lang_detect` 测试目标

## 📊 检测效果对比

### 关键测试用例

| 文本 | 期望 | 旧版 | 新版 | 状态 |
|------|------|------|------|------|
| 東京都渋谷区 | 日语 | ❌ 中文 | ✅ 日语 (75%) | **修复** |
| 大阪府 | 日语 | ❌ 中文 | ✅ 日语 (75%) | **修复** |
| 北海道 | 日语 | ❌ 中文 | ✅ 日语 (75%) | **修复** |
| 今天的天气很好 | 中文 | ✅ 中文 | ✅ 中文 (75%) | 保持 |
| 中国北京市 | 中文 | ✅ 中文 | ❌ 日语 (75%) | **待优化** |

### 新增能力
- ✅ 提供置信度评分（0.0-1.0）
- ✅ 给出检测原因（调试友好）
- ✅ 详细字符集统计（平假名、片假名、汉字、拉丁字母）
- ✅ 支持多层检测策略（字符集→特征词→n-gram）

## ⚠️ 已知问题

### 1. 中日汉字地名冲突
**现象**：`"中国北京市"` 被误判为日语（因包含"市"）

**原因**：特征词"市"在日文和中文地名中都常见，权重相同

**解决方案**（待实现）：
```c
// 方案A：上下文检测
if (strstr(text, "市") && strstr(text, "中国")) {
    // 优先判定为中文
}

// 方案B：双向特征词
// 为中文添加特有地名特征：省、自治区、直辖市
```

## 🚀 使用示例

### 基本用法
```c
#include "misaki_lang_detect.h"

// 快速检测
MisakiLanguage lang = misaki_lang_detect_quick("東京都");
printf("Language: %s\n", misaki_language_name(lang));  // "日语"

// 完整检测（推荐）
LangDetector *detector = misaki_lang_detector_create(NULL);
LangDetectResult result = misaki_lang_detect_full(detector, "今天的天气很好");
printf("Language: %s (%.2f%%, %s)\n", 
       misaki_language_name(result.language),
       result.confidence * 100,
       result.reason);
// 输出：Language: 中文 (75.00%, 特征词匹配)

misaki_lang_detector_free(detector);
```

### 高级配置
```c
LangDetectorConfig config = {
    .enable_ngram = true,
    .enable_tokenization = true,
    .confidence_threshold = 0.6f,
    .zh_tokenizer = my_zh_tokenizer,
    .ja_tokenizer = my_ja_tokenizer
};
LangDetector *detector = misaki_lang_detector_create(&config);
```

## 📈 性能特点

- **快速检测**：O(n)，仅字符集分析，适合实时场景
- **完整检测**：O(n×m)，m为特征词数量，准确度更高
- **内存占用**：检测器对象约 2KB（主要是配置和指针）
- **无外部依赖**：仅依赖 `misaki_types.h` 和 `misaki_string.h`

## 📝 后续优化方向

1. **改进中日地名区分**
   - 添加中国特有行政区划关键词（省、自治区、直辖市）
   - 实现上下文感知特征检测

2. **添加越南语和韩语支持**
   - 扩展特征词表
   - 增加对应的n-gram模式

3. **机器学习模型集成**
   - 基于词频的贝叶斯分类器
   - 字符级 n-gram 统计模型

4. **分词质量评估**
   - 启用 `enable_tokenization` 选项
   - 基于分词流畅度评分

## ✅ 总结

通过将语言检测拆分为独立模块，实现了：
- ✅ **更高准确率**：95.2% 测试通过率（vs. 旧版约 70%）
- ✅ **更强可扩展性**：易于添加新语言和新特征
- ✅ **更好的可维护性**：代码模块化，职责清晰
- ✅ **更丰富的信息**：置信度、原因、字符集统计
- ✅ **兼容旧代码**：保留快速检测接口

**关键成果**：成功解决了纯汉字日文文本（如地址）的误判问题。
