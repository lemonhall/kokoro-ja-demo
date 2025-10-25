package com.lsl.kokoro_ja_android

/**
 * 英文到中文拼音的转换器
 * 
 * 核心策略：
 * 1. 常用词使用音译词典（如 iPhone → ai feng）
 * 2. 未知词使用字母拼读（如 xyz → ai ke si wai zei）
 * 
 * ⚠️ 警告：这是一个妥协方案！
 * - 发音会很奇怪，但至少有声音
 * - 适合在中文音色下"勉强"读英文单词
 * 
 * @author Kokoro-Android-Demo Project
 */
object EnglishToChinesePinyin {
    
    /**
     * 常用英文单词音译表
     * 
     * 来源：
     * - 常见品牌名
     * - 科技术语
     * - 日常用语
     */
    private val translationMap = mapOf(
        // 科技品牌
        "apple" to "ai po er",
        "iphone" to "ai feng",
        "ipad" to "ai pai de",
        "mac" to "mai ke",
        "macbook" to "mai ke bu ke",
        "android" to "an zhuo",
        "google" to "gu ge",
        "microsoft" to "wei ruan",
        "windows" to "chuang kou",
        
        // 常用科技词
        "computer" to "dian nao",
        "smartphone" to "zhi neng shou ji",
        "tablet" to "ping ban",
        "laptop" to "bi ji ben",
        "desktop" to "tai shi ji",
        "internet" to "yin te wang",
        "wifi" to "wai fai",
        "bluetooth" to "lan ya",
        "email" to "yi mei er",
        "app" to "a pu",
        "software" to "ruan jian",
        "hardware" to "ying jian",
        
        // 常用词
        "hello" to "ha lou",
        "world" to "shi jie",
        "yes" to "ye si",
        "no" to "nou",
        "ok" to "ou kei",
        "okay" to "ou kei",
        "good" to "gu de",
        "bad" to "bai de",
        "test" to "ce shi",
        "demo" to "yan shi",
        "example" to "li zi",
        "presentation" to "pu li sen tei shen",
        
        // 数字（英文读法）
        "one" to "wan",
        "two" to "tu",
        "three" to "si rui",
        "four" to "fo",
        "five" to "fai fu",
        "six" to "si ke si",
        "seven" to "sai wen",
        "eight" to "ei te",
        "nine" to "nai en",
        "ten" to "ten",
    )
    
    /**
     * 字母到中文拼音的映射（英文字母发音）
     * 
     * 例如：A → ei, B → bi, C → xi
     */
    private val letterMap = mapOf(
        'a' to "ei",
        'b' to "bi",
        'c' to "xi",
        'd' to "di",
        'e' to "yi",
        'f' to "ai fu",
        'g' to "ji",
        'h' to "ei qi",
        'i' to "ai",
        'j' to "jie",
        'k' to "kai",
        'l' to "ai er",
        'm' to "ai mu",
        'n' to "en",
        'o' to "ou",
        'p' to "pi",
        'q' to "kiu",
        'r' to "a er",
        's' to "ai si",
        't' to "ti",
        'u' to "you",
        'v' to "wei",
        'w' to "da bu liu",
        'x' to "ai ke si",
        'y' to "wai",
        'z' to "zei"
    )
    
    /**
     * 转换英文单词为中文拼音
     * 
     * @param word 英文单词
     * @return 中文拼音（用空格分隔）
     */
    fun convert(word: String): String {
        if (word.isEmpty()) return ""
        
        val lower = word.lowercase().trim()
        
        // 1. 先查常用词音译表
        translationMap[lower]?.let { return it }
        
        // 2. 处理常见缩写（全大写）
        if (word.all { it.isUpperCase() || !it.isLetter() }) {
            // 如 "CPU" → "xi pi you"
            return word.filter { it.isLetter() }
                .map { letterMap[it.lowercaseChar()] ?: it.toString() }
                .joinToString(" ")
        }
        
        // 3. 逐字母拼读
        return lower.filter { it.isLetter() }
            .mapNotNull { letterMap[it] }
            .joinToString(" ")
    }
    
    /**
     * 批量转换（用于处理多个单词）
     * 
     * @param text 英文文本（可能包含空格）
     * @return 中文拼音
     */
    fun convertText(text: String): String {
        if (text.isEmpty()) return ""
        
        // 按空格分割单词
        val words = text.split(Regex("\\s+")).filter { it.isNotEmpty() }
        
        return words.joinToString(" ") { convert(it) }
    }
    
    /**
     * 获取音译信息（用于调试）
     */
    fun getTranslationInfo(word: String): String {
        val lower = word.lowercase().trim()
        
        return when {
            translationMap.containsKey(lower) -> {
                "音译: ${translationMap[lower]} (词典)"
            }
            else -> {
                "字母拼读: ${convert(word)}"
            }
        }
    }
}
