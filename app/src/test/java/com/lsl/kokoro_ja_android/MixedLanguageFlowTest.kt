package com.lsl.kokoro_ja_android

import org.junit.Test
import org.junit.Assert.*

/**
 * 混合语言处理流程测试
 * 
 * 测试从输入文本到音素输出的完整流程
 */
class MixedLanguageFlowTest {
    
    @Test
    fun testSegmentation_ChineseEnglishChinese() {
        val text = "我蛮喜欢apple手机的"
        
        println("=".repeat(60))
        println("测试文本: $text")
        println("=".repeat(60))
        
        // 1. 分段
        val segments = LanguageDetector.segmentByLanguage(text)
        
        println("\n【步骤1：语言分段】")
        segments.forEachIndexed { index, segment ->
            println("  段${index + 1}: \"${segment.text}\" → ${segment.language.displayName}")
        }
        
        // 验证分段结果
        println("\n【验证】期待分成 3 段:")
        println("  段1: 中文 '我蛮喜欢'")
        println("  段2: 英文 'apple'")
        println("  段3: 中文 '手机的'")
        
        // 实际验证
        assertTrue("应该分成3段", segments.size == 3)
        
        assertEquals("第1段应该是中文", LanguageDetector.Language.CHINESE, segments[0].language)
        assertEquals("第1段文本", "我蛮喜欢", segments[0].text)
        
        assertEquals("第2段应该是英文", LanguageDetector.Language.ENGLISH, segments[1].language)
        assertEquals("第2段文本", "apple", segments[1].text)
        
        assertEquals("第3段应该是中文", LanguageDetector.Language.CHINESE, segments[2].language)
        assertEquals("第3段文本", "手机的", segments[2].text)
    }
    
    @Test
    fun testCharacterLanguageDetection() {
        println("\n" + "=".repeat(60))
        println("测试单个字符语言检测")
        println("=".repeat(60))
        
        val testCases = listOf(
            '我' to "中文",
            '蛮' to "中文",
            '喜' to "中文",
            '欢' to "中文",
            'a' to "英文",
            'p' to "英文",
            'p' to "英文",
            'l' to "英文",
            'e' to "英文",
            '手' to "中文",
            '机' to "中文",
            '的' to "中文"
        )
        
        println("\n逐字检测:")
        for ((char, expected) in testCases) {
            val code = char.code
            val isChinese = code in 0x4E00..0x9FFF
            val isEnglish = (code in 0x0041..0x005A) || (code in 0x0061..0x007A)
            
            val detected = when {
                isChinese -> "中文"
                isEnglish -> "英文"
                else -> "其他"
            }
            
            println("  '$char' (U+${code.toString(16).uppercase()}) → $detected")
            assertEquals("字符 '$char' 应该是 $expected", expected, detected)
        }
    }
    
    @Test
    fun testDetailedSegmentation() {
        val text = "我蛮喜欢apple手机的"
        
        println("\n" + "=".repeat(60))
        println("详细分段过程模拟")
        println("=".repeat(60))
        
        println("\n输入: \"$text\"")
        println("\n逐字分析:")
        
        var currentLanguage: String? = null
        val currentSegment = StringBuilder()
        val segments = mutableListOf<Pair<String, String>>()
        
        for (char in text) {
            val code = char.code
            val charLang = when {
                code in 0x4E00..0x9FFF -> "中文"
                (code in 0x0041..0x005A) || (code in 0x0061..0x007A) -> "英文"
                else -> "其他"
            }
            
            println("  处理: '$char' → $charLang (当前段语言: ${currentLanguage ?: "null"})")
            
            // 语言切换
            if (currentLanguage != null && charLang != currentLanguage && charLang != "其他") {
                if (currentSegment.isNotEmpty()) {
                    segments.add(currentSegment.toString() to currentLanguage)
                    println("    → 保存段: \"$currentSegment\" ($currentLanguage)")
                    currentSegment.clear()
                }
                currentLanguage = charLang
            } else if (currentLanguage == null && charLang != "其他") {
                currentLanguage = charLang
            }
            
            currentSegment.append(char)
        }
        
        // 保存最后一段
        if (currentSegment.isNotEmpty() && currentLanguage != null) {
            segments.add(currentSegment.toString() to currentLanguage)
            println("    → 保存段: \"$currentSegment\" ($currentLanguage)")
        }
        
        println("\n【分段结果】")
        segments.forEachIndexed { index, (text, lang) ->
            println("  段${index + 1}: \"$text\" → $lang")
        }
        
        assertEquals("应该分成3段", 3, segments.size)
    }
}
