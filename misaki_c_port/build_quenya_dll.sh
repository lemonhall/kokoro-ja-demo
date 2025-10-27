#!/bin/bash
# build_quenya_dll.sh
# 在 WSL2 中使用 MinGW-w64 交叉编译昆雅语支持的 Windows DLL

set -e

echo "🧝 Misaki C Port - Quenya (昆雅语) Windows DLL 交叉编译"
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

# 创建构建目录（独立于 build_windows）
BUILD_DIR="build_quenya"
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

echo ""
echo "📦 配置 CMake（Windows 目标 + 昆雅语支持）..."

cmake .. \
  -DCMAKE_SYSTEM_NAME=Windows \
  -DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc \
  -DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++ \
  -DCMAKE_RC_COMPILER=x86_64-w64-mingw32-windres \
  -DBUILD_SHARED_LIBS=ON \
  -DCMAKE_BUILD_TYPE=Release

echo ""
echo "🔨 编译 Windows DLL（包含昆雅语模块）..."
make misaki_shared -j$(nproc)

echo ""
echo "✅ 编译完成！"
echo ""
echo "生成的文件："
ls -lh libmisaki.dll 2>/dev/null || echo "  ❌ libmisaki.dll 未生成"
echo ""
echo "📋 下一步："
echo ""
echo "1. 复制 DLL 到测试目录："
echo "   mkdir -p ../build_quenya_test"
echo "   cp libmisaki.dll ../build_quenya_test/"
echo "   cp -r ../extracted_data ../build_quenya_test/"
echo ""
echo "2. 创建 Python 测试脚本测试昆雅语 TTS"
echo ""
echo "🧝 Aiya! Quenya support is ready! ✨"
