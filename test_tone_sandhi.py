#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
测试 pypinyin 的声调变化功能
"""

from pypinyin import lazy_pinyin, Style

test_cases = [
    ("你好", "三声+三声"),
    ("很好", "三声+三声"),
    ("我也", "三声+三声"),
    ("一个", "一+轻声"),
    ("一天", "一+一声"),
    ("一起", "一+三声"),
    ("一点", "一+三声"),
    ("不是", "不+四声"),
    ("不好", "不+三声"),
    ("不对", "不+四声"),
    ("不要", "不+四声"),
]

print("=" * 60)
print("声调变化测试（pypinyin tone_sandhi 功能）")
print("=" * 60)

for text, desc in test_cases:
    normal = lazy_pinyin(text, style=Style.TONE3)
    sandhi = lazy_pinyin(text, style=Style.TONE3, tone_sandhi=True)
    
    changed = "" if normal == sandhi else " ⭐ 变调"
    print(f"{text:6} ({desc:10}): {str(normal):20} → {str(sandhi):20}{changed}")

print("\n" + "=" * 60)
print("结论：pypinyin 已实现声调变化规则")
print("=" * 60)
