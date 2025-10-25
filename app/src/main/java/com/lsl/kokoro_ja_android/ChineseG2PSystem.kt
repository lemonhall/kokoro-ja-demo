package com.lsl.kokoro_ja_android

import android.content.Context
import org.json.JSONObject

/**
 * 中文 G2P (Grapheme-to-Phoneme) 系统
 * 
 * 特性：
 * - ✅ 纯 Kotlin 实现，无外部依赖
 * - ✅ 加载 pypinyin 导出的 20902 个汉字拼音字典
 * - ✅ 结合 ChinesePinyinToIPA 实现完整 G2P
 * 
 * 流程：
 * 汉字 → 拼音查询 → ChinesePinyinToIPA → IPA 音素
 * 
 * 示例：
 * "你好世界" → "ni3 hao3 shi4 jie4" → "n i↑ x au↑ ʂ ʃ̥↘ tɕ iɛ↘"
 * 
 * @author Kokoro-Android-Demo Project
 */
class ChineseG2PSystem(private val context: Context) {
    
    /**
     * 拼音字典（懒加载）
     */
    private val pinyinDict: Map<Char, String> by lazy {
        loadPinyinDict()
    }
    
    /**
     * 从 assets 加载拼音字典
     */
    private fun loadPinyinDict(): Map<Char, String> {
        val json = context.assets.open("pinyin_dict.json").bufferedReader().use { it.readText() }
        val jsonObj = JSONObject(json)
        
        val dict = mutableMapOf<Char, String>()
        
        jsonObj.keys().forEach { key ->
            if (key.length == 1) {
                dict[key[0]] = jsonObj.getString(key)
            }
        }
        
        println("📚 加载拼音字典: ${dict.size} 个汉字")
        
        return dict
    }
    
    /**
     * 文本 → IPA 音素
     * 
     * 流程：
     * 1. 预处理标点（全角→半角）
     * 2. 逐字查询拼音字典
     * 3. 拼音 → IPA 转换
     * 
     * @param text 输入文本（中文）
     * @return IPA 音素字符串
     */
    fun textToPhonemes(text: String): String {
        if (text.isEmpty()) return ""
        
        // 1. 预处理：标点符号映射
        val processedText = mapPunctuation(text)
        
        // 2. 逐字转拼音
        val pinyinList = mutableListOf<String>()
        
        for (char in processedText) {
            when {
                isChinese(char) -> {
                    // 查询字典获取带声调的拼音
                    val pinyinWithTone = pinyinDict[char] ?: "unknown"
                    pinyinList.add(pinyinWithTone)
                }
                // 空格和标点保留
                char.isWhitespace() || isPunctuation(char) -> {
                    pinyinList.add(char.toString())
                }
                // 英文和数字直接保留
                else -> {
                    pinyinList.add(char.toString())
                }
            }
        }
        
        // 3. 拼音 → IPA
        val ipaList = mutableListOf<String>()
        
        for (item in pinyinList) {
            when {
                // 拼音转IPA
                item.matches(Regex("[a-z]+[1-5]")) -> {
                    val ipa = ChinesePinyinToIPA.convert(item, simplifyTone = true)
                    ipaList.add(ipa)
                }
                // 标点和其他字符保留
                else -> {
                    ipaList.add(item)
                }
            }
        }
        
        // 4. 用空格连接（Kokoro 格式）
        return ipaList.joinToString("")
    }
    
    /**
     * 判断字符是否是中文汉字
     */
    private fun isChinese(char: Char): Boolean {
        val code = char.code
        return code in 0x4E00..0x9FFF ||   // CJK Unified Ideographs
               code in 0x3400..0x4DBF ||   // CJK Extension A
               code in 0x20000..0x2A6DF || // CJK Extension B
               code in 0xF900..0xFAFF      // CJK Compatibility
    }
    
    /**
     * 标点符号映射
     * 
     * 将中文标点映射为英文标点
     */
    private fun mapPunctuation(text: String): String {
        return text
            .replace('、', ',')
            .replace('，', ',')
            .replace('。', '.')
            .replace('．', '.')
            .replace('！', '!')
            .replace('：', ':')
            .replace('；', ';')
            .replace('？', '?')
            .replace('《', '"')
            .replace('》', '"')
            .replace('「', '"')
            .replace('」', '"')
            .replace('【', '"')
            .replace('】', '"')
            .replace('（', '(')
            .replace('）', ')')
    }
    
    /**
     * 判断字符是否是标点符号
     */
    private fun isPunctuation(char: Char): Boolean {
        val punctuations = setOf(
            ',', '.', '!', '?', ':', ';', '"', '\'', '(', ')', '[', ']', '{', '}',
            '、', '，', '。', '！', '？', '：', '；', 
            '“', '”', '‘', '’', '（', '）'
        )
        return char in punctuations
    }
    
    /**
     * 获取转换详情（用于调试）
     */
    fun getConversionDetails(text: String): List<CharInfo> {
        val result = mutableListOf<CharInfo>()
        
        for (char in text) {
            when {
                isChinese(char) -> {
                    val pinyinWithTone = pinyinDict[char] ?: "unknown"
                    // 提取无声调的拼音
                    val pinyin = pinyinWithTone.replace(Regex("[1-5]$"), "")
                    val ipa = if (pinyinWithTone != "unknown") {
                        ChinesePinyinToIPA.convert(pinyinWithTone, simplifyTone = true)
                    } else {
                        "?"
                    }
                    result.add(CharInfo(char, pinyin, pinyinWithTone, ipa))
                }
                else -> {
                    result.add(CharInfo(char, char.toString(), char.toString(), char.toString()))
                }
            }
        }
        
        return result
    }
    
    /**
     * 字符转换详情（用于调试）
     */
    data class CharInfo(
        val char: Char,              // 原始字符
        val pinyin: String,          // 拼音（无声调）
        val pinyinWithTone: String,  // 拼音（带声调）
        val ipa: String              // IPA 音素
    )
}
