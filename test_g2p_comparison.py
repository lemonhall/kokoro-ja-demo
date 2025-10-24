"""
G2P 准确度对比测试
对比 Python (MeCab + Misaki) 和 Kotlin (Kuromoji + OpenJTalkG2P) 的音素输出

运行方法:
    uv run python test_g2p_comparison.py
"""

from ja import text_to_phonemes as python_g2p
import subprocess
import json

# 测试句子集合
TEST_SENTENCES = [
    # 1. 简单问候
    {
        "id": 1,
        "text": "こんにちは、私はレモンと申します。",
        "translation": "你好，我叫柠檬。",
        "category": "问候"
    },
    
    # 2. 包含汉字的句子
    {
        "id": 2,
        "text": "今日は良い天気ですね。",
        "translation": "今天天气真好啊。",
        "category": "日常对话"
    },
    
    # 3. 地名 + 名词
    {
        "id": 3,
        "text": "東京大学からの留学生です。",
        "translation": "我是来自东京大学的留学生。",
        "category": "自我介绍"
    },
    
    # 4. 长句子
    {
        "id": 4,
        "text": "彼女は日本語を勉強していますが、まだ上手ではありません。",
        "translation": "她正在学习日语，但还不是很擅长。",
        "category": "复杂句"
    },
    
    # 5. 包含促音、长音
    {
        "id": 5,
        "text": "学校へ行って、友達と遊びました。",
        "translation": "去了学校，和朋友玩了。",
        "category": "动词时态"
    }
]


def get_python_phonemes(text: str) -> str:
    """使用 Python 的 MeCab + Misaki 获取音素"""
    try:
        phonemes = python_g2p(text)
        return phonemes
    except Exception as e:
        return f"ERROR: {str(e)}"


def get_kotlin_phonemes(text: str) -> str:
    """
    通过调用 Android 应用获取 Kotlin 版本的音素
    
    方法1: 创建一个简单的测试函数在 Kotlin 中
    方法2: 通过 adb 调用 Android 应用
    
    这里我们先返回占位符，需要你在 Android 端添加日志输出
    """
    # TODO: 从 Android 日志或测试输出中获取
    # 临时方案：手动在 Kotlin 中打印日志，然后复制到这里
    return "PLACEHOLDER - 需要从 Android 获取"


def calculate_similarity(phonemes1: str, phonemes2: str) -> float:
    """
    计算两个音素序列的相似度（简单的字符级 Levenshtein 距离）
    """
    if phonemes1 == phonemes2:
        return 100.0
    
    # Levenshtein 距离
    m, n = len(phonemes1), len(phonemes2)
    dp = [[0] * (n + 1) for _ in range(m + 1)]
    
    for i in range(m + 1):
        dp[i][0] = i
    for j in range(n + 1):
        dp[0][j] = j
    
    for i in range(1, m + 1):
        for j in range(1, n + 1):
            if phonemes1[i-1] == phonemes2[j-1]:
                dp[i][j] = dp[i-1][j-1]
            else:
                dp[i][j] = min(
                    dp[i-1][j] + 1,    # 删除
                    dp[i][j-1] + 1,    # 插入
                    dp[i-1][j-1] + 1   # 替换
                )
    
    distance = dp[m][n]
    max_len = max(m, n)
    similarity = (1 - distance / max_len) * 100 if max_len > 0 else 100.0
    
    return similarity


def main():
    print("=" * 80)
    print("G2P 准确度对比测试".center(80))
    print("=" * 80)
    print()
    
    results = []
    
    for sentence in TEST_SENTENCES:
        print(f"\n{'=' * 80}")
        print(f"测试 #{sentence['id']}: {sentence['category']}")
        print(f"{'=' * 80}")
        print(f"📝 原文: {sentence['text']}")
        print(f"🌐 译文: {sentence['translation']}")
        print()
        
        # 获取 Python 版本的音素
        python_phonemes = get_python_phonemes(sentence['text'])
        print(f"🐍 Python (MeCab + Misaki):")
        print(f"   {python_phonemes}")
        print()
        
        # 获取 Kotlin 版本的音素（需要手动填充）
        kotlin_phonemes = "待填充"  # TODO: 从 Android 获取
        print(f"🤖 Kotlin (Kuromoji + OpenJTalk):")
        print(f"   {kotlin_phonemes}")
        print()
        
        # 计算相似度
        if kotlin_phonemes != "待填充":
            similarity = calculate_similarity(python_phonemes, kotlin_phonemes)
            match = "✅ 完全一致" if similarity == 100.0 else f"⚠️ 相似度: {similarity:.1f}%"
            print(f"📊 对比结果: {match}")
            
            if similarity < 100.0:
                print(f"   差异分析:")
                print(f"   Python: {python_phonemes}")
                print(f"   Kotlin: {kotlin_phonemes}")
        else:
            print(f"⏳ 等待填充 Kotlin 输出...")
        
        results.append({
            "id": sentence['id'],
            "text": sentence['text'],
            "python": python_phonemes,
            "kotlin": kotlin_phonemes,
            "similarity": calculate_similarity(python_phonemes, kotlin_phonemes) if kotlin_phonemes != "待填充" else 0
        })
    
    # 汇总统计
    print(f"\n{'=' * 80}")
    print("📊 测试汇总".center(80))
    print(f"{'=' * 80}")
    
    total = len(results)
    filled = sum(1 for r in results if r['kotlin'] != "待填充")
    
    if filled > 0:
        avg_similarity = sum(r['similarity'] for r in results) / filled
        perfect_matches = sum(1 for r in results if r['similarity'] == 100.0)
        
        print(f"\n总测试数: {total}")
        print(f"已完成: {filled}")
        print(f"平均相似度: {avg_similarity:.1f}%")
        print(f"完全匹配: {perfect_matches}/{filled}")
    else:
        print(f"\n⚠️ 还没有填充 Kotlin 输出，请按照下面的步骤操作：")
        print()
        print("=" * 80)
        print("📱 Kotlin 端操作步骤".center(80))
        print("=" * 80)
        print()
        print("1. 在 MainActivity.kt 的 synthesizeCustomText 中添加日志：")
        print()
        print('   println("G2P_TEST: $text -> $phonemes")')
        print()
        print("2. 在 Android 应用中依次输入以下句子：")
        print()
        for i, sentence in enumerate(TEST_SENTENCES, 1):
            print(f"   {i}. {sentence['text']}")
        print()
        print("3. 使用 adb 查看日志：")
        print()
        print("   adb logcat -s System.out:I | grep G2P_TEST")
        print()
        print("4. 将日志中的音素输出复制到本脚本的 KOTLIN_RESULTS 字典中")
    
    # 保存结果到 JSON
    with open('g2p_comparison_results.json', 'w', encoding='utf-8') as f:
        json.dump(results, f, ensure_ascii=False, indent=2)
    
    print(f"\n✅ 结果已保存到: g2p_comparison_results.json")
    print()


if __name__ == "__main__":
    main()
