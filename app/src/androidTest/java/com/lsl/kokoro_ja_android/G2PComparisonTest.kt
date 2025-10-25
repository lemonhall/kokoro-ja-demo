package com.lsl.kokoro_ja_android

import android.content.Context
import androidx.test.ext.junit.runners.AndroidJUnit4
import androidx.test.platform.app.InstrumentationRegistry
import org.junit.Test
import org.junit.runner.RunWith

/**
 * G2P 对比测试 - Android 端
 * 
 * 运行方法：
 * adb shell am instrument -w -e class com.lsl.kokoro_ja_android.G2PComparisonTest com.lsl.kokoro_ja_android.test/androidx.test.runner.AndroidJUnitRunner
 */
@RunWith(AndroidJUnit4::class)
class G2PComparisonTest {
    
    private val context: Context = InstrumentationRegistry.getInstrumentation().targetContext
    
    // 测试句子列表（与 Python 端保持一致）
    private val testSentences = listOf(
        "こんにちは、私はレモンと申します。",
        "今日は良い天気ですね。",
        "東京大学からの留学生です。",
        "彼女は日本語を勉強していますが、まだ上手ではありません。",
        "学校へ行って、友達と遊びました。"
    )
    
    @Test
    fun testAllSentences() {
        println("========== G2P_COMPARISON_START ==========")
        
        // 初始化 G2P 系统
        val g2pSystem = JapaneseG2PSystem(context)
        
        testSentences.forEachIndexed { index, text ->
            try {
                val phonemes = g2pSystem.textToPhonemes(text)
                // 输出格式：G2P_RESULT|序号|文本|音素
                println("G2P_RESULT|${index + 1}|$text|$phonemes")
            } catch (e: Exception) {
                println("G2P_ERROR|${index + 1}|$text|${e.message}")
            }
        }
        
        println("========== G2P_COMPARISON_END ==========")
    }
}
