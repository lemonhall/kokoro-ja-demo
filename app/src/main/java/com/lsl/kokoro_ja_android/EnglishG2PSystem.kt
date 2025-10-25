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
     * 策略：
     * 1. 先查 CMUdict 词典（126k 单词）
     * 2. 未知单词：转换为中文拼音（音译方案）
     * 3. 中文拼音再转 IPA
     * 
     * ⚠️ 注意：音译方案会产生奇怪的发音！
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
            // 1. 查询 CMUdict 词典
            val dictPhonemes = pronunciationDict[word]
            
            if (dictPhonemes != null) {
                // 词典命中：直接使用 IPA 音素
                phonemesList.add(dictPhonemes)
            } else {
                // 2. 词典未命中：使用音译方案
                val pinyin = EnglishToChinesePinyin.convert(word)
                
                if (pinyin.isNotEmpty()) {
                    // 将拼音转换为 IPA（复用 ChineseG2PSystem 的逻辑）
                    val ipaPhonemes = convertPinyinToIPA(pinyin)
                    phonemesList.add(ipaPhonemes)
                    
                    println("⚠️ 英文音译: $word → $pinyin → $ipaPhonemes")
                }
            }
        }
        
        return phonemesList.joinToString(" ")
    }
    
    /**
     * 将拼音转换为 IPA（简化版，复用中文 G2P 的逻辑）
     * 
     * 输入："ai feng" (拼音，空格分隔)
     * 输出："ai fəŋ" (IPA)
     */
    private fun convertPinyinToIPA(pinyin: String): String {
        // 简化处理：将拼音转为 TONE3 格式（加声调5 = 轻声）
        val pinyinWithTone = pinyin.split(" ")
            .map { it + "5" }  // 都用轻声
            .joinToString(" ")
        
        // 使用 ChinesePinyinToIPA 转换
        val ipaList = pinyinWithTone.split(" ")
            .map { ChinesePinyinToIPA.convert(it, simplifyTone = true) }
        
        return ipaList.joinToString(" ")
    }
    
    /**
     * 判断是否是英文字符
     */
    fun isEnglish(char: Char): Boolean {
        return char in 'a'..'z' || char in 'A'..'Z'
    }
}
