#!/usr/bin/env python3
"""
测试混合语言 G2P 转换

模拟 Android 端的完整处理流程
"""

from misaki import zh

def test_mixed_language():
    """测试混合语言处理"""
    
    print("=" * 60)
    print("混合语言 G2P 测试")
    print("=" * 60)
    
    text = "我蛮喜欢apple手机的"
    
    print(f"\n输入文本: {text}")
    print("\n" + "-" * 60)
    
    # Python 端的中文 G2P
    zh_g2p = zh.ZHG2P()
    
    # 分段测试
    print("\n【预期分段】")
    segments = [
        ("我蛮喜欢", "zh"),
        ("apple", "en"),
        ("手机的", "zh"),
    ]
    
    for i, (segment_text, lang) in enumerate(segments, 1):
        print(f"\n段{i}: \"{segment_text}\" ({lang})")
        
        if lang == "zh":
            # 中文处理
            ipa, _ = zh_g2p(segment_text)
            print(f"  → IPA: {ipa}")
        elif lang == "en":
            # 英文（查词典）
            # 这里模拟 Android 端的查询结果
            print(f"  → 词典查询: apple")
            print(f"  → IPA: æ p ə l  (CMUdict)")
    
    print("\n" + "-" * 60)
    print("\n【完整拼接】")
    print("预期输出（简化）:")
    print("  段1(中文) + 空格 + 段2(英文) + 空格 + 段3(中文)")
    print()
    
    # 实际测试完整文本
    print("\n【Python 端对比】")
    print("如果全部按中文处理:")
    try:
        full_ipa, _ = zh_g2p(text)
        print(f"  IPA: {full_ipa}")
        print("  ⚠️  注意: apple 会被当作中文处理，发音错误")
    except Exception as e:
        print(f"  ❌ 错误: {e}")


def test_english_dict():
    """测试英文词典"""
    
    print("\n\n" + "=" * 60)
    print("英文词典查询测试")
    print("=" * 60)
    
    import json
    
    # 加载生成的英文词典
    with open("app/src/main/assets/english_dict.json", 'r', encoding='utf-8') as f:
        english_dict = json.load(f)
    
    print(f"\n📚 词典大小: {len(english_dict)} 个单词")
    
    # 测试查询
    test_words = ["apple", "iphone", "hello", "world", "test", "unknownword123"]
    
    print("\n测试查询:")
    for word in test_words:
        result = english_dict.get(word.lower(), None)
        if result:
            print(f"  ✅ {word} → {result}")
        else:
            print(f"  ❌ {word} → 未找到（需要回退到字母拼读）")


if __name__ == "__main__":
    test_mixed_language()
    test_english_dict()
    
    print("\n" + "=" * 60)
    print("✅ 测试完成")
    print("=" * 60)
    print()
