#!/usr/bin/env python3
from fugashi import Tagger

tagger = Tagger()

# 测试几个词
words = ["こんにちは", "私", "学生", "です"]

for word in words:
    result = tagger(word)
    for token in result:
        print(f"词: {token.surface}")
        print(f"  feature: {token.feature}")
        print(f"  pron: {getattr(token.feature, 'pron', 'N/A')}")
        print(f"  kana: {getattr(token.feature, 'kana', 'N/A')}")
        print()
