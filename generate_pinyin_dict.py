#!/usr/bin/env python3
"""
生成拼音字典 JSON 文件

目的: 将 pypinyin 的拼音数据导出为 JSON，供 Android Kotlin 代码使用
"""

from pypinyin import pinyin, Style
import json

def generate_common_chars_dict():
    """生成常用汉字拼音字典"""
    
    # 常用汉字范围 (3500个常用字)
    common_chars = []
    
    # GB2312 一级汉字 (3755个)
    for code in range(0x4E00, 0x9FA6):  # CJK 统一汉字
        char = chr(code)
        common_chars.append(char)
    
    print(f"📝 正在生成拼音字典... (共 {len(common_chars)} 个汉字)")
    
    pinyin_dict = {}
    
    for i, char in enumerate(common_chars):
        try:
            # 使用 TONE3 风格，数字标调
            result = pinyin(
                char, 
                style=Style.TONE3, 
                neutral_tone_with_five=True,
                heteronym=False  # 只取第一个读音
            )
            
            if result and result[0]:
                py = result[0][0]
                pinyin_dict[char] = py
            
            # 进度提示
            if (i + 1) % 1000 == 0:
                print(f"   已处理: {i + 1}/{len(common_chars)}")
        
        except Exception as e:
            print(f"   ⚠️ 处理 '{char}' 时出错: {e}")
            continue
    
    return pinyin_dict


def main():
    """主函数"""
    
    print("=" * 60)
    print("生成 Android 拼音字典")
    print("=" * 60)
    print()
    
    # 1. 生成字典
    pinyin_dict = generate_common_chars_dict()
    
    print(f"\n✅ 字典生成完成！")
    print(f"   包含 {len(pinyin_dict)} 个汉字")
    
    # 2. 保存为 JSON
    output_file = "app/src/main/assets/pinyin_dict.json"
    
    with open(output_file, 'w', encoding='utf-8') as f:
        json.dump(pinyin_dict, f, ensure_ascii=False, indent=None)
    
    file_size = len(json.dumps(pinyin_dict, ensure_ascii=False)) / 1024
    
    print(f"\n💾 已保存到: {output_file}")
    print(f"   文件大小: {file_size:.2f} KB")
    
    # 3. 生成测试样例
    test_chars = "你好世界中文测试知识就是力量"
    print(f"\n🔍 测试样例:")
    for char in test_chars:
        py = pinyin_dict.get(char, "???")
        print(f"   {char} → {py}")
    
    print("\n✅ 完成！")
    print("\n📝 下一步:")
    print("   1. Kotlin 代码加载 pinyin_dict.json")
    print("   2. 实现汉字→拼音查询")
    print("   3. 结合 ChinesePinyinToIPA 完成 G2P")
    print()


if __name__ == "__main__":
    main()
