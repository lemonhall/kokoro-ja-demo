# Misaki C Port 架构设计

## 1. 总体架构

### 1.1 分层设计

```
┌─────────────────────────────────────────┐
│        Application Layer                │
│  (Android/iOS/Python Bindings)          │
└─────────────────────────────────────────┘
                    ↓
┌─────────────────────────────────────────┐
│         Public API Layer                │
│  (misaki.h, tokenizer.h, g2p.h)         │
└─────────────────────────────────────────┘
                    ↓
┌─────────────────────────────────────────┐
│      Language Specific Layer            │
│  (zh_*, ja_*, ko_*, en_*)               │
└─────────────────────────────────────────┘
                    ↓
┌─────────────────────────────────────────┐
│         Core Algorithm Layer            │
│  (Trie, Viterbi, DP, HMM)               │
└─────────────────────────────────────────┘
                    ↓
┌─────────────────────────────────────────┐
│          Data Layer                     │
│  (Dictionary, Rules, Models)            │
└─────────────────────────────────────────┘
```

### 1.2 模块依赖关系

```
Public API (misaki.h)
    ├── Tokenizer API
    │   ├── zh_segmenter (jieba port)
    │   ├── ja_segmenter (mecab port)
    │   └── ko_segmenter
    │
    └── G2P API
        ├── zh_g2p (pypinyin port)
        ├── ja_g2p (openjtalk port)
        ├── ko_g2p (g2pK port)
        └── en_g2p (cmudict)

Core Algorithms
    ├── trie.c          (Trie 树)
    ├── viterbi.c       (Viterbi 算法)
    ├── hmm.c           (隐马尔可夫模型)
    └── dag.c           (有向无环图)

Utils
    ├── utf8.c          (UTF-8 处理)
    ├── hashmap.c       (哈希表)
    └── memory.c        (内存管理)
```

## 2. 核心数据结构

### 2.1 Token 表示

```c
/**
 * Token 结构：分词后的词单元
 */
typedef struct Token {
    char *text;          // 原始文本
    char *tag;           // 词性标签 (POS tag)
    char *phonemes;      // 音素序列
    char *whitespace;    // 后续空白
    
    // 内部字段
    int start;           // 起始位置
    int length;          // 长度
    double score;        // 置信度分数
} Token;

/**
 * TokenList: Token 数组
 */
typedef struct TokenList {
    Token **tokens;      // Token 指针数组
    int count;           // Token 数量
    int capacity;        // 容量
} TokenList;
```

### 2.2 Trie 树（前缀树）

用于快速词典匹配（中文、日文）

```c
/**
 * Trie Node: 词典树节点
 */
typedef struct TrieNode {
    // Unicode code point (支持中日韩)
    uint32_t codepoint;
    
    // 如果是词尾，存储词信息
    char *word;              // 完整词
    double frequency;        // 词频
    char *tag;               // 词性
    
    // 子节点（哈希表实现，支持稀疏Unicode）
    struct TrieNode **children;
    int children_count;
    int children_capacity;
    
    // 是否是词尾
    bool is_word;
} TrieNode;

/**
 * Trie 树
 */
typedef struct Trie {
    TrieNode *root;
    int word_count;
} Trie;

// Trie API
Trie* trie_create(void);
void trie_insert(Trie *t, const char *word, double freq, const char *tag);
TrieNode* trie_search(Trie *t, const char *word);
bool trie_has_prefix(Trie *t, const char *prefix);
void trie_free(Trie *t);
```

### 2.3 DAG (有向无环图)

用于中文分词的路径枚举

```c
/**
 * DAG Edge: 分词路径
 */
typedef struct DAGEdge {
    int from;            // 起始位置
    int to;              // 结束位置
    char *word;          // 词
    double weight;       // 权重（词频）
} DAGEdge;

/**
 * DAG: 所有可能的分词路径
 */
typedef struct DAG {
    DAGEdge **edges;     // 边数组
    int edge_count;
    int *route;          // 最优路径（DP 结果）
} DAG;

// DAG API
DAG* dag_create(int text_length);
void dag_add_edge(DAG *dag, int from, int to, const char *word, double weight);
void dag_compute_route(DAG *dag); // 动态规划求最优路径
void dag_free(DAG *dag);
```

