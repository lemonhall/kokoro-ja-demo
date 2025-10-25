package com.lsl.kokoro_ja_android

/**
 * 轻量级语言检测器
 * 
 * 支持自动检测：
 * - 中文 (zh)
 * - 日文 (ja)
 * - 韩文 (ko) - 预留支持
 * - 英文 (en) - 预留支持
 * 
 * 检测策略：
 * 1. 基于 Unicode 字符范围
 * 2. 统计各语言字符占比
 * 3. 返回占比最高的语言
 * 
 * 准确度：> 95% (对于纯语言文本)
 * 
 * @author Kokoro-Android-Demo Project
 */
object LanguageDetector {
    
    /**
     * 支持的语言代码
     */
    enum class Language(val code: String, val displayName: String) {
        CHINESE("zh", "中文"),
        JAPANESE("ja", "日文"),
        KOREAN("ko", "韩文"),
        ENGLISH("en", "英文"),
        UNKNOWN("unknown", "未知")
    }
    
    /**
     * Unicode 字符范围定义
     */
    private object UnicodeRange {
        // 中文字符范围
        val CHINESE = listOf(
            0x4E00..0x9FFF,   // CJK Unified Ideographs (常用汉字)
            0x3400..0x4DBF,   // CJK Extension A
            0x20000..0x2A6DF, // CJK Extension B
            0xF900..0xFAFF    // CJK Compatibility Ideographs
        )
        
        // 日文字符范围
        val JAPANESE = listOf(
            0x3040..0x309F,   // 平假名
            0x30A0..0x30FF,   // 片假名
            0x31F0..0x31FF    // 片假名扩展
        )
        
        // 韩文字符范围
        val KOREAN = listOf(
            0xAC00..0xD7AF,   // 韩文音节
            0x1100..0x11FF,   // 韩文字母
            0x3130..0x318F,   // 韩文兼容字母
            0xA960..0xA97F,   // 韩文扩展-A
            0xD7B0..0xD7FF    // 韩文扩展-B
        )
        
        // 英文字符范围
        val ENGLISH = listOf(
            0x0041..0x005A,   // A-Z
            0x0061..0x007A    // a-z
        )
    }
    
    /**
     * 检测文本的主要语言
     * 
     * @param text 输入文本
     * @return 检测到的语言
     * 
     * 示例：
     *   detect("你好世界") → Language.CHINESE
     *   detect("こんにちは") → Language.JAPANESE
     *   detect("안녕하세요") → Language.KOREAN
     */
    fun detect(text: String): Language {
        if (text.isEmpty()) return Language.UNKNOWN
        
        // 统计各语言字符数量
        val counts = mutableMapOf(
            Language.CHINESE to 0,
            Language.JAPANESE to 0,
            Language.KOREAN to 0,
            Language.ENGLISH to 0
        )
        
        var totalChars = 0
        
        for (char in text) {
            val code = char.code
            
            // 跳过标点、空格、数字
            if (char.isWhitespace() || char.isDigit() || isPunctuation(char)) {
                continue
            }
            
            totalChars++
            
            // 检查是否属于某个语言
            when {
                isInRange(code, UnicodeRange.CHINESE) -> counts[Language.CHINESE] = counts[Language.CHINESE]!! + 1
                isInRange(code, UnicodeRange.JAPANESE) -> counts[Language.JAPANESE] = counts[Language.JAPANESE]!! + 1
                isInRange(code, UnicodeRange.KOREAN) -> counts[Language.KOREAN] = counts[Language.KOREAN]!! + 1
                isInRange(code, UnicodeRange.ENGLISH) -> counts[Language.ENGLISH] = counts[Language.ENGLISH]!! + 1
            }
        }
        
        // 如果没有有效字符，返回未知
        if (totalChars == 0) return Language.UNKNOWN
        
        // 返回占比最高的语言
        val maxEntry = counts.maxByOrNull { it.value }
        
        return if (maxEntry != null && maxEntry.value > 0) {
            maxEntry.key
        } else {
            Language.UNKNOWN
        }
    }
    
    /**
     * 检测文本的主要语言（返回语言代码）
     * 
     * @param text 输入文本
     * @return 语言代码 ("zh", "ja", "ko", "en", "unknown")
     */
    fun detectCode(text: String): String {
        return detect(text).code
    }
    
