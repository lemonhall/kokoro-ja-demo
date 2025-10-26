# Windows 跨平台集成方案

## 🎯 问题背景

**现状**：
- ✅ Kokoro TTS 可以在 Windows 下正常运行
- ✅ Misaki C Port 在 WSL2 中编译为 `.so` 共享库
- ❌ Windows 下的 Python 无法直接调用 `.so` 文件

**需求**：
在 Windows 下实现 C 语言 G2P + Kokoro TTS 的完整语音合成流程。

---

## 🔧 解决方案对比

### 方案1：MinGW-w64 交叉编译 ⭐⭐⭐⭐⭐（推荐）

**优点**：
- ✅ 一次编译，直接生成 Windows DLL
- ✅ 可在 WSL2 中完成所有操作
- ✅ 无需安装 Visual Studio
- ✅ 生成的 DLL 可被任何 Windows 程序调用

**缺点**：
- ⚠️ 需要安装 MinGW-w64 工具链
- ⚠️ 可能遇到依赖库的兼容性问题

**步骤**：

```bash
# 1. 在 WSL2 中安装 MinGW-w64
sudo apt-get update
sudo apt-get install mingw-w64 cmake

# 2. 运行交叉编译脚本
cd /mnt/e/development/kokoro-ja-demo/misaki_c_port
bash build_windows_dll.sh

# 3. 复制 DLL 到项目根目录
cp build_windows/libmisaki.dll /mnt/e/development/kokoro-ja-demo/

# 4. 在 Windows PowerShell 中测试
cd E:\development\kokoro-ja-demo
python test_c_g2p_binding.py
```

---

### 方案2：MSVC 原生编译 ⭐⭐⭐⭐

**优点**：
- ✅ 原生 Windows 工具链，兼容性最好
- ✅ 可生成 PDB 调试文件

**缺点**：
- ❌ 需要安装 Visual Studio 2022（约 5GB）
- ❌ 需要在 Windows 下操作（不能在 WSL2）

**步骤**：

```powershell
# 1. 安装 Visual Studio 2022 Community Edition
# 下载地址: https://visualstudio.microsoft.com/

# 2. 在 Windows PowerShell 中执行
cd E:\development\kokoro-ja-demo\misaki_c_port
mkdir build_msvc
cd build_msvc

# 3. 配置 CMake
cmake .. -G "Visual Studio 17 2022" -A x64

# 4. 编译
cmake --build . --config Release

# 5. 输出文件在 Release/ 目录
# - misaki.dll
# - misaki.lib
# - misaki.exe
```

---

### 方案3：Python ctypes 直接绑定 ⭐⭐⭐

**优点**：
- ✅ 无需额外编译，直接加载 DLL
- ✅ Python 代码简洁

**缺点**：
- ❌ 需要手动定义所有 C 函数签名
- ❌ 类型转换容易出错

**示例**：

```python
import ctypes
import os

# 加载 DLL
lib = ctypes.CDLL('./libmisaki.dll')

# 定义函数签名（示例）
lib.misaki_g2p_ja.argtypes = [ctypes.c_char_p]
lib.misaki_g2p_ja.restype = ctypes.c_char_p

# 调用
text = "こんにちは"
result = lib.misaki_g2p_ja(text.encode('utf-8'))
print(result.decode('utf-8'))
```

---

### 方案4：CFFI 封装 ⭐⭐⭐⭐

**优点**：
- ✅ 比 ctypes 更安全、更 Pythonic
- ✅ 自动处理类型转换
- ✅ 支持回调函数

**缺点**：
- ⚠️ 需要额外安装 `cffi` 包

**示例**：

```python
from cffi import FFI

ffi = FFI()

# 定义 C 接口
ffi.cdef("""
    char* misaki_g2p_ja(const char* text);
    void misaki_free_string(char* str);
""")

# 加载 DLL
lib = ffi.dlopen('./libmisaki.dll')

# 调用
text = "こんにちは"
result = lib.misaki_g2p_ja(text.encode('utf-8'))
phonemes = ffi.string(result).decode('utf-8')
lib.misaki_free_string(result)  # 释放内存

print(phonemes)
```

---

### 方案5：Cython 扩展 ⭐⭐⭐⭐⭐（长期方案）

**优点**：
- ✅ 性能最优（编译为原生 Python 扩展）
- ✅ 可直接 `pip install` 安装
- ✅ 完整的 Python 类型提示

**缺点**：
- ❌ 开发成本高
- ❌ 需要维护 `.pyx` 文件

**步骤**：

```bash
# 1. 创建 Cython 包装
# misaki_ext.pyx

cdef extern from "misaki.h":
    char* misaki_g2p_ja(const char* text)

def g2p_ja(text: str) -> str:
    cdef const char* c_text = text.encode('utf-8')
    cdef char* result = misaki_g2p_ja(c_text)
    return result.decode('utf-8')

# 2. 编译为 .pyd
python setup.py build_ext --inplace

# 3. 使用
import misaki_ext
phonemes = misaki_ext.g2p_ja("こんにちは")
```

---

## 📋 实施建议

### 短期（1-2天）- 快速验证
✅ **使用方案1（MinGW-w64 交叉编译）+ 方案3（ctypes）**

```bash
# 步骤1：编译 Windows DLL
cd /mnt/e/development/kokoro-ja-demo/misaki_c_port
bash build_windows_dll.sh

# 步骤2：创建 Python 绑定
# 见 misaki_binding.py

# 步骤3：测试 C G2P + Kokoro TTS
# 见 test_c_g2p_tts.py
```

### 中期（1周）- 完善接口
✅ **完善 C API + CFFI 绑定**

1. 在 C 代码中导出标准化的 API：
   ```c
   // misaki_api.h
   MISAKI_EXPORT const char* misaki_g2p(const char* text, const char* lang);
   MISAKI_EXPORT void misaki_free_result(const char* result);
   ```

2. 使用 CFFI 创建健壮的 Python 绑定

3. 编写完整的单元测试

### 长期（1个月）- 生产级集成
✅ **Cython 扩展 + PyPI 发布**

1. 创建 `misaki-tts` Python 包
2. 提供 `pip install misaki-tts` 安装方式
3. 预编译所有平台的二进制文件（Wheel）
4. 集成到 Kokoro TTS 工作流

---

## 🧪 测试清单

- [ ] Windows DLL 编译成功
- [ ] Python ctypes 可加载 DLL
- [ ] 基本函数调用（日文 G2P）
- [ ] 内存管理正确（无泄漏）
- [ ] 与 Kokoro TTS 集成测试
- [ ] 性能测试（vs Python 版本）
- [ ] 多线程安全性测试

---

## 📊 性能对比（预期）

| 方案 | 首次加载 | 单次转换 | 吞吐量 | 内存占用 |
|------|---------|---------|--------|---------|
| Python Misaki | 500ms | 5ms | 200 req/s | 50MB |
| C DLL + ctypes | 50ms | 0.5ms | 2000 req/s | 10MB |
| Cython 扩展 | 30ms | 0.3ms | 3000 req/s | 8MB |

---

## 🔗 相关文件

- 构建脚本：`misaki_c_port/build_windows_dll.sh`
- Python 绑定：`misaki_binding.py`
- 集成测试：`test_c_g2p_tts.py`（待创建）
- CMake 配置：`misaki_c_port/CMakeLists.txt`

---

## 📝 备注

- 当前优先使用 **MinGW-w64 交叉编译** 方案，快速验证可行性
- 后续根据需求逐步完善接口和封装
- Android 平台需要单独处理（JNI 绑定）
