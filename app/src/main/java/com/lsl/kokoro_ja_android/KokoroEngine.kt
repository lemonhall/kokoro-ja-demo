package com.lsl.kokoro_ja_android

import ai.onnxruntime.*
import android.content.Context
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import java.io.File
import java.nio.FloatBuffer
import java.nio.LongBuffer

/**
 * Kokoro TTS ONNX 推理引擎
 * 
 * 使用方法：
 * ```kotlin
 * val engine = KokoroEngine(context)
 * engine.initialize("kokoro_latest_int8.onnx")
 * val audio = engine.synthesize(inputIds, voiceEmbedding)
 * ```
 */
class KokoroEngine(private val context: Context) {
    
    private var session: OrtSession? = null
    private val env = OrtEnvironment.getEnvironment()
    
    /**
     * 初始化模型
     * @param modelName assets 中的模型文件名
     */
    suspend fun initialize(modelName: String = "kokoro_latest_int8.onnx") = withContext(Dispatchers.IO) {
        try {
            // 从 assets 复制模型到缓存目录
            val modelFile = File(context.cacheDir, modelName)
            if (!modelFile.exists()) {
                context.assets.open(modelName).use { input ->
                    modelFile.outputStream().use { output ->
                        input.copyTo(output)
                    }
                }
            }
            
            // 创建 session 选项
            val sessionOptions = OrtSession.SessionOptions().apply {
                // 启用所有优化
                setOptimizationLevel(OrtSession.SessionOptions.OptLevel.ALL_OPT)
                
                // 禁用 NNAPI，使用纯 CPU（FP32 模型更兼容）
                // NNAPI 对 FP32 的支持有限，可能导致 ConvInteger 错误
                println("⚙️ 使用 CPU 运行（最佳兼容性）")
            }
            
            // 创建推理 session
            session = env.createSession(modelFile.absolutePath, sessionOptions)
            
            // 打印模型信息
            session?.let { s ->
                println("✅ Kokoro ONNX 模型加载成功")
                println("输入:")
                s.inputNames.forEach { name ->
                    println("  - $name: ${s.inputInfo[name]?.info}")
                }
                println("输出:")
                s.outputNames.forEach { name ->
                    println("  - $name: ${s.outputInfo[name]?.info}")
                }
            }
            
        } catch (e: Exception) {
            throw RuntimeException("模型加载失败: ${e.message}", e)
        }
    }
    
    /**
     * 文本转语音推理
     * 
     * @param inputIds 音素 token IDs (包含 BOS 和 EOS)
     * @param voiceEmbedding 语音嵌入向量 [256]
     * @param speed 语速，默认 1.0
     * @return 音频波形数据 (Float array, 24kHz 采样率)
     */
    suspend fun synthesize(
        inputIds: LongArray,
        voiceEmbedding: FloatArray,
        speed: Double = 1.0
    ): FloatArray = withContext(Dispatchers.Default) {
        val currentSession = session ?: throw IllegalStateException("模型未初始化，请先调用 initialize()")
        
        try {
            // 准备输入张量
            val inputIdsShape = longArrayOf(1, inputIds.size.toLong())
            val voiceShape = longArrayOf(1, 256)
            val speedShape = longArrayOf()
            
            val inputIdsTensor = OnnxTensor.createTensor(
                env,
                LongBuffer.wrap(inputIds),
                inputIdsShape
            )
            
            val voiceTensor = OnnxTensor.createTensor(
                env,
                FloatBuffer.wrap(voiceEmbedding),
                voiceShape
            )
            
            val speedTensor = OnnxTensor.createTensor(
                env,
                doubleArrayOf(speed)
            )
            
            // 执行推理
            val inputs = mapOf(
                "input_ids" to inputIdsTensor,
                "ref_s" to voiceTensor,
                "speed" to speedTensor
            )
            
            val outputs = currentSession.run(inputs)
            
            // 获取输出（第一个输出是 waveform）
            val waveform = when (val value = outputs[0].value) {
                is FloatArray -> value  // 一维数组
                is Array<*> -> {
                    // 二维数组，取第一个
                    @Suppress("UNCHECKED_CAST")
                    (value as Array<FloatArray>)[0]
                }
                else -> throw RuntimeException("不支持的输出类型: ${value?.javaClass}")
            }
            
            // 清理资源
            inputIdsTensor.close()
            voiceTensor.close()
            speedTensor.close()
            outputs.close()
            
            waveform
            
        } catch (e: Exception) {
            throw RuntimeException("推理失败: ${e.message}", e)
        }
    }
    
    /**
     * 释放资源
     */
    fun release() {
        session?.close()
        session = null
    }
}

/**
 * 音素词汇表
 * 从 Python 词汇表转换而来
 */
object KokoroVocab {
    // 这里需要完整的词汇表映射
    // 可以从 Python 导出: python -c "from kokoro import KPipeline; p = KPipeline('j'); print(p.model.vocab)"
    val vocab = mapOf(
        ';' to 1,
        ':' to 2,
        ',' to 3,
        '.' to 4,
        '!' to 5,
        '?' to 6,
        '—' to 9,
        '…' to 10,
        // ... 更多映射
        // 日文音素
        'k' to 53,
        'o' to 57,
        'ɲ' to 114,
        'i' to 51,
        'ʨ' to 21,
        'β' to 75,
        'a' to 43,
        // TODO: 添加完整的词汇表
    )
    
    /**
     * 将音素字符串转换为 input_ids
     */
    fun phonemesToIds(phonemes: String): LongArray {
        val ids = mutableListOf<Long>(0) // BOS
        for (char in phonemes) {
            vocab[char]?.let { ids.add(it.toLong()) }
        }
        ids.add(0) // EOS
        return ids.toLongArray()
    }
}
