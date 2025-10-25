"""
导出中文语音嵌入数据供 Android 使用

基于 export_voices_for_android.py，修改为支持中文语音
关键改动：
1. lang_code='z' (中文)
2. 导出 zf_xiaoxiao 等中文音色
3. 保留完整 510 帧嵌入数据，支持动态帧选择
"""

from kokoro import KPipeline
import numpy as np
import struct
import os

# 初始化中文 Pipeline
print("初始化 Kokoro (中文模式)...")
pipeline = KPipeline(lang_code='z')  # 'z' = 中文

# 可用的中文语音列表
# 注意：具体可用音色需要查看 kokoro 文档或测试
available_voices = ['zf_xiaoxiao']  # 先导出主要的中文女声

# 创建输出目录
os.makedirs('app/src/main/assets/voices', exist_ok=True)
os.makedirs('models/chinese', exist_ok=True)

print("\n导出中文语音嵌入数据...")
print("=" * 60)

for voice_name in available_voices:
    try:
        # 加载语音嵌入
        print(f"\n处理中文语音: {voice_name}")
        voices_tensor = pipeline.load_voice(voice_name)
        print(f"  📊 张量形状: {voices_tensor.shape}")
        
        # 验证形状是否符合预期 [510, 1, 256]
        expected_shape = (510, 1, 256)
        if voices_tensor.shape != expected_shape:
            print(f"  ⚠️  警告: 形状不匹配! 预期 {expected_shape}, 实际 {voices_tensor.shape}")
        
        # 转换为 numpy
        voices_data = voices_tensor.numpy()
        
        # 1. 保存为 .npy 格式（用于 Python 测试和验证）
        npy_file = f'models/chinese/{voice_name}.npy'
        np.save(npy_file, voices_data)
        npy_size = os.path.getsize(npy_file) / 1024
        print(f"  ✅ NumPy 格式: {npy_file} ({npy_size:.2f} KB)")
        
        # 2. 保存为二进制格式（供 Android 使用）
        # 关键：保存完整 510 帧嵌入，支持动态帧选择！
        all_embeddings = voices_data[:, 0, :].astype(np.float32)  # [510, 256]
        
        bin_file = f'app/src/main/assets/voices/{voice_name}.bin'
        with open(bin_file, 'wb') as f:
            # 写入元数据
            f.write(struct.pack('i', 256))  # embedding_dim
            f.write(struct.pack('i', 510))  # num_frames
            
            # 写入所有 510 帧的嵌入数据
            f.write(all_embeddings.tobytes())
        
        bin_size = os.path.getsize(bin_file) / 1024
        print(f"  ✅ 二进制格式: {bin_file} ({bin_size:.2f} KB)")
        print(f"     包含 {all_embeddings.shape[0]} 帧嵌入数据 (支持动态帧选择)")
        
        # 3. 数据验证
        print(f"  📈 数据统计:")
        print(f"     值域: [{all_embeddings.min():.4f}, {all_embeddings.max():.4f}]")
        print(f"     均值: {all_embeddings.mean():.4f}")
        print(f"     标准差: {all_embeddings.std():.4f}")
        
        # 4. 验证不同帧之间的差异
        frame_0 = all_embeddings[0, :]
        frame_10 = all_embeddings[10, :]
        frame_50 = all_embeddings[50, :]
        
        diff_0_10 = np.mean(np.abs(frame_0 - frame_10))
        diff_0_50 = np.mean(np.abs(frame_0 - frame_50))
        
        print(f"  📊 帧间差异 (验证韵律变化):")
        print(f"     Frame 0 vs Frame 10: {diff_0_10:.4f}")
        print(f"     Frame 0 vs Frame 50: {diff_0_50:.4f}")
        print(f"     ✅ 长句韵律差异明显: {diff_0_50 > diff_0_10}")
        
    except Exception as e:
        print(f"  ❌ 导出失败: {e}")
        import traceback
        traceback.print_exc()

# 更新 VoiceEmbeddingLoader.kt 添加中文音色
kotlin_update = """
// 在 VoiceEmbeddingLoader.kt 中添加中文音色支持

/**
 * 获取可用的语音列表
 */
object AvailableVoices {
    // 日文音色
    val japanese = listOf(
        "jf_nezumi",  // 女声
        "jf_hanako",  // 女声
        "jm_shinji"   // 男声
    )
    
    // 中文音色
    val chinese = listOf(
        "zf_xiaoxiao"  // 女声
    )
    
    // 所有音色
    val all = japanese + chinese
}

/**
 * 音色语言映射
 */
fun getLanguage(voiceName: String): String {
    return when (voiceName) {
        in AvailableVoices.japanese -> "ja"
        in AvailableVoices.chinese -> "zh"
        else -> throw IllegalArgumentException("未知音色: $voiceName")
    }
}
"""

