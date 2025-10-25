#!/usr/bin/env python3
"""测试 pyopenjtalk 如何使用词典"""

import pyopenjtalk

# 测试文本
text = "私は学生です"

print("=" * 60)
print("测试 pyopenjtalk 分词结果")
print("=" * 60)
print(f"输入: {text}\n")

# 获取分词结果
words = pyopenjtalk.run_frontend(text)

for i, word in enumerate(words):
    print(f"词 {i+1}:")
    print(f"  surface: {word['string']}")
    print(f"  pron: {word.get('pron', 'N/A')}")
    print(f"  pos: {word.get('pos', 'N/A')}")
    print(f"  acc: {word.get('acc', 'N/A')}")
    print(f"  mora_size: {word.get('mora_size', 'N/A')}")
    print()

print("\n" + "=" * 60)
print("pyopenjtalk 使用的词典信息")
print("=" * 60)

try:
    # 尝试获取词典路径
    import pyopenjtalk
    print("pyopenjtalk 已安装")
    
    # 检查是否有词典信息
    import inspect
    print(f"\npyopenjtalk 模块路径: {inspect.getfile(pyopenjtalk)}")
    
except Exception as e:
    print(f"错误: {e}")

print("\n说明:")
print("- pyopenjtalk 内置了 OpenJTalk 和词典数据")
print("- 不需要单独下载 UniDic")
print("- 提供完整的日文分词、词性标注、读音信息")
