#!/usr/bin/env python3
"""
从 Python misaki 提取数据并转换为 C 语言可用的格式

目标：
1. 提取所有语言的词典数据
2. 分析数据结构
3. 生成统计报告
4. 转换为简化的文本格式（第一阶段）
5. 后续可转换为二进制格式（第二阶段）

输出目录：misaki_c_port/extracted_data/
"""

import json
import os
import sys
from pathlib import Path
from collections import Counter

# 输出目录
OUTPUT_DIR = Path("misaki_c_port/extracted_data")
OUTPUT_DIR.mkdir(parents=True, exist_ok=True)

# 源数据目录
SOURCE_DIR = Path("misaki/misaki/data")

def analyze_file_structure(file_path):
    """分析文件结构"""
    print(f"\n{'='*60}")
    print(f"分析文件: {file_path.name}")
    print(f"{'='*60}")
    
    file_size = file_path.stat().st_size / 1024  # KB
    print(f"文件大小: {file_size:.2f} KB")
    
    if file_path.suffix == '.json':
        with open(file_path, 'r', encoding='utf-8') as f:
            data = json.load(f)
        
        print(f"数据类型: {type(data).__name__}")
        
        if isinstance(data, dict):
            print(f"词条数量: {len(data)}")
            
            # 分析键值结构
            sample_items = list(data.items())[:5]
            print(f"\n示例数据:")
            for key, value in sample_items:
                key_str = key if len(key) <= 30 else key[:30] + "..."
                val_str = str(value) if len(str(value)) <= 50 else str(value)[:50] + "..."
                print(f"  {key_str:30} → {val_str}")
            
            # 统计键长度
            key_lengths = [len(k) for k in data.keys()]
            print(f"\n键长度统计:")
            print(f"  最小: {min(key_lengths)}")
            print(f"  最大: {max(key_lengths)}")
            print(f"  平均: {sum(key_lengths)/len(key_lengths):.2f}")
            
            # 统计值长度
            if sample_items and isinstance(sample_items[0][1], str):
                val_lengths = [len(str(v)) for v in data.values()]
                print(f"\n值长度统计:")
                print(f"  最小: {min(val_lengths)}")
                print(f"  最大: {max(val_lengths)}")
                print(f"  平均: {sum(val_lengths)/len(val_lengths):.2f}")
        
        return data
    
    elif file_path.suffix == '.txt':
        with open(file_path, 'r', encoding='utf-8') as f:
            lines = [line.strip() for line in f if line.strip()]
        
        print(f"行数: {len(lines)}")
        print(f"\n前 10 行:")
        for line in lines[:10]:
            print(f"  {line}")
        
        # 统计长度
        lengths = [len(line) for line in lines]
        print(f"\n行长度统计:")
        print(f"  最小: {min(lengths)}")
        print(f"  最大: {max(lengths)}")
        print(f"  平均: {sum(lengths)/len(lengths):.2f}")
        
        return lines
    
    return None

def extract_english_dict():
    """提取英文词典 (CMUdict)"""
    print(f"\n{'#'*60}")
    print(f"# 提取英文词典")
    print(f"{'#'*60}")
    
    output_dir = OUTPUT_DIR / "en"
    output_dir.mkdir(exist_ok=True)
    
    # 美式英语
    us_gold = analyze_file_structure(SOURCE_DIR / "us_gold.json")
    us_silver = analyze_file_structure(SOURCE_DIR / "us_silver.json")
    
    # 英式英语
    gb_gold = analyze_file_structure(SOURCE_DIR / "gb_gold.json")
    gb_silver = analyze_file_structure(SOURCE_DIR / "gb_silver.json")
    
    # 合并词典（优先 gold）
    us_dict = {**us_silver, **us_gold}
    gb_dict = {**gb_silver, **gb_gold}
    
    print(f"\n合并后:")
    print(f"  美式英语: {len(us_dict)} 词条")
    print(f"  英式英语: {len(gb_dict)} 词条")
    
    # 导出为文本格式（易于 C 语言解析）
    with open(output_dir / "us_dict.txt", 'w', encoding='utf-8') as f:
        for word, ipa in sorted(us_dict.items()):
            f.write(f"{word}\t{ipa}\n")
    
    with open(output_dir / "gb_dict.txt", 'w', encoding='utf-8') as f:
        for word, ipa in sorted(gb_dict.items()):
            f.write(f"{word}\t{ipa}\n")
    
    print(f"\n✅ 已导出:")
    print(f"  {output_dir / 'us_dict.txt'}")
    print(f"  {output_dir / 'gb_dict.txt'}")
    
    # 生成统计信息
    stats = {
        'us_total': len(us_dict),
        'gb_total': len(gb_dict),
        'us_unique': len(set(us_dict.keys()) - set(gb_dict.keys())),
        'gb_unique': len(set(gb_dict.keys()) - set(us_dict.keys())),
        'common': len(set(us_dict.keys()) & set(gb_dict.keys())),
    }
    
    with open(output_dir / "stats.json", 'w', encoding='utf-8') as f:
        json.dump(stats, f, indent=2, ensure_ascii=False)
    
    return us_dict, gb_dict

