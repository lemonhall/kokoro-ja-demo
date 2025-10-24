"""
导出语音嵌入数据供 Android 使用
"""

from kokoro import KPipeline
import numpy as np
import struct
import os

# 初始化
print("初始化 Kokoro...")
pipeline = KPipeline(lang_code='j')

# 可用的日文语音列表
available_voices = ['jf_nezumi', 'jf_hanako', 'jm_shinji']

# 创建输出目录
os.makedirs('app/src/main/assets', exist_ok=True)
os.makedirs('models', exist_ok=True)

print("\n导出语音嵌入数据...")

for voice_name in available_voices:
    try:
        # 加载语音嵌入
        print(f"\n处理语音: {voice_name}")
        voices_tensor = pipeline.load_voice(voice_name)  # [510, 1, 256]
        print(f"  张量形状: {voices_tensor.shape}")
        
        # 转换为 numpy
        voices_data = voices_tensor.numpy()
        
        # 1. 保存为 .npy 格式（包含完整数据）
        npy_file = f'models/{voice_name}.npy'
        np.save(npy_file, voices_data)
        print(f"  ✅ 已保存 NumPy 格式: {npy_file}")
        
        # 2. 保存为自定义二进制格式（只保存第一个嵌入，供 Android 使用）
        # 提取第一个嵌入 [256]
        first_embedding = voices_data[0, 0, :].astype(np.float32)
        
        bin_file = f'app/src/main/assets/{voice_name}.bin'
        with open(bin_file, 'wb') as f:
            # 写入维度
            f.write(struct.pack('i', 256))
            # 写入数据
            f.write(first_embedding.tobytes())
        
        file_size = os.path.getsize(bin_file) / 1024
        print(f"  ✅ 已保存二进制格式: {bin_file} ({file_size:.2f} KB)")
        
        # 3. 验证数据
        print(f"  数据范围: [{first_embedding.min():.4f}, {first_embedding.max():.4f}]")
        print(f"  平均值: {first_embedding.mean():.4f}")
        
    except Exception as e:
        print(f"  ❌ 失败: {e}")

# 创建一个加载器类的 Kotlin 代码
kotlin_loader = """package com.lsl.kokoro_ja_android

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
        "jf_nezumi",  // 女声
        "jf_hanako",  // 女声
        "jm_shinji"   // 男声
    )
}
"""

# 保存 Kotlin 代码
loader_file = 'app/src/main/java/com/lsl/kokoro_ja_android/VoiceEmbeddingLoader.kt'
with open(loader_file, 'w', encoding='utf-8') as f:
    f.write(kotlin_loader)

print(f"\n✅ Kotlin 加载器已生成: {loader_file}")

print("\n" + "="*60)
print("导出完成!")
print("="*60)

print("\n📁 生成的文件:")
print("  Python/测试用:")
for voice in available_voices:
    npy_file = f'models/{voice}.npy'
    if os.path.exists(npy_file):
        size = os.path.getsize(npy_file) / 1024
        print(f"    - {npy_file} ({size:.2f} KB)")

print("\n  Android 使用:")
for voice in available_voices:
    bin_file = f'app/src/main/assets/{voice}.bin'
    if os.path.exists(bin_file):
        size = os.path.getsize(bin_file) / 1024
        print(f"    - {bin_file} ({size:.2f} KB)")

print(f"\n  Kotlin 代码:")
print(f"    - {loader_file}")

print("\n📝 在 Android 中使用:")
print("""
// 加载语音嵌入
val voiceEmbedding = VoiceEmbeddingLoader.load(context, "jf_nezumi")

// 或使用其他语音
val hanako = VoiceEmbeddingLoader.load(context, "jf_hanako")
val shinji = VoiceEmbeddingLoader.load(context, "jm_shinji")
""")
