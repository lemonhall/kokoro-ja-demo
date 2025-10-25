"""
G2P 准确度对比测试
对比 Python (MeCab + Misaki) 和 Kotlin (Kuromoji + OpenJTalkG2P) 的音素输出

运行方法:
    uv run python test_g2p_comparison.py
"""

# pip install misaki[ja]
from kokoro import KPipeline
import subprocess
import json

# 初始化日语 pipeline
print("正在初始化 Kokoro Pipeline...")
pipeline = KPipeline(lang_code='j')
print("初始化完成！")

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
    """
    使用 Python 的 Kokoro Pipeline 获取音素
    
    Args:
        text: 日语文本
    Returns:
        音素字符串（空格分隔）
    """
    try:
        generator = pipeline(text, voice='jf_nezumi', speed=1, split_pattern=r'\n+')
        # 提取所有音素并拼接
        phonemes_list = []
        for gs, ps, audio in generator:
            phonemes_list.append(ps)
        return ' '.join(phonemes_list)
    except Exception as e:
        return f"ERROR: {str(e)}"


def get_kotlin_phonemes_from_instrumentation(sentence_id: int, text: str) -> str | None:
    """
    通过 Android Instrumentation Test 获取 Kotlin 版本的音素输出
    
    Args:
        sentence_id: 句子编号
        text: 输入的日语文本
    Returns:
        音素字符串，如果失败返回 None
    """
    # 这个函数不再需要了，我们会一次性获取所有结果
    return None