print("\n" + "=" * 60)
print("📝 Kotlin 代码更新建议")
print("=" * 60)
print(kotlin_update)

print("\n" + "=" * 60)
print("✅ 导出完成!")
print("=" * 60)

# 输出文件列表
print("\n📁 生成的文件:")

print("\n  Python/测试用 (models/chinese/):")
for voice in available_voices:
    npy_file = f'models/chinese/{voice}.npy'
    if os.path.exists(npy_file):
        size = os.path.getsize(npy_file) / 1024
        print(f"    ✅ {voice}.npy ({size:.2f} KB)")

print("\n  Android 使用 (app/src/main/assets/voices/):")
for voice in available_voices:
    bin_file = f'app/src/main/assets/voices/{voice}.bin'
    if os.path.exists(bin_file):
        size = os.path.getsize(bin_file) / 1024
        print(f"    ✅ {voice}.bin ({size:.2f} KB)")

# 测试验证
print("\n" + "=" * 60)
print("🧪 Python 端验证测试")
print("=" * 60)

try:
    test_text = "中文测试，长句朗读"
    
    print(f"\n测试文本: {test_text}")
    
    # 使用 Pipeline 生成音频
    result = list(pipeline(test_text, voice='zf_xiaoxiao', speed=1))
    
    if result:
        gs, ps, audio = result[0]
        phoneme_length = len(ps)
        audio_duration = len(audio) / 24000.0
        
        print(f"  文本: {gs}")
        print(f"  音素: {ps}")
        print(f"  音素长度: {phoneme_length}")
        print(f"  音频时长: {audio_duration:.2f} 秒")
        print(f"  应选择帧: {phoneme_length - 1}")
        
        # 验证动态帧选择
        voices_tensor = pipeline.load_voice('zf_xiaoxiao')
        frame_index = phoneme_length - 1
        selected_embedding = voices_tensor[frame_index, 0, :]
        
        print(f"  ✅ 动态帧选择验证:")
        print(f"     选择的帧: {frame_index} (共 {voices_tensor.shape[0]} 帧)")
        print(f"     嵌入维度: {selected_embedding.shape}")
        
        # 保存测试音频
        import soundfile as sf
        test_audio_file = 'models/chinese/test_xiaoxiao.wav'
        sf.write(test_audio_file, audio, 24000)
        print(f"\n  💾 测试音频已保存: {test_audio_file}")
        
    else:
        print("  ⚠️  未生成音频")
        
except Exception as e:
    print(f"\n  ❌ 测试失败: {e}")
    import traceback
    traceback.print_exc()

# 使用说明
print("\n" + "=" * 60)
print("📖 Android 端使用说明")
print("=" * 60)

print("""
1. 加载中文语音嵌入:
   val voiceEmbedding = VoiceEmbeddingLoader.load(context, "zf_xiaoxiao")

2. 根据音素长度动态选择帧:
   val phonemeLength = inputIds.size - 2  // 去掉 BOS 和 EOS
   val selectedEmbedding = voiceEmbedding.getFrameByPhonemeLength(phonemeLength)

3. 完整推理流程:
   // 中文文本 → 拼音 → IPA 音素
   val phonemes = chineseG2P.textToPhonemes("中文测试")
   
   // 音素 → input_ids
   val inputIds = KokoroVocabFull.phonemesToIds(phonemes)
   
   // 加载中文语音嵌入并动态选择帧
   val voiceEmbedding = VoiceEmbeddingLoader.load(context, "zf_xiaoxiao")
   val phonemeLength = inputIds.size - 2
   val embedding = voiceEmbedding.getFrameByPhonemeLength(phonemeLength)
   
   // TTS 推理
   val waveform = engine.synthesize(inputIds, embedding, speed = 1.0)

⚠️ 注意事项:
   - 必须保留完整 510 帧数据
   - 必须根据音素长度动态选择帧
   - 短句用第一帧，长句用对应帧
   - 这是音质的关键！
""")

print("\n🎉 全部完成!")
print("\n下一步:")
print("  1. ✅ 中文语音嵌入已导出")
print("  2. ⏳ 创建 ChinesePinyinToIPA.kt")
print("  3. ⏳ 集成 TinyPinyin 库")
print("  4. ⏳ 实现 ChineseG2PSystem.kt")
print("  5. ⏳ 修改 MainActivity 支持双语言")
print()
