#!/usr/bin/env python3
#-*- coding: utf-8 -*-

# 简单测试：验证"长得"是否在词典第最后

with open('misaki_c_port/extracted_data/zh/dict_full.txt', 'r', encoding='utf-8') as f:
    lines = f.readlines()
    
print(f"词典总行数: {len(lines)}")
print("\n最后10行:")
for line in lines[-10:]:
    print(line.rstrip())

print("\n查找'长得':")
for i, line in enumerate(lines):
    if line.startswith('长得\t'):
        print(f"第{i+1}行: {line.rstrip()}")
