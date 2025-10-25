#!/usr/bin/env python3
"""
generate_ja_dict.py

从 MeCab UniDic 字典生成日文词典文件

这个脚本会读取 MeCab 的 UniDic 字典数据，并提取以下信息：
- 词汇本身
- 词频（cost 值）
- 词性标注

生成的字典文件格式为 TSV：
word\tfreq\tpos

UniDic 是什么？
- MeCab 的标准日文词典
- 包含约 70万+ 词条
- 文件大小约 400MB
- 安装方式：pip install unidic

License: MIT
"""

import sys
import os
from pathlib import Path

def find_unidic_path():
    """查找 UniDic 的安装路径"""
    try:
        import unidic
        dic_path = unidic.DICDIR
        if os.path.exists(dic_path):
            return dic_path
    except ImportError:
        pass
    
    # 尝试常见路径
    common_paths = [
        "/usr/local/lib/mecab/dic/unidic",
        "/usr/lib/mecab/dic/unidic",
        os.path.expanduser("~/.local/lib/python3.*/site-packages/unidic/dicdir"),
    ]
    
    for path in common_paths:
        if os.path.exists(path):
            return path
    
    return None

def parse_mecab_dict(dict_path):
    """解析 MeCab 词典文件"""
    words = []
    
    # MeCab 词典通常在多个 CSV 文件中
    csv_files = list(Path(dict_path).glob("*.csv"))
    
    print(f"找到 {len(csv_files)} 个词典文件")
    
    for csv_file in csv_files:
        if csv_file.name.startswith("unk"):
            continue  # 跳过未知词模板
        
        print(f"处理文件: {csv_file.name}")
        
        try:
            with open(csv_file, 'r', encoding='utf-8') as f:
                for line_num, line in enumerate(f, 1):
                    try:
                        parts = line.strip().split(',')
                        if len(parts) < 11:
                            continue
                        
                        surface = parts[0]  # 词汇本身
                        pos = parts[4]      # 词性（品词）
                        cost = parts[3]     # cost 值（负数，越小越常用）
                        
                        # 转换 cost 为频率（取反后归一化）
                        try:
                            cost_val = int(cost)
                            freq = max(0, -cost_val)  # 常用词有更高的频率
                        except ValueError:
                            freq = 1000
                        
                        words.append((surface, freq, pos))
                        
                    except Exception as e:
                        if line_num % 10000 == 0:
                            print(f"  警告: 第 {line_num} 行解析失败: {e}")
                        continue
                        
        except Exception as e:
            print(f"  错误: 无法读取文件 {csv_file}: {e}")
            continue
    
    return words

def save_dict(words, output_path):
    """保存词典到文件"""
    output_dir = os.path.dirname(output_path)
    if output_dir and not os.path.exists(output_dir):
        os.makedirs(output_dir)
    
    # 按频率排序（高频词在前）
    words_sorted = sorted(words, key=lambda x: x[1], reverse=True)
    
    # 去重（保留频率最高的）
    seen = set()
    unique_words = []
    for word, freq, pos in words_sorted:
        if word not in seen:
            seen.add(word)
            unique_words.append((word, freq, pos))
    
    # 写入文件
    with open(output_path, 'w', encoding='utf-8') as f:
        for word, freq, pos in unique_words:
            f.write(f"{word}\t{freq}\t{pos}\n")
    
    print(f"\n成功生成词典文件: {output_path}")
    print(f"总词条数: {len(unique_words)}")

def generate_fallback_dict(output_path):
    """生成一个基础的后备词典（如果找不到 UniDic）"""
    print("未找到 UniDic，生成基础词典...")
    
    basic_words = [
        # 基础问候语
        ("こんにちは", 10000, "感動詞"),
        ("さようなら", 8000, "感動詞"),
        ("おはよう", 9000, "感動詞"),
        ("ありがとう", 9500, "感動詞"),
        
        # 代词和助词
        ("私", 10000, "代名詞"),
        ("あなた", 9000, "代名詞"),
        ("彼", 8000, "代名詞"),
        ("彼女", 8000, "代名詞"),
        ("は", 10000, "助詞"),
        ("が", 10000, "助詞"),
        ("を", 10000, "助詞"),
        ("に", 10000, "助詞"),
        ("で", 10000, "助詞"),
        ("と", 10000, "助詞"),
        ("の", 10000, "助詞"),
        
        # 常用名词
        ("学生", 9000, "名詞"),
        ("先生", 9000, "名詞"),
        ("学校", 9000, "名詞"),
        ("会社", 9000, "名詞"),
        ("日本", 10000, "名詞"),
        ("東京", 9500, "名詞"),
        
        # 常用动词
        ("行く", 9000, "動詞"),
        ("来る", 9000, "動詞"),
        ("食べる", 8500, "動詞"),
        ("飲む", 8500, "動詞"),
        ("見る", 8500, "動詞"),
        ("読む", 8000, "動詞"),
        ("書く", 8000, "動詞"),
        
        # 辅助动词
        ("です", 10000, "助動詞"),
        ("ます", 10000, "助動詞"),
        ("ました", 9500, "助動詞"),
        ("ですか", 9000, "助動詞"),
    ]
    
    output_dir = os.path.dirname(output_path)
    if output_dir and not os.path.exists(output_dir):
        os.makedirs(output_dir)
    
    with open(output_path, 'w', encoding='utf-8') as f:
        for word, freq, pos in basic_words:
            f.write(f"{word}\t{freq}\t{pos}\n")
    
    print(f"\n生成基础词典: {output_path}")
    print(f"词条数: {len(basic_words)}")
    print("\n提示: 这只是一个基础词典，建议安装完整的 UniDic:")
    print("  pip install unidic")
    print("  python -m unidic download")

def main():
    output_path = "extracted_data/ja/dict.txt"
    
    print("=" * 60)
    print("日文词典生成工具")
    print("=" * 60)
    
    # 查找 UniDic
    dict_path = find_unidic_path()
    
    if dict_path:
        print(f"\n找到 UniDic 路径: {dict_path}")
        
        # 解析词典
        print("\n开始解析词典...")
        words = parse_mecab_dict(dict_path)
        
        if words:
            # 保存词典
            save_dict(words, output_path)
        else:
            print("警告: 未能从 UniDic 提取词汇，生成基础词典")
            generate_fallback_dict(output_path)
    else:
        print("\n未找到 UniDic 安装")
        print("请先安装 UniDic:")
        print("  pip install unidic")
        print("  python -m unidic download")
        print()
        generate_fallback_dict(output_path)
    
    print("\n完成!")

if __name__ == "__main__":
    main()
