#!/usr/bin/env python3
"""
generate_ja_dict.py

从 MeCab UniDic 提取日文词典，生成 C 语言分词器可用的格式

输出格式：word\tfreq\tpos
"""

import os
import sys
from pathlib import Path

try:
    import MeCab
except ImportError:
    print("错误：需要安装 MeCab-python3")
    print("运行：pip install mecab-python3 unidic")
    sys.exit(1)

def extract_unidic_dict(output_file: str):
    """从 UniDic 提取词典"""
    
    # 初始化 MeCab
    try:
        tagger = MeCab.Tagger()
    except RuntimeError as e:
        print(f"错误：MeCab 初始化失败")
        print(f"请运行：python -m unidic download")
        sys.exit(1)
    
    # 获取 UniDic 字典路径
    dic_info = tagger.dictionary_info()
    if not dic_info:
        print("错误：无法获取字典信息")
        sys.exit(1)
    
    dic_path = Path(dic_info.filename).parent
    print(f"UniDic 路径: {dic_path}")
    
    # 读取词典文件
    word_dict = {}
    csv_files = list(dic_path.glob("*.csv"))
    
    if not csv_files:
        print(f"警告：在 {dic_path} 未找到 CSV 文件")
        print("尝试查找其他字典文件...")
    
    # UniDic 字典文件通常在 dicrc 指定的目录
    for csv_file in csv_files:
        print(f"处理: {csv_file.name}")
        
        try:
            with open(csv_file, 'r', encoding='utf-8') as f:
                for line_no, line in enumerate(f, 1):
                    try:
                        fields = line.strip().split(',')
                        if len(fields) < 11:
                            continue
                        
                        surface = fields[0]  # 表层形式
                        pos = fields[4]      # 词性
                        cost = int(fields[3]) if fields[3].lstrip('-').isdigit() else 0
                        
                        # 转换成本为频率（简化）
                        freq = max(1.0, 10000.0 / (abs(cost) + 1))
                        
                        # 保存最高频率
                        if surface not in word_dict or word_dict[surface][0] < freq:
                            word_dict[surface] = (freq, pos)
                    
                    except Exception as e:
                        if line_no <= 10:  # 只打印前几个错误
                            print(f"  行 {line_no} 解析失败: {e}")
                        continue
        
        except Exception as e:
            print(f"  读取失败: {e}")
            continue
    
    # 如果没有找到词典，生成基础词典
    if not word_dict:
        print("\n生成基础日文词典...")
        # 基础词汇
        basic_words = {
            # 基本假名
            "あ": (1000, "感動詞"), "い": (1000, "動詞"), "う": (1000, "動詞"),
            "え": (1000, "名詞"), "お": (1000, "接頭辞"),
            "か": (1000, "助詞"), "き": (1000, "動詞"), "く": (1000, "動詞"),
            "け": (1000, "名詞"), "こ": (1000, "代名詞"),
            "さ": (1000, "接尾辞"), "し": (1000, "動詞"), "す": (1000, "動詞"),
            "せ": (1000, "動詞"), "そ": (1000, "代名詞"),
            "た": (1000, "助動詞"), "ち": (1000, "名詞"), "つ": (1000, "助詞"),
            "て": (1000, "助詞"), "と": (1000, "助詞"),
            "な": (1000, "助動詞"), "に": (1000, "助詞"), "ぬ": (1000, "動詞"),
            "ね": (1000, "助詞"), "の": (1000, "助詞"),
            "は": (1000, "助詞"), "ひ": (1000, "名詞"), "ふ": (1000, "動詞"),
            "へ": (1000, "助詞"), "ほ": (1000, "名詞"),
            "ま": (1000, "助詞"), "み": (1000, "動詞"), "む": (1000, "動詞"),
            "め": (1000, "動詞"), "も": (1000, "助詞"),
            "や": (1000, "助詞"), "ゆ": (1000, "動詞"), "よ": (1000, "助詞"),
            "ら": (1000, "助動詞"), "り": (1000, "動詞"), "る": (1000, "動詞"),
            "れ": (1000, "助動詞"), "ろ": (1000, "動詞"),
            "わ": (1000, "助詞"), "を": (1000, "助詞"), "ん": (1000, "名詞"),
            
            # 常用词
            "こんにちは": (10000, "感動詞"),
            "ありがとう": (10000, "感動詞"),
            "さようなら": (8000, "感動詞"),
            "おはよう": (8000, "感動詞"),
            "こんばんは": (8000, "感動詞"),
            "すみません": (9000, "感動詞"),
            "ごめんなさい": (7000, "感動詞"),
            
            "私": (10000, "代名詞"),
            "あなた": (9000, "代名詞"),
            "彼": (8000, "代名詞"),
            "彼女": (8000, "代名詞"),
            "これ": (9000, "代名詞"),
            "それ": (9000, "代名詞"),
            "あれ": (8000, "代名詞"),
            
            "です": (10000, "助動詞"),
            "ます": (10000, "助動詞"),
            "ました": (9000, "助動詞"),
            "でした": (9000, "助動詞"),
            
            "学生": (9000, "名詞"),
            "先生": (9000, "名詞"),
            "友達": (8000, "名詞"),
            "家族": (8000, "名詞"),
            "会社": (9000, "名詞"),
            "学校": (9000, "名詞"),
            
            "日本": (10000, "名詞"),
            "日本語": (10000, "名詞"),
            "東京": (9000, "名詞"),
            "大阪": (8000, "名詞"),
            
            "食べる": (9000, "動詞"),
            "飲む": (9000, "動詞"),
            "見る": (9000, "動詞"),
            "聞く": (9000, "動詞"),
            "話す": (9000, "動詞"),
            "書く": (9000, "動詞"),
            "読む": (9000, "動詞"),
            "行く": (9000, "動詞"),
            "来る": (9000, "動詞"),
            "する": (10000, "動詞"),
            
            "元気": (9000, "形容動詞"),
            "綺麗": (8000, "形容動詞"),
            "便利": (8000, "形容動詞"),
            
            "コンピュータ": (8000, "名詞"),
            "カタカナ": (7000, "名詞"),
            
            # 助词
            "が": (10000, "助詞"),
            "を": (10000, "助詞"),
            "に": (10000, "助詞"),
            "で": (10000, "助詞"),
            "と": (9000, "助詞"),
            "から": (9000, "助詞"),
            "まで": (9000, "助詞"),
            "より": (8000, "助詞"),
            
            # 疑问
            "ですか": (9000, "助動詞"),
            "ますか": (9000, "助動詞"),
            
            # 其他
            "おかあさん": (8000, "名詞"),
            "おとうさん": (8000, "名詞"),
        }
        word_dict.update(basic_words)
    
    # 写入文件
    output_path = Path(output_file)
    output_path.parent.mkdir(parents=True, exist_ok=True)
    
    with open(output_file, 'w', encoding='utf-8') as f:
        # 按频率排序
        sorted_words = sorted(word_dict.items(), key=lambda x: x[1][0], reverse=True)
        
        for word, (freq, pos) in sorted_words:
            f.write(f"{word}\t{freq:.1f}\t{pos}\n")
    
    print(f"\n✅ 成功生成词典: {output_file}")
    print(f"   总词数: {len(word_dict)}")
    print(f"   文件大小: {output_path.stat().st_size / 1024:.1f} KB")

if __name__ == "__main__":
    output_file = "extracted_data/ja/dict.txt"
    extract_unidic_dict(output_file)
