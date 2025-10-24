# Kokoro 日文语音合成演示

这是一个使用 Kokoro TTS 引擎进行日文文本转语音的演示项目。

> ⚠️ **项目状态**: 技术验证阶段，**移动端实用性较差**，仅供学习参考。

## 📸 实际运行效果

![Android 运行效果 - 性能问题](slow_fail.jpg)

**问题一目了然**：
- 6.3 秒音频需要 10 秒推理 (RTF 1.62x)
- 音质：ONNX 版本比 PyTorch 低沉浑厚
- 结论：**不适合实时语音合成**

## 📊 项目成果总结

### ✅ 已实现功能

- **PyTorch 原生版本** - 在 PC 上完美运行，音质优秀
- **ONNX 模型导出** - 成功将 82M 参数模型转换为 ONNX 格式
- **Android 应用集成** - 完整的 Android TTS 应用，支持自定义文本输入
- **简化 G2P 转换** - 基于规则的假名到音素映射，无需 MeCab

### ❌ 存在问题

#### 1. **音质问题** (严重)
- ONNX 模型声音**比 PyTorch 低沉浑厚**，音色不自然
- 原因：ONNX 导出时的数值精度损失或算子实现差异
- 状态：**未解决**

#### 2. **性能问题** (严重)
- **FP32 模型** (310MB): RTF ~1.6x，推理10秒生成6秒音频
- **INT8 模型** (109MB): ConvInteger 算子不支持，无法加载
- **NNAPI 加速**: 只支持 7% 节点 (233/3348)，加速效果极微
- 结论：**移动端推理速度不可用**

#### 3. **G2P 准确度问题** (中等)
- 简化版 G2P 只支持假名直接映射，无法处理汉字
- 缺少重音、特殊读音等高级功能
- 适用场景：仅限假名输入

### 💡 技术总结

| 技术点 | 结论 | 说明 |
|---------|------|------|
| **ONNX 导出** | ✅ 可行 | 模型能跑，但音质有问题 |
| **INT8 量化** | ❌ 不可用 | ConvInteger 算子在 ONNX Runtime Android 版不支持 |
| **FP16 转换** | ❌ 不可用 | 类型不匹配错误 (tensor(float16) vs tensor(float)) |
| **FP32 模型** | ⚠️ 能用但慢 | 310MB，推理10秒，RTF 1.6x |
| **NNAPI 加速** | ⚠️ 效果极微 | 仅 7% 节点被加速，大部分还是 CPU |
| **简化 G2P** | ✅ 勉强可用 | 只支持假名，准确度不如 MeCab |

### 🎯 最终评价

**原型阶段 (Prototype)** - 成功证明了技术路线可行，但离实用还有距离。

**主要阻碍**：
1. ONNX 模型音质不佳（需要优化导出流程）
2. 移动端推理性能不足（需要更小的模型或更强的芯片）
3. 量化算子兼容性问题（ONNX Runtime 的限制）

**适用场景**：
- ✅ PC 端学习和实验
- ✅ ONNX 导出流程参考
- ❌ 生产环境移动端应用

## 环境要求

- Python 3.13+
- uv (推荐的包管理器)

## 安装步骤

### 1. 安装依赖

```bash
uv sync
```

### 2. 下载 UniDic 日文字典（必须！）

```bash
# 安装 unidic 包
uv add unidic

# 下载字典数据（约 526MB）
uv run python -m unidic download
```

## 快速开始

### PyTorch 版本（原始演示）

```bash
# 运行原始示例
uv run python ja.py

# 或简单示例
uv run python example_simple.py
```

### ONNX 版本（移动端部署）

```bash
# 1. 导出 ONNX 模型
uv run python export_onnx.py

# 2. 量化模型（可选）
uv run python quantize_int8.py kokoro_latest.onnx

# 3. 测试 ONNX 模型
uv run python test_onnx.py
```

## 项目文件说明

### Python 脚本

| 文件 | 说明 | 状态 |
|------|------|------|
| `ja.py` | 原始 PyTorch 示例（官方演示） | ✅ 工作正常 |
| `example_simple.py` | 简化版使用示例 | ✅ 工作正常 |
| `export_onnx.py` | 导出最新 Kokoro 模型到 ONNX | ✅ 工作正常 |
| `quantize_int8.py` | INT8 量化工具 (310MB → 109MB) | ⚠️ 生成成功，但不可用 |
| `convert_fp16.py` | FP16 转换工具 (310MB → 155MB) | ❌ 类型错误 |
| `test_onnx.py` | ONNX 模型测试脚本 | ✅ 工作正常 |
| `generate_japanese_presets.py` | 生成预设句子的 G2P 数据 | ✅ 工作正常 |

### Android 应用

