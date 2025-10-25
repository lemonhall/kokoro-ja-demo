#!/usr/bin/env python3
"""
测试 Android 端的自动语言检测和 G2P 转换

目的: 生成测试用例，验证完整的多语言处理流程
"""

from misaki import zh, ja

def test_multilingual_g2p():
    """测试多语言 G2P 转换"""
    
    print("=" * 60)
    print("多语言 G2P 测试")
    print("=" * 60)
    
    # 中文 G2P
    zh_g2p = zh.ZHG2P()
    
    test_cases = [
        # 中文
        ("你好世界", "zh"),
        ("中文测试", "zh"),
        ("知识就是力量", "zh"),
        
        # 日文
        ("こんにちは", "ja"),
        ("ありがとう", "ja"),
        ("日本語テスト", "ja"),
        
        # 韩文
        ("안녕하세요", "ko"),
        ("감사합니다", "ko"),
    ]
    
    print("\n📊 测试用例:\n")
    
    for text, expected_lang in test_cases:
        print(f"\n原文: {text}")
        print(f"预期语言: {expected_lang}")
        
        if expected_lang == "zh":
            ipa, _ = zh_g2p(text)
            print(f"Python IPA: {ipa}")
            print(f"🎯 Android 端应该自动检测为中文并调用 ChineseG2PSystem")
        
        elif expected_lang == "ja":
            print(f"🎯 Android 端应该自动检测为日文并调用 JapaneseG2PSystem")
        
        elif expected_lang == "ko":
            print(f"⚠️  韩文暂未支持")
        
        print("-" * 60)


def generate_test_sentences():
    """生成测试句子"""
    
    print("\n\n" + "=" * 60)
    print("生成 Android 测试句子")
    print("=" * 60)
    
    zh_g2p = zh.ZHG2P()
    
    sentences = [
        "你好世界",
        "中文语音合成",
        "今天天气很好",
        "知识就是力量",
    ]
    
    print("\n📝 中文测试句子（可直接用于 Android 测试）:\n")
    
    for text in sentences:
        ipa, _ = zh_g2p(text)
        
        # 简化声调符号（使用箭头）
        simplified = zh.ZHG2P.retone(ipa)
        
        print(f"原文: {text}")
        print(f"IPA:  {ipa}")
        print(f"简化: {simplified}")
        print()


def main():
    """主函数"""
    try:
        test_multilingual_g2p()
        generate_test_sentences()
        
        print("\n" + "=" * 60)
        print("✅ 测试完成!")
        print("=" * 60)
        print("\n📝 下一步:")
        print("   1. 在 Android 端输入这些测试句子")
        print("   2. 验证语言自动检测是否正确")
        print("   3. 验证 IPA 输出是否与 Python 端一致")
        print("\n")
        
    except Exception as e:
        print(f"\n❌ 错误: {e}")
        import traceback
        traceback.print_exc()


if __name__ == "__main__":
    main()
