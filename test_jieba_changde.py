#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import jieba

# 测试 jieba 是否正确分词
text = "他长得太像他爸爸了"
result = list(jieba.cut(text))
print("默认分词:", " / ".join(result))

# 检查"长得"是否在词典中
print("'长得'在词典中:", '长得' in jieba.dt.FREQ)

# 强制将"长得"作为一个词
jieba.suggest_freq('长得', True)
result2 = list(jieba.cut(text))
print("调整后分词:", " / ".join(result2))

# 测试其他常见"得"字结构
test_cases = [
    "显得很年轻",
    "觉得不错",
    "变得更好",
    "长得像他妈妈"
]

for case in test_cases:
    result = list(jieba.cut(case))
    print(f"{case}: {' / '.join(result)}")
