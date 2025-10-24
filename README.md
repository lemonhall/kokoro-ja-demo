# Kokoro 日文语音合成演示

这是一个使用 Kokoro TTS 引擎进行日文文本转语音的演示项目。

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

| 文件 | 说明 |
|------|------|
| `ja.py` | 原始 PyTorch 示例（官方演示） |
| `example_simple.py` | 简化版使用示例 |
| `export_onnx.py` | 导出最新 Kokoro 模型到 ONNX |
| `quantize_int8.py` | INT8 量化工具（310MB → 109MB） |
| `convert_fp16.py` | FP16 转换工具（310MB → 155MB） |
| `test_onnx.py` | ONNX 模型测试脚本 |

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
- **INT8 量化**: ~109MB（推荐移动端）
- **FP16 转换**: ~155MB

## 技术栈

- **TTS 引擎**: Kokoro-82M
- **日文分词**: MeCab + UniDic
- **音素转换**: Misaki (JAG2P)
- **模型导出**: ONNX Runtime
- **音频处理**: SoundFile

## 许可证

- Kokoro TTS: Apache 2.0
- 本项目：仅供学习和研究使用
