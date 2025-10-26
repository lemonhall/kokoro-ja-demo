"""
misaki_binding.py

Python ctypes 绑定 - 调用 C 语言版本的 Misaki G2P

用法:
    from misaki_binding import MisakiG2P
    
    g2p = MisakiG2P()
    phonemes = g2p.convert("東京都渋谷区", lang='ja')
    print(phonemes)  # "t o ː k j o ː t o ɕ i b ɯ j a k ɯ"

依赖:
    - Windows: libmisaki.dll
    - Linux/WSL: libmisaki.so
    - macOS: libmisaki.dylib
"""

import ctypes
import os
import sys
import platform
from pathlib import Path

class MisakiG2P:
    """
    Misaki G2P C 库的 Python 封装
    """
    
    def __init__(self, lib_path=None):
        """
        初始化 Misaki G2P
        
        Args:
            lib_path: C 库路径，如果为 None 则自动查找
        """
        if lib_path is None:
            lib_path = self._find_library()
        
        if not os.path.exists(lib_path):
            raise FileNotFoundError(
                f"❌ 找不到 Misaki C 库: {lib_path}\n"
                f"请先编译 C 库：\n"
                f"  WSL/Linux: cd misaki_c_port/build && make\n"
                f"  Windows:   cd misaki_c_port && bash build_windows_dll.sh"
            )
        
        print(f"✅ 加载 Misaki C 库: {lib_path}")
        self.lib = ctypes.CDLL(lib_path)
        
        # 配置函数签名（待实现）
        # self._setup_functions()
    
    def _find_library(self):
        """
        自动查找 C 库文件
        """
        system = platform.system()
        base_dir = Path(__file__).parent / "misaki_c_port"
        
        # 根据操作系统选择库文件
        if system == "Windows":
            lib_name = "libmisaki.dll"
            search_paths = [
                base_dir / "build_windows" / lib_name,
                base_dir / "build" / lib_name,
                Path(".") / lib_name,
            ]
        elif system == "Linux":
            lib_name = "libmisaki.so"
            search_paths = [
                base_dir / "build" / lib_name,
                Path(".") / lib_name,
            ]
        elif system == "Darwin":  # macOS
            lib_name = "libmisaki.dylib"
            search_paths = [
                base_dir / "build" / lib_name,
                Path(".") / lib_name,
            ]
        else:
            raise OSError(f"不支持的操作系统: {system}")
        
        # 查找库文件
        for path in search_paths:
            if path.exists():
                return str(path)
        
        # 如果都找不到，返回默认路径（让后面报错）
        return str(search_paths[0])
    
    def _setup_functions(self):
        """
        配置 C 函数的签名
        TODO: 根据 C API 补充完整
        """
        # 示例：假设有一个 misaki_g2p 函数
        # self.lib.misaki_g2p.argtypes = [ctypes.c_char_p, ctypes.c_char_p]
        # self.lib.misaki_g2p.restype = ctypes.c_char_p
        pass
    
    def convert(self, text, lang='ja'):
        """
        文本转音素
        
        Args:
            text: 输入文本
            lang: 语言代码 ('ja', 'zh', 'en')
        
        Returns:
            phonemes: 音素字符串
        """
        # TODO: 实现实际的 C 函数调用
        raise NotImplementedError(
            "C API 绑定尚未完成，请先实现 C 库的导出接口"
        )
    
    def __del__(self):
        """
        清理资源
        """
        if hasattr(self, 'lib'):
            # TODO: 调用 C 库的清理函数
            pass

def test_binding():
    """
    测试 ctypes 绑定
    """
    print("🧪 测试 Misaki Python 绑定")
    print("=" * 50)
    
    try:
        g2p = MisakiG2P()
        print("✅ 成功加载 C 库")
        
        # TODO: 实际测试
        # result = g2p.convert("こんにちは", lang='ja')
        # print(f"结果: {result}")
        
    except Exception as e:
        print(f"❌ 测试失败: {e}")
        sys.exit(1)

if __name__ == "__main__":
    test_binding()
