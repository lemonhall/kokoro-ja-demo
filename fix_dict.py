#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
修复日文词典：去重并确保新词被正确添加
"""

import sys

# 读取原始词典（前68087行）
print("读取原始词典...")
original_words = {}
with open('extracted_data/ja/words.txt', 'r', encoding='utf-8') as f:
    for line in f:
        line = line.strip()
        if not line:
            continue
        parts = line.split('\t')
        if len(parts) >= 3:
            word, pron, freq = parts[0], parts[1], parts[2]
            if word not in original_words:
                original_words[word] = (pron, freq, '名詞')

print(f"原始词典: {len(original_words)} 个词")

# 读取新词
print("读取补充词...")
new_words = {}
for filename in ['extracted_data/ja/ja_supplement_dict.tsv', 'extracted_data/ja/ja_verb_supplement.tsv']:
    try:
        with open(filename, 'r', encoding='utf-8') as f:
            for line in f:
                line = line.strip()
                if not line or line.startswith('#'):
                    continue
                parts = line.split('\t')
                if len(parts) >= 4:
                    word, pron, freq, pos = parts[0], parts[1], parts[2], parts[3]
                    # 新词优先，频率更高
                    if word not in new_words:
                        new_words[word] = (pron, freq, pos)
    except FileNotFoundError:
        print(f"警告: {filename} 不存在")

print(f"补充词: {len(new_words)} 个")

# 合并（新词覆盖旧词）
final_dict = original_words.copy()
final_dict.update(new_words)

print(f"合并后: {len(final_dict)} 个唯一词")

# 写入新词典
print("写入词典...")
with open('extracted_data/ja/ja_pron_dict.tsv', 'w', encoding='utf-8') as f:
    for word, (pron, freq, pos) in final_dict.items():
        f.write(f"{word}\t{pron}\t{freq}\t{pos}\n")

print(f"✅ 完成！词典已保存到 extracted_data/ja/ja_pron_dict.tsv")
print(f"总词数: {len(final_dict)}")

# 验证关键词
test_words = ["飲んでいる", "飲んで", "読みながら", "美味しい", "ありがとうございます"]
print("\n验证关键词:")
for word in test_words:
    if word in final_dict:
        pron, freq, pos = final_dict[word]
        print(f"  ✅ {word}: {pron} ({pos}, freq={freq})")
    else:
        print(f"  ❌ {word}: 未找到")
