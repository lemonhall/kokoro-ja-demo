# Kokoro TTS 移动端移植指南

## 概述

将 Kokoro TTS 移植到手机需要以下步骤：
1. 导出为 ONNX 格式
2. 量化模型以减小体积和提升速度
3. 集成到移动应用

## 方案选择

### 推荐方案：使用现成的 ONNX 模型

**最简单的方式是直接使用已经转换好的模型：**

```bash
# 安装 kokoro-onnx
pip install kokoro-onnx
```

**模型下载：**
- 完整模型：`kokoro-v1.0.onnx` (~326MB)
- 量化模型：`kokoro-v1.0-int8.onnx` (~80MB)
- 语音数据：`voices-v1.0.bin`

下载地址：
- GitHub: https://github.com/thewh1teagle/kokoro-onnx
- Hugging Face: https://huggingface.co/onnx-community/Kokoro-82M-v1.0-ONNX
- INT8 量化版: https://huggingface.co/NeuML/kokoro-int8-onnx

## 模型对比

| 版本 | 大小 | 速度 | 质量 | 推荐场景 |
|------|------|------|------|----------|
| PyTorch 原版 | ~326MB | 慢 | 最高 | 服务器/开发 |
| ONNX FP32 | ~326MB | 中等 | 最高 | 高端设备 |
| ONNX FP16 | ~163MB | 快 | 高 | 中端设备 |
| ONNX INT8 | ~80MB | 最快 | 良好 | 移动端/嵌入式 |

## 移动端集成方案

### Android 集成

**依赖库：**
```gradle
dependencies {
    implementation 'com.microsoft.onnxruntime:onnxruntime-android:latest'
}
```

**示例项目参考：**
- https://github.com/isaiahbjork/expo-kokoro-onnx (React Native)
- https://pub.dev/packages/kokoro_tts_flutter (Flutter)

**关键步骤：**
1. 将模型文件放入 `assets` 目录
2. 使用 ONNX Runtime Android 加载模型
3. 实现日文文本预处理（MeCab/UniDic）

### iOS 集成

**使用 ONNX Runtime for iOS：**
```swift
import onnxruntime_objc
```

**或使用 Core ML 转换版本（需要额外转换）**

## 自己导出和量化（高级）

如果你想自己控制整个流程：

### 1. 导出到 ONNX

```python
import torch
from kokoro import KPipeline

# 加载模型
pipeline = KPipeline(lang_code='j')

# 准备导出脚本
# 参考：https://github.com/thewh1teagle/kokoro-onnx/blob/main/scripts/export.py

# 主要挑战：
# - 需要修改原模型代码去除 numpy 操作
# - 处理 LSTM 层（不支持 dynamo）
# - 自定义 STFT 层（避免复数）
# - InstanceNorm1d 需要设置 affine=True
```

### 2. 量化模型

**使用 ONNX Runtime 量化工具：**

```python
from onnxruntime.quantization import quantize_dynamic, QuantType

# INT8 动态量化（最简单）
quantize_dynamic(
    model_input='kokoro-v1.0.onnx',
    model_output='kokoro-v1.0-int8.onnx',
    weight_type=QuantType.QInt8,
    # 排除对质量敏感的层
    nodes_to_exclude=['sensitive_layer_names']
)
```

**FP16 转换：**

```python
from onnxconverter_common import float16

# 转换为 FP16
model_fp16 = float16.convert_float_to_float16(model)
```

### 3. 测试量化质量

```python
import numpy as np
from onnxruntime import InferenceSession

# 加载原始和量化模型
sess_orig = InferenceSession('kokoro-v1.0.onnx')
sess_quant = InferenceSession('kokoro-v1.0-int8.onnx')

# 对比输出质量
# 使用 mel-spectrogram 差异作为评估指标
```

## 性能优化技巧

### 1. 使用 INT8 量化
- 减少 75% 的模型大小
- 在移动 CPU 上速度提升 2-4x
- 质量损失最小（对 Kokoro 来说非常鲁棒）

### 2. 预加载字典
- MeCab/UniDic 字典可以打包到 APK
- 避免运行时下载

### 3. 文本预处理优化
- 日文分词（MeCab）可以用纯 C++ 实现
- 考虑使用 UniDic Lite 版本减少体积

### 4. 流式生成
- Kokoro 支持按句生成
- 可以边生成边播放

## 完整的移动端工作流

```
用户输入日文文本
    ↓
MeCab 分词 + UniDic 字典
    ↓
转换为音素序列
    ↓
ONNX Runtime 推理（INT8 量化模型）
    ↓
生成音频波形
    ↓
播放或保存
```

## 文件清单（移动端需要）

```
your-app/
├── assets/
│   ├── kokoro-v1.0-int8.onnx      # 量化模型 (~80MB)
│   ├── voices-v1.0.bin             # 语音数据
│   └── unidic/                     # 日文字典（或 unidic-lite）
│       └── dicdir/                 # ~400MB（可用 lite 版减少到 ~50MB）
```

## 推荐实现路径

### 方案 A：快速原型（推荐）
1. 下载 INT8 量化模型
2. 使用现成的 Flutter/React Native 封装
3. 专注于 UI 和用户体验

### 方案 B：原生开发
1. Android: Kotlin + ONNX Runtime Android
2. 集成 MeCab Android 版本
3. 自己实现文本处理管线

### 方案 C：完全控制
1. 自己导出和量化模型
2. 调整层级排除策略优化质量
3. 使用自定义的 ONNX Runtime 构建

## 性能预期

**在中端 Android 设备上（INT8 量化）：**
- 模型加载：< 1 秒
- 单句生成（10-20 字）：1-3 秒
- 内存占用：~150MB

**在 M1 Mac 上：**
- 接近实时（RTF < 0.1）

## 注意事项

1. **UniDic 字典很大**：考虑使用 unidic-lite 或在线方案
2. **首次加载慢**：可以预加载模型和字典
3. **质量 vs 大小**：INT8 对 Kokoro 效果很好，可以放心使用
4. **测试设备**：在目标设备上实测性能

## 参考资源

- kokoro-onnx 项目：https://github.com/thewh1teagle/kokoro-onnx
- ONNX Runtime Mobile：https://onnxruntime.ai/docs/tutorials/mobile/
- 导出和量化详解：https://www.adrianlyjak.com/p/onnx/
- Flutter 封装：https://pub.dev/packages/kokoro_tts_flutter
- Expo 封装：https://github.com/isaiahbjork/expo-kokoro-onnx