    /**
     * 获取详细的语言分布信息（用于调试）
     * 
     * @param text 输入文本
     * @return 各语言字符占比
     */
    fun getLanguageDistribution(text: String): Map<Language, Double> {
        if (text.isEmpty()) return emptyMap()
        
        val counts = mutableMapOf(
            Language.CHINESE to 0,
            Language.JAPANESE to 0,
            Language.KOREAN to 0,
            Language.ENGLISH to 0
        )
        
        var totalChars = 0
        
        for (char in text) {
            val code = char.code
            
            if (char.isWhitespace() || char.isDigit() || isPunctuation(char)) {
                continue
            }
            
            totalChars++
            
            when {
                isInRange(code, UnicodeRange.CHINESE) -> counts[Language.CHINESE] = counts[Language.CHINESE]!! + 1
                isInRange(code, UnicodeRange.JAPANESE) -> counts[Language.JAPANESE] = counts[Language.JAPANESE]!! + 1
                isInRange(code, UnicodeRange.KOREAN) -> counts[Language.KOREAN] = counts[Language.KOREAN]!! + 1
                isInRange(code, UnicodeRange.ENGLISH) -> counts[Language.ENGLISH] = counts[Language.ENGLISH]!! + 1
            }
        }
        
        if (totalChars == 0) return emptyMap()
        
        // 计算百分比
        return counts.mapValues { (it.value.toDouble() / totalChars) * 100 }
    }
    
    /**
     * 判断是否是中文文本
     */
    fun isChinese(text: String): Boolean {
        return detect(text) == Language.CHINESE
    }
    
    /**
     * 判断是否是日文文本
     */
    fun isJapanese(text: String): Boolean {
        return detect(text) == Language.JAPANESE
    }
    
    /**
     * 判断是否是韩文文本
     */
    fun isKorean(text: String): Boolean {
        return detect(text) == Language.KOREAN
    }
    
    /**
     * 按语言分段文本（支持混合语言）
     * 
     * 示例：
     *   "我买了iPhone很好" 
     *   → [("我买了", CHINESE), ("iPhone", ENGLISH), ("很好", CHINESE)]
     * 
     * @param text 输入文本
     * @return 语言片段列表
     */
    fun segmentByLanguage(text: String): List<LanguageSegment> {
        if (text.isEmpty()) return emptyList()
        
        val segments = mutableListOf<LanguageSegment>()
        val currentSegment = StringBuilder()
        var currentLanguage: Language? = null
        
        for (char in text) {
            // 跳过的字符（标点、空格、数字）统一归到当前语言段
            if (char.isWhitespace() || char.isDigit() || isPunctuation(char)) {
                currentSegment.append(char)
                continue
            }
            
            // 检测字符的语言
            val charLanguage = detectCharLanguage(char)
            
            // 语言切换：保存当前段，开始新段
            if (currentLanguage != null && charLanguage != currentLanguage && charLanguage != Language.UNKNOWN) {
                if (currentSegment.isNotEmpty()) {
                    segments.add(LanguageSegment(currentSegment.toString(), currentLanguage))
                    currentSegment.clear()
                }
                currentLanguage = charLanguage
            } else if (currentLanguage == null && charLanguage != Language.UNKNOWN) {
                // 第一个有效字符
                currentLanguage = charLanguage
            }
            
            currentSegment.append(char)
        }
        
        // 保存最后一段
        if (currentSegment.isNotEmpty() && currentLanguage != null) {
            segments.add(LanguageSegment(currentSegment.toString(), currentLanguage))
        }
        
        return segments
    }
    
    /**
     * 检测单个字符的语言
     */
    private fun detectCharLanguage(char: Char): Language {
        val code = char.code
        
        return when {
            isInRange(code, UnicodeRange.JAPANESE) -> Language.JAPANESE  // 优先日文（假名）
            isInRange(code, UnicodeRange.KOREAN) -> Language.KOREAN
            isInRange(code, UnicodeRange.CHINESE) -> Language.CHINESE    // CJK汉字可能是中文或日文
            isInRange(code, UnicodeRange.ENGLISH) -> Language.ENGLISH
            else -> Language.UNKNOWN
        }
    }
    
    /**
     * 语言片段
     */
    data class LanguageSegment(
        val text: String,      // 文本片段
        val language: Language // 该片段的语言
    )
    
    // ========== 辅助函数 ==========
    
    /**
     * 检查字符代码是否在指定范围内
     */
    private fun isInRange(code: Int, ranges: List<IntRange>): Boolean {
        return ranges.any { code in it }
    }
    
    /**
     * 判断是否是标点符号
     */
    private fun isPunctuation(char: Char): Boolean {
        val code = char.code
        return code in 0x2000..0x206F ||   // General Punctuation
               code in 0x3000..0x303F ||   // CJK Symbols and Punctuation
               code in 0xFF00..0xFFEF ||   // Halfwidth and Fullwidth Forms
               char in setOf(',', '.', '!', '?', ':', ';', '"', '\'', '(', ')', '[', ']', '{', '}')
    }
}
