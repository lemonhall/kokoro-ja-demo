# Misaki C Port

**目标**：将 Python 版 misaki G2P 引擎移植为纯 C 实现，支持 iOS/Android 无缝集成。

## 📋 项目概述

### 为什么需要 C 版本？

1. **跨平台原生支持**
   - iOS 和 Android 都可以直接调用
   - 无需打包 Python 解释器
   - APK/IPA 大小不会增加

2. **性能优势**
   - 纯 C 实现，速度更快
   - 内存占用更小
   - 适合移动端实时处理

3. **依赖最小化**
   - 不依赖 Python 运行时
   - 不依赖重型 NLP 库
   - 只需要基础的 C 标准库

### 原始项目

- **源项目**: [hexgrad/misaki](https://github.com/hexgrad/misaki)
- **许可证**: MIT License
- **作者**: hexgrad (Kokoro TTS 作者)

## 🏗️ 架构设计

### 分层架构

```
misaki_c_port/
├── README.md                  # 本文件
├── ARCHITECTURE.md            # 架构设计文档
├── LICENSE                    # MIT License
│
├── include/                   # 公共头文件
│   ├── misaki.h              # 主API
│   ├── misaki_types.h        # 数据类型定义
│   └── misaki_config.h       # 配置选项
│
├── src/                       # 源代码
│   ├── core/                  # 核心模块
│   │   ├── tokenizer.c       # 分词器接口
│   │   ├── g2p.c            # G2P 接口
│   │   └── utils.c          # 工具函数
│   │
│   ├── segmenters/           # 分词实现（每个语言一个）
│   │   ├── zh_segmenter.c   # 中文分词 (jieba 移植)
│   │   ├── ja_segmenter.c   # 日文分词 (MeCab 移植)
│   │   └── ko_segmenter.c   # 韩文分词
│   │
│   ├── g2p/                  # G2P 实现
│   │   ├── zh_g2p.c         # 中文 G2P
│   │   ├── ja_g2p.c         # 日文 G2P
│   │   ├── ko_g2p.c         # 韩文 G2P
│   │   └── en_g2p.c         # 英文 G2P
│   │
│   └── data/                 # 数据加载
│       ├── dict_loader.c    # 词典加载器
│       └── resource.c       # 资源管理
│
├── data/                     # 数据文件
│   ├── zh/                  # 中文数据
│   │   ├── dict.txt         # 词典
│   │   ├── pinyin.txt       # 拼音表
│   │   └── rules.txt        # 规则
│   │
│   ├── ja/                  # 日文数据
│   │   ├── dict.txt
│   │   └── kana2phone.txt
│   │
│   ├── ko/                  # 韩文数据
│   └── en/                  # 英文数据
│
├── tests/                   # 测试代码
│   ├── test_zh.c
│   ├── test_ja.c
│   └── test_integration.c
│
├── bindings/                # 语言绑定
│   ├── android/            # Android JNI
│   ├── ios/                # iOS Objective-C
│   └── python/             # Python ctypes (用于测试)
│
└── tools/                   # 工具脚本
    ├── extract_data.py     # 从 Python misaki 提取数据
    ├── convert_dict.py     # 转换词典格式
    └── benchmark.c         # 性能测试
```

## 📊 模块划分

### 第一层：核心抽象

**tokenizer.h / tokenizer.c**
```c
// 分词器接口（所有语言通用）
typedef struct Tokenizer Tokenizer;
typedef struct Token {
    char *text;       // 词文本
    char *tag;        // 词性标注
    char *phonemes;   // 音素
} Token;

Tokenizer* tokenizer_create(const char *lang);
Token** tokenizer_segment(Tokenizer *t, const char *text, int *count);
void tokenizer_free(Tokenizer *t);
```

**g2p.h / g2p.c**
```c
// G2P 接口（所有语言通用）
typedef struct G2P G2P;

G2P* g2p_create(const char *lang);
char* g2p_convert(G2P *g, const char *text);
void g2p_free(G2P *g);
```

### 第二层：语言特定实现

#### 中文模块

**zh_segmenter.c** (jieba 移植)
- 基于 Trie 树的词典匹配
- 隐马尔可夫模型（HMM）未登录词识别
- 动态规划求最大概率路径

**zh_g2p.c** (pypinyin + tone sandhi)
- 拼音词典查询
- 声调变化规则 (tone sandhi)
- 儿化音处理

#### 日文模块

**ja_segmenter.c** (MeCab 移植)
- 基于 Viterbi 算法的形态分析
- UniDic 词典
- 词性标注

**ja_g2p.c** (OpenJTalk)
- 假名到音素映射
- 促音、长音处理
- 音调（pitch accent）标注

#### 韩文模块

**ko_segmenter.c**
- 基于规则的分词
- Jamo 分解

**ko_g2p.c** (g2pK 移植)
- 韩文字母到音素
- 连音规则

#### 英文模块

**en_g2p.c**
- CMUdict 词典
- 未知词处理（字母拼读）
- 同形异音词消歧

## 🔧 关键技术难点

### 1. 中文分词 (jieba 移植)

**核心算法**：
- **Trie 树**：词典前缀树，O(n) 快速匹配
- **DAG (有向无环图)**：枚举所有可能的分词组合
- **动态规划**：求最大概率路径
- **HMM**：处理未登录词

**数据结构**：
```c
typedef struct TrieNode {
    char word[64];           // 词
    double freq;             // 词频
    struct TrieNode *children[65536]; // Unicode 子节点
} TrieNode;
```

### 2. 日文分词 (MeCab 移植)

**核心算法**：
- **Viterbi 算法**：求最优路径
- **Cost Model**：基于成本的路径选择

**挑战**：
- UniDic 词典很大（~100MB）
- 需要压缩存储

### 3. 拼音处理 (pypinyin 移植)

**核心**：
- 多音字词典（20,902 个汉字）
- 声调标注（5 种声调）
- 变调规则（tone sandhi）

### 4. 数据文件管理

**格式选择**：
- ❌ JSON：解析慢，占用大
- ✅ **二进制格式**：快速加载，紧凑存储
- ✅ **mmap**：内存映射文件，按需加载

## 📅 开发计划

### 阶段 1：基础框架 (1 周)

- [ ] 创建项目结构
- [ ] 定义核心接口（tokenizer, g2p）
- [ ] 实现工具函数（UTF-8 处理、内存管理）
- [ ] 编写测试框架

### 阶段 2：中文支持 (2-3 周)

- [ ] **中文分词器** (jieba 移植)
  - [ ] Trie 树实现
  - [ ] DAG 构建
  - [ ] 动态规划求解
  - [ ] HMM 未登录词识别
  
- [ ] **中文 G2P**
  - [ ] 拼音词典加载
  - [ ] 声韵母分离
  - [ ] 声调处理
  - [ ] 儿化音规则

### 阶段 3：日文支持 (2-3 周)

- [ ] **日文分词器** (MeCab 核心)
  - [ ] Viterbi 算法
  - [ ] UniDic 词典压缩
  - [ ] 假名转换
  
- [ ] **日文 G2P**
  - [ ] 假名到音素映射
  - [ ] OpenJTalk 规则移植

### 阶段 4：英文/韩文支持 (1-2 周)

- [ ] 英文 CMUdict
- [ ] 韩文 g2pK 移植

### 阶段 5：集成与优化 (1 周)

- [ ] Android JNI 绑定
- [ ] iOS Objective-C 绑定
- [ ] 性能优化
- [ ] 文档完善

## 🎯 设计原则

### 1. 零依赖
- 只依赖 C 标准库
- 所有 NLP 逻辑自己实现
- 数据文件自包含

### 2. 内存安全
- 明确的内存管理
- 避免内存泄漏
- 边界检查

### 3. 高性能
- 数据结构优化（Trie, Hash Table）
- 缓存友好
- 最小化内存分配

### 4. 可移植性
- 标准 C99
- 跨平台编译
- 大小端兼容

## 📖 参考资源

### 原始项目
- [hexgrad/misaki](https://github.com/hexgrad/misaki) - Python 版本
- [PaddleSpeech](https://github.com/PaddlePaddle/PaddleSpeech) - 中文前端参考

### 分词算法
- [jieba](https://github.com/fxsjy/jieba) - 中文分词
- [MeCab](https://taku910.github.io/mecab/) - 日文分词
- [KoNLPy](https://konlpy.org/) - 韩文分词

### G2P 引擎
- [pypinyin](https://github.com/mozillazg/python-pinyin) - 中文拼音
- [OpenJTalk](http://open-jtalk.sp.nitech.ac.jp/) - 日文音素
- [g2pK](https://github.com/Kyubyong/g2pK) - 韩文音素

## 🤝 贡献指南

本项目是 [Kokoro-ja-demo](https://github.com/lemonhall/kokoro-ja-demo) 的子项目。

### 代码风格
- 遵循 C99 标准
- 使用 4 空格缩进
- 函数命名：`module_action()` (如 `zh_segment()`)
- 变量命名：`snake_case`

### 测试要求
- 每个模块都有对应的单元测试
- 性能测试（与 Python 版本对比）
- 内存泄漏检测（valgrind）

## 📄 许可证

MIT License - 继承自原始 misaki 项目

---

**状态**: 🚧 规划阶段

**开始日期**: 2025-10-25

**预计完成**: 2025-12-15 (约 7-8 周)
