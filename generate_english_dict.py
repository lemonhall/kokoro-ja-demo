#!/usr/bin/env python3
"""
生成英文发音字典

使用完整的 CMU Pronouncing Dictionary (CMUdict)
包含 13 万+ 英文单词的发音数据
"""

import json
import re
import pronouncing

def arpabet_to_ipa(arpabet):
    """
    将 ARPAbet 音素转换为 IPA
    
    ARPAbet 是 CMUdict 使用的音标系统
    """
    # ARPAbet → IPA 映射表
    mapping = {
        # 元音
        'AA': 'ɑ', 'AE': 'æ', 'AH': 'ʌ', 'AO': 'ɔ', 'AW': 'aʊ',
        'AX': 'ə', 'AXR': 'ɚ', 'AY': 'aɪ', 'EH': 'ɛ', 'ER': 'ɝ',
        'EY': 'eɪ', 'IH': 'ɪ', 'IX': 'ɨ', 'IY': 'i', 'OW': 'oʊ',
        'OY': 'ɔɪ', 'UH': 'ʊ', 'UW': 'u', 'UX': 'ʉ',
        
        # 辅音
        'B': 'b', 'CH': 'ʧ', 'D': 'd', 'DH': 'ð', 'DX': 'ɾ',
        'EL': 'l̩', 'EM': 'm̩', 'EN': 'n̩', 'F': 'f', 'G': 'ɡ',
        'HH': 'h', 'JH': 'ʤ', 'K': 'k', 'L': 'l', 'M': 'm',
        'N': 'n', 'NG': 'ŋ', 'NX': 'ɾ̃', 'P': 'p', 'Q': 'ʔ',
        'R': 'r', 'S': 's', 'SH': 'ʃ', 'T': 't', 'TH': 'θ',
        'V': 'v', 'W': 'w', 'WH': 'ʍ', 'Y': 'j', 'Z': 'z',
        'ZH': 'ʒ'
    }
    
    # 移除重音标记 (0, 1, 2)
    phoneme = re.sub(r'[012]', '', arpabet)
    
    return mapping.get(phoneme, phoneme.lower())


def generate_full_cmudict():
    """
    生成完整的 CMUdict 发音字典
    
    使用 pronouncing 库加载完整的 CMU 发音词典
    包含 13 万+ 单词
    """
    
    print("📚 加载 CMU Pronouncing Dictionary...")
    
    # 获取 CMUdict 的所有词条
    from pronouncing import cmudict
    
    pronunciation_dict = {}
    total_words = 0
    processed_words = 0
    
    # CMUdict 原始数据
    for word, pronunciations in cmudict.dict().items():
        total_words += 1
        
        # 只取第一个发音（多音字问题）
        if pronunciations:
            arpabet_list = pronunciations[0]  # 如 ['AY1', 'F', 'OW1', 'N']
            
            # 转换为 IPA
            ipa_phonemes = [arpabet_to_ipa(p) for p in arpabet_list]
            pronunciation_dict[word.lower()] = " ".join(ipa_phonemes)
            
            processed_words += 1
        
        # 进度提示
        if total_words % 10000 == 0:
            print(f"   已处理: {total_words} 个单词...")
    
    print(f"\n✅ 字典生成完成！")
    print(f"   总词条: {total_words}")
    print(f"   有效词条: {processed_words}")
    
    return pronunciation_dict


def main():
    """主函数"""
    
    print("=" * 60)
    print("生成英文发音字典")
    print("=" * 60)
    print()
    
    # 生成字典
    pronunciation_dict = generate_full_cmudict()
    
    print(f"✅ 字典生成完成！")
    print(f"   包含 {len(pronunciation_dict)} 个单词")
    
    # 保存为 JSON
    output_file = "app/src/main/assets/english_dict.json"
    
    with open(output_file, 'w', encoding='utf-8') as f:
        json.dump(pronunciation_dict, f, ensure_ascii=False, indent=2)
    
    file_size = len(json.dumps(pronunciation_dict, ensure_ascii=False)) / 1024
    
    print(f"\n💾 已保存到: {output_file}")
    print(f"   文件大小: {file_size:.2f} KB")
    
    # 测试样例
    print(f"\n🔍 测试样例:")
    test_words = ["iphone", "hello", "world", "test", "one", "a"]
    for word in test_words:
        ipa = pronunciation_dict.get(word, "???")
        print(f"   {word} → {ipa}")
    
    print("\n✅ 完成！")
    print("\n📝 下一步:")
    print("   1. Kotlin 代码加载 english_dict.json")
    print("   2. 实现 EnglishG2PSystem")
    print("   3. 集成到 UnifiedG2PSystem")
    print()


if __name__ == "__main__":
    main()
