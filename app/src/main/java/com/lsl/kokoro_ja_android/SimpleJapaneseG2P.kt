package com.lsl.kokoro_ja_android

/**
 * 简化的日文 G2P (Grapheme-to-Phoneme) 转换器
 * 
 * 基于假名到音素的直接映射
 * 注意：这是简化版本，准确度不如 MeCab，但对常见日文够用
 */
object SimpleJapaneseG2P {
    
    // 平假名到音素映射表
    private val hiraganaToPhoneme = mapOf(
        // あ行
        "あ" to "a", "い" to "i", "う" to "ɯ", "え" to "e", "お" to "o",
        
        // か行
        "か" to "ka", "き" to "kʲi", "く" to "kɯ", "け" to "ke", "こ" to "ko",
        "が" to "ɡa", "ぎ" to "ɡʲi", "ぐ" to "ɡɯ", "げ" to "ɡe", "ご" to "ɡo",
        
        // さ行
        "さ" to "sa", "し" to "ɕi", "す" to "sɨ", "せ" to "se", "そ" to "so",
        "ざ" to "za", "じ" to "ʑi", "ず" to "zɨ", "ぜ" to "ze", "ぞ" to "zo",
        
        // た行
        "た" to "ta", "ち" to "ʨi", "つ" to "tsɨ", "て" to "te", "と" to "to",
        "だ" to "da", "ぢ" to "ʥi", "づ" to "zɨ", "で" to "de", "ど" to "do",
        
        // な行
        "な" to "na", "に" to "ɲi", "ぬ" to "nɯ", "ね" to "ne", "の" to "no",
        
        // は行
        "は" to "ha", "ひ" to "çi", "ふ" to "ɸɯ", "へ" to "he", "ほ" to "ho",
        "ば" to "ba", "び" to "bʲi", "ぶ" to "bɯ", "べ" to "be", "ぼ" to "bo",
        "ぱ" to "pa", "ぴ" to "pʲi", "ぷ" to "pɯ", "ぺ" to "pe", "ぽ" to "po",
        
        // ま行
        "ま" to "ma", "み" to "mʲi", "む" to "mɯ", "め" to "me", "も" to "mo",
        
        // や行
        "や" to "ja", "ゆ" to "jɯ", "よ" to "jo",
        
        // ら行
        "ら" to "ɾa", "り" to "ɾʲi", "る" to "ɾɯ", "れ" to "ɾe", "ろ" to "ɾo",
        
        // わ行
        "わ" to "βa", "ゐ" to "i", "ゑ" to "e", "を" to "o", "ん" to "ɴ",
        
        // 拗音（きゃ、きゅ、きょ等）
        "きゃ" to "kʲa", "きゅ" to "kʲɯ", "きょ" to "kʲo",
        "しゃ" to "ɕa", "しゅ" to "ɕɯ", "しょ" to "ɕo",
        "ちゃ" to "ʨa", "ちゅ" to "ʨɯ", "ちょ" to "ʨo",
        "にゃ" to "ɲa", "にゅ" to "ɲɯ", "にょ" to "ɲo",
        "ひゃ" to "ça", "ひゅ" to "çɯ", "ひょ" to "ço",
        "みゃ" to "mʲa", "みゅ" to "mʲɯ", "みょ" to "mʲo",
        "りゃ" to "ɾʲa", "りゅ" to "ɾʲɯ", "りょ" to "ɾʲo",
        "ぎゃ" to "ɡʲa", "ぎゅ" to "ɡʲɯ", "ぎょ" to "ɡʲo",
        "じゃ" to "ʥa", "じゅ" to "ʥɯ", "じょ" to "ʥo",
        "びゃ" to "bʲa", "びゅ" to "bʲɯ", "びょ" to "bʲo",
        "ぴゃ" to "pʲa", "ぴゅ" to "pʲɯ", "ぴょ" to "pʲo",
        
        // 长音符号
        "ー" to "ː",
        
        // 促音（小つ）
        "っ" to "ʔ",
        
        // 标点符号
        "、" to ",", "。" to ".", "！" to "!", "？" to "?",
        "，" to ",", "．" to ".",
    )
    
