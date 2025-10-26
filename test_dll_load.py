"""
test_dll_load.py

快速测试 Windows DLL 加载

用法（在 Windows PowerShell 中）：
    python test_dll_load.py
"""

import ctypes
import os
import sys

def test_dll_load():
    """测试 DLL 是否能正常加载"""
    
    print("🧪 测试 Misaki Windows DLL 加载")
    print("=" * 50)
    
    # DLL 路径
    dll_path = r"E:\development\kokoro-ja-demo\libmisaki.dll"
    
    # 检查文件是否存在
    if not os.path.exists(dll_path):
        print(f"❌ DLL 文件不存在: {dll_path}")
        print(f"\n请先编译 DLL：")
        print(f"  wsl bash -c \"cd /mnt/e/development/kokoro-ja-demo/misaki_c_port && bash quick_test_windows_build.sh\"")
        sys.exit(1)
    
    # 显示文件信息
    dll_size = os.path.getsize(dll_path) / 1024
    print(f"✅ 找到 DLL: {dll_path}")
    print(f"   大小: {dll_size:.1f} KB")
    print()
    
    # 尝试加载 DLL
    try:
        print("📦 加载 DLL...")
        lib = ctypes.CDLL(dll_path)
        print("✅ DLL 加载成功！")
        print()
        
        # 尝试查找一些符号（如果导出的话）
        print("🔍 检查导出的符号...")
        
        # 这里列出一些可能导出的函数名
        # 注意：需要在 C 代码中使用 __declspec(dllexport) 或 CMake 配置导出
        possible_functions = [
            'misaki_utf8_decode',
            'misaki_string_new',
            'misaki_trie_create',
            # 更多函数...
        ]
        
        found_functions = []
        for func_name in possible_functions:
            try:
                func = getattr(lib, func_name)
                found_functions.append(func_name)
            except AttributeError:
                pass
        
        if found_functions:
            print(f"✅ 找到 {len(found_functions)} 个导出函数:")
            for func in found_functions:
                print(f"   - {func}")
        else:
            print("⚠️  未找到明确导出的函数")
            print("   这是正常的，需要在 C 代码中添加导出声明")
        
        print()
        print("=" * 50)
        print("✅ DLL 加载测试完成！")
        print()
        print("📋 下一步：")
        print("1. 在 C 代码中添加函数导出（__declspec(dllexport)）")
        print("2. 定义 Python 函数签名（ctypes.argtypes/restype）")
        print("3. 调用 C 函数并验证功能")
        print()
        print("参考文档：")
        print("  - misaki_c_port/docs/WINDOWS_INTEGRATION.md")
        print("  - misaki_binding.py")
        
    except OSError as e:
        print(f"❌ DLL 加载失败: {e}")
        print()
        print("可能的原因：")
        print("1. DLL 依赖的其他库不存在（如 msvcrt.dll）")
        print("2. DLL 不是为 Windows 编译的")
        print("3. 架构不匹配（32位 vs 64位）")
        sys.exit(1)

if __name__ == "__main__":
    test_dll_load()
