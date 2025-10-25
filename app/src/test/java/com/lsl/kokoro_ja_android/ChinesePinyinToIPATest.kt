package com.lsl.kokoro_ja_android

import org.junit.Test
import org.junit.Assert.*

/**
 * ChinesePinyinToIPA 单元测试
 * 
 * 测试策略：
 * 1. 基础声母韵母组合
 * 2. 特殊情况（卷舌音、平舌音）
 * 3. 不同声调
 * 4. 边界情况
 * 
 * 对照基准：Python 端 misaki 的输出
 */
class ChinesePinyinToIPATest {
    
    @Test
    fun testStatistics() {
        val stats = ChinesePinyinToIPA.getStatistics()
        
        assertEquals(21, stats["initials"])  // 21 个声母
        assertEquals(40, stats["finals"])    // 40 个韵母
        assertEquals(5, stats["tones"])      // 5 个声调
        assertEquals(1, stats["special_zh_ch_sh_r"])  // 卷舌音特殊处理
        assertEquals(1, stats["special_z_c_s"])       // 平舌音特殊处理
    }
    
    @Test
    fun testBasicConversion() {
        // 简单的声母+韵母+声调
        assertEquals("ni↓", ChinesePinyinToIPA.convert("ni3"))
        assertEquals("xau̯↓", ChinesePinyinToIPA.convert("hao3"))
        assertEquals("ma→", ChinesePinyinToIPA.convert("ma1"))
        assertEquals("pʰa↗", ChinesePinyinToIPA.convert("pa2"))
    }
    
    @Test
    fun testZeroInitial() {
        // 零声母（无声母）
        assertEquals("a→", ChinesePinyinToIPA.convert("a1"))
        assertEquals("ou̯↘", ChinesePinyinToIPA.convert("ou4"))
        assertEquals("en→", ChinesePinyinToIPA.convert("en1"))
    }
    
    @Test
    fun testRetroflexInitials() {
        // 卷舌音 zh, ch, sh, r + i
        // 应该使用特殊的韵母映射
        assertEquals("ʈʂɻ̩→", ChinesePinyinToIPA.convert("zhi1"))
        assertEquals("ʈʂʰɻ̩→", ChinesePinyinToIPA.convert("chi1"))
        assertEquals("ʂɻ̩↘", ChinesePinyinToIPA.convert("shi4"))
        assertEquals("ɻɻ̩↘", ChinesePinyinToIPA.convert("ri4"))
    }
    
    @Test
    fun testDentalInitials() {
        // 平舌音 z, c, s + i
        assertEquals("ʦɹ̩→", ChinesePinyinToIPA.convert("zi1"))
        assertEquals("ʦʰɹ̩↘", ChinesePinyinToIPA.convert("ci4"))
        assertEquals("sɹ̩→", ChinesePinyinToIPA.convert("si1"))
    }
    
    @Test
    fun testAllTones() {
        // 测试所有 5 个声调
        assertEquals("ma→", ChinesePinyinToIPA.convert("ma1"))   // 第一声
        assertEquals("ma↗", ChinesePinyinToIPA.convert("ma2"))   // 第二声
        assertEquals("ma↓", ChinesePinyinToIPA.convert("ma3"))   // 第三声
        assertEquals("ma↘", ChinesePinyinToIPA.convert("ma4"))   // 第四声
        assertEquals("ma", ChinesePinyinToIPA.convert("ma5"))    // 轻声
    }
    
    @Test
    fun testComplexFinals() {
        // 复杂韵母
        assertEquals("pei̯→", ChinesePinyinToIPA.convert("bei1"))   // 复韵母 ei
        assertEquals("tau̯↘", ChinesePinyinToIPA.convert("dao4"))   // 复韵母 ao
        assertEquals("pan→", ChinesePinyinToIPA.convert("ban1"))    // 鼻韵母 an
        assertEquals("pəŋ↗", ChinesePinyinToIPA.convert("beng2"))  // 鼻韵母 eng
    }
    
    @Test
    fun testISeriesFinals() {
        // i 系韵母
        assertEquals("ʨja→", ChinesePinyinToIPA.convert("jia1"))      // ia
        assertEquals("ʨje↘", ChinesePinyinToIPA.convert("jie4"))      // ie
        assertEquals("ʨjau̯↘", ChinesePinyinToIPA.convert("jiao4"))   // iao
        assertEquals("ʨjɛn↘", ChinesePinyinToIPA.convert("jian4"))    // ian
        assertEquals("ʨin→", ChinesePinyinToIPA.convert("jin1"))      // in
        assertEquals("ʨjaŋ↗", ChinesePinyinToIPA.convert("jiang2"))  // iang
        assertEquals("ʨiŋ→", ChinesePinyinToIPA.convert("jing1"))     // ing
    }
    
