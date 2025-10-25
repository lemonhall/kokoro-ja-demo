#!/usr/bin/env python3
"""
验证 Kotlin ChinesePinyinToIPA 的转换结果

对比 Python 端的 misaki 输出，确保一致性
"""

from misaki import zh

def test_pinyin_to_ipa():
    """测试拼音转IPA的转换"""
    
    g2p = zh.ZHG2P()
    
    test_cases = [
        ("你", "ni3"),
        ("好", "hao3"),
        ("知", "zhi1"),
        ("识", "shi2"),
        ("妈", "ma1"),
        ("麻", "ma2"),
        ("马", "ma3"),
        ("骂", "ma4"),
        ("吗", "ma5"),
        ("中", "zhong1"),
        ("文", "wen2"),
        ("资", "zi1"),
        ("次", "ci4"),
        ("思", "si1"),
    ]
    
    print("=" * 60)
    print("Python 端拼音→IPA转换测试")
    print("=" * 60)
    
    for char, pinyin in test_cases:
        # 使用 misaki 的直接转换
        ipa = zh.ZHG2P.py2ipa(pinyin)
        print(f"{char} ({pinyin:8s}) → {ipa}")
    
    print("\n" + "=" * 60)
    print("完整句子测试")
    print("=" * 60)
    
    sentences = [
        "你好",
        "中文测试",
        "知识就是力量",
    ]
    
    for text in sentences:
        ipa, _ = g2p(text)
        print(f"\n{text}:")
        print(f"  IPA: {ipa}")


if __name__ == "__main__":
    test_pinyin_to_ipa()
