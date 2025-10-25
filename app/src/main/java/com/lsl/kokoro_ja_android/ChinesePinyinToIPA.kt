package com.lsl.kokoro_ja_android

/**
 * 中文拼音 → IPA 音素转换器
 * 
 * 基于 misaki/transcription.py 的规则移植到 Kotlin
 * 
 * 功能：
 * - 将带声调的拼音（如 ni3, hao3）转换为 IPA 音标
 * - 支持 21 个声母、40 个韵母、5 个声调
 * - 处理特殊情况：卷舌音、平舌音、儿化音等
 * 
 * 示例：
 *   "ni3" → "ni˧˩˧"  (或简化为 "ni↓")
 *   "hao3" → "xau̯˧˩˧" (或简化为 "xau̯↓")
 * 
 * @author Kokoro-Android-Demo Project
 * @see misaki中文G2P分析报告.md
 */
object ChinesePinyinToIPA {
    
    /**
     * 声母映射表 (21个)
     * 
     * 拼音声母 → IPA 符号
     * 注意：中文的"清浊"与英文不同，b/d/g 是不送气清音
     */
    private val initialMap = mapOf(
        "b" to "p",      // 不送气清塞音 (爸)
        "p" to "pʰ",     // 送气清塞音 (怕)
        "m" to "m",      // 双唇鼻音 (妈)
        "f" to "f",      // 唇齿擦音 (发)
        "d" to "t",      // 不送气清塞音 (大)
        "t" to "tʰ",     // 送气清塞音 (他)
        "n" to "n",      // 齿龈鼻音 (那)
        "l" to "l",      // 齿龈边音 (拉)
        "g" to "k",      // 不送气清塞音 (哥)
        "k" to "kʰ",     // 送气清塞音 (科)
        "h" to "x",      // 软腭擦音 (喝) - 默认用 x
        "j" to "ʨ",      // 腭化塞擦音 (鸡)
        "q" to "ʨʰ",     // 送气腭化塞擦音 (气)
        "x" to "ɕ",      // 腭化擦音 (西)
        "zh" to "ʈʂ",    // 卷舌塞擦音 (知) - Unicode: \u0288\u0282 或直接用字符
        "ch" to "ʈʂʰ",   // 送气卷舌塞擦音 (吃)
        "sh" to "ʂ",     // 卷舌擦音 (是)
        "r" to "ɻ",      // 卷舌近音 (日) - 默认用 ɻ
        "z" to "ʦ",      // 齿塞擦音 (资)
        "c" to "ʦʰ",     // 送气齿塞擦音 (次)
        "s" to "s"       // 齿擦音 (思)
    )
    
    /**
     * 韵母映射表 (40个)
     * 
     * 格式：拼音韵母 → IPA 音素列表
     * {tone} 是占位符，会被实际声调符号替换
     */
    private val finalMap = mapOf(
        // 单韵母
        "a" to listOf("a{tone}"),
        "o" to listOf("w", "o{tone}"),  // 单独的 o 实际读作 wo
        "e" to listOf("ɤ{tone}"),
        "i" to listOf("i{tone}"),
        "u" to listOf("u{tone}"),
        "ü" to listOf("y{tone}"),  // v 也可以表示 ü
        
        // 复韵母 (ai, ei, ao, ou)
        "ai" to listOf("ai̯{tone}"),
        "ei" to listOf("ei̯{tone}"),
        "ao" to listOf("au̯{tone}"),
        "ou" to listOf("ou̯{tone}"),
        
        // 鼻韵母 (an, en, ang, eng, ong)
        "an" to listOf("a{tone}", "n"),
        "en" to listOf("ə{tone}", "n"),
        "ang" to listOf("a{tone}", "ŋ"),
        "eng" to listOf("ə{tone}", "ŋ"),
        "ong" to listOf("ʊ{tone}", "ŋ"),
        
        // i 系韵母
        "ia" to listOf("j", "a{tone}"),
        "ie" to listOf("j", "e{tone}"),
        "iao" to listOf("j", "au̯{tone}"),
        "iou" to listOf("j", "ou̯{tone}"),  // 有时写作 iu
        "ian" to listOf("j", "ɛ{tone}", "n"),
        "in" to listOf("i{tone}", "n"),
        "iang" to listOf("j", "a{tone}", "ŋ"),
        "ing" to listOf("i{tone}", "ŋ"),
        "iong" to listOf("j", "ʊ{tone}", "ŋ"),
        
        // u 系韵母
        "ua" to listOf("w", "a{tone}"),
        "uo" to listOf("w", "o{tone}"),
        "uai" to listOf("w", "ai̯{tone}"),
        "uei" to listOf("w", "ei̯{tone}"),  // 有时写作 ui
        "uan" to listOf("w", "a{tone}", "n"),
        "uen" to listOf("w", "ə{tone}", "n"),  // 有时写作 un
        "uang" to listOf("w", "a{tone}", "ŋ"),
        "ueng" to listOf("w", "ə{tone}", "ŋ"),
        
        // ü 系韵母
        "üe" to listOf("ɥ", "e{tone}"),  // 月 yue
        "üan" to listOf("ɥ", "ɛ{tone}", "n"),  // 冤 yuan
        "ün" to listOf("y{tone}", "n")   // 晕 yun
    )
    
