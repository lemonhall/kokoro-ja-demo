# Windows 构建目录说明

本目录包含 Misaki C Port 的 Windows 构建产物和测试脚本。

## 📦 核心文件

### 编译产物（已提交到 Git）
- **`libmisaki.dll`** (744KB)
  - Windows 动态链接库（MinGW-w64 交叉编译）
  - 可被 Python、Kotlin、Swift 等语言通过 FFI 调用
  - 包含完整的多语言 G2P 功能（中文/日文/英文）
  - ⚠️ **编译一次很费劲，已加入版本控制**

- **`misaki.exe`**
  - Windows 命令行可执行文件
  - 用于快速测试 G2P 功能
  - 使用方法：`.\misaki.exe "要转换的文本"`

### 测试脚本
- **`test_dll_api.py`**
  - 基础 DLL API 测试脚本
  - 演示如何使用 Python ctypes 加载 DLL
  - 测试单个文本的 G2P 转换
  - 使用：`python test_dll_api.py "测试文本"`

- **`test_c_g2p_kokoro.py`**
  - **完整的 TTS 测试脚本（C G2P + Kokoro）**
  - 生成两个音频文件用于对比：
    - `misaki_c_version.wav` - 使用 C 语言版 Misaki G2P
    - `misaki_python_version.wav` - 使用 Python 原版 Misaki G2P
  - 使用：`e:/development/kokoro-ja-demo/.venv/Scripts/python.exe test_c_g2p_kokoro.py "今天天气很好"`
  - 中文语音使用 `zf_xiaoxiao` 声线

- **`run_misaki.ps1`**
  - PowerShell 便捷启动脚本（如果有）

## 📂 数据目录

### `extracted_data/`
包含运行时需要的词典和模型数据：

#### 中文 (`zh/`)
- `dict_full.txt` - 完整中文词典
- `dict_merged.txt` - 合并词典
- `phrase_pinyin.txt` - 词组拼音（含声调变化）
- `hmm_prob_start.txt` - HMM 初始概率
- `hmm_prob_trans.txt` - HMM 转移概率
- `hmm_prob_emit.txt` - HMM 发射概率（35224 个字符）

#### 日文 (`ja/`)
- `ja_pron_dict.tsv` - 日文发音词典（从 UniDic 提取）

#### 英文 (`en/`)
- 英文 G2P 词典

## 🔧 编译方法

### 在 WSL2 中交叉编译
```bash
cd /mnt/e/development/kokoro-ja-demo/misaki_c_port
bash build_windows_dll.sh
```

编译产物会自动输出到本目录。

## 🚀 使用示例

### 1. 命令行快速测试
```bash
.\misaki.exe "こんにちは世界"
```

### 2. Python 调用 DLL
```python
from pathlib import Path
import ctypes

# 加载 DLL
dll_path = Path(__file__).parent / "libmisaki.dll"
lib = ctypes.CDLL(str(dll_path.absolute()))

# 初始化
lib.misaki_init(b"extracted_data")

# 文本转音素
output = ctypes.create_string_buffer(1024)
lib.misaki_text_to_phonemes(b"今天天气很好", output, 1024)
print(output.value.decode('utf-8'))

# 清理
lib.misaki_cleanup()
```

### 3. 生成 TTS 音频（对比版本）
```bash
# 使用虚拟环境中的 Python
e:/development/kokoro-ja-demo/.venv/Scripts/python.exe test_c_g2p_kokoro.py "今天天气很好"

# 生成两个音频文件：
# - misaki_c_version.wav (C 版本)
# - misaki_python_version.wav (Python 版本)
```

## 📋 DLL API 接口

### 核心函数
```c
// 初始化引擎
int misaki_init(const char *data_dir);

// 文本转音素（自动检测语言）
int misaki_text_to_phonemes(
    const char *text,
    char *output_buffer,
    int buffer_size
);

// 文本转音素（指定语言）
int misaki_text_to_phonemes_lang(
    const char *text,
    const char *lang,  // "ja"=日文, "zh"=中文, "en"=英文
    char *output_buffer,
    int buffer_size
);

// 清理资源
void misaki_cleanup(void);

// 获取版本号
const char* misaki_get_version(void);
```

## 🎯 关键特性

- ✅ **自动语言检测**：支持中日英混合文本
- ✅ **高性能**：C 语言实现，速度快
- ✅ **跨平台**：Windows DLL，可被多种语言调用
- ✅ **完整功能**：
  - 中文分词（jieba HMM 算法）
  - 日文分词（MeCab 分词逻辑）
  - 英文 G2P
  - 声调标注（→ ↗ ↓ 表示声调）

## ⚠️ 注意事项

1. **数据目录路径**：
   - 相对路径：`extracted_data`（推荐，DLL 与数据同级）
   - 绝对路径：完整路径

2. **DLL 加载**：
   - 必须使用绝对路径或切换到 DLL 目录
   - Windows 下相对路径可能无法找到依赖

3. **编码问题**：
   - 输入输出统一使用 UTF-8
   - Python 中需要 `.encode('utf-8')` 和 `.decode('utf-8')`

4. **内存管理**：
   - 调用 `misaki_init()` 后必须调用 `misaki_cleanup()`
   - 输出缓冲区由调用者分配（建议 1024-2048 字节）

## 🔗 相关文档

- CMake 配置：`../CMakeLists.txt`
- API 头文件：`../include/misaki_api.h`
- API 实现：`../src/api/misaki_api.c`
- Windows 集成文档：`../docs/WINDOWS_INTEGRATION.md`

## 📝 版本信息

- **Misaki 版本**：0.3.0
- **编译器**：MinGW-w64
- **目标平台**：Windows x64

---

**最后更新**：2025-10-26
