#!/usr/bin/env python3
"""
测试 Python 端的混合语言分段逻辑
"""

import re

def test_python_regex():
    """测试 misaki 使用的正则表达式"""
    
    # misaki 的分段正则
    pattern = r'([A-Za-z \'-]*[A-Za-z][A-Za-z \'-]*)|([^A-Za-z]+)'
    
    test_cases = [
        "我蛮喜欢apple手机的",
        "今天的presentation很成功",
        "我买了iPhone很好",
        "I don't like it 我不喜欢",
        "这是MacBook Pro很贵",
    ]
    
    print("=" * 60)
    print("Python 端分段测试（misaki 正则）")
    print("=" * 60)
    
    for text in test_cases:
        print(f"\n输入: {text}")
        print("分段结果:")
        
        segments = re.findall(pattern, text)
        for i, (en, zh) in enumerate(segments, 1):
            en, zh = en.strip(), zh.strip()
            if en:
                print(f"  段{i}: \"{en}\" → 英文")
            elif zh:
                print(f"  段{i}: \"{zh}\" → 中文")


def test_our_approach():
    """测试我们当前的逐字检测方法"""
    
    print("\n\n" + "=" * 60)
    print("Android 端分段测试（逐字 Unicode 检测）")
    print("=" * 60)
    
    test_cases = [
        "我蛮喜欢apple手机的",
        "今天的presentation很成功",
    ]
    
    for text in test_cases:
        print(f"\n输入: {text}")
        print("分段结果:")
        
        segments = []
        current_segment = []
        current_lang = None
        
        for char in text:
            code = ord(char)
            
            # 判断语言
            if 0x4E00 <= code <= 0x9FFF:
                lang = "中文"
            elif (0x0041 <= code <= 0x005A) or (0x0061 <= code <= 0x007A):
                lang = "英文"
            elif char.isspace() or char in ",.!?;:":
                # 空格和标点归到当前语言
                if current_segment:
                    current_segment.append(char)
                continue
            else:
                lang = "其他"
            
            # 语言切换
            if current_lang and lang != current_lang and lang != "其他":
                if current_segment:
                    segments.append(("".join(current_segment), current_lang))
                    current_segment = []
                current_lang = lang
            elif current_lang is None and lang != "其他":
                current_lang = lang
            
            current_segment.append(char)
        
        # 最后一段
        if current_segment and current_lang:
            segments.append(("".join(current_segment), current_lang))
        
        for i, (seg_text, lang) in enumerate(segments, 1):
            print(f"  段{i}: \"{seg_text}\" → {lang}")


if __name__ == "__main__":
    test_python_regex()
    test_our_approach()
    
    print("\n" + "=" * 60)
    print("对比总结")
    print("=" * 60)
    print("\nPython 正则的优势:")
    print("  ✅ 英文单词之间的空格不会导致切分")
    print("  ✅ 支持撇号、连字符（如 don't, high-quality）")
    print("  ✅ 更符合自然语言的分段逻辑")
    print("\nAndroid 逐字检测的问题:")
    print("  ❌ 空格会被当作分隔符")
    print("  ❌ 'presentation' 可能被当成多段")
    print("  ❌ 不符合自然语言的分段逻辑")
    print()
