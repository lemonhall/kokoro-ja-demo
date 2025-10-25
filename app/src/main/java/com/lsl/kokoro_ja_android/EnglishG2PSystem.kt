package com.lsl.kokoro_ja_android

import android.content.Context
import org.json.JSONObject

/**
 * 英文 G2P (Grapheme-to-Phoneme) 系统
 * 
 * 特性：
 * - ✅ 纯 Kotlin 实现，无外部依赖
 * - ✅ 加载 CMUdict 词典（常用英文单词）
 * - ✅ 基于词典的精确发音
 * - ✅ 未知单词回退到字母拼读
 * 
 * 流程：
 * 单词 → 词典查询 → IPA 音素
 * 
 * 示例：
 * "iPhone" → "aɪ f oʊ n"
 * "hello" → "h ʌ l oʊ"
 * 
 * @author Kokoro-Android-Demo Project
 */
class EnglishG2PSystem(private val context: Context) {
    
    /**
     * 发音字典（懒加载）
     */
    private val pronunciationDict: Map<String, String> by lazy {
        loadPronunciationDict()
    }
    
    /**
     * 从 assets 加载发音字典
     */
    private fun loadPronunciationDict(): Map<String, String> {
        val json = context.assets.open("english_dict.json").bufferedReader().use { it.readText() }
        val jsonObj = JSONObject(json)
        
        val dict = mutableMapOf<String, String>()
        
        jsonObj.keys().forEach { key ->
            dict[key] = jsonObj.getString(key)
        }
        
        println("📚 加载英文发音字典: ${dict.size} 个单词")
        
        return dict
    }
    
    /**
     * 文本 → IPA 音素
     * 
     * @param text 输入文本（英文）
     * @return IPA 音素字符串
     */
    fun textToPhonemes(text: String): String {
        if (text.isEmpty()) return ""
        
        // 分词：按空格和标点分割
        val words = text.lowercase()
            .split(Regex("[\\s,.!?;:]+"))
            .filter { it.isNotEmpty() }
        
        val phonemesList = mutableListOf<String>()
        
        for (word in words) {
            // 查询词典
            val phonemes = pronunciationDict[word] ?: fallbackPronunciation(word)
            
            if (phonemes.isNotEmpty()) {
                phonemesList.add(phonemes)
            }
        }
        
        return phonemesList.joinToString(" ")
    }
    
    /**
     * 未知单词的回退策略：逐字母拼读
     * 
     * 例如："xyz" → "ɛks waɪ zi"
     */
    private fun fallbackPronunciation(word: String): String {
        val letterPhonemes = word.map { char ->
            pronunciationDict[char.toString()] ?: char.toString()
        }
        
        return letterPhonemes.joinToString(" ")
    }
    
    /**
     * 判断是否是英文字符
     */
    fun isEnglish(char: Char): Boolean {
        return char in 'a'..'z' || char in 'A'..'Z'
    }
}
