package com.lsl.kokoro_ja_android

/**
 * 预设的日文句子和对应的音素
 * 自动生成 - 由 generate_japanese_presets.py 生成
 * 
 * 临时方案：预先转换好常用句子
 * TODO: 后续集成 MeCab 实现完整的 G2P 转换
 */
object JapanesePresets {
    
    data class JapaneseSentence(
        val text: String,          // 日文文本
        val phonemes: String,      // 音素
        val translation: String    // 中文翻译（用于显示）
    )
    
    val sentences = listOf(
        JapaneseSentence(
            text = "こんにちは",
            phonemes = "koɲɲiʨiβa",
            translation = "你好"
        ),
        JapaneseSentence(
            text = "ありがとう",
            phonemes = "aɾʲiɡatoː",
            translation = "谢谢"
        ),
        JapaneseSentence(
            text = "さようなら",
            phonemes = "sajoːnaɾa",
            translation = "再见"
        ),
        JapaneseSentence(
            text = "おはよう",
            phonemes = "ohajoː",
            translation = "早上好"
        ),
        JapaneseSentence(
            text = "こんばんは",
            phonemes = "kombaɴ βa",
            translation = "晚上好"
        ),
        JapaneseSentence(
            text = "すみません",
            phonemes = "sɨmʲimaseɴ",
            translation = "对不起"
        ),
        JapaneseSentence(
            text = "はい",
            phonemes = "hai",
            translation = "是"
        ),
        JapaneseSentence(
            text = "いいえ",
            phonemes = "iːe",
            translation = "不"
        ),
        JapaneseSentence(
            text = "おいしい",
            phonemes = "oiɕiː",
            translation = "好吃"
        ),
        JapaneseSentence(
            text = "きれい",
            phonemes = "kʲiɾeː",
            translation = "漂亮"
        ),
        JapaneseSentence(
            text = "わかりました",
            phonemes = "βakaɾʲi maɕita",
            translation = "我明白了"
        ),
        JapaneseSentence(
            text = "どういたしまして",
            phonemes = "doː itaɕi maɕite",
            translation = "不客气"
        ),
        JapaneseSentence(
            text = "お元気ですか",
            phonemes = "o ɡeŋkʲi desɨ ka",
            translation = "你好吗"
        ),
        JapaneseSentence(
            text = "大丈夫です",
            phonemes = "daiʥoːbɯ desɨ",
            translation = "没关系"
        ),
        JapaneseSentence(
            text = "頑張って",
            phonemes = "ɡambaʔte",
            translation = "加油"
        ),
        JapaneseSentence(
            text = "今日はとても良い天気ですね、公園で散歩しませんか",
            phonemes = "kʲoː βa totemo joi teŋkʲi desɨ ne koːeɴ de sampo ɕi maseɴ ka",
            translation = "今天天气真好呀，一起去公园散步吧"
        ),
    )
    
    /**
     * 根据日文文本查找音素
     */
    fun getPhonemes(text: String): String? {
        return sentences.find { it.text == text }?.phonemes
    }
    
    /**
     * 获取所有日文文本列表（用于 Spinner 显示）
     */
    fun getTextList(): List<String> {
        return sentences.map { "${it.text} (${it.translation})" }
    }
}
