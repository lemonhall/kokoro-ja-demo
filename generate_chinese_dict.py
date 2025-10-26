#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
生成中文词典（从 jieba 提取）

输出格式：
词<Tab>词频

目标：10万+ 词汇
"""

import jieba
import os
from pathlib import Path

def generate_chinese_dict():
    """从 jieba 词典提取中文词汇"""
    
    # jieba 默认词典路径
    jieba.initialize()
    
    # 获取 jieba 词典文件路径
    jieba_dict_path = jieba.get_dict_file()
    
    print(f"📖 读取 jieba 词典: {jieba_dict_path.name if hasattr(jieba_dict_path, 'name') else jieba_dict_path}")
    
    # 读取 jieba 词典
    words = []
    dict_file = jieba.get_dict_file()
    
    # jieba 的词典文件可能是二进制模式打开的
    if hasattr(dict_file, 'mode') and 'b' in dict_file.mode:
        # 二进制模式
        for line in dict_file:
            line = line.decode('utf-8').strip()
            parts = line.split()
            if len(parts) >= 2:
                word = parts[0]
                try:
                    freq = int(parts[1])
                except:
                    continue
                
                # 过滤条件：
                # 1. 只保留中文词汇（至少包含一个中文字符）
                # 2. 过滤掉纯数字、纯字母
                # 3. 词频 > 0
                if freq > 0 and any('\u4e00' <= c <= '\u9fff' for c in word):
                    words.append((word, freq))
    else:
        # 文本模式或者需要重新打开
        dict_path = dict_file if isinstance(dict_file, str) else dict_file.name
        with open(dict_path, 'r', encoding='utf-8') as f:
            for line in f:
                parts = line.strip().split()
                if len(parts) >= 2:
                    word = parts[0]
                    try:
                        freq = int(parts[1])
                    except:
                        continue
                    
                    # 过滤条件：同上
                    if freq > 0 and any('\u4e00' <= c <= '\u9fff' for c in word):
                        words.append((word, freq))
    
    # 按词频排序（从高到低）
    words.sort(key=lambda x: x[1], reverse=True)
    
    print(f"✅ 提取到 {len(words)} 个中文词汇")
    
    # 输出到文件
    output_dir = Path(__file__).parent / "misaki_c_port" / "extracted_data" / "zh"
    output_dir.mkdir(parents=True, exist_ok=True)
    
    output_file = output_dir / "dict_full.txt"
    
    with open(output_file, 'w', encoding='utf-8') as f:
        for word, freq in words:
            f.write(f"{word}\t{freq}\n")
    
    print(f"💾 词典已保存: {output_file}")
    print(f"📊 词汇数量: {len(words)}")
    
    # 统计信息
    print("\n📊 词典统计:")
    print(f"  - 总词数: {len(words)}")
    print(f"  - 最高频词: {words[0][0]} (频率: {words[0][1]})")
    print(f"  - 最低频词: {words[-1][0]} (频率: {words[-1][1]})")
    
    # 词长分布
    length_dist = {}
    for word, _ in words:
        length = len(word)
        length_dist[length] = length_dist.get(length, 0) + 1
    
    print("\n📏 词长分布:")
    for length in sorted(length_dist.keys())[:10]:
        print(f"  - {length}字词: {length_dist[length]} 个")
    
    # 生成统计 JSON
    import json
    stats_file = output_dir / "stats.json"
    with open(stats_file, 'w', encoding='utf-8') as f:
        json.dump({
            "total_words": len(words),
            "max_freq": words[0][1],
            "min_freq": words[-1][1],
            "length_distribution": length_dist,
            "top_10_words": [{"word": w, "freq": f} for w, f in words[:10]],
        }, f, ensure_ascii=False, indent=2)
    
    print(f"📊 统计信息已保存: {stats_file}")
    
    return output_file

if __name__ == "__main__":
    print("🚀 开始生成中文词典...\n")
    output_file = generate_chinese_dict()
    print(f"\n✨ 完成！词典文件: {output_file}")