    /**
     * 卷舌音后的特殊韵母 (zh, ch, sh, r + i)
     * 
     * zhi, chi, shi, ri 的 "i" 读作 [ɻ̩] (卷舌元音)
     */
    private val finalAfterZhChShR = mapOf(
        "i" to listOf("ɻ̩{tone}")  // 或用 ʐ̩
    )
    
    /**
     * 平舌音后的特殊韵母 (z, c, s + i)
     * 
     * zi, ci, si 的 "i" 读作 [ɹ̩] (前舌元音)
     */
    private val finalAfterZCS = mapOf(
        "i" to listOf("ɹ̩{tone}")  // 或用 z̩
    )
    
    /**
     * 声调映射表 (5个)
     * 
     * 1-5 对应五个声调，使用 IPA 调值符号
     */
    private val toneMap = mapOf(
        1 to "˥",      // 第一声 (阴平) - 高平调
        2 to "˧˥",     // 第二声 (阳平) - 中升调
        3 to "˧˩˧",    // 第三声 (上声) - 降升调
        4 to "˥˩",     // 第四声 (去声) - 全降调
        5 to ""        // 轻声 - 无标记
    )
    
    /**
     * 简化的声调符号 (可选)
     * 
     * 用箭头代替复杂的 IPA 符号，更直观
     */
    private val toneSimplified = mapOf(
        "˧˩˧" to "↓",   // 第三声 - 优先匹配长的符号
        "˥" to "→",
        "˧˥" to "↗",
        "˥˩" to "↘"
    )
    
    /**
     * 主转换函数：拼音 → IPA
     * 
     * @param pinyin 带声调的拼音，如 "ni3", "hao3", "zhong1"
     * @param simplifyTone 是否使用简化的声调符号（箭头）
     * @return IPA 音素字符串
     * 
     * 示例：
     *   convert("ni3") → "ni˧˩˧" 或 "ni↓"
     *   convert("hao3") → "xau̯˧˩˧" 或 "xau̯↓"
     *   convert("zhi1") → "ʈʂɻ̩˥" 或 "ʈʂɻ̩→"
     */
    fun convert(pinyin: String, simplifyTone: Boolean = true): String {
        if (pinyin.isEmpty()) return ""
        
        // 1. 提取声调 (1-5)
        val tone = extractTone(pinyin)
        
        // 2. 移除声调，得到无声调拼音
        val normalPinyin = removeTone(pinyin)
        
        // 3. 提取声母和韵母
        val initial = extractInitial(normalPinyin)
        val final = extractFinal(normalPinyin, initial)
        
        // 4. 转换为 IPA 音素
        val phonemeParts = mutableListOf<String>()
        
        // 添加声母（如果有）
        if (initial != null) {
            initialMap[initial]?.let { phonemeParts.add(it) }
        }
        
        // 添加韵母（根据声母选择不同的映射表）
        val finalPhonemes = when {
            // 卷舌音后的特殊处理
            initial in setOf("zh", "ch", "sh", "r") && final in finalAfterZhChShR ->
                finalAfterZhChShR[final]
            
            // 平舌音后的特殊处理
            initial in setOf("z", "c", "s") && final in finalAfterZCS ->
                finalAfterZCS[final]
            
            // 常规韵母
            else -> finalMap[final]
        }
        
        if (finalPhonemes != null) {
            phonemeParts.addAll(finalPhonemes)
        } else {
            // 未知韵母，直接保留
            phonemeParts.add(final)
        }
        
        // 5. 应用声调
        var result = applyTone(phonemeParts, tone)
        
        // 6. 可选：简化声调符号
        if (simplifyTone) {
            result = simplifyToneMarks(result)
        }
        
        return result
    }
    