def extract_japanese_dict():
    """提取日文词典"""
    print(f"\n{'#'*60}")
    print(f"# 提取日文词典")
    print(f"{'#'*60}")
    
    output_dir = OUTPUT_DIR / "ja"
    output_dir.mkdir(exist_ok=True)
    
    words = analyze_file_structure(SOURCE_DIR / "ja_words.txt")
    
    # 直接复制（已经是文本格式）
    with open(output_dir / "words.txt", 'w', encoding='utf-8') as f:
        for word in words:
            f.write(f"{word}\n")
    
    print(f"\n✅ 已导出:")
    print(f"  {output_dir / 'words.txt'}")
    
    # 统计信息
    stats = {
        'total': len(words),
        'unique': len(set(words)),
    }
    
    with open(output_dir / "stats.json", 'w', encoding='utf-8') as f:
        json.dump(stats, f, indent=2, ensure_ascii=False)
    
    return words

def extract_vietnamese_dict():
    """提取越南文词典"""
    print(f"\n{'#'*60}")
    print(f"# 提取越南文词典")
    print(f"{'#'*60}")
    
    output_dir = OUTPUT_DIR / "vi"
    output_dir.mkdir(exist_ok=True)
    
    acronyms = analyze_file_structure(SOURCE_DIR / "vi_acronyms.json")
    symbols = analyze_file_structure(SOURCE_DIR / "vi_symbols.json")
    teencode = analyze_file_structure(SOURCE_DIR / "vi_teencode.json")
    
    # 导出
    with open(output_dir / "acronyms.txt", 'w', encoding='utf-8') as f:
        for word, replacement in sorted(acronyms.items()):
            f.write(f"{word}\t{replacement}\n")
    
    with open(output_dir / "symbols.txt", 'w', encoding='utf-8') as f:
        for symbol, replacement in sorted(symbols.items()):
            f.write(f"{symbol}\t{replacement}\n")
    
    with open(output_dir / "teencode.txt", 'w', encoding='utf-8') as f:
        for code, replacement in sorted(teencode.items()):
            f.write(f"{code}\t{replacement}\n")
    
    print(f"\n✅ 已导出:")
    print(f"  {output_dir / 'acronyms.txt'}")
    print(f"  {output_dir / 'symbols.txt'}")
    print(f"  {output_dir / 'teencode.txt'}")
    
    return acronyms, symbols, teencode

def extract_chinese_data():
    """提取中文数据（从 pypinyin）"""
    print(f"\n{'#'*60}")
    print(f"# 提取中文数据")
    print(f"{'#'*60}")
    
    output_dir = OUTPUT_DIR / "zh"
    output_dir.mkdir(exist_ok=True)
    
    try:
        from pypinyin import pinyin_dict
        
        print(f"pypinyin 词典大小: {len(pinyin_dict.pinyin_dict)}")
        
        # 导出拼音词典
        with open(output_dir / "pinyin_dict.txt", 'w', encoding='utf-8') as f:
            for char_code, pinyin_str in sorted(pinyin_dict.pinyin_dict.items()):
                char = chr(char_code)
                f.write(f"{char}\t{pinyin_str}\n")
        
        print(f"\n✅ 已导出:")
        print(f"  {output_dir / 'pinyin_dict.txt'}")
        
        stats = {
            'total': len(pinyin_dict.pinyin_dict),
        }
        
        with open(output_dir / "stats.json", 'w', encoding='utf-8') as f:
            json.dump(stats, f, indent=2, ensure_ascii=False)
        
    except ImportError:
        print("⚠️  pypinyin 未安装，跳过中文数据提取")
        print("   提示: 运行 `uv pip install pypinyin` 安装")

