#!/usr/bin/env python3
"""
测试 pypinyin 拼音转换

目的: 了解 pypinyin 的实际输出格式，为 Android 移植提供参考
"""

from pypinyin import lazy_pinyin, Style
from misaki import zh

def test_pypinyin():
    """测试 pypinyin 的不同输出风格"""
    
    print("=" * 60)
    print("Task 0.2: 测试 pypinyin 拼音转换")
    print("=" * 60)
    
    test_sentences = [
        "你好世界",
        "中文测试",
        "知识就是力量",
        "我行行好吧",  # 多音字测试
        "2024年10月25日",  # 数字测试
        "iPhone 15 很贵",  # 混合文本
        "儿童二十耳朵",  # er 音测试
        "资次思",  # 平舌音
        "知吃是",  # 卷舌音
    ]
    
    print("\n📋 测试句子列表:\n")
    
    for i, text in enumerate(test_sentences, 1):
        print(f"\n{i}. 测试句子: {text}")
        print("-" * 60)
        
        # 1. TONE3 风格 (数字标调)
        pinyin_tone3 = lazy_pinyin(
            text, 
            style=Style.TONE3, 
            neutral_tone_with_five=True
        )
        print(f"   TONE3 风格: {' '.join(pinyin_tone3)}")
        
        # 2. NORMAL 风格 (无声调)
        pinyin_normal = lazy_pinyin(text, style=Style.NORMAL)
        print(f"   NORMAL 风格: {' '.join(pinyin_normal)}")
        
        # 3. TONE 风格 (带声调符号)
        pinyin_tone = lazy_pinyin(text, style=Style.TONE)
        print(f"   TONE 风格: {' '.join(pinyin_tone)}")
        
        # 4. FINALS 风格 (仅韵母)
        pinyin_finals = lazy_pinyin(text, style=Style.FINALS)
        print(f"   FINALS 风格: {' '.join(pinyin_finals)}")
        
        # 5. INITIALS 风格 (仅声母)
        pinyin_initials = lazy_pinyin(text, style=Style.INITIALS)
        print(f"   INITIALS 风格: {' '.join(pinyin_initials)}")


def test_misaki_g2p():
    """测试 misaki 完整的 G2P 转换"""
    
    print("\n\n" + "=" * 60)
    print("Task 0.3: 测试 Misaki 完整 G2P 转换")
    print("=" * 60)
    
    g2p = zh.ZHG2P()
    
    test_sentences = [
        "你好世界",
        "中文测试，长句朗读",
        "知识就是力量",
        "资次思",  # 平舌音
        "知吃是",  # 卷舌音
    ]
    
    print("\n📊 完整转换流程:\n")
    
    for i, text in enumerate(test_sentences, 1):
        print(f"\n{i}. 原文: {text}")
        print("-" * 60)
        
        # 获取拼音
        pinyins = lazy_pinyin(
            text, 
            style=Style.TONE3, 
            neutral_tone_with_five=True
        )
        print(f"   拼音: {' '.join(pinyins)}")
        
        # G2P 转换
        ipa, _ = g2p(text)
        print(f"   IPA: {ipa}")
        
        # 逐字分析
        print(f"\n   📝 逐字分析:")
        for j, char in enumerate(text):
            if char.strip() and '\u4E00' <= char <= '\u9FFF':  # 汉字范围
                char_pinyin = pinyins[j] if j < len(pinyins) else "?"
                char_ipa = zh.ZHG2P.word2ipa(char)
                print(f"      {char} → {char_pinyin} → {char_ipa}")


def test_tone_mapping():
    """测试声调转换"""
    
    print("\n\n" + "=" * 60)
    print("声调符号测试")
    print("=" * 60)
    
    test_words = [
        ("妈", "ma1", "第一声 (阴平)"),
        ("麻", "ma2", "第二声 (阳平)"),
        ("马", "ma3", "第三声 (上声)"),
        ("骂", "ma4", "第四声 (去声)"),
        ("吗", "ma5", "轻声"),
    ]
    
    print("\n声调符号对照:\n")
    
    g2p = zh.ZHG2P()
    
    for char, pinyin, desc in test_words:
        ipa = zh.ZHG2P.py2ipa(pinyin)
        simplified = zh.ZHG2P.retone(ipa)
        print(f"   {char} ({pinyin}) - {desc}")
        print(f"      完整 IPA: {ipa}")
        print(f"      简化版: {simplified}")
        print()


def main():
    """主函数"""
    try:
        test_pypinyin()
        test_misaki_g2p()
        test_tone_mapping()
        
        print("\n" + "=" * 60)
        print("✅ 测试完成!")
        print("=" * 60)
        print("\n📝 下一步:")
        print("   1. 查看 misaki中文G2P分析报告.md")
        print("   2. 开始实现 ChinesePinyinToIPA.kt")
        print("   3. 集成 TinyPinyin 库")
        print("\n")
        
    except Exception as e:
        print(f"\n❌ 错误: {e}")
        import traceback
        traceback.print_exc()


if __name__ == "__main__":
    main()