    @Test
    fun testUSeriesFinals() {
        // u 系韵母
        assertEquals("kwa→", ChinesePinyinToIPA.convert("gua1"))      // ua
        assertEquals("kwo↓", ChinesePinyinToIPA.convert("guo3"))      // uo
        assertEquals("kwai̯↘", ChinesePinyinToIPA.convert("guai4"))   // uai
        assertEquals("kwan→", ChinesePinyinToIPA.convert("guan1"))    // uan
    }
    
    @Test
    fun testUmlautFinals() {
        // ü 系韵母
        assertEquals("y→", ChinesePinyinToIPA.convert("ü1"))         // ü
        assertEquals("ɥe↘", ChinesePinyinToIPA.convert("üe4"))       // üe (月)
        assertEquals("ɥɛn↗", ChinesePinyinToIPA.convert("üan2"))     // üan (冤)
        assertEquals("yn→", ChinesePinyinToIPA.convert("ün1"))       // ün (晕)
    }
    
    @Test
    fun testBatchConversion() {
        // 批量转换
        val pinyins = listOf("ni3", "hao3", "shi4", "jie4")
        val result = ChinesePinyinToIPA.convertBatch(pinyins)
        
        // 验证格式：用空格分隔
        assertTrue(result.contains(" "))
        
        // 验证包含所有音素
        assertTrue(result.contains("ni↓"))
        assertTrue(result.contains("xau̯↓"))
        assertTrue(result.contains("ʂɻ̩↘"))
        assertTrue(result.contains("ʨje↘"))
    }
    
    @Test
    fun testPinyinValidation() {
        // 有效的拼音
        assertTrue(ChinesePinyinToIPA.isValidPinyin("ni3"))
        assertTrue(ChinesePinyinToIPA.isValidPinyin("hao3"))
        assertTrue(ChinesePinyinToIPA.isValidPinyin("zhi1"))
        
        // 无效的拼音
        assertFalse(ChinesePinyinToIPA.isValidPinyin(""))        // 空字符串
        assertFalse(ChinesePinyinToIPA.isValidPinyin("ni"))      // 缺少声调
        assertFalse(ChinesePinyinToIPA.isValidPinyin("ni0"))     // 无效声调
        assertFalse(ChinesePinyinToIPA.isValidPinyin("ni6"))     // 无效声调
        assertFalse(ChinesePinyinToIPA.isValidPinyin("xyz1"))    // 无效拼音
    }
    
    @Test
    fun testWithoutToneSimplification() {
        // 测试不简化声调符号
        val result = ChinesePinyinToIPA.convert("ma1", simplifyTone = false)
        assertEquals("ma˥", result)  // 使用 IPA 调值符号
        
        val result2 = ChinesePinyinToIPA.convert("ma3", simplifyTone = false)
        assertEquals("ma˧˩˧", result2)  // 第三声的完整 IPA 符号
    }
    
    @Test
    fun testRealWorldSentences() {
        // 真实句子测试（对照 Python 端输出）
        
        // "你好" → ni3 hao3
        val result1 = ChinesePinyinToIPA.convertBatch(listOf("ni3", "hao3"))
        assertEquals("ni↓ xau̯↓", result1)
        
        // "中文测试" → zhong1 wen2 ce4 shi4
        val result2 = ChinesePinyinToIPA.convertBatch(
            listOf("zhong1", "wen2", "ce4", "shi4")
        )
        assertEquals("ʈʂʊ→ŋ wə↗n ʦʰɤ↘ ʂɻ̩↘", result2)
        
        // "知识" → zhi1 shi2
        val result3 = ChinesePinyinToIPA.convertBatch(listOf("zhi1", "shi2"))
        assertEquals("ʈʂɻ̩→ ʂɻ̩↗", result3)
    }
    
    @Test
    fun testEdgeCases() {
        // 边界情况
        
        // 只有韵母（零声母）
        assertEquals("a→", ChinesePinyinToIPA.convert("a1"))
        assertEquals("e→", ChinesePinyinToIPA.convert("e1"))
        
        // 轻声
        assertEquals("de", ChinesePinyinToIPA.convert("de5"))
        assertEquals("ma", ChinesePinyinToIPA.convert("ma5"))
        
        // 最长的拼音
        assertEquals("ʈʂʰwaŋ→", ChinesePinyinToIPA.convert("chuang1"))
    }
    
    @Test
    fun testSpecialCombinations() {
        // 特殊组合
        
        // iou → jou̯ (有)
        assertEquals("ʨjou̯↗", ChinesePinyinToIPA.convert("jiou2"))
        
        // uei → wei̯ (威)
        assertEquals("wei̯→", ChinesePinyinToIPA.convert("uei1"))
        
        // uen → wən (温)
        assertEquals("wən→", ChinesePinyinToIPA.convert("uen1"))
    }
}
