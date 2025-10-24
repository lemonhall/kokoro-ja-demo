# Kokoro 日文语音合成演示

这是一个使用 Kokoro TTS 引擎进行日文文本转语音的演示项目。

## 环境要求

- Python 3.13+
- uv (Python 包管理工具)

## 安装步骤

### 1. 安装依赖

```bash
uv sync
```

### 2. 下载 UniDic 日文字典（重要！）

这是关键步骤，缺少这一步会导致 MeCab 初始化失败：

```bash
# 先安装 unidic 包
uv add unidic

# 下载 UniDic 字典数据（约 526MB）
uv run python -m unidic download
```

## 运行

```bash
uv run python ja.py
```

程序会读取日文文本并生成语音文件 `0.wav`。

## 常见问题

### MeCab 初始化失败

如果遇到以下错误：

```
RuntimeError: Failed initializing MeCab
param.cpp(69) [ifs] no such file or directory: ...\unidic\dicdir\mecabrc
```

**解决方案：** 确保已经执行了 `uv run python -m unidic download` 下载字典数据。

### 警告信息

运行时可能会看到一些警告，这些是正常的：

- `WARNING: Defaulting repo_id to hexgrad/Kokoro-82M` - 默认使用 Kokoro-82M 模型
- `UserWarning: dropout option adds dropout after all but last recurrent layer` - PyTorch 模型结构警告
- `FutureWarning: torch.nn.utils.weight_norm is deprecated` - PyTorch API 弃用警告

这些警告不影响程序正常运行。

## 依赖说明

项目主要依赖：

- `kokoro` - TTS 语音合成引擎
- `misaki[ja]` - 日文文本处理（包含 fugashi、cutlet 等）
- `unidic` - 日文形态分析字典（MeCab 需要）
- `soundfile` - 音频文件处理

## 技术栈

- **TTS 引擎**: Kokoro-82M
- **日文分词**: MeCab + UniDic
- **罗马音转换**: Cutlet
- **音频处理**: SoundFile

## 自定义

修改 `ja.py` 中的参数：

- `text` - 要转换的日文文本
- `voice` - 语音类型（如 `jf_nezumi`）
- `speed` - 语速（默认 1.0）
