#!/usr/bin/env python3
"""
extract_ja_dict.py

从 UniDic 词典提取日文词汇+读音映射表

生成格式: TSV (词汇<TAB>读音<TAB>词频<TAB>词性)

License: MIT
"""

import sys
import os
from pathlib import Path

try:
    from fugashi import Tagger
except ImportError:
    print("❌ 错误：需要安装 fugashi")
    print("运行：uv sync")
    sys.exit(1)

def load_word_list(file_path, limit=None):
    """从文件加载词汇列表"""
    words = []
    with open(file_path, 'r', encoding='utf-8') as f:
        for line in f:
            word = line.strip()
            if word and not word.startswith('#'):
                words.append(word)
                if limit and len(words) >= limit:
                    break
    return words

def extract_pronunciations(tagger, words, verbose=True):
    """
    从 MeCab 提取词汇的读音信息
    
    返回: dict {word: (pron, kana, pos, freq_estimate)}
    """
    word_dict = {}
    failed = 0
    
    for i, word in enumerate(words):
        if verbose and (i + 1) % 1000 == 0:
            print(f"  处理进度: {i+1}/{len(words)} ({100*(i+1)/len(words):.1f}%)")
        
        try:
            result = tagger(word)
            if not result:
                failed += 1
                continue
            
            for token in result:
                surface = token.surface
                feature = token.feature
                
                # 获取读音（优先使用 pron，其次 kana）
                pron = getattr(feature, 'pron', None)
                kana = getattr(feature, 'kana', None)
                reading = pron or kana or surface
                
                # 获取词性
                pos1 = getattr(feature, 'pos1', 'Unknown')
                
                # 估计词频（基于词性）
                freq_map = {
                    "助詞": 15000,
                    "助動詞": 12000,
                    "代名詞": 10000,
                    "名詞": 8000,
                    "動詞": 7000,
                    "形容詞": 6000,
                    "形容動詞": 6000,
                    "副詞": 5500,
                    "感動詞": 5000,
                    "接続詞": 5000,
                    "連体詞": 4500,
                    "接頭辞": 4000,
                    "接尾辞": 4000,
                }
                freq = freq_map.get(pos1, 3000)
                
                # 基于词长调整（短词通常更常用）
                freq = freq * (1.0 + 1.0 / max(len(surface), 1))
                
                # 保存（如果同一词有多个token，保留第一个）
                if surface not in word_dict:
                    word_dict[surface] = (reading, kana or reading, pos1, int(freq))
                    
        except Exception as e:
            if verbose:
                print(f"  ⚠️  处理失败: {word} - {e}")
            failed += 1
    
    if verbose:
        print(f"\n✅ 提取完成:")
        print(f"  - 成功: {len(word_dict)} 个词汇")
        print(f"  - 失败: {failed} 个词汇")
    
    return word_dict

def save_dictionary(word_dict, output_file, verbose=True):
    """保存词典到 TSV 文件"""
    output_dir = os.path.dirname(output_file)
    if output_dir:
        os.makedirs(output_dir, exist_ok=True)
    
    # 按词频排序
    sorted_words = sorted(word_dict.items(), key=lambda x: x[1][3], reverse=True)
    
    with open(output_file, 'w', encoding='utf-8') as f:
        # 写入表头
        f.write("# 日文词汇读音词典\n")
        f.write(f"# 生成时间: {__import__('datetime').datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n")
        f.write(f"# 词条数: {len(word_dict)}\n")
        f.write("# 格式: 词汇<TAB>读音<TAB>词频<TAB>词性\n")
        f.write("#\n")
        
        # 写入数据
        for word, (pron, kana, pos, freq) in sorted_words:
            f.write(f"{word}\t{pron}\t{freq}\t{pos}\n")
    
    if verbose:
        print(f"\n💾 词典已保存: {output_file}")
        print(f"📊 词条数: {len(word_dict)}")
        
        # 统计
        file_size = os.path.getsize(output_file)
        print(f"📊 文件大小: {file_size/1024/1024:.2f} MB")
        
        # 词性分布
        pos_count = {}
        for _, (_, _, pos, _) in word_dict.items():
            pos_count[pos] = pos_count.get(pos, 0) + 1
        
        print(f"\n📊 词性分布（前10）:")
        for pos, count in sorted(pos_count.items(), key=lambda x: x[1], reverse=True)[:10]:
            print(f"  {pos:15s}: {count:6d} ({100*count/len(word_dict):5.1f}%)")

def main():
    print("="*70)
    print("  从 UniDic 提取日文词汇读音词典")
    print("="*70)
    
    # 配置
    word_list_file = "misaki/misaki/data/ja_words.txt"
    output_file = "extracted_data/ja/ja_pron_dict.tsv"
    word_limit = 50000  # 提取前 5万个词
    
    # 检查词汇列表文件
    if not os.path.exists(word_list_file):
        print(f"❌ 错误：词汇列表文件不存在: {word_list_file}")
        return 1
    
    # 初始化 MeCab
    print(f"\n📖 正在初始化 MeCab...")
    try:
        tagger = Tagger()
        print("✅ MeCab 初始化成功")
    except Exception as e:
        print(f"❌ MeCab 初始化失败: {e}")
        return 1
    
    # 加载词汇列表
    print(f"\n📚 正在加载词汇列表: {word_list_file}")
    words = load_word_list(word_list_file, limit=word_limit)
    print(f"✅ 已加载 {len(words)} 个词汇")
    
    # 提取读音
    print(f"\n🔍 正在提取读音信息...")
    word_dict = extract_pronunciations(tagger, words)
    
    # 保存
    save_dictionary(word_dict, output_file)
    
    # 显示示例
    print(f"\n📝 示例数据（前10个）:")
    for i, (word, (pron, kana, pos, freq)) in enumerate(list(word_dict.items())[:10], 1):
        print(f"  {i:2d}. {word:15s} → {pron:15s} ({pos:10s}, freq={freq})")
    
    print(f"\n✨ 完成！")
    print(f"\n💡 使用方法:")
    print(f"   C 代码可以加载此文件: {output_file}")
    print(f"   格式: word<TAB>pronunciation<TAB>frequency<TAB>pos")
    
    return 0

if __name__ == "__main__":
    sys.exit(main())
