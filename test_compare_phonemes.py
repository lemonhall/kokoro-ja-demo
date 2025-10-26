"""对比C版本和Python版本的音素输出"""
import sys
sys.path.insert(0, 'E:/development/kokoro-ja-demo')

from misaki import text_to_ipa

# 测试词汇
test_words = [
    "心情",
    "新颖", 
    "经营",
    "音乐"
]

print("=" * 60)
print("Python版本 (Misaki) 音素输出")
print("=" * 60)

for word in test_words:
    ipa = text_to_ipa(word, language="zh")
    print(f"{word}: {ipa}")
    # 打印每个字符的详细信息
    print(f"  字符详情: {[f'{c}(U+{ord(c):04X})' for c in ipa]}")
    print()
