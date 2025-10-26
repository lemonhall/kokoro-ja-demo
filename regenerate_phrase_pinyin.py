#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
重新生成词组拼音词典（应用声调变化）

使用 pypinyin 的 tone_sandhi=True 来自动处理声调变化
"""

import json
from pathlib import Path
from pypinyin import lazy_pinyin, Style

def main():
    print("🚀 重新生成词组拼音词典（应用声调变化）...\n")
    
    # 读取 pypinyin 的词组词典
    pypinyin_path = Path(".venv/Lib/site-packages/pypinyin/phrases_dict.json")
    
    if not pypinyin_path.exists():
        print(f"❌ 找不到文件: {pypinyin_path}")
        return
    
    print(f"📖 读取: {pypinyin_path}")
    with open(pypinyin_path, 'r', encoding='utf-8') as f:
        phrases_data = json.load(f)
    
    print(f"✅ 词组数量: {len(phrases_data)}\n")
    
    # ⭐ 重要：使用 pypinyin 的 lazy_pinyin 来获取变调后的拼音
    print("🔄 转换格式（应用声调变化）...")
    output_data = []
    
    for phrase in phrases_data.keys():
        # 使用 pypinyin 的 tone_sandhi=True 来应用声调变化
        pinyins = lazy_pinyin(phrase, style=Style.TONE, tone_sandhi=True)
        
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
    
    # 显示示例（声调变化）
    print("🎯 声调变化示例:")
    test_phrases = ["你好", "很好", "一起", "一天", "不是", "不好", "长城", "长大"]
    for phrase in test_phrases:
        pinyin = next((p for ph, p in output_data if ph == phrase), None)
        if pinyin:
            print(f"  {phrase} → {pinyin}")

if __name__ == "__main__":
    main()
