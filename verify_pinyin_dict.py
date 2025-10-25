#!/usr/bin/env python3
"""
验证 Android 端的拼音字典质量

对比 pypinyin 实时输出和字典数据的一致性
"""

from pypinyin import pinyin, Style
import json

def verify_dict_quality():
    """验证字典质量"""
    
    print("=" * 60)
    print("验证拼音字典质量")
    print("=" * 60)
    
    # 加载字典
    with open("app/src/main/assets/pinyin_dict.json", 'r', encoding='utf-8') as f:
        pinyin_dict = json.load(f)
    
    print(f"\n📚 字典包含: {len(pinyin_dict)} 个汉字")
    
    # 测试句子
    test_sentences = [
        "你好世界",
        "中文语音合成",
        "今天天气很好",
        "知识就是力量",
        "我爱编程",
        "人工智能",
        "深度学习",
    ]
    
    total_chars = 0
    matched_chars = 0
    mismatched = []
    
    print("\n🔍 验证测试句子:\n")
    
    for text in test_sentences:
        print(f"原文: {text}")
        
        for char in text:
            if '\u4E00' <= char <= '\u9FFF':  # 汉字范围
                total_chars += 1
                
                # pypinyin 实时输出
                py_real = pinyin(
                    char, 
                    style=Style.TONE3, 
                    neutral_tone_with_five=True,
                    heteronym=False
                )[0][0]
                
                # 字典数据
                py_dict = pinyin_dict.get(char, "???")
                
                if py_real == py_dict:
                    matched_chars += 1
                    print(f"  ✅ {char} → {py_real}")
                else:
                    mismatched.append((char, py_real, py_dict))
                    print(f"  ⚠️  {char} → pypinyin: {py_real}, 字典: {py_dict}")
        
        print()
    
    # 统计
    accuracy = (matched_chars / total_chars * 100) if total_chars > 0 else 0
    
    print("=" * 60)
    print("📊 验证结果:")
    print("=" * 60)
    print(f"总字符数: {total_chars}")
    print(f"匹配数: {matched_chars}")
    print(f"不匹配数: {len(mismatched)}")
    print(f"准确率: {accuracy:.2f}%")
    
    if mismatched:
        print("\n⚠️  不匹配的字符:")
        for char, real, dict_val in mismatched[:10]:  # 只显示前10个
            print(f"  {char}: pypinyin={real}, 字典={dict_val}")
    
    if accuracy == 100.0:
        print("\n✅ 字典质量完美！")
    elif accuracy >= 99.0:
        print("\n✅ 字典质量优秀！")
    elif accuracy >= 95.0:
        print("\n⚠️  字典质量良好，但有少量差异")
    else:
        print("\n❌ 字典可能需要重新生成")


def test_android_conversion():
    """模拟 Android 端的转换流程"""
    
    print("\n\n" + "=" * 60)
    print("模拟 Android 端转换流程")
    print("=" * 60)
    
    # 加载字典
    with open("app/src/main/assets/pinyin_dict.json", 'r', encoding='utf-8') as f:
        pinyin_dict = json.load(f)
    
    test_text = "你好世界"
    
    print(f"\n原文: {test_text}")
    print("\n步骤:")
    
    # 步骤1: 查询字典
    pinyins = []
    for char in test_text:
        py = pinyin_dict.get(char, "unknown")
        pinyins.append(py)
        print(f"  1. {char} → 字典查询 → {py}")
    
    print(f"\n拼音列表: {' '.join(pinyins)}")
    
    # 步骤2: 会传给 ChinesePinyinToIPA.convert()
    print("\n  2. 下一步将调用 ChinesePinyinToIPA.convert()")
    print("     每个拼音会被转换为 IPA 音素")
    
    print("\n✅ Android 端流程模拟完成")


def main():
    """主函数"""
    try:
        verify_dict_quality()
        test_android_conversion()
        
        print("\n" + "=" * 60)
        print("✅ 验证完成!")
        print("=" * 60)
        print()
        
    except Exception as e:
        print(f"\n❌ 错误: {e}")
        import traceback
        traceback.print_exc()


if __name__ == "__main__":
    main()
