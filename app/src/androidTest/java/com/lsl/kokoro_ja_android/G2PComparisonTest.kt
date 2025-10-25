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
        // === 原有测试 ===
        "こんにちは、私はレモンと申します。",
        "今日は良い天気ですね。",
        "東京大学からの留学生です。",
        "彼女は日本語を勉強していますが、まだ上手ではありません。",
        "学校へ行って、友達と遊びました。",
        
        // === 新增测试 ===
        "新聞を読んでいます。",                      // 6. 拨音测试
        "きょうはきゅうりょうびです。",                // 7. 拗音测试
        "かぎをかけてください。",                    // 8. 浊音测试
        "おねえさんはきれいです。",                  // 9. 长音测试
        "私は本を読むのが好きです。",                  // 10. 助词测试
        "切符を買いました。",                          // 11. 促音+拨音
        "先生がいらっしゃいました。",                // 12. 敬语
        "わかりません。",                              // 13. 否定形
        "何時に会いましょうか。",                      // 14. 疑问句
        "三つのりんごを食べました。",                // 15. 数量表达
        "雨が降り始めました。",                        // 16. 复合动词
        "先生に褒められました。",                      // 17. 被动态
        "子供に宿題をさせます。",                      // 18. 使役态
        "暇なら、遊びに来てください。",              // 19. 假定形
        "コンピューターを使っています。"                // 20. 外来语
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
