#!/usr/bin/env python3
"""测试 pypinyin 对 '余' 的处理"""

from pypinyin import pinyin, Style

text = "余"

print(f"字: {text}")
print(f"拼音（带声调）: {pinyin(text, style=Style.TONE)}")
print(f"拼音（数字声调）: {pinyin(text, style=Style.TONE3)}")
print(f"拼音（不带声调）: {pinyin(text, style=Style.NORMAL)}")

# 测试其他 y- 开头的字
test_chars = ["余", "鱼", "雨", "语", "玉", "育"]
for char in test_chars:
    py = pinyin(char, style=Style.TONE3)[0][0]
    print(f"{char} → {py}")
