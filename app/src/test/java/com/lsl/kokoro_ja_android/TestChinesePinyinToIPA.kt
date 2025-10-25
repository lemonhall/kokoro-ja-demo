package com.lsl.kokoro_ja_android

/**
 * 简单的测试程序，用于验证 ChinesePinyinToIPA
 */
fun main() {
    println("=".repeat(60))
    println("ChinesePinyinToIPA 测试")
    println("=".repeat(60))
    
    // 测试用例
    val testCases = listOf(
        "ni3" to "你",
        "hao3" to "好",
        "zhi1" to "知",
        "shi2" to "识",
        "ma1" to "妈",
        "ma2" to "麻",
        "ma3" to "马",
        "ma4" to "骂",
        "ma5" to "吗",
        "zhong1" to "中",
        "wen2" to "文",
        "zi1" to "资",
        "ci4" to "次",
        "si1" to "思"
    )
    
    println("\n基础测试:")
    testCases.forEach { (pinyin, char) ->
        val ipa = ChinesePinyinToIPA.convert(pinyin)
        println("  $char ($pinyin) → $ipa")
    }
    
    println("\n批量转换测试:")
    val sentence1 = listOf("ni3", "hao3")
    val result1 = ChinesePinyinToIPA.convertBatch(sentence1)
    println("  你好: $result1")
    
    val sentence2 = listOf("zhong1", "wen2", "ce4", "shi4")
    val result2 = ChinesePinyinToIPA.convertBatch(sentence2)
    println("  中文测试: $result2")
    
    println("\n统计信息:")
    val stats = ChinesePinyinToIPA.getStatistics()
    stats.forEach { (key, value) ->
        println("  $key: $value")
    }
    
    println("\n验证测试:")
    println("  ni3 有效? ${ChinesePinyinToIPA.isValidPinyin("ni3")}")
    println("  ni 有效? ${ChinesePinyinToIPA.isValidPinyin("ni")}")
    println("  xyz1 有效? ${ChinesePinyinToIPA.isValidPinyin("xyz1")}")
    
    println("\n完成!")
}
