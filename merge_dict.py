#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
合并中文词典：dict_full.txt + proper_nouns.txt

策略：
1. 加载大词典 dict_full.txt
2. 加载专有名词 proper_nouns.txt
3. 合并去重（专有名词优先，因为词频更高）
4. 输出到 dict_merged.txt
"""

from pathlib import Path

def load_dict(file_path):
    """加载词典文件"""
    words = {}
    with open(file_path, 'r', encoding='utf-8') as f:
        for line in f:
            parts = line.strip().split('\t')
            if len(parts) >= 2:
                word = parts[0]
                freq = int(parts[1])
                words[word] = freq
    return words

def main():
    print("🚀 开始合并词典...\n")
    
    data_dir = Path(__file__).parent / "misaki_c_port" / "extracted_data" / "zh"
    
    # 1. 加载大词典
    dict_full_path = data_dir / "dict_full.txt"
    print(f"📖 加载大词典: {dict_full_path}")
    dict_full = load_dict(dict_full_path)
    print(f"   ✅ {len(dict_full)} 个词汇")
    
    # 2. 加载专有名词
    proper_nouns_path = data_dir / "proper_nouns.txt"
    print(f"📖 加载专有名词: {proper_nouns_path}")
    proper_nouns = load_dict(proper_nouns_path)
    print(f"   ✅ {len(proper_nouns)} 个专有名词")
    
    # 3. 合并（专有名词优先，覆盖原词典中的低频词）
    print("\n🔄 合并词典...")
    merged = dict_full.copy()
    
    new_count = 0
    updated_count = 0
    
    for word, freq in proper_nouns.items():
        if word in merged:
            if freq > merged[word]:
                merged[word] = freq
                updated_count += 1
        else:
            merged[word] = freq
            new_count += 1
    
    print(f"   ✅ 新增: {new_count} 个词汇")
    print(f"   ✅ 更新: {updated_count} 个词汇（提升词频）")
    print(f"   ✅ 总计: {len(merged)} 个词汇")
    
    # 4. 按词频排序
    print("\n📊 按词频排序...")
    sorted_words = sorted(merged.items(), key=lambda x: x[1], reverse=True)
    
    # 5. 输出到文件
    output_file = data_dir / "dict_merged.txt"
    print(f"💾 保存合并词典: {output_file}")
    
    with open(output_file, 'w', encoding='utf-8') as f:
        for word, freq in sorted_words:
            f.write(f"{word}\t{freq}\n")
    
    print(f"\n✨ 完成！合并词典：{len(merged)} 个词汇")
    
    # 显示前10个高频词
    print("\n🔝 前10个高频词:")
    for i, (word, freq) in enumerate(sorted_words[:10], 1):
        print(f"   {i}. {word} ({freq})")

if __name__ == "__main__":
    main()