def generate_summary_report():
    """生成总结报告"""
    print(f"\n{'#'*60}")
    print(f"# 生成总结报告")
    print(f"{'#'*60}")
    
    report_path = OUTPUT_DIR / "EXTRACTION_REPORT.md"
    
    with open(report_path, 'w', encoding='utf-8') as f:
        f.write("# Misaki 数据提取报告\n\n")
        f.write(f"**提取时间**: {__import__('datetime').datetime.now()}\n\n")
        
        f.write("## 📊 数据统计\n\n")
        
        # 英文
        if (OUTPUT_DIR / "en" / "stats.json").exists():
            with open(OUTPUT_DIR / "en" / "stats.json", 'r') as sf:
                en_stats = json.load(sf)
            f.write("### 英文词典\n\n")
            f.write(f"- 美式英语: {en_stats['us_total']:,} 词条\n")
            f.write(f"- 英式英语: {en_stats['gb_total']:,} 词条\n")
            f.write(f"- 共同词汇: {en_stats['common']:,} 词条\n")
            f.write(f"- 美式独有: {en_stats['us_unique']:,} 词条\n")
            f.write(f"- 英式独有: {en_stats['gb_unique']:,} 词条\n\n")
        
        # 日文
        if (OUTPUT_DIR / "ja" / "stats.json").exists():
            with open(OUTPUT_DIR / "ja" / "stats.json", 'r') as sf:
                ja_stats = json.load(sf)
            f.write("### 日文词典\n\n")
            f.write(f"- 总词条: {ja_stats['total']:,}\n")
            f.write(f"- 去重后: {ja_stats['unique']:,}\n\n")
        
        # 中文
        if (OUTPUT_DIR / "zh" / "stats.json").exists():
            with open(OUTPUT_DIR / "zh" / "stats.json", 'r') as sf:
                zh_stats = json.load(sf)
            f.write("### 中文词典\n\n")
            f.write(f"- 拼音表: {zh_stats['total']:,} 汉字\n\n")
        
        f.write("## 📁 文件结构\n\n")
        f.write("```\n")
        f.write("extracted_data/\n")
        for lang_dir in sorted(OUTPUT_DIR.iterdir()):
            if lang_dir.is_dir():
                f.write(f"├── {lang_dir.name}/\n")
                for file in sorted(lang_dir.iterdir()):
                    size = file.stat().st_size / 1024
                    f.write(f"│   ├── {file.name} ({size:.1f} KB)\n")
        f.write("```\n\n")
        
        f.write("## 🔧 数据格式\n\n")
        f.write("所有词典文件都采用 **制表符分隔** (TSV) 格式：\n\n")
        f.write("```\n")
        f.write("word<TAB>phonemes\n")
        f.write("```\n\n")
        f.write("**示例**:\n")
        f.write("```\n")
        f.write("apple\tæpəl\n")
        f.write("hello\thəlˈoʊ\n")
        f.write("```\n\n")
        
        f.write("## ✅ 下一步\n\n")
        f.write("1. 分析数据结构，设计 C 语言数据类型\n")
        f.write("2. 实现文本格式加载器（TSV parser）\n")
        f.write("3. 后续转换为二进制格式（可选）\n")
    
    print(f"\n✅ 报告已生成: {report_path}")

def main():
    """主函数"""
    print("=" * 60)
    print("Misaki 数据提取工具")
    print("=" * 60)
    
    # 检查源目录
    if not SOURCE_DIR.exists():
        print(f"❌ 错误: 源目录不存在: {SOURCE_DIR}")
        print("   请确保已克隆 misaki 仓库")
        sys.exit(1)
    
    # 提取各语言数据
    extract_english_dict()
    extract_japanese_dict()
    extract_vietnamese_dict()
    extract_chinese_data()
    
    # 生成报告
    generate_summary_report()
    
    print(f"\n{'='*60}")
    print("✅ 数据提取完成！")
    print(f"{'='*60}")
    print(f"\n输出目录: {OUTPUT_DIR.absolute()}")
    print(f"查看报告: {OUTPUT_DIR / 'EXTRACTION_REPORT.md'}")

if __name__ == "__main__":
    main()
