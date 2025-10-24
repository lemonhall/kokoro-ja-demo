package com.lsl.kokoro_ja_android

import android.media.AudioAttributes
import android.media.AudioFormat
import android.media.AudioTrack
import android.os.Bundle
import android.widget.ArrayAdapter
import android.widget.Button
import android.widget.Spinner
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import androidx.lifecycle.lifecycleScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext

class MainActivity : AppCompatActivity() {
    
    private val engine = KokoroEngine(this)
    private var audioTrack: AudioTrack? = null
    private var currentSentenceIndex = 0
    
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        
        val statusText = findViewById<TextView>(R.id.statusText)
        val sentenceSpinner = findViewById<Spinner>(R.id.sentenceSpinner)
        val synthesizeButton = findViewById<Button>(R.id.synthesizeButton)
        
        // 设置日文句子选择器
        val adapter = ArrayAdapter(
            this,
            android.R.layout.simple_spinner_item,
            JapanesePresets.getTextList()
        )
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item)
        sentenceSpinner.adapter = adapter
        
        // 初始化模型
        lifecycleScope.launch {
            try {
                statusText.text = "正在加载模型...\n这可能需要几秒钟"
                engine.initialize("kokoro_fp32.onnx")
                statusText.text = "✅ 模型加载成功\n\n选择一句日文，点击合成"
                synthesizeButton.isEnabled = true
            } catch (e: Exception) {
                statusText.text = "❌ 模型加载失败:\n${e.message}"
                e.printStackTrace()
            }
        }
        
        // 合成按钮
        synthesizeButton.setOnClickListener {
            currentSentenceIndex = sentenceSpinner.selectedItemPosition
            lifecycleScope.launch {
                synthesizeSentence(currentSentenceIndex, statusText)
            }
        }
    }
    
    private suspend fun synthesizeSentence(index: Int, statusText: TextView) {
        try {
            val sentence = JapanesePresets.sentences[index]
            
            statusText.text = "正在合成...\n" +
                    "日文: ${sentence.text}\n" +
                    "意思: ${sentence.translation}\n" +
                    "音素: ${sentence.phonemes}"
            
            // 计时开始
            val startTime = System.currentTimeMillis()
            
            // 转换音素为 input_ids
            val inputIds = KokoroVocabFull.phonemesToIds(sentence.phonemes)
            
            // 加载真实的语音嵌入
            val voiceEmbedding = VoiceEmbeddingLoader.load(this, "jf_nezumi")
            
            val preprocessTime = System.currentTimeMillis() - startTime
            
            // 推理（尝试不同的语速以改善音色）
            val inferenceStart = System.currentTimeMillis()
            val waveform = engine.synthesize(inputIds, voiceEmbedding, speed = 1.0)
            val inferenceTime = System.currentTimeMillis() - inferenceStart
            
            // 调试信息
            val maxVal = waveform.maxOrNull() ?: 0f
            val minVal = waveform.minOrNull() ?: 0f
            val totalTime = System.currentTimeMillis() - startTime
            val audioDuration = waveform.size / 24000.0
            val rtf = totalTime / 1000.0 / audioDuration  // Real-Time Factor
            
            println("🎵 音频生成: 长度=${waveform.size}, 最大值=$maxVal, 最小值=$minVal")
            println("⏱️ 性能: 预处理=${preprocessTime}ms, 推理=${inferenceTime}ms, 总耗时=${totalTime}ms, RTF=${String.format("%.2f", rtf)}")
            
            statusText.text = "✅ 合成成功!\n" +
                    "日文: ${sentence.text}\n" +
                    "意思: ${sentence.translation}\n" +
                    "音频: ${String.format("%.2f", audioDuration)}秒\n" +
                    "音量: [$minVal, $maxVal]\n" +
                    "⚙️ 性能:\n" +
                    "  预处理: ${preprocessTime}ms\n" +
                    "  推理: ${inferenceTime}ms\n" +
                    "  总耗时: ${totalTime}ms\n" +
                    "  RTF: ${String.format("%.2fx", rtf)}\n" +
                    "正在播放..."
            
            // 播放音频
            playAudio(waveform)
            
            // 播放完成
            statusText.text = "✅ 播放完成!\n" +
                    "日文: ${sentence.text}\n" +
                    "意思: ${sentence.translation}\n" +
                    "可以选择其他句子继续测试"
            
        } catch (e: Exception) {
            statusText.text = "❌ 合成失败:\n${e.message}"
            e.printStackTrace()
        }
    }
    
    private suspend fun playAudio(waveform: FloatArray) = withContext(Dispatchers.IO) {
        val sampleRate = 24000
        val bufferSize = AudioTrack.getMinBufferSize(
            sampleRate,
            AudioFormat.CHANNEL_OUT_MONO,
            AudioFormat.ENCODING_PCM_FLOAT
        )
        
        println("🔊 开始播放: 采样率=$sampleRate, 缓冲=$bufferSize, 数据长度=${waveform.size}")
        
        audioTrack = AudioTrack.Builder()
            .setAudioAttributes(
                AudioAttributes.Builder()
                    .setUsage(AudioAttributes.USAGE_MEDIA)
                    .setContentType(AudioAttributes.CONTENT_TYPE_SPEECH)
                    .build()
            )
            .setAudioFormat(
                AudioFormat.Builder()
                    .setEncoding(AudioFormat.ENCODING_PCM_FLOAT)
                    .setSampleRate(sampleRate)
                    .setChannelMask(AudioFormat.CHANNEL_OUT_MONO)
                    .build()
            )
            .setBufferSizeInBytes(bufferSize.coerceAtLeast(waveform.size * 4))
            .setTransferMode(AudioTrack.MODE_STATIC)  // 使用静态模式
            .build()
        
        audioTrack?.apply {
            // 写入数据
            val written = write(waveform, 0, waveform.size, AudioTrack.WRITE_BLOCKING)
            println("💾 写入样本数: $written")
            
            // 设置音量（最大）
            setVolume(AudioTrack.getMaxVolume())
            
            // 播放
            play()
            println("▶️ 开始播放")
            
            // 等待播放完成（静态模式会自动停止）
            while (playState == AudioTrack.PLAYSTATE_PLAYING) {
                Thread.sleep(100)
            }
            
            println("⏹️ 播放完成")
            stop()
            release()
        }
    }
    
    override fun onDestroy() {
        super.onDestroy()
        engine.release()
        audioTrack?.release()
    }
}
