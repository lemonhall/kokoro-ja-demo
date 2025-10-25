package com.lsl.kokoro_ja_android

import org.junit.Test
import org.junit.Assert.*

/**
 * 混合语言检测和分段测试
 */
class MixedLanguageTest {
    
    @Test
    fun testSegmentation_ChineseEnglish() {
        val text = "我买了iPhone很好"
        val segments = LanguageDetector.segmentByLanguage(text)
        
        assertEquals(3, segments.size)
        
        // 第1段：中文
        assertEquals("我买了", segments[0].text)
        assertEquals(LanguageDetector.Language.CHINESE, segments[0].language)
        
        // 第2段：英文
        assertEquals("iPhone", segments[1].text)
        assertEquals(LanguageDetector.Language.ENGLISH, segments[1].language)
        
        // 第3段：中文
        assertEquals("很好", segments[2].text)
        assertEquals(LanguageDetector.Language.CHINESE, segments[2].language)
    }
    
    @Test
    fun testSegmentation_JapaneseEnglish() {
        val text = "この手機smartphone"
        val segments = LanguageDetector.segmentByLanguage(text)
        
        // 应该有2段
        assertTrue(segments.size >= 2)
        
        // 最后一段应该是英文
        val lastSegment = segments.last()
        assertTrue(lastSegment.text.contains("smartphone"))
        assertEquals(LanguageDetector.Language.ENGLISH, lastSegment.language)
    }
    
    @Test
    fun testSegmentation_ChineseJapanese() {
        val text = "你好こんにちは"
        val segments = LanguageDetector.segmentByLanguage(text)
        
        assertEquals(2, segments.size)
        
        // 第1段：中文
        assertEquals("你好", segments[0].text)
        assertEquals(LanguageDetector.Language.CHINESE, segments[0].language)
        
        // 第2段：日文
        assertEquals("こんにちは", segments[1].text)
        assertEquals(LanguageDetector.Language.JAPANESE, segments[1].language)
    }
    
    @Test
    fun testSegmentation_WithPunctuation() {
        val text = "我买了iPhone，很好！"
        val segments = LanguageDetector.segmentByLanguage(text)
        
        // 标点应该附加到前一个语言段
        assertTrue(segments.any { it.text.contains("，") || it.text.contains("！") })
    }
    
    @Test
    fun testSegmentation_PureChinese() {
        val text = "你好世界"
        val segments = LanguageDetector.segmentByLanguage(text)
        
        // 纯中文应该只有1段
        assertEquals(1, segments.size)
        assertEquals(text, segments[0].text)
        assertEquals(LanguageDetector.Language.CHINESE, segments[0].language)
    }
    
    @Test
    fun testSegmentation_PureJapanese() {
        val text = "こんにちは"
        val segments = LanguageDetector.segmentByLanguage(text)
        
        // 纯日文应该只有1段
        assertEquals(1, segments.size)
        assertEquals(text, segments[0].text)
        assertEquals(LanguageDetector.Language.JAPANESE, segments[0].language)
    }
}
