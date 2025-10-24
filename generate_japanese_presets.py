"""
生成日文预设句子的音素数据供 Android 使用
使用 Kokoro 的 G2P 转换
"""

from kokoro import KPipeline

# 初始化
print("初始化 Kokoro...")
pipeline = KPipeline(lang_code='j')

# 预设的日文句子
sentences = [
    ("こんにちは", "你好"),
    ("ありがとう", "谢谢"),
    ("さようなら", "再见"),
    ("おはよう", "早上好"),
    ("こんばんは", "晚上好"),
    ("すみません", "对不起"),
    ("はい", "是"),
    ("いいえ", "不"),
    ("おいしい", "好吃"),
    ("きれい", "漂亮"),
    ("わかりました", "我明白了"),
    ("どういたしまして", "不客气"),
    ("お元気ですか", "你好吗"),
    ("大丈夫です", "没关系"),
    ("頑張って", "加油"),
    # 长句子（用于性能测试）
    ("今日はとても良い天気ですね、公園で散歩しませんか", "今天天气真好呀，一起去公园散步吧"),
]

print("\n正在转换音素...\n")

kotlin_code = """package com.lsl.kokoro_ja_android

/**
 * 预设的日文句子和对应的音素
 * 自动生成 - 由 generate_japanese_presets.py 生成
 * 
 * 临时方案：预先转换好常用句子
 * TODO: 后续集成 MeCab 实现完整的 G2P 转换
 */
object JapanesePresets {
    
    data class JapaneseSentence(
        val text: String,          // 日文文本
        val phonemes: String,      // 音素
        val translation: String    // 中文翻译（用于显示）
    )
    
    val sentences = listOf(
"""

for text, translation in sentences:
    # 使用 pipeline 进行 G2P 转换
    # pipeline 返回的是 generator，我们只需要音素
    generator = pipeline(text, voice='jf_nezumi', speed=1)
    
    # 获取第一个结果
    for graphemes, phonemes, audio in generator:
        print(f"{text:15s} -> {phonemes:20s} ({translation})")
        
        kotlin_code += f"""        JapaneseSentence(
            text = "{text}",
            phonemes = "{phonemes}",
            translation = "{translation}"
        ),
"""
        break  # 只需要第一个

kotlin_code += """    )
    
    /**
     * 根据日文文本查找音素
     */
    fun getPhonemes(text: String): String? {
        return sentences.find { it.text == text }?.phonemes
    }
    
    /**
     * 获取所有日文文本列表（用于 Spinner 显示）
     */
    fun getTextList(): List<String> {
        return sentences.map { "${it.text} (${it.translation})" }
    }
}
"""

# 保存
output_file = "app/src/main/java/com/lsl/kokoro_ja_android/JapanesePresets.kt"
with open(output_file, 'w', encoding='utf-8') as f:
    f.write(kotlin_code)

print(f"\n✅ 已生成: {output_file}")
print(f"✅ 共 {len(sentences)} 个句子")
