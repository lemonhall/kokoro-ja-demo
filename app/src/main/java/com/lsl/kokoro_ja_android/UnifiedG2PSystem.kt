package com.lsl.kokoro_ja_android

import android.content.Context

/**
 * 统一的 G2P (Grapheme-to-Phoneme) 系统
 * 
 * 特性：
 * - ✅ 自动语言检测
 * - ✅ 支持中文、日文（韩文预留）
 * - ✅ 无需用户手动切换
 * - ✅ 智能选择对应的 G2P 引擎
 * 
 * 使用示例：
 * ```kotlin
 * val g2p = UnifiedG2PSystem(context)
 * 
 * // 自动检测并转换
 * val phonemes1 = g2p.textToPhonemes("你好世界")  // 自动识别中文
 * val phonemes2 = g2p.textToPhonemes("こんにちは") // 自动识别日文
 * val phonemes3 = g2p.textToPhonemes("안녕하세요")  // 未来支持韩文
 * ```
 * 
 * @author Kokoro-Android-Demo Project
 */
class UnifiedG2PSystem(private val context: Context) {
    
    /**
     * 日文 G2P 系统（延迟初始化）
     */
    private val japaneseG2P: JapaneseG2PSystem by lazy {
        JapaneseG2PSystem(context)
    }
    
    /**
     * 中文 G2P 系统（延迟初始化）
     */
    private val chineseG2P: ChineseG2PSystem by lazy {
        ChineseG2PSystem(context)
    }
    
    /**
     * 韩文 G2P 系统（预留，未实现）
     */
    // private val koreanG2P: KoreanG2PSystem by lazy {
    //     KoreanG2PSystem(context)
    // }
    
    /**
     * 文本 → IPA 音素（自动检测语言）
     * 
     * @param text 输入文本（任意语言）
     * @return IPA 音素字符串
     * 
     * 流程：
     * 1. 自动检测文本语言
     * 2. 选择对应的 G2P 引擎
     * 3. 转换为音素
     */
    fun textToPhonemes(text: String): String {
        if (text.isEmpty()) return ""
        
        // 1. 检测语言
        val language = LanguageDetector.detect(text)
        
        // 2. 根据语言选择 G2P 系统
        return when (language) {
            LanguageDetector.Language.CHINESE -> {
                chineseG2P.textToPhonemes(text)
            }
            LanguageDetector.Language.JAPANESE -> {
                japaneseG2P.textToPhonemes(text)
            }
            LanguageDetector.Language.KOREAN -> {
                // 未来支持韩文
                // koreanG2P.textToPhonemes(text)
                throw UnsupportedOperationException("韩文暂未支持，敬请期待！")
            }
            else -> {
                // 未知语言，尝试日文（兼容旧版本）
                japaneseG2P.textToPhonemes(text)
            }
        }
    }
    
    /**
     * 获取推荐的语音嵌入名称
     * 
     * 根据检测到的语言，返回对应的默认音色
     * 
     * @param text 输入文本
     * @return 语音嵌入名称
     */
    fun getRecommendedVoice(text: String): String {
        val language = LanguageDetector.detect(text)
        
        return when (language) {
            LanguageDetector.Language.CHINESE -> "zf_xiaoxiao"  // 中文女声
            LanguageDetector.Language.JAPANESE -> "jf_nezumi"   // 日文女声
            LanguageDetector.Language.KOREAN -> "kr_default"    // 韩文（预留）
            else -> "jf_nezumi"  // 默认日文
        }
    }
    
    /**
     * 获取检测到的语言信息（用于 UI 显示）
     * 
     * @param text 输入文本
     * @return 语言信息
     */
    fun getDetectedLanguageInfo(text: String): LanguageInfo {
        val language = LanguageDetector.detect(text)
        val voice = getRecommendedVoice(text)
        val distribution = LanguageDetector.getLanguageDistribution(text)
        
        return LanguageInfo(
            language = language,
            languageCode = language.code,
            languageName = language.displayName,
            recommendedVoice = voice,
            distribution = distribution
        )
    }
    
    /**
     * 获取分词/转换详情（用于调试）
     * 
     * @param text 输入文本
     * @return 转换详情字符串
     */
    fun getConversionDetails(text: String): String {
        val language = LanguageDetector.detect(text)
        
        return when (language) {
            LanguageDetector.Language.CHINESE -> {
                val details = chineseG2P.getConversionDetails(text)
                buildString {
                    appendLine("语言: 中文")
                    appendLine("字符详情:")
                    details.forEach { info ->
                        appendLine("  ${info.char} → ${info.pinyin} → ${info.pinyinWithTone} → ${info.ipa}")
                    }
                }
            }
            LanguageDetector.Language.JAPANESE -> {
                val details = japaneseG2P.getTokenizationDetails(text)
                buildString {
                    appendLine("语言: 日文")
                    appendLine("分词详情:")
                    details.forEach { info ->
                        appendLine("  ${info.surface} [${info.actualReading}] (${info.pos})")
                    }
                }
            }
            else -> {
                "未知语言或不支持"
            }
        }
    }
    
    /**
     * 语言检测信息
     */
    data class LanguageInfo(
        val language: LanguageDetector.Language,
        val languageCode: String,
        val languageName: String,
        val recommendedVoice: String,
        val distribution: Map<LanguageDetector.Language, Double>
    )
}
