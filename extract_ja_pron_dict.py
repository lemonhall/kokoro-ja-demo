#!/usr/bin/env python3
"""
从 MeCab/UniDic 提取日文词汇+读音词典

生成格式：TSV (词汇\t读音\t词频\t词性)
例如：
こんにちは	コンニチワ	1000	感動詞
私	ワタシ	5000	代名詞
学生	ガクセイ	3000	名詞
"""

import sys

try:
    from fugashi import Tagger
except ImportError:
    print("❌ 错误：需要安装 fugashi")
    print("运行：pip install fugashi unidic-lite")
    sys.exit(1)

def extract_dictionary():
    """从 MeCab 提取词典数据"""
    print("📖 正在初始化 MeCab...")
    tagger = Tagger()
    
    # 测试用的常用词汇列表
    test_words = [
        # 基础问候
        "こんにちは", "さようなら", "おはよう", "ありがとう",
        # 代词
        "私", "あなた", "彼", "彼女",
        # 助词
        "は", "が", "を", "に", "で", "と", "の",
        # 名词
        "学生", "先生", "学校", "会社", "日本", "東京",
        # 动词
        "行く", "来る", "食べる", "飲む", "見る", "読む", "書く",
        # 形容词
        "大きい", "小さい", "美しい", "元気",
        # 片假名
        "コンピュータ", "テスト", "プログラム",
        # 辅助动词
        "です", "ます", "ました", "ですか",
    ]
    
    word_dict = {}
    
    print(f"\n📝 处理 {len(test_words)} 个测试词汇...")
    
    for word in test_words:
        result = tagger(word)
        if result:
            for token in result:
                surface = token.surface
                feature = token.feature
                
                # 获取读音
                pron = feature.pron if hasattr(feature, 'pron') and feature.pron else None
                kana = feature.kana if hasattr(feature, 'kana') and feature.kana else None
                reading = pron or kana or surface
                
                # 获取词性
                pos = feature.pos1 if hasattr(feature, 'pos1') else "Unknown"
                
                # 简单的词频估计（基于词性）
                freq = 10000
                if pos == "助詞":
                    freq = 15000
                elif pos == "助動詞":
                    freq = 12000
                elif pos == "代名詞":
                    freq = 10000
                elif pos == "名詞":
                    freq = 8000
                elif pos == "動詞":
                    freq = 7000
                    
                word_dict[surface] = (reading, freq, pos)
                
                print(f"  {surface:15s} → {reading:15s} ({pos})")
    
    return word_dict

def save_dictionary(word_dict, output_file):
    """保存词典到文件"""
    print(f"\n💾 保存词典到: {output_file}")
    
    with open(output_file, 'w', encoding='utf-8') as f:
        # 写入表头
        f.write("# 日文词汇读音词典\n")
        f.write("# 格式：词汇<TAB>读音<TAB>词频<TAB>词性\n")
        f.write("#\n")
        
        # 按词频排序
        sorted_words = sorted(word_dict.items(), key=lambda x: x[1][1], reverse=True)
        
        for word, (reading, freq, pos) in sorted_words:
            f.write(f"{word}\t{reading}\t{freq}\t{pos}\n")
    
    print(f"✅ 成功保存 {len(word_dict)} 个词汇")

def main():
    output_file = "extracted_data/ja/pron_dict.txt"
    
    print("="*60)
    print("  从 MeCab/UniDic 提取日文读音词典")
    print("="*60)
    
    # 创建输出目录
    import os
    os.makedirs(os.path.dirname(output_file), exist_ok=True)
    
    # 提取词典
    word_dict = extract_dictionary()
    
    # 保存
    save_dictionary(word_dict, output_file)
    
    print("\n✨ 完成！")
    print(f"\n提示：C 代码可以直接加载 {output_file}")
    print("      格式：word\\tpronunciation\\tfrequency\\tpos")

if __name__ == "__main__":
    main()
