#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
将 HMM JSON 转换为 C 友好的格式

由于发射概率数据较大（6857+7439+6409+14519=35224个字符），
我们生成一个简单的文本格式方便 C 代码解析
"""

import json
from pathlib import Path

def convert_hmm_to_text():
    print("🚀 转换 HMM 模型为 C 友好格式...\n")
    
    # 加载 JSON
    hmm_file = Path("misaki_c_port/extracted_data/zh/hmm_model.json")
    with open(hmm_file, 'r', encoding='utf-8') as f:
        data = json.load(f)
    
    output_dir = Path("misaki_c_port/extracted_data/zh")
    
    # 1. 导出初始概率（简单）
    start_file = output_dir / "hmm_prob_start.txt"
    with open(start_file, 'w', encoding='utf-8') as f:
        for state in ['B', 'E', 'M', 'S']:
            prob = data['prob_start'][state]
            f.write(f"{state}\t{prob}\n")
    
    print(f"✅ 初始概率: {start_file}")
    
    # 2. 导出转移概率（简单）
    trans_file = output_dir / "hmm_prob_trans.txt"
    with open(trans_file, 'w', encoding='utf-8') as f:
        for from_state in ['B', 'E', 'M', 'S']:
            for to_state, prob in data['prob_trans'][from_state].items():
                f.write(f"{from_state}\t{to_state}\t{prob}\n")
    
    print(f"✅ 转移概率: {trans_file}")
    
    # 3. 导出发射概率（较大）
    # 格式：状态 \t 字符 \t 概率
    emit_file = output_dir / "hmm_prob_emit.txt"
    total_chars = 0
    
    with open(emit_file, 'w', encoding='utf-8') as f:
        for state in ['B', 'E', 'M', 'S']:
            for char, prob in data['prob_emit'][state].items():
                f.write(f"{state}\t{char}\t{prob}\n")
                total_chars += 1
    
    print(f"✅ 发射概率: {emit_file}")
    print(f"   - 总字符数: {total_chars}")
    
    # 4. 统计信息
    print("\n📊 各状态发射字符数:")
    for state in ['B', 'E', 'M', 'S']:
        count = len(data['prob_emit'][state])
        print(f"   {state}: {count} 个字符")
    
    print("\n✅ 转换完成！")
    print("\n💡 C 代码可以按行读取这些文件，避免复杂的 JSON 解析")

if __name__ == "__main__":
    convert_hmm_to_text()