### 2.4 HMM (隐马尔可夫模型)

用于未登录词识别

```c
/**
 * HMM Model: 用于未登录词识别
 */
typedef struct HMM {
    // 状态: B(Begin), M(Middle), E(End), S(Single)
    double start_prob[4];      // 初始概率
    double trans_prob[4][4];   // 转移概率
    double emit_prob[4][65536]; // 发射概率（Unicode）
} HMM;

// HMM API
HMM* hmm_load(const char *model_file);
char** hmm_viterbi(HMM *hmm, const char *text, int *count);
void hmm_free(HMM *hmm);
```

## 3. 中文模块详细设计

### 3.1 中文分词 (jieba 移植)

**算法流程**：

```
输入文本
    ↓
1. 基于 Trie 树构建 DAG
   (枚举所有可能的词)
    ↓
2. 动态规划求最大概率路径
   DP[i] = max(DP[j] + log(P(word[j:i])))
    ↓
3. 对未登录词片段使用 HMM
   (Viterbi 算法)
    ↓
4. 合并结果
    ↓
输出 Token 列表
```

**核心函数**：

```c
/**
 * 中文分词器
 */
typedef struct ZHSegmenter {
    Trie *dict;              // 词典 (Trie 树)
    HMM *hmm;                // HMM 模型
    double min_freq;         // 最小词频阈值
} ZHSegmenter;

// API
ZHSegmenter* zh_segmenter_create(const char *dict_path);
TokenList* zh_segment(ZHSegmenter *seg, const char *text);
void zh_segmenter_free(ZHSegmenter *seg);

// 内部函数
DAG* zh_build_dag(ZHSegmenter *seg, const char *text);
void zh_compute_route(DAG *dag);
TokenList* zh_extract_tokens(const char *text, DAG *dag, HMM *hmm);
```

### 3.2 中文 G2P (pypinyin 移植)

**算法流程**：

```
输入文本
    ↓
1. 查询拼音词典
   (汉字 → 拼音)
    ↓
2. 多音字消歧
   (基于上下文选择)
    ↓
3. 声韵母分离
   (pinyin → initial + final)
    ↓
4. 声调变化规则
   (tone sandhi)
    ↓
5. 儿化音处理
   (erhua)
    ↓
6. 转换为 IPA
    ↓
输出音素序列
```

**核心数据结构**：

```c
/**
 * 拼音词典条目
 */
typedef struct PinyinEntry {
    uint32_t hanzi;          // 汉字 (Unicode)
    char *pinyin[8];         // 多个读音
    int pinyin_count;
    double freq[8];          // 各读音频率
} PinyinEntry;

/**
 * 中文 G2P
 */
typedef struct ZHG2P {
    PinyinEntry *dict;       // 拼音词典 (20902 个汉字)
    int dict_size;
    
    // Tone Sandhi 规则
    ToneSandhiRules *sandhi;
    
    // 儿化音规则
    ErhuaRules *erhua;
} ZHG2P;

// API
ZHG2P* zh_g2p_create(const char *dict_path);
char* zh_g2p_convert(ZHG2P *g2p, TokenList *tokens);
void zh_g2p_free(ZHG2P *g2p);
```

## 4. 日文模块详细设计

### 4.1 日文分词 (MeCab 核心移植)

**算法**: Viterbi 算法

```
输入文本
    ↓
1. 构建 Lattice（词格）
   枚举所有可能的词
    ↓
2. Viterbi 算法求最优路径
   cost[i] = min(cost[j] + edge_cost(j, i) + node_cost(i))
    ↓
3. 回溯最优路径
    ↓
输出 Token 列表
```

**核心数据结构**：

