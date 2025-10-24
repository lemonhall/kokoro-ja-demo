package com.lsl.kokoro_ja_android

import android.content.Context
import java.nio.ByteBuffer
import java.nio.ByteOrder

/**
 * 语音嵌入加载器
 * 支持加载完整的 510 帧嵌入数据，实现动态帧选择
 */
object VoiceEmbeddingLoader {
    
    /**
     * 语音嵌入数据类
     * @param embeddings 完整的 510 帧嵌入数据 [510][256]
     * @param embeddingDim 嵌入维度（256）
     * @param numFrames 帧数（510）
     */
    data class VoiceEmbedding(
        val embeddings: Array<FloatArray>,
        val embeddingDim: Int,
        val numFrames: Int
    ) {
        /**
         * 根据音素长度动态选择帧
         * 模拟 Pipeline 的逻辑: pack[len(ps)-1]
         * @param phonemeLength 音素数量（不含 BOS/EOS）
         * @return 对应帧的嵌入向量 [256]
         */
        fun getFrameByPhonemeLength(phonemeLength: Int): FloatArray {
            val frameIndex = (phonemeLength - 1).coerceIn(0, numFrames - 1)
            return embeddings[frameIndex]
        }
        
        override fun equals(other: Any?): Boolean {
            if (this === other) return true
            if (javaClass != other?.javaClass) return false
            other as VoiceEmbedding
            return embeddingDim == other.embeddingDim && 
                   numFrames == other.numFrames
        }
        
        override fun hashCode(): Int {
            var result = embeddingDim
            result = 31 * result + numFrames
            return result
        }
    }
    
    /**
     * 从 assets 加载语音嵌入（完整 510 帧）
     * @param context Android Context
     * @param voiceName 语音名称，如 "jf_nezumi"
     * @return VoiceEmbedding 包含完整 510 帧嵌入数据
     */
    fun load(context: Context, voiceName: String = "jf_nezumi"): VoiceEmbedding {
        val fileName = "$voiceName.bin"
        
        context.assets.open(fileName).use { inputStream ->
            // 读取所有数据
            val allBytes = inputStream.readBytes()
            val byteBuffer = ByteBuffer.wrap(allBytes).order(ByteOrder.LITTLE_ENDIAN)
            
            // 读取维度信息（小端序）
            val embeddingDim = byteBuffer.int
            val numFrames = byteBuffer.int
            
            require(embeddingDim == 256) { "语音嵌入维度必须是 256，实际: $embeddingDim" }
            require(numFrames == 510) { "帧数必须是 510，实际: $numFrames" }
            
            // 读取所有帧的数据 [510][256]
            val embeddings = Array(numFrames) { FloatArray(embeddingDim) }
            for (frame in 0 until numFrames) {
                for (dim in 0 until embeddingDim) {
                    embeddings[frame][dim] = byteBuffer.float
                }
            }
            
            return VoiceEmbedding(embeddings, embeddingDim, numFrames)
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
