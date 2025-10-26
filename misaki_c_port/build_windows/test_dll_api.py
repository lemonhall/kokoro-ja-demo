"""
test_dll_api.py

测试 C DLL API
直接调用 misaki_text_to_phonemes 函数
"""

import ctypes
from pathlib import Path
import sys

def test_dll_api(text):
    """测试 DLL API"""
    
    # 加载 DLL（使用绝对路径）
    import os
    dll_path = Path(__file__).parent / "libmisaki.dll"
    if not dll_path.exists():
        print(f"❌ 找不到 DLL: {dll_path}")
        return
    
    print(f"📦 加载 DLL: {dll_path}")
    # 切换到 DLL 所在目录，确保能找到依赖
    old_cwd = os.getcwd()
    os.chdir(dll_path.parent)
    lib = ctypes.CDLL(str(dll_path.absolute()))
    os.chdir(old_cwd)
    
    # 定义函数签名
    lib.misaki_init.argtypes = [ctypes.c_char_p]
    lib.misaki_init.restype = ctypes.c_int
    
    lib.misaki_text_to_phonemes.argtypes = [
        ctypes.c_char_p,  # text
        ctypes.c_char_p,  # output_buffer
        ctypes.c_int      # buffer_size
    ]
    lib.misaki_text_to_phonemes.restype = ctypes.c_int
    
    lib.misaki_get_version.argtypes = []
    lib.misaki_get_version.restype = ctypes.c_char_p
    
    lib.misaki_cleanup.argtypes = []
    lib.misaki_cleanup.restype = None
    
    # 获取版本
    version = lib.misaki_get_version().decode('utf-8')
    print(f"✅ Misaki 版本: {version}\n")
    
    # 初始化（当前目录下的 extracted_data）
    data_dir = b"extracted_data"
    print(f"🚀 初始化 Misaki（数据目录: {data_dir.decode('utf-8')}）...")
    result = lib.misaki_init(data_dir)
    if result != 0:
        print("❌ 初始化失败")
        return
    print("✅ 初始化成功\n")
    
    # 转换文本
    print(f"📝 输入文本: {text}")
    output_buffer = ctypes.create_string_buffer(1024)
    text_bytes = text.encode('utf-8')
    
    result = lib.misaki_text_to_phonemes(
        text_bytes,
        output_buffer,
        len(output_buffer)
    )
    
    if result == 0:
        phonemes = output_buffer.value.decode('utf-8')
        print(f"✅ 音素输出: {phonemes}\n")
    else:
        print("❌ G2P 转换失败\n")
    
    # 清理
    lib.misaki_cleanup()
    print("🧹 清理完成")

if __name__ == "__main__":
    text = sys.argv[1] if len(sys.argv) > 1 else "こんにちは世界"
    test_dll_api(text)
