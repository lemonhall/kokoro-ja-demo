"""
导出 Kokoro 词汇表供 Android 使用
"""

from kokoro import KPipeline
import json

# 初始化
pipeline = KPipeline(lang_code='j')
vocab = pipeline.model.vocab

print("正在导出词汇表...")
print(f"总共 {len(vocab)} 个字符")

# 生成 Kotlin 代码
kotlin_code = """// 自动生成的词汇表
// 由 export_vocab_for_android.py 生成

package com.lsl.kokoro_ja_android

object KokoroVocabFull {
    val vocab = mapOf(
"""

for char, idx in sorted(vocab.items(), key=lambda x: x[1]):
    # 转义特殊字符
    if char == '\\':
        char_repr = "'\\\\'"
    elif char == '\'':
        char_repr = "'\\''"
    elif char == '\n':
        char_repr = "'\\n'"
    elif char == '\r':
        char_repr = "'\\r'"
    elif char == '\t':
        char_repr = "'\\t'"
    else:
        char_repr = f"'{char}'"
    
    kotlin_code += f"        {char_repr} to {idx},\n"

kotlin_code = kotlin_code.rstrip(',\n') + "\n    )\n"
kotlin_code += """
    /**
     * 将音素字符串转换为 input_ids
     */
    fun phonemesToIds(phonemes: String): LongArray {
        val ids = mutableListOf<Long>(0) // BOS
        for (char in phonemes) {
            vocab[char]?.let { ids.add(it.toLong()) }
        }
        ids.add(0) // EOS
        return ids.toLongArray()
    }
}
"""

# 保存
output_file = "app/src/main/java/com/lsl/kokoro_ja_android/KokoroVocabFull.kt"
with open(output_file, 'w', encoding='utf-8') as f:
    f.write(kotlin_code)

print(f"✅ 词汇表已导出到: {output_file}")
print(f"\n示例使用:")
print(f"  val inputIds = KokoroVocabFull.phonemesToIds(\"koɲɲiʨiβa\")")

# 同时生成 JSON 格式
json_output = "models/vocab.json"
import os
os.makedirs("models", exist_ok=True)
with open(json_output, 'w', encoding='utf-8') as f:
    json.dump(vocab, f, ensure_ascii=False, indent=2)

print(f"✅ JSON 格式已导出到: {json_output}")
