# Kokoro TTS 导出到 ONNX 完整指南

## ✅ 已完成 - 最新版本导出成功！

本项目已成功将 Kokoro 0.9.4 导出为 ONNX 格式，并完成量化优化。

## 快速开始

### 1. 导出最新模型

```bash
# 导出 ONNX 模型（~310MB）
uv run python export_onnx.py

# 指定不同的模型仓库
uv run python export_onnx.py --repo-id hexgrad/Kokoro-82M-v1.1-zh
```

### 2. 量化模型

```bash
# INT8 量化（推荐移动端）- 压缩到 ~109MB
uv run python quantize_int8.py kokoro_latest.onnx

# FP16 转换（中端设备）- 压缩到 ~155MB
uv run python convert_fp16.py kokoro_latest.onnx
```

## 文件说明

| 脚本 | 功能 | 输出 |
|------|------|------|
| `export_onnx.py` | 导出最新 Kokoro 模型到 ONNX | `kokoro_latest.onnx` (~310MB) |
| `quantize_int8.py` | INT8 动态量化 | `kokoro_latest_int8.onnx` (~109MB) |
| `convert_fp16.py` | FP16 半精度转换 | `kokoro_latest_fp16.onnx` (~155MB) |

## 技术细节

### 导出过程中的关键点

1. **使用内置的 ONNX 包装器**
   ```python
   from kokoro.model import KModel, KModelForONNX
   
   kmodel = KModel(repo_id='hexgrad/Kokoro-82M', disable_complex=True)
   onnx_model = KModelForONNX(kmodel)
   ```

2. **`disable_complex=True` 必须设置**
   - STFT 层默认使用复数运算
   - ONNX 不支持复数类型
   - 设置此参数使用实数版本的 STFT

3. **LSTM 层限制**
   - 不能使用 `dynamo=True` 导出
   - 必须使用传统的 TorchScript 方式
   - 会有一些 PackPadded/PadPacked 警告，可以忽略

4. **InstanceNorm1d 警告**
   - 会看到 `train=True` 的警告
   - 这是正常的，不影响推理

### 模型输入输出

**输入:**
- `input_ids`: `[batch, seq_length]` (int64) - 音素 token IDs，包含 BOS(0) 和 EOS(0)
- `ref_s`: `[batch, 256]` (float32) - 语音参考嵌入（128维韵律 + 128维内容）
- `speed`: scalar (float/double) - 语速控制，默认 1.0

**输出:**
- `waveform`: `[time]` (float32) - 生成的音频波形，24kHz 采样率
- `duration`: `[seq_length]` (int64) - 每个音素的持续时间（帧数）

### 量化效果对比

| 版本 | 大小 | 压缩率 | 速度 | 质量 | 推荐场景 |
|------|------|--------|------|------|----------|
| FP32 原始 | 310 MB | - | 基准 | 最高 | 服务器/开发 |
| FP16 | ~155 MB | 50% | 1.5-2x* | 很高 | GPU/NPU设备 |
| INT8 | ~109 MB | 65% | 2-4x | 良好 | **移动端（推荐）** |

*FP16 在 CPU 上可能没有加速，需要支持 FP16 的硬件

## 移动端集成

### Android 示例

```kotlin
import ai.onnxruntime.*

// 加载模型
val env = OrtEnvironment.getEnvironment()
val session = env.createSession("kokoro_latest_int8.onnx")

// 准备输入
val inputIds = longArrayOf(0, 23, 45, 67, 0) // BOS + 音素 + EOS
val refS = FloatArray(256) { /* 语音嵌入 */ }
val speed = 1.0

// 推理
val inputs = mapOf(
    "input_ids" to OrtUtil.reshape(inputIds, longArrayOf(1, -1)),
    "ref_s" to OrtUtil.reshape(refS, longArrayOf(1, 256)),
    "speed" to doubleArrayOf(speed)
)
val outputs = session.run(inputs)
val waveform = outputs[0].value as FloatArray
```

### 所需文件（移动端）

```
app/src/main/assets/
├── kokoro_latest_int8.onnx    # INT8 量化模型 (~109MB)
├── voices.bin                  # 语音嵌入数据
└── unidic-lite/               # 日文词典（可选，用于文本处理）
```

## 性能测试

### 桌面端 (Windows CPU)
- **FP32**: ~3-5s/句
- **INT8**: ~1-2s/句 (2-3x 加速)

### 预期移动端性能
- **中端手机 (Snapdragon 8 Gen 1)**: ~1-2s/句
- **高端手机 (Snapdragon 8 Gen 3)**: ~0.5-1s/句
- **内存占用**: ~150-200MB

## 常见问题

### Q: 导出时提示 "complex numbers not supported"？
**A:** 确保使用 `disable_complex=True` 创建模型

### Q: 量化后质量下降明显？
**A:** Kokoro 对量化很鲁棒，INT8 基本不会影响质量。如果有问题，可以尝试：
- 使用 FP16 代替 INT8
- 排除敏感层（需要实验确定）

### Q: 在手机上运行很慢？
**A:** 检查：
- 是否使用了 INT8 量化版本
- ONNX Runtime 是否启用了硬件加速
- 是否使用了 NNAPI/CoreML 等移动端优化

### Q: 如何获取 `ref_s` 语音嵌入？
**A:** 从预训练的语音文件中提取，或使用 Kokoro 提供的默认语音。参考 `voices.bin` 文件。

## 下一步计划

- [ ] 创建 Android 演示 App
- [ ] 添加 iOS 支持和 Core ML 转换
- [ ] 优化文本预处理管线（MeCab 移动端版本）
- [ ] 测试不同硬件上的性能
- [ ] 实现流式生成

## 参考资源

- [Kokoro GitHub](https://github.com/hexgrad/Kokoro-TTS)
- [ONNX Runtime Mobile](https://onnxruntime.ai/docs/tutorials/mobile/)
- [Android ONNX 集成指南](https://onnxruntime.ai/docs/tutorials/mobile/android.html)
- [移动端量化最佳实践](https://onnxruntime.ai/docs/performance/model-optimizations/quantization.html)