```c
/**
 * MeCab Lattice Node
 */
typedef struct LatticeNode {
    int pos;                 // 位置
    char *surface;           // 表层形式
    char *feature;           // 特征（词性等）
    char *reading;           // 读音（假名）
    
    double cost;             // 累积成本
    struct LatticeNode *prev; // 前驱节点
} LatticeNode;

/**
 * 日文分词器
 */
typedef struct JASegmenter {
    Trie *dict;              // UniDic 词典
    double **trans_cost;     // 转移成本矩阵
    int dict_size;
} JASegmenter;

// API
JASegmenter* ja_segmenter_create(const char *dict_path);
TokenList* ja_segment(JASegmenter *seg, const char *text);
void ja_segmenter_free(JASegmenter *seg);
```

### 4.2 日文 G2P (OpenJTalk 移植)

**算法**: 规则映射

```c
/**
 * 假名到音素映射表
 */
typedef struct KanaPhonemeMap {
    char *kana;              // 假名
    char *phoneme;           // 音素
} KanaPhonemeMap;

/**
 * 日文 G2P
 */
typedef struct JAG2P {
    KanaPhonemeMap *map;     // 映射表
    int map_size;
} JAG2P;

// API
JAG2P* ja_g2p_create(void);
char* ja_g2p_convert(JAG2P *g2p, TokenList *tokens);
void ja_g2p_free(JAG2P *g2p);
```

## 5. 数据文件格式

### 5.1 二进制词典格式

**设计原则**：
- 快速加载（mmap）
- 紧凑存储
- 支持增量更新

**格式定义**：

```c
/**
 * 词典文件头
 */
typedef struct DictHeader {
    char magic[4];           // "MSKD" (Misaki Dict)
    uint32_t version;        // 版本号
    uint32_t word_count;     // 词条数量
    uint32_t index_offset;   // 索引偏移
    uint32_t data_offset;    // 数据偏移
    char checksum[32];       // MD5 校验和
} DictHeader;

/**
 * 词典索引（Trie 树序列化）
 */
typedef struct DictIndex {
    uint32_t node_count;     // 节点数
    TrieNode *nodes;         // 节点数组（紧凑存储）
} DictIndex;
```

### 5.2 数据文件列表

```
data/
├── zh/
│   ├── dict.bin         # 中文词典（jieba）
│   ├── pinyin.bin       # 拼音表（pypinyin）
│   ├── hmm.bin          # HMM 模型
│   └── sandhi.bin       # 声调规则
│
├── ja/
│   ├── dict.bin         # UniDic 词典
│   ├── cost.bin         # 转移成本
│   └── kana_phone.bin   # 假名音素映射
│
├── ko/
│   ├── dict.bin
│   └── jamo.bin         # Jamo 分解表
│
└── en/
    └── cmudict.bin      # CMU 发音词典
```

## 6. 性能优化策略

### 6.1 内存优化

1. **Trie 树压缩**
   - 使用 Double-Array Trie (DAT)
   - 节省 50-70% 内存

2. **String Interning**
   - 相同字符串只存储一份
   - 使用指针共享

3. **内存池**
   - Token 使用内存池分配
   - 避免频繁 malloc/free

### 6.2 计算优化

1. **缓存**
   - LRU 缓存常用词的分词结果
   - 缓存热词的拼音

2. **查表优化**
   - 预计算常用转换
   - 使用位运算加速

3. **并行化**
   - 长文本分段并行处理
   - SIMD 优化 UTF-8 解析

## 7. Android/iOS 集成

### 7.1 Android JNI

```java
public class Misaki {
    static {
        System.loadLibrary("misaki");
    }
    
    public native long create(String lang);
    public native String convert(long handle, String text);
    public native void destroy(long handle);
}
```

### 7.2 iOS Objective-C

```objective-c
@interface Misaki : NSObject

- (instancetype)initWithLanguage:(NSString *)lang;
- (NSString *)convert:(NSString *)text;

@end
```

## 8. 测试策略

### 8.1 单元测试

- 每个模块独立测试
- 覆盖率 > 80%

### 8.2 集成测试

- 与 Python 版本结果对比
- 准确率测试

### 8.3 性能测试

- 吞吐量（tokens/sec）
- 内存占用
- 加载时间

---

**下一步**: 开始实现 [Core Utils](./docs/IMPLEMENTATION.md)
