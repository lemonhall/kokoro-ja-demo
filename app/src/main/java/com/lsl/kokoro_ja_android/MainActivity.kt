package com.lsl.kokoro_ja_android

import android.media.AudioAttributes
import android.media.AudioFormat
import android.media.AudioTrack
import android.os.Bundle
import android.widget.Button
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import androidx.lifecycle.lifecycleScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext

class MainActivity : AppCompatActivity() {
    
    private val engine = KokoroEngine(this)
    private var audioTrack: AudioTrack? = null
    
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        
        val statusText = findViewById<TextView>(R.id.statusText)
        val testButton = findViewById<Button>(R.id.testButton)
        
        // 初始化模型
        lifecycleScope.launch {
            try {
                statusText.text = "正在加载模型..."
                engine.initialize("kokoro_latest_int8.onnx")
                statusText.text = "✅ 模型加载成功\n点击按钮测试"
                testButton.isEnabled = true
            } catch (e: Exception) {
                statusText.text = "❌ 模型加载失败:\n${e.message}"
            }
        }
        
        // 测试按钮
        testButton.setOnClickListener {
            lifecycleScope.launch {
                testSynthesis(statusText)
            }
        }
    }
    
    private suspend fun testSynthesis(statusText: TextView) {
        try {
            statusText.text = "正在生成语音..."
            
            // 示例: "こんにちは" 的音素和 input_ids
            // phonemes: "koɲɲiʨiβa"
            // input_ids: [0, 53, 57, 114, 114, 51, 21, 51, 75, 43, 0]
            val inputIds = longArrayOf(0, 53, 57, 114, 114, 51, 21, 51, 75, 43, 0)
            
            // 加载真实的语音嵌入
            val voiceEmbedding = VoiceEmbeddingLoader.load(this, "jf_nezumi")
            
            // 推理
            val waveform = engine.synthesize(inputIds, voiceEmbedding)
            
            statusText.text = "✅ 生成成功!\n" +
                    "波形长度: ${waveform.size}\n" +
                    "时长: ${String.format("%.2f", waveform.size / 24000.0)}秒\n" +
                    "正在播放..."
            
            // 播放音频
            playAudio(waveform)
            
        } catch (e: Exception) {
            statusText.text = "❌ 生成失败:\n${e.message}"
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
            .build()
        
        audioTrack?.apply {
            play()
            write(waveform, 0, waveform.size, AudioTrack.WRITE_BLOCKING)
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