def run_android_g2p_tests() -> dict[int, str]:
    """
    运行 Android 端的 G2P 测试，一次性获取所有结果
    
    Returns:
        字典: {sentence_id: phonemes}
    """
    import re
    
    print("\n🤖 正在运行 Android 端 G2P 测试...")
    print("⚠️  请稍等，这可能需要 10-20 秒...\n")
    
    try:
        # 运行 Android Instrumentation Test
        result = subprocess.run(
            [
                'adb', 'shell', 'am', 'instrument', '-w',
                '-e', 'class', 'com.lsl.kokoro_ja_android.G2PComparisonTest',
                'com.lsl.kokoro_ja_android.test/androidx.test.runner.AndroidJUnitRunner'
            ],
            capture_output=True,
            timeout=60
        )
        
        if result.returncode != 0:
            print(f"❌ 测试运行失败")
            print(f"stdout: {result.stdout.decode('utf-8', errors='ignore')}")
            print(f"stderr: {result.stderr.decode('utf-8', errors='ignore')}")
            return {}
        
        # 从 logcat 获取测试输出（避免编码问题）
        logcat_result = subprocess.run(
            ['adb', 'logcat', '-d', '-s', 'System.out:I'],
            capture_output=True
        )
        
        output = logcat_result.stdout.decode('utf-8', errors='ignore')
        
        # 解析输出，提取 G2P_RESULT 行
        # 格式: G2P_RESULT|sequence_id|text|phonemes
        kotlin_results = {}
        
        for line in output.split('\n'):
            match = re.search(r'G2P_RESULT\|(\d+)\|(.+?)\|(.+?)$', line)
            if match:
                sentence_id = int(match.group(1))
                # text = match.group(2)
                phonemes = match.group(3).strip()
                kotlin_results[sentence_id] = phonemes
                print(f"✅ 测试 #{sentence_id} 完成")
        
        if not kotlin_results:
            print("⚠️  未找到 G2P 结果，检查测试输出:")
            # 只显示包含 G2P 的行
            for line in output.split('\n'):
                if 'G2P' in line:
                    print(line)
        
        return kotlin_results
        
    except subprocess.TimeoutExpired:
        print("❌ 测试超时，请检查 Android 应用是否安装")
        return {}
    except FileNotFoundError:
        print("❌ 错误: 找不到 adb 命令")
        return {}
    except Exception as e:
        print(f"❌ 错误: {str(e)}")
        return {}


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
    import sys
    
    print("=" * 80)
    print("G2P 准确度对比测试".center(80))
    print("=" * 80)
    print()
    
    # 检查是否有 adb 命令
    try:
        result = subprocess.run(['adb', 'devices'], capture_output=True, text=True)
        if 'device' not in result.stdout:
            print("⚠️  警告: 未检测到 Android 设备")
            print("请确保:")
            print("  1. Android 设备已通过 USB 连接")
            print("  2. 已启用 USB 调试模式")
            print("  3. 运行 'adb devices' 可以看到设备")
            print()
            response = input("是否继续？(y/n): ")
            if response.lower() != 'y':
                return
    except FileNotFoundError:
        print("❌ 错误: 找不到 adb 命令")
        print("请安装 Android SDK Platform Tools 并配置环境变量")
        return
    
    # 选择模式
    auto_mode = True  # 默认使用自动模式
    
    # 如果是自动模式，先运行 Android 测试
    kotlin_results = {}
    if auto_mode:
        print("\n" + "=" * 80)
        print("🤖 全自动测试模式".center(80))
        print("=" * 80)
        print("\n⚠️  提示:")
        print("  - 无需手动操作 Android 应用")
        print("  - 系统将自动运行 Instrumentation Test")
        print("  - 整个过程约 10-20 秒")
        print()
        
        kotlin_results = run_android_g2p_tests()
    
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
        print(f"🐍 Python (Kokoro + Misaki):")
        print(f"   {python_phonemes}")
        print()
        
        # 获取 Kotlin 版本的音素（从预先运行的测试结果中）
        kotlin_phonemes = kotlin_results.get(sentence['id'], None)
        
        if kotlin_phonemes:
            print(f"🤖 Kotlin (Kuromoji + OpenJTalk):")
            print(f"   {kotlin_phonemes}")
            print()
            
            # 计算相似度
            similarity = calculate_similarity(python_phonemes, kotlin_phonemes)
            match = "✅ 完全一致" if similarity == 100.0 else f"⚠️ 相似度: {similarity:.1f}%"
            print(f"📊 对比结果: {match}")
            
            if similarity < 100.0:
                print(f"\n   📋 差异分析:")
                print(f"   Python: {python_phonemes}")
                print(f"   Kotlin: {kotlin_phonemes}")
        else:
            kotlin_phonemes = "未获取"
            similarity = 0.0
            if auto_mode:
                print(f"❌ 超时未获取到 Kotlin 输出")
            else:
                print(f"🤖 Kotlin (Kuromoji + OpenJTalk):")
                print(f"   {kotlin_phonemes}")
        
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
    filled = sum(1 for r in results if r['kotlin'] != "未获取")
    
    if filled > 0:
        avg_similarity = sum(r['similarity'] for r in results) / total
        perfect_matches = sum(1 for r in results if r['similarity'] == 100.0)
        
        print(f"\n总测试数: {total}")
        print(f"成功获取: {filled}/{total}")
        print(f"平均相似度: {avg_similarity:.1f}%")
        print(f"完全匹配: {perfect_matches}/{filled}")
        
        # 判断准确度是否达标
        if avg_similarity >= 95.0:
            print(f"\n✅ 恭喜！准确度达到 {avg_similarity:.1f}%，已超过 95% 的目标！")
        elif avg_similarity >= 90.0:
            print(f"\n⚠️  准确度 {avg_similarity:.1f}%，接近目标但未达到 95%")
        else:
            print(f"\n❌ 准确度 {avg_similarity:.1f}%，需要进一步优化")
        
        # 显示失败的测试
        failed = [r for r in results if r['similarity'] < 95.0 and r['kotlin'] != "未获取"]
        if failed:
            print(f"\n需要改进的测试:")
            for r in failed:
                print(f"  - 测试 #{r['id']}: 相似度 {r['similarity']:.1f}%")
    else:
        print(f"\n⚠️ 未获取到任何 Kotlin 输出")
        print("\n请检查:")
        print("  1. Android 应用是否已打开")
        print("  2. 是否在提示后输入了对应的句子")
        print("  3. MainActivity.kt 中是否已添加 G2P_TEST 日志输出")
        print("\n建议重新运行测试，选择自动模式")
    
    # 保存结果到 JSON
    with open('g2p_comparison_results.json', 'w', encoding='utf-8') as f:
        json.dump(results, f, ensure_ascii=False, indent=2)
    
    print(f"\n✅ 结果已保存到: g2p_comparison_results.json")
    print()


if __name__ == "__main__":
    main()
