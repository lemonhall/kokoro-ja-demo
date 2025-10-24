package com.lsl.kokoro_ja_android

import android.content.Context
import java.io.DataInputStream
import java.nio.ByteBuffer
import java.nio.ByteOrder

/**
 * 语音嵌入加载器
 */
object VoiceEmbeddingLoader {
    
    /**
     * 从 assets 加载语音嵌入
     * @param context Android Context
     * @param voiceName 语音名称，如 "jf_nezumi"
     * @return FloatArray[256] 语音嵌入向量
     */
    fun load(context: Context, voiceName: String = "jf_nezumi"): FloatArray {
        val fileName = "$voiceName.bin"
        
        context.assets.open(fileName).use { inputStream ->
            val dis = DataInputStream(inputStream)
            
            // 读取维度
            val dimension = dis.readInt()
            require(dimension == 256) { "语音嵌入维度必须是 256，实际: $dimension" }
            
            // 读取数据
            val buffer = ByteArray(dimension * 4)
            dis.readFully(buffer)
            
            // 转换为 FloatArray
            val byteBuffer = ByteBuffer.wrap(buffer).order(ByteOrder.LITTLE_ENDIAN)
            val embedding = FloatArray(dimension)
            for (i in 0 until dimension) {
                embedding[i] = byteBuffer.float
            }
            
            return embedding
        }
    }
    
    /**
     * 获取可用的语音列表
     */
    val availableVoices = listOf(
        "jf_nezumi"  // 女声 (当前可用)
        // 注意: jf_hanako 和 jm_shinji 在 Kokoro-82M 中不可用
        // 需要使用更新的模型版本或自定义语音
    )
}
