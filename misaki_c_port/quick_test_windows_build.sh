#!/bin/bash
# quick_test_windows_build.sh
# 快速测试 Windows DLL 编译和使用

set -e

echo "🧪 Misaki C Port - Windows DLL 快速测试"
echo "========================================"

# 步骤1：检查 MinGW-w64
echo ""
echo "1️⃣ 检查 MinGW-w64 工具链..."
if ! command -v x86_64-w64-mingw32-gcc &> /dev/null; then
    echo "❌ 未安装 MinGW-w64"
    echo ""
    echo "请运行："
    echo "  sudo apt-get update && sudo apt-get install mingw-w64 cmake"
    exit 1
fi
echo "✅ MinGW-w64 已安装"

# 步骤2：编译 Windows DLL
echo ""
echo "2️⃣ 编译 Windows DLL..."
cd "$(dirname "$0")"
rm -rf build_windows
mkdir -p build_windows
cd build_windows

cmake .. \
  -DCMAKE_SYSTEM_NAME=Windows \
  -DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc \
  -DCMAKE_RC_COMPILER=x86_64-w64-mingw32-windres \
  -DBUILD_SHARED_LIBS=ON \
  -DCMAKE_BUILD_TYPE=Release \
  > /dev/null 2>&1

make -j$(nproc) > /dev/null 2>&1

if [ -f "libmisaki.dll" ]; then
    echo "✅ 编译成功：libmisaki.dll"
    ls -lh libmisaki.dll
else
    echo "❌ 编译失败"
    exit 1
fi

# 步骤3：测试 Windows 可执行文件
echo ""
echo "3️⃣ 测试 Windows 可执行文件..."
if [ -f "misaki.exe" ]; then
    echo "✅ 找到 misaki.exe"
    
    # 在 WSL 中通过 wine 测试（如果有的话）
    if command -v wine &> /dev/null; then
        echo ""
        echo "📝 通过 wine 测试："
        wine misaki.exe "Hello" 2>/dev/null || echo "⚠️  wine 测试失败（正常，需要在真实 Windows 环境）"
    fi
else
    echo "❌ 未找到 misaki.exe"
fi

# 步骤4：复制 DLL 到项目根目录
echo ""
echo "4️⃣ 复制 DLL 到项目根目录..."
cp libmisaki.dll /mnt/e/development/kokoro-ja-demo/
echo "✅ 已复制到 E:\\development\\kokoro-ja-demo\\libmisaki.dll"

# 步骤5：生成测试说明
echo ""
echo "════════════════════════════════════════"
echo "✅ Windows DLL 编译完成！"
echo "════════════════════════════════════════"
echo ""
echo "📋 下一步："
echo ""
echo "1. 在 Windows PowerShell 中测试可执行文件："
echo "   E:\\development\\kokoro-ja-demo\\misaki_c_port\\build_windows\\misaki.exe \"こんにちは\""
echo ""
echo "2. 使用 Python ctypes 加载 DLL："
echo "   cd E:\\development\\kokoro-ja-demo"
echo "   python misaki_binding.py"
echo ""
echo "3. 集成 Kokoro TTS 进行真实语音合成："
echo "   python test_c_g2p_tts.py --text \"東京都渋谷区\" --voice jf_nezumi"
echo ""
echo "📁 生成的文件："
echo "   - libmisaki.dll       (Windows 共享库)"
echo "   - misaki.exe          (Windows 可执行文件)"
echo "   - libmisaki.dll.a     (导入库)"
echo ""
