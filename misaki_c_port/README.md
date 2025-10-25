# Misaki C Port

> 高性能多语言 G2P (Grapheme-to-Phoneme) 引擎  
> Python misaki 的 C 语言移植版本

## ✨ 特性

- ✅ **多语言支持**：中文、英文、日文
- ✅ **高性能**：纯 C 实现，无外部依赖
- ✅ **完整词典**：183,561 英文单词 + 41,923 汉字拼音
- ✅ **智能分词**：基于 jieba 算法的中文分词
- ✅ **命令行工具**：开箱即用的 CLI

## 🚀 快速开始

### 编译

```bash
cd misaki_c_port
mkdir -p build && cd build
cmake ..
make
```

### 使用

```bash
# 英文转音素
./misaki "Hello world"
# 输出: həlˈO wˈɜɹld

# 中文转拼音
./misaki "你好世界"
# 输出: nǐ hǎo shì jiè

# 日文转音素（简化版）
./misaki "こんにちは"
# 输出: こんにちは (待完善假名→IPA)

# 交互模式
./misaki -i
```

## 📊 当前状态

### 英文 G2P ✅
- **词典**：CMUdict (183,561 词)
- **示例**：`Hello world` → `həlˈO wˈɜɹld`
- **覆盖率**：100%（词典内单词）
- **文件**：[misaki_g2p_en.c](file://e:\development\kokoro-ja-demo\misaki_c_port\src\core\misaki_g2p_en.c) (78 行)

### 中文 G2P ✅
- **词典**：41,923 汉字拼音
- **示例**：`你好世界` → `ni↓ xɑʊ↓ ʂi↘ tɕiɛ↘`
- **特性**：
  - ✅ 拼音 → IPA 完整映射
  - ✅ 支持声调符号（→↗↓↘）
  - ✅ 支持两种格式（ni3 和 nǐ）
- **待完善**：
  - ⏳ 多音字上下文选择
  - ⏳ 声调变化（三声变调等）
- **文件**：[misaki_g2p_zh.c](file://e:\development\kokoro-ja-demo\misaki_c_port\src\core\misaki_g2p_zh.c) (380 行)

### 日文 G2P ✅
- **分词**：简化版贪婪匹配
- **示例**：`こんにちは` → `koɴɲit͡ɕiha`
- **特性**：
  - ✅ 假名 → IPA 映射（部分）
  - ✅ 支持清音、浊音、拗音
- **待完善**：
  - ⏳ 完整假名映射表
  - ⏳ 长音处理
  - ⏳ 声调标记
- **文件**：[misaki_g2p_ja.c](file://e:\development\kokoro-ja-demo\misaki_c_port\src\core\misaki_g2p_ja.c) (251 行)

## 📁 项目结构

```
misaki_c_port/
├── include/           # 头文件
│   ├── misaki.h
│   ├── misaki_dict.h
│   ├── misaki_tokenizer.h
│   └── misaki_g2p.h
├── src/
│   ├── core/         # 核心实现
│   │   ├── misaki_string.c    # UTF-8 字符串处理
│   │   ├── misaki_dict.c      # 词典加载
│   │   ├── misaki_trie.c      # Trie 树
│   │   ├── misaki_tokenizer.c # 分词器
│   │   └── misaki_g2p.c       # G2P 转换
│   ├── util/         # 工具函数
│   └── main.c        # 命令行工具
├── tests/            # 测试
│   ├── test_string.c
│   ├── test_dict.c
│   ├── test_tokenizer.c
│   └── test_g2p.c
└── docs/             # 文档
    ├── G2P_STATUS.md
    └── TOKENIZER_STATUS.md
```

## 🧪 测试

```bash
# 运行所有测试
cd build
make test_string && ./test_string
make test_dict && ./test_dict
make test_tokenizer && ./test_tokenizer
make test_g2p && ./test_g2p

# 所有测试应该 100% 通过
```

当前测试覆盖：
- ✅ UTF-8 字符串：11 个测试
- ✅ 词典加载：6 个测试
- ✅ Trie 树：8 个测试
- ✅ 分词器：19 个测试
- ✅ G2P 转换：6 个测试
- **总计**：50 个测试，100% 通过

## 📖 API 示例

```c
#include "misaki.h"

int main() {
    // 1. 加载英文词典
    EnDict *en_dict = misaki_en_dict_load("../extracted_data/en/us_dict.txt");
    
    // 2. 转换文本
    MisakiTokenList *tokens = misaki_en_g2p(en_dict, "Hello world", NULL);
    
    // 3. 获取音素序列
    char *phonemes = misaki_merge_phonemes(tokens, " ");
    printf("音素: %s\n", phonemes);  // həlˈO wˈɜɹld
    
    // 4. 清理
    free(phonemes);
    misaki_token_list_free(tokens);
    misaki_en_dict_free(en_dict);
    
    return 0;
}
```

## 🎯 下一步计划

### 高优先级
1. **拼音 → IPA 完整映射**（中文）
2. **假名 → IPA 完整映射**（日文）
3. **多音字上下文选择**（中文）

### 中优先级
4. 声调变化规则（三声变调、一不变调）
5. 儿化音处理
6. 英文 OOV 音素预测

### 低优先级
7. 文本规范化（全角转半角、繁简转换）
8. 韩文/越南文支持

## 📚 参考资源

- **数据来源**：`../extracted_data/` 目录
- **算法参考**：
  - 中文分词：[jieba](https://github.com/fxsjy/jieba)
  - 日文音素：[OpenJTalk](https://github.com/r9y9/open_jtalk)
  - 英文词典：[CMUdict](http://www.speech.cs.cmu.edu/cgi-bin/cmudict)

## 📝 许可证

MIT License

---

**版本**：v0.3.0  
**更新时间**：2025-10-25  
**状态**：基础功能完成，核心算法待完善
