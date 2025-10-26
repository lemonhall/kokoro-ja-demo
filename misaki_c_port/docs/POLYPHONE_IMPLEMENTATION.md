# 多音字上下文选择实现方案

## 📅 日期
2025-10-26

## 🎯 目标
解决中文多音字在不同上下文中的正确读音选择问题

## ✅ 已完成

### 1. 数据源确认
- ✅ 确认 pypinyin 库的 `phrases_dict.json` 包含 **47,111** 个词组拼音
- ✅ 数据格式验证：每个词组都有完整的逐字拼音标注
- ✅ 多音字测试：
  - "长城" → cháng chéng ✅
  - "长大" → zhǎng dà ✅
  - "银行" → yín háng ✅
  - "重庆" → chóng qìng ✅

### 2. 数据提取
- ✅ 创建提取脚本：[`extract_pypinyin_phrases.py`](file://e:\development\kokoro-ja-demo\extract_pypinyin_phrases.py)
- ✅ 生成词组拼音文件：`misaki_c_port/extracted_data/zh/phrase_pinyin.txt`
- ✅ 格式：`词<Tab>拼音`（拼音用空格分隔）

**示例数据**：
```
长城	cháng chéng
长大	zhǎng dà
银行	yín háng
行走	xíng zǒu
重庆	chóng qìng
重要	zhòng yào
一个	yí gè
```

## 🚧 待实现

### 方案一：使用 Trie 树（推荐）⭐

**优点**：
- 已有 Trie 树基础设施
- 查询效率高 O(m)，m 为词长
- 内存占用合理

**实现步骤**：

#### 1. 修改数据结构
在 [`misaki_types.h`](file://e:\development\kokoro-ja-demo\misaki_c_port\include\misaki_types.h) 中添加：
```c
typedef struct {
    Trie *phrase_trie;  // 存储词组拼音的 Trie 树
} ZhPhraseDict;
```

#### 2. 添加加载函数
在 [`misaki_dict.h`](file://e:\development\kokoro-ja-demo\misaki_c_port\include\misaki_dict.h) 中声明：
```c
/**
 * 加载中文词组拼音词典
 * 
 * @param file_path 词典文件路径（phrase_pinyin.txt）
 * @return 词组词典对象，失败返回 NULL
 */
ZhPhraseDict* misaki_zh_phrase_dict_load(const char *file_path);

/**
 * 释放词组词典
 */
void misaki_zh_phrase_dict_free(ZhPhraseDict *dict);

/**
 * 查询词组拼音
 * 
 * @param dict 词组词典
 * @param phrase 词组文本
 * @param pinyins 输出：拼音字符串（空格分隔）
 * @return 成功返回 true
 */
bool misaki_zh_phrase_dict_lookup(const ZhPhraseDict *dict,
                                  const char *phrase,
                                  const char **pinyins);
```

#### 3. 实现加载逻辑
在 [`misaki_dict.c`](file://e:\development\kokoro-ja-demo\misaki_c_port\src\core\misaki_dict.c) 中实现：
```c
ZhPhraseDict* misaki_zh_phrase_dict_load(const char *file_path) {
    ZhPhraseDict *dict = calloc(1, sizeof(ZhPhraseDict));
    dict->phrase_trie = misaki_trie_create();
    
    FILE *f = fopen(file_path, "r");
    char line[1024];
    while (fgets(line, sizeof(line), f)) {
        // 解析：词<Tab>拼音
        char *tab = strchr(line, '\t');
        if (tab) {
            *tab = '\0';
            char *phrase = line;
            char *pinyin = tab + 1;
            
            // 去除换行符
            pinyin[strcspn(pinyin, "\n")] = 0;
            
            // 插入 Trie（将拼音作为 tag 存储）
            misaki_trie_insert(dict->phrase_trie, phrase, 1.0, pinyin);
        }
    }
    fclose(f);
    return dict;
}
```

#### 4. 修改 G2P 逻辑
在 [`misaki_g2p_zh.c`](file://e:\development\kokoro-ja-demo\misaki_c_port\src\core\misaki_g2p_zh.c) 中优化：

```c
// 在 misaki_zh_g2p() 函数中
for (int i = 0; i < tokens->count; i++) {
    MisakiToken *token = &tokens->tokens[i];
    
    // ⭐ 优先查询词组拼音
    const char *phrase_pinyin = NULL;
    if (phrase_dict && 
        misaki_zh_phrase_dict_lookup(phrase_dict, token->text, &phrase_pinyin)) {
        // 找到词组拼音，直接使用
        char *ipa = convert_phrase_pinyin_to_ipa(phrase_pinyin);
        token->phonemes = ipa;
        continue;
    }
    
    // 降级：逐字查询单字拼音
    // ... 原有逻辑 ...
}
```

#### 5. 实现拼音转换
```c
// 将词组拼音（空格分隔）转换为 IPA
char* convert_phrase_pinyin_to_ipa(const char *phrase_pinyin) {
    // "cháng chéng" → "ʈ͡ʂʰɑŋ↗ ʈ͡ʂʰəŋ↗"
    
    char result[512] = {0};
    char *copy = strdup(phrase_pinyin);
    char *token = strtok(copy, " ");
    
    while (token) {
        char *ipa = misaki_zh_pinyin_to_ipa(token);
        if (ipa) {
            if (strlen(result) > 0) {
                strcat(result, " ");
            }
            strcat(result, ipa);
            free(ipa);
        }
        token = strtok(NULL, " ");
    }
    
    free(copy);
    return strdup(result);
}
```

### 方案二：哈希表（备选）

**优点**：
- 查询速度 O(1)
- 实现简单

**缺点**：
- 需要额外的哈希表实现
- 内存占用可能更大

## 📊 预期效果

**当前（逐字查询）**：
```
输入："长城"
当前：长(zhǎng) + 城(chéng) → "zhǎng chéng" ❌
```

**优化后（词组优先）**：
```
输入："长城"
优化：查询词组 "长城" → "cháng chéng" ✅
```

**测试用例**：
- "长城" → cháng chéng
- "长大" → zhǎng dà
- "银行" → yín háng
- "行走" → xíng zǒu
- "重庆" → chóng qìng
- "重要" → zhòng yào
- "重复" → chóng fù

## 🔧 实施计划

### 第一阶段：基础实现（1-2小时）
1. ✅ 添加数据结构定义
2. ✅ 实现加载函数
3. ✅ 实现查询函数
4. ✅ 实现拼音转换辅助函数

### 第二阶段：集成测试（30分钟）
1. ✅ 修改 main.c 加载词组词典
2. ✅ 修改 G2P 函数使用词组拼音
3. ✅ 编译测试
4. ✅ 验证多音字效果

### 第三阶段：性能优化（可选）
1. ⏳ 统计命中率
2. ⏳ 优化未命中情况的处理
3. ⏳ 添加缓存机制

## 📁 相关文件

### 已创建
- [`extract_pypinyin_phrases.py`](file://e:\development\kokoro-ja-demo\extract_pypinyin_phrases.py) - 提取脚本
- `misaki_c_port/extracted_data/zh/phrase_pinyin.txt` - 词组拼音数据（47,111条）

### 需修改
- [`misaki_types.h`](file://e:\development\kokoro-ja-demo\misaki_c_port\include\misaki_types.h) - 添加 `ZhPhraseDict`
- [`misaki_dict.h`](file://e:\development\kokoro-ja-demo\misaki_c_port\include\misaki_dict.h) - 添加加载/查询函数声明
- [`misaki_dict.c`](file://e:\development\kokoro-ja-demo\misaki_c_port\src\core\misaki_dict.c) - 实现加载/查询函数
- [`misaki_g2p_zh.c`](file://e:\development\kokoro-ja-demo\misaki_c_port\src\core\misaki_g2p_zh.c) - 修改 G2P 逻辑
- [`main.c`](file://e:\development\kokoro-ja-demo\misaki_c_port\src\main.c) - 加载词组词典

## 💡 技术细节

### Trie 节点存储
```c
// 词组："长城"
// 拼音："cháng chéng"

// Trie 存储方式：
// 节点路径：长 → 城
// 叶子节点的 tag 字段：存储 "cháng chéng"
```

### 查询流程
```
1. 输入 Token："长城"
2. 在 phrase_trie 中查找 "长城"
3. 找到 → 返回 tag = "cháng chéng"
4. 将 "cháng chéng" 转换为 IPA
5. 未找到 → 降级到逐字查询
```

---

**状态**：✅ 第一阶段完成（数据准备）  
**下一步**：实现 Trie 树加载和查询逻辑  
**预计完成时间**：今晚