| 文件 | 说明 | 状态 |
|------|------|------|
| `app/src/main/java/.../MainActivity.kt` | 主界面 | ✅ 功能完整 |
| `app/src/main/java/.../KokoroEngine.kt` | ONNX 推理引擎 | ✅ 能运行 |
| `app/src/main/java/.../SimpleJapaneseG2P.kt` | 简化 G2P 转换器 | ⚠️ 只支持假名 |
| `app/src/main/java/.../JapanesePresets.kt` | 16 个预设句子 | ✅ 自动生成 |
| `app/src/main/java/.../VoiceEmbeddingLoader.kt` | 语音嵌入加载器 | ✅ 工作正常 |
| `app/src/main/java/.../KokoroVocabFull.kt` | 完整词汇表 (206 个音素) | ✅ 完整 |
| `app/src/main/assets/kokoro_fp32.onnx` | FP32 模型 (310MB) | ⚠️ 慢 |
| `app/src/main/assets/jf_nezumi.bin` | 女声嵌入 | ✅ 工作正常 |

## 文档

- [README.md](README.md) - 本文件
- [导出到onnx.md](导出到onnx.md) - ONNX 导出和量化技术文档
- [MOBILE_PORTING.md](MOBILE_PORTING.md) - 移动端移植指南

## 常见问题

### MeCab 初始化失败

错误信息：
```
RuntimeError: Failed initializing MeCab
param.cpp(69) [ifs] no such file or directory: ...mecabrc
```

**解决方案：** 确保已经执行 `uv run python -m unidic download` 下载字典。

### ONNX 模型太大

原始 ONNX 模型约 310MB，使用量化可以减小：
- **INT8 量化**: ~109MB（⚠️ ConvInteger 算子不支持，不可用）
- **FP16 转换**: ~155MB（❌ 类型错误，不可用）
- **FP32 原始**: 310MB（✅ 可用，但慢）

### Android 上 ConvInteger 错误

错误信息：
```
Error code - ORT_NOT_IMPLEMENTED - Could not find an implementation for ConvInteger(10)
```

**原因：** ONNX Runtime 1.17.1 的 Android 版本不支持 INT8 量化模型的 ConvInteger 算子。

**解决方案：** 使用 FP32 原始模型，虽然慢但能用。

### Android 推理太慢

**现象：** 6 秒音频需要 10 秒推理 (RTF 1.6x)

**原因：**
1. FP32 模型太大 (310MB)
2. NNAPI 只支持 7% 节点，大部分还是 CPU 计算
3. 82M 参数模型对移动端过大

**结论：** 当前方案不适合生产环境使用。

### ONNX 音质不好

**现象：** ONNX 模型声音比 PyTorch 低沉浑厚

**原因：** ONNX 导出时的数值精度损失或算子实现差异

**状态：** 未解决，需要深入优化导出流程

## 技术栈

- **TTS 引擎**: Kokoro-82M
- **日文分词**: MeCab + UniDic
- **音素转换**: Misaki (JAG2P)
- **模型导出**: ONNX Runtime
- **音频处理**: SoundFile

## 许可证

- Kokoro TTS: Apache 2.0
- 本项目：仅供学习和研究使用

---

## 🛤️ 开发经验总结

### 成功经验

1. **ONNX 导出流程**
   - 使用 `torch.onnx.export` 成功导出 82M 参数模型
   - 需要精心处理 `dynamic_axes` 和 `opset_version`
   - 语音嵌入必须使用真实数据，不能随机生成

2. **Android ONNX Runtime 集成**
   - 使用 `ai.onnxruntime:onnxruntime-android:1.17.1`
   - 需要将模型从 assets 复制到缓存目录
   - 音频播放使用 `AudioTrack` + `MODE_STATIC` 模式

3. **简化 G2P 方案**
   - 基于规则的假名到音素映射
   - 无需 MeCab 等重型依赖
   - 适合轻量级应用

### 失败教训

1. **INT8 量化不可用**
   - `onnxruntime.quantization.quantize_dynamic` 生成的模型包含 ConvInteger
   - ONNX Runtime Android 版不支持 ConvInteger 算子
   - 教训：移动端优先验证算子兼容性

2. **FP16 转换失败**
   - `onnxconverter_common.float16.convert_float_to_float16` 产生类型错误
   - 节点输出声明为 `tensor(float16)` 但运行时期望 `tensor(float)`
   - 教训：需要更细致的类型一致性处理

3. **NNAPI 加速效果有限**
   - 虽然启用了 NNAPI，但只有 7% 节点被支持
   - 大部分计算仍然在 CPU 上执行
   - 教训：不要过分依赖硬件加速，模型大小是关键

4. **ONNX 音质问题难解决**
   - ONNX 和 PyTorch 的音色差异明显
   - 可能是数值精度、算子实现或导出配置问题
   - 教训：需要更深入的模型导出优化知识

### 性能数据

**测试环境：**
- 设备：华为 RTE-AL00 (Android 12)
- 芯片：高通 (支持 NNAPI)
- 模型：FP32 (310MB)

**性能表现：**
- 短句 (2-3秒)：推理 ~1-2秒
- 长句 (6-7秒)：推理 ~10秒
- RTF (Real-Time Factor): ~1.6x
- NNAPI 节点支持率：7% (233/3348)

**结论：** 不符合实时语音合成要求 (RTF < 1.0)
