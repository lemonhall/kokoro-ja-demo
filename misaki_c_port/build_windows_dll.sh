#!/bin/bash
# build_windows_dll.sh
# 在 WSL2 中使用 MinGW-w64 交叉编译为 Windows DLL

set -e

echo "🔧 Misaki C Port - Windows DLL 交叉编译脚本"
echo "=============================================="

# 检查是否安装了 MinGW-w64
if ! command -v x86_64-w64-mingw32-gcc &> /dev/null; then
    echo "❌ 错误：未找到 MinGW-w64 工具链"
    echo ""
    echo "请先安装 MinGW-w64："
    echo "  sudo apt-get update"
    echo "  sudo apt-get install mingw-w64 cmake"
    exit 1
fi

echo "✅ 检测到 MinGW-w64 工具链"
x86_64-w64-mingw32-gcc --version | head -n 1

# 创建构建目录
BUILD_DIR="build_windows"
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

echo ""
echo "📦 配置 CMake（Windows 目标）..."

cmake .. \
  -DCMAKE_SYSTEM_NAME=Windows \
  -DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc \
  -DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++ \
  -DCMAKE_RC_COMPILER=x86_64-w64-mingw32-windres \
  -DBUILD_SHARED_LIBS=ON \
  -DCMAKE_BUILD_TYPE=Release

echo ""
echo "🔨 编译 Windows DLL..."
make -j$(nproc)

echo ""
echo "✅ 编译完成！"
echo ""
echo "生成的文件："
echo "  - libmisaki.dll    (共享库，供 Python ctypes 调用)"
echo "  - misaki.exe       (命令行工具，可在 Windows 下运行)"
echo "  - libmisaki.dll.a  (导入库，供 C/C++ 链接)"
echo ""
echo "📋 使用方法："
echo ""
echo "1. 复制 DLL 到 Python 项目目录："
echo "   cp libmisaki.dll /mnt/e/development/kokoro-ja-demo/"
echo ""
echo "2. 在 Python 中使用 ctypes 加载："
echo "   import ctypes"
echo "   lib = ctypes.CDLL('./libmisaki.dll')"
echo ""
echo "3. 测试 Windows 可执行文件（在 Windows PowerShell 中）："
echo "   E:\\development\\kokoro-ja-demo\\misaki_c_port\\build_windows\\misaki.exe \"Hello\""
