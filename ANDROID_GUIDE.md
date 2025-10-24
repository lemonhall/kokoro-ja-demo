# Kokoro TTS Android 集成指南

## 📱 项目结构

```
app/
├── src/main/
│   ├── java/com/lsl/kokoro_ja_android/
│   │   ├── MainActivity.kt           # 主界面
│   │   ├── KokoroEngine.kt          # ONNX 推理引擎
│   │   └── KokoroVocabFull.kt       # 音素词汇表（自动生成）
│   ├── res/
│   │   └── layout/activity_main.xml # 布局文件
│   └── assets/                       # 模型文件存放位置
│       ├── kokoro_latest_int8.onnx  # INT8 量化模型 (~109MB)
│       └── voices.bin               # 语音嵌入数据（可选）
└── build.gradle.kts                  # 依赖配置
```

## 🚀 快速开始

### 1. 准备模型文件

```bash
# 1. 导出 ONNX 模型
uv run python export_onnx.py

# 2. 量化为 INT8（推荐 Android 使用）
uv run python quantize_int8.py kokoro_latest.onnx

# 3. 导出词汇表
uv run python export_vocab_for_android.py

# 4. 导出语音嵌入数据
uv run python export_voices_for_android.py
```

✅ **已完成！**现在你应该有以下文件：
- `kokoro_latest_int8.onnx` (~109MB) - 量化模型
- `app/src/main/assets/jf_nezumi.bin` (1KB) - 语音嵌入
- `app/src/main/java/.../KokoroVocabFull.kt` - 词汇表
- `app/src/main/java/.../VoiceEmbeddingLoader.kt` - 语音加载器

### 2. 复制模型到 Android 项目

✅ **语音嵌入已自动导出**到 `app/src/main/assets/jf_nezumi.bin`

**现在只需复制 ONNX 模型：**

```bash
# 复制 INT8 模型（推荐）
cp kokoro_latest_int8.onnx app/src/main/assets/
```

**文件清单检查：**
```
app/src/main/assets/
├── kokoro_latest_int8.onnx  ✅ (需要手动复制)
└── jf_nezumi.bin            ✅ (已自动生成)
```

⚠️ **注意**: 
- INT8 模型 (~109MB) 通过 NNAPI 在 Android 上可以正常运行
- FP32 模型 (~310MB) 也可以用，但体积更大
- **不要将模型文件提交到 git**（已在 .gitignore 中排除）

### 3. 同步 Gradle 依赖

在 Android Studio 中点击 "Sync Project with Gradle Files"，会自动下载：
- ONNX Runtime Android (1.17.1)
- Kotlin Coroutines

### 4. 运行应用

1. 连接 Android 设备或启动模拟器
2. 点击 Run 按钮
3. 应用会自动加载模型
4. 点击"测试语音合成"按钮生成并播放语音

## 🔧 核心 API

### KokoroEngine

```kotlin
// 初始化
val engine = KokoroEngine(context)
engine.initialize("kokoro_latest_int8.onnx")

// 加载语音嵌入（现在使用真实数据！）
val voiceEmbedding = VoiceEmbeddingLoader.load(context, "jf_nezumi")

// 推理
val inputIds = KokoroVocabFull.phonemesToIds("koɲɲiʨiβa") // "こんにちは"
val waveform = engine.synthesize(inputIds, voiceEmbedding, speed = 1.0)

// 释放资源
engine.release()
```

### 音素转换

```kotlin
// 使用自动生成的词汇表
val phonemes = "koɲɲiʨiβa"  // 从日文 g2p 转换得到
val inputIds = KokoroVocabFull.phonemesToIds(phonemes)
// 结果: [0, 53, 57, 114, 114, 51, 21, 51, 75, 43, 0]
```

## 📊 性能优化

### 1. 使用 NNAPI 加速

```kotlin
val sessionOptions = OrtSession.SessionOptions().apply {
    addNnapi()  // 启用 Android Neural Networks API
}
```

INT8 量化模型在 NNAPI 上有最佳性能。

### 2. 预期性能

| 设备类型 | 模型 | 推理时间 | 备注 |
|---------|------|---------|------|
| 高端 (Snapdragon 8 Gen 3) | INT8 | ~0.5s | 实时合成 |
| 中端 (Snapdragon 778G) | INT8 | ~1-2s | 可接受 |
| 低端设备 | FP32 | ~3-5s | 较慢 |

### 3. 内存优化

- INT8 模型: ~150MB 内存占用
- FP32 模型: ~350MB 内存占用

## 🎤 语音嵌入

### ✅ 已完成！

语音嵌入已经导出并自动集成：

```kotlin
// 加载 jf_nezumi 语音（默认）
val voiceEmbedding = VoiceEmbeddingLoader.load(context)

// 或明确指定
val voiceEmbedding = VoiceEmbeddingLoader.load(context, "jf_nezumi")
```

### 可用语音

当前 Kokoro-82M 模型只包含：
- `jf_nezumi` - 女声 ✅

如需更多语音，请：
1. 使用更新的 Kokoro 模型版本
2. 或自己训练语音嵌入

## 📝 待完成事项

- [x] 实现完整的语音嵌入加载 ✅
- [x] 导出完整词汇表 ✅
- [ ] 添加日文文本的 G2P 转换（MeCab Android 版本）
- [ ] 支持多种语音选择
- [ ] 实现流式合成
- [ ] 添加音频保存功能
- [ ] 性能监控和优化

## 🐛 常见问题

### Q: 模型加载失败？

A: 检查：
1. 模型文件是否在 `assets/` 目录
2. 文件名是否正确
3. APK 是否正确打包了 assets（检查 APK 大小）

### Q: NNAPI 不可用？

A: 
- Android 8.1+ 才支持 NNAPI
- 某些设备可能不支持，会自动回退到 CPU
- INT8 操作需要硬件支持

### Q: 生成的语音是噪音？

A: 
- ✅ **已解决！**现在使用真实的语音嵌入
- 确认 `jf_nezumi.bin` 在 `assets/` 目录
- 验证 input_ids 转换正确

## 🔗 参考资源

- [ONNX Runtime Android 文档](https://onnxruntime.ai/docs/tutorials/mobile/android.html)
- [Android NNAPI 指南](https://developer.android.com/ndk/guides/neuralnetworks)
- [Kokoro TTS 官方](https://github.com/hexgrad/Kokoro-TTS)