    // 片假名到音素映射（与平假名相同）
    private val katakanaToPhoneme = mapOf(
        // ア行
        "ア" to "a", "イ" to "i", "ウ" to "ɯ", "エ" to "e", "オ" to "o",
        
        // カ行
        "カ" to "ka", "キ" to "kʲi", "ク" to "kɯ", "ケ" to "ke", "コ" to "ko",
        "ガ" to "ɡa", "ギ" to "ɡʲi", "グ" to "ɡɯ", "ゲ" to "ɡe", "ゴ" to "ɡo",
        
        // サ行
        "サ" to "sa", "シ" to "ɕi", "ス" to "sɨ", "セ" to "se", "ソ" to "so",
        "ザ" to "za", "ジ" to "ʑi", "ズ" to "zɨ", "ゼ" to "ze", "ゾ" to "zo",
        
        // タ行
        "タ" to "ta", "チ" to "ʨi", "ツ" to "tsɨ", "テ" to "te", "ト" to "to",
        "ダ" to "da", "ヂ" to "ʥi", "ヅ" to "zɨ", "デ" to "de", "ド" to "do",
        
        // ナ行
        "ナ" to "na", "ニ" to "ɲi", "ヌ" to "nɯ", "ネ" to "ne", "ノ" to "no",
        
        // ハ行
        "ハ" to "ha", "ヒ" to "çi", "フ" to "ɸɯ", "ヘ" to "he", "ホ" to "ho",
        "バ" to "ba", "ビ" to "bʲi", "ブ" to "bɯ", "ベ" to "be", "ボ" to "bo",
        "パ" to "pa", "ピ" to "pʲi", "プ" to "pɯ", "ペ" to "pe", "ポ" to "po",
        
        // マ行
        "マ" to "ma", "ミ" to "mʲi", "ム" to "mɯ", "メ" to "me", "モ" to "mo",
        
        // ヤ行
        "ヤ" to "ja", "ユ" to "jɯ", "ヨ" to "jo",
        
        // ラ行
        "ラ" to "ɾa", "リ" to "ɾʲi", "ル" to "ɾɯ", "レ" to "ɾe", "ロ" to "ɾo",
        
        // ワ行
        "ワ" to "βa", "ヰ" to "i", "ヱ" to "e", "ヲ" to "o", "ン" to "ɴ",
        
        // 拗音
        "キャ" to "kʲa", "キュ" to "kʲɯ", "キョ" to "kʲo",
        "シャ" to "ɕa", "シュ" to "ɕɯ", "ショ" to "ɕo",
        "チャ" to "ʨa", "チュ" to "ʨɯ", "チョ" to "ʨo",
        "ニャ" to "ɲa", "ニュ" to "ɲɯ", "ニョ" to "ɲo",
        "ヒャ" to "ça", "ヒュ" to "çɯ", "ヒョ" to "ço",
        "ミャ" to "mʲa", "ミュ" to "mʲɯ", "ミョ" to "mʲo",
        "リャ" to "ɾʲa", "リュ" to "ɾʲɯ", "リョ" to "ɾʲo",
        "ギャ" to "ɡʲa", "ギュ" to "ɡʲɯ", "ギョ" to "ɡʲo",
        "ジャ" to "ʥa", "ジュ" to "ʥɯ", "ジョ" to "ʥo",
        "ビャ" to "bʲa", "ビュ" to "bʲɯ", "ビョ" to "bʲo",
        "ピャ" to "pʲa", "ピュ" to "pʲɯ", "ピョ" to "pʲo",
        
        // 长音
        "ー" to "ː",
        
        // 促音
        "ッ" to "ʔ",
    )
    
    /**
     * 将日文文本转换为音素
     * 
     * @param text 日文文本（平假名、片假名、汉字混合）
     * @return 音素字符串
     */
    fun textToPhonemes(text: String): String {
        val phonemes = StringBuilder()
        var i = 0
        
        while (i < text.length) {
            // 尝试匹配2字符（拗音）
            if (i + 1 < text.length) {
                val twoChar = text.substring(i, i + 2)
                val phoneme = hiraganaToPhoneme[twoChar] ?: katakanaToPhoneme[twoChar]
                if (phoneme != null) {
                    phonemes.append(phoneme)
                    i += 2
                    continue
                }
            }
            
            // 匹配1字符
            val oneChar = text[i].toString()
            val phoneme = hiraganaToPhoneme[oneChar] 
                ?: katakanaToPhoneme[oneChar]
                ?: oneChar // 无法识别的字符保留原样
            
            phonemes.append(phoneme)
            i++
        }
        
        return phonemes.toString()
    }
    
    /**
     * 检查文本是否只包含假名（不含汉字）
     * 简化版：只检查是否是汉字，其他都允许
     */
    fun isKanaOnly(text: String): Boolean {
        // 汉字的 Unicode 范围
        val kanjiRanges = listOf(
            0x4E00..0x9FFF,  // CJK Unified Ideographs
            0x3400..0x4DBF,  // CJK Extension A
            0x20000..0x2A6DF // CJK Extension B
        )
        
        return text.none { char ->
            kanjiRanges.any { range -> char.code in range }
        }
    }
}
