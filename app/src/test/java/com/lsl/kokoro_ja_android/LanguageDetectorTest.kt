package com.lsl.kokoro_ja_android

import org.junit.Test
import org.junit.Assert.*

/**
 * 语言检测器单元测试
 */
class LanguageDetectorTest {
    
    @Test
    fun testDetectChinese() {
        // 纯中文
        assertEquals(
            LanguageDetector.Language.CHINESE,
            LanguageDetector.detect("你好世界")
        )
        
        assertEquals(
            LanguageDetector.Language.CHINESE,
            LanguageDetector.detect("今天天气很好")
        )
        
        assertEquals(
            LanguageDetector.Language.CHINESE,
            LanguageDetector.detect("我爱编程")
        )
        
        // 中文 + 标点
        assertEquals(
            LanguageDetector.Language.CHINESE,
            LanguageDetector.detect("你好，世界！")
        )
    }
    
    @Test
    fun testDetectJapanese() {
        // 平假名
        assertEquals(
            LanguageDetector.Language.JAPANESE,
            LanguageDetector.detect("こんにちは")
        )
        
        assertEquals(
            LanguageDetector.Language.JAPANESE,
            LanguageDetector.detect("ありがとう")
        )
        
        // 片假名
        assertEquals(
            LanguageDetector.Language.JAPANESE,
            LanguageDetector.detect("コンニチハ")
        )
        
        // 混合假名
        assertEquals(
            LanguageDetector.Language.JAPANESE,
            LanguageDetector.detect("こんにちはコンニチハ")
        )
        
        // 日文 + 标点
        assertEquals(
            LanguageDetector.Language.JAPANESE,
            LanguageDetector.detect("こんにちは！")
        )
    }
    
    @Test
    fun testDetectKorean() {
        // 韩文
        assertEquals(
            LanguageDetector.Language.KOREAN,
            LanguageDetector.detect("안녕하세요")
        )
        
        assertEquals(
            LanguageDetector.Language.KOREAN,
            LanguageDetector.detect("감사합니다")
        )
        
        // 韩文 + 标点
        assertEquals(
            LanguageDetector.Language.KOREAN,
            LanguageDetector.detect("안녕하세요!")
        )
    }
    
    @Test
    fun testDetectCode() {
        // 测试返回语言代码
        assertEquals("zh", LanguageDetector.detectCode("你好"))
        assertEquals("ja", LanguageDetector.detectCode("こんにちは"))
        assertEquals("ko", LanguageDetector.detectCode("안녕하세요"))
        assertEquals("unknown", LanguageDetector.detectCode(""))
    }
    
    @Test
    fun testConvenienceMethods() {
        // 测试便捷方法
        assertTrue(LanguageDetector.isChinese("你好世界"))
        assertFalse(LanguageDetector.isChinese("こんにちは"))
        
        assertTrue(LanguageDetector.isJapanese("こんにちは"))
        assertFalse(LanguageDetector.isJapanese("你好"))
        
        assertTrue(LanguageDetector.isKorean("안녕하세요"))
        assertFalse(LanguageDetector.isKorean("你好"))
    }
    
    @Test
    fun testEmptyInput() {
        assertEquals(
            LanguageDetector.Language.UNKNOWN,
            LanguageDetector.detect("")
        )
    }
    
    @Test
    fun testOnlyPunctuation() {
        // 只有标点和空格
        assertEquals(
            LanguageDetector.Language.UNKNOWN,
            LanguageDetector.detect("。，！？")
        )
        
        assertEquals(
            LanguageDetector.Language.UNKNOWN,
            LanguageDetector.detect("   ")
        )
    }
    
    @Test
    fun testGetLanguageDistribution() {
        // 测试语言分布统计
        val dist1 = LanguageDetector.getLanguageDistribution("你好世界")
        assertTrue(dist1[LanguageDetector.Language.CHINESE]!! > 90.0)
        
        val dist2 = LanguageDetector.getLanguageDistribution("こんにちは")
        assertTrue(dist2[LanguageDetector.Language.JAPANESE]!! > 90.0)
        
        val dist3 = LanguageDetector.getLanguageDistribution("안녕하세요")
        assertTrue(dist3[LanguageDetector.Language.KOREAN]!! > 90.0)
    }
    
    @Test
    fun testMixedLanguage() {
        // 混合语言（应该返回占比最高的）
        // 注意：实际中文和日文汉字 Unicode 范围有重叠，
        // 但纯汉字会被识别为中文，因为中文范围更广
        
        val result = LanguageDetector.detect("你好こんにちは")
        // 这个测试可能因为字符数量而有不同结果
        // 只要不是 UNKNOWN 就说明检测器在工作
        assertNotEquals(LanguageDetector.Language.UNKNOWN, result)
    }
}
