# Misaki 数据提取报告

**提取时间**: 2025-10-25 21:21:50.349811

## 📊 数据统计

### 英文词典

- 美式英语: 183,561 词条
- 英式英语: 197,116 词条
- 共同词汇: 164,229 词条
- 美式独有: 19,332 词条
- 英式独有: 32,887 词条

### 日文词典

- 总词条: 147,571
- 去重后: 147,571

### 中文词典

- 拼音表: 41,923 汉字

## 📁 文件结构

```
extracted_data/
├── en/
│   ├── gb_dict.txt (4994.8 KB)
│   ├── stats.json (0.1 KB)
│   ├── us_dict.txt (4694.4 KB)
├── ja/
│   ├── stats.json (0.0 KB)
│   ├── words.txt (2020.2 KB)
├── vi/
│   ├── acronyms.txt (106.4 KB)
│   ├── symbols.txt (0.7 KB)
│   ├── teencode.txt (6.4 KB)
├── zh/
│   ├── pinyin_dict.txt (483.7 KB)
│   ├── stats.json (0.0 KB)
```

## 🔧 数据格式

所有词典文件都采用 **制表符分隔** (TSV) 格式：

```
word<TAB>phonemes
```

**示例**:
```
apple	æpəl
hello	həlˈoʊ
```

## ✅ 下一步

1. 分析数据结构，设计 C 语言数据类型
2. 实现文本格式加载器（TSV parser）
3. 后续转换为二进制格式（可选）
