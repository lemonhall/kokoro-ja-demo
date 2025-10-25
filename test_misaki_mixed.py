#!/usr/bin/env python3
"""
测试 Python 端的中英混合音素输出
"""

from misaki import zh

def test_mixed_language():
    """测试混合语言的音素输出"""
    
    zh_g2p = zh.ZHG2P()
    
    test_cases = [
        "我蛮喜欢apple手机的",
        "今天的presentation很成功",
        "我买了iPhone很好",
    ]
    
    print("=" * 60)
    print("Python 端混合语言音素测试")
    print("=" * 60)
    
    for text in test_cases:
        print(f"\n输入: {text}")
        
        # 调用 ZHG2P（没有 en_callable，英文会被替换成 ❓）
        ipa, _ = zh_g2p(text)
        
        print(f"IPA (无英文处理): {ipa}")
        print(f"  ⚠️  注意: 英文部分被替换成 ❓")
        
        # 如果提供了 en_callable
        def simple_en_g2p(en_text):
            """简化的英文 G2P（每个字母一个音素）"""
            return en_text.lower()
        
        ipa_with_en, _ = zh_g2p(text, en_callable=simple_en_g2p)
        print(f"IPA (简化英文处理): {ipa_with_en}")


if __name__ == "__main__":
    test_mixed_language()
    
    print("\n" + "=" * 60)
    print("关键发现")
    print("=" * 60)
    print("\n1. misaki 的 ZHG2P 默认会把英文替换成 ❓")
    print("2. 需要提供 en_callable 才能正确处理英文")
    print("3. 音素之间用空格分隔")
    print("4. 中英文段之间也用空格分隔")
    print()