    /**
     * 批量转换：拼音列表 → IPA 字符串
     * 
     * @param pinyins 拼音列表，如 ["ni3", "hao3", "shi4", "jie4"]
     * @param simplifyTone 是否简化声调
     * @return IPA 音素字符串，用空格连接
     * 
     * 示例：
     *   convertBatch(["ni3", "hao3"]) → "ni↓ xau̯↓"
     */
    fun convertBatch(pinyins: List<String>, simplifyTone: Boolean = true): String {
        return pinyins.joinToString(" ") { convert(it, simplifyTone) }
    }
    
    // ========== 辅助函数 ==========
    
    /**
     * 提取声调 (1-5)
     * 
     * 从拼音末尾提取数字声调
     */
    private fun extractTone(pinyin: String): Int {
        val lastChar = pinyin.lastOrNull() ?: return 5
        return if (lastChar.isDigit()) {
            lastChar.digitToInt().coerceIn(1, 5)
        } else {
            5  // 默认轻声
        }
    }
    
    /**
     * 移除声调标记
     * 
     * 去掉拼音末尾的数字
     */
    private fun removeTone(pinyin: String): String {
        return pinyin.replace(Regex("[0-9]$"), "")
    }
    
    /**
     * 提取声母
     * 
     * 优先匹配双字符声母 (zh, ch, sh)
     */
    private fun extractInitial(pinyin: String): String? {
        if (pinyin.isEmpty()) return null
        
        // 1. 优先匹配双字符声母
        if (pinyin.length >= 2) {
            val twoChar = pinyin.substring(0, 2)
            if (twoChar in setOf("zh", "ch", "sh")) {
                return twoChar
            }
        }
        
        // 2. 匹配单字符声母
        val firstChar = pinyin.first().toString()
        return if (firstChar in initialMap.keys) {
            firstChar
        } else {
            null  // 零声母
        }
    }
    
    /**
     * 提取韵母
     * 
     * 从拼音中去掉声母后剩余的部分
     */
    private fun extractFinal(pinyin: String, initial: String?): String {
        return if (initial != null) {
            pinyin.removePrefix(initial)
        } else {
            pinyin
        }
    }
    
    /**
     * 应用声调
     * 
     * 将 {tone} 占位符替换为实际的声调符号
     */
    private fun applyTone(parts: List<String>, tone: Int): String {
        val toneSymbol = toneMap[tone] ?: ""
        return parts.joinToString("") { part ->
            part.replace("{tone}", toneSymbol)
        }
    }
    
    /**
     * 简化声调符号
     * 
     * 将复杂的 IPA 调值符号替换为箭头
     * 注意：必须先替换长的符号（˧˩˧），再替换短的（˥）
     */
    private fun simplifyToneMarks(ipa: String): String {
        var result = ipa
        // 按照符号长度从长到短替换，避免部分替换问题
        toneSimplified.entries
            .sortedByDescending { it.key.length }
            .forEach { (ipaSymbol, arrow) ->
                result = result.replace(ipaSymbol, arrow)
            }
        return result
    }
    
    // ========== 测试与验证 ==========
    
    /**
     * 获取统计信息（用于调试）
     */
    fun getStatistics(): Map<String, Int> {
        return mapOf(
            "initials" to initialMap.size,
            "finals" to finalMap.size,
            "tones" to toneMap.size,
            "special_zh_ch_sh_r" to finalAfterZhChShR.size,
            "special_z_c_s" to finalAfterZCS.size
        )
    }
    
    /**
     * 验证拼音格式是否正确
     */
    fun isValidPinyin(pinyin: String): Boolean {
        if (pinyin.isEmpty()) return false
        
        // 必须以数字 1-5 结尾
        val lastChar = pinyin.lastOrNull() ?: return false
        if (!lastChar.isDigit() || lastChar.digitToInt() !in 1..5) {
            return false
        }
        
        // 移除声调后检查声母韵母
        val normal = removeTone(pinyin)
        val initial = extractInitial(normal)
        val final = extractFinal(normal, initial)
        
        // 韵母必须在映射表中
        return final in finalMap.keys || 
               final in finalAfterZhChShR.keys || 
               final in finalAfterZCS.keys
    }
}
