#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
从 pypinyin 提取词组拼音词典

输入：pypinyin 的 phrases_dict.json
输出：phrase_pinyin.txt（格式：词<Tab>拼音）
"""

import json
from pathlib import Path

def main():
    print("🚀 从 pypinyin 提取词组拼音词典...\n")
    
    # 读取 pypinyin 的词组词典
    pypinyin_path = Path(".venv/Lib/site-packages/pypinyin/phrases_dict.json")
    
    if not pypinyin_path.exists():
        print(f"❌ 找不到文件: {pypinyin_path}")
        return
    
    print(f"📖 读取: {pypinyin_path}")
    with open(pypinyin_path, 'r', encoding='utf-8') as f:
        phrases_data = json.load(f)
    
    print(f"✅ 词组数量: {len(phrases_data)}\n")
    
    # 转换格式
    print("🔄 转换格式...")
    output_data = []
    
    for phrase, pinyins_list in phrases_data.items():
        # pinyins_list 格式: [['zhǎng'], ['dà']]
        # 我们需要转换为: "zhǎng dà"
        pinyins = []
        for pinyin_group in pinyins_list:
            if pinyin_group:  # 通常是单元素列表
                pinyins.append(pinyin_group[0])
        
        if pinyins:
            pinyin_str = " ".join(pinyins)
            output_data.append((phrase, pinyin_str))
    
    print(f"✅ 转换完成: {len(output_data)} 个词组\n")
    
    # 保存到文件
    output_dir = Path("misaki_c_port/extracted_data/zh")
    output_dir.mkdir(parents=True, exist_ok=True)
    
    output_file = output_dir / "phrase_pinyin.txt"
    
    print(f"💾 保存到: {output_file}")
    with open(output_file, 'w', encoding='utf-8') as f:
        for phrase, pinyin in output_data:
            f.write(f"{phrase}\t{pinyin}\n")
    
    print(f"✅ 完成！\n")
    
    # 显示示例
    print("📝 示例（前20个）:")
    for i, (phrase, pinyin) in enumerate(output_data[:20], 1):
        print(f"  {i}. {phrase} → {pinyin}")
    
    # 显示多音字示例
    print("\n🎯 多音字示例:")
    test_phrases = ["长城", "长大", "银行", "行走", "重庆", "重要", "重复", "重量"]
    for phrase in test_phrases:
        pinyin = next((p for ph, p in output_data if ph == phrase), None)
        if pinyin:
            print(f"  {phrase} → {pinyin}")

if __name__ == "__main__":
    main()
