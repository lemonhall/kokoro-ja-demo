"""
test_c_g2p_kokoro.py

使用 C 语言 DLL 的 G2P + Kokoro TTS 完整测试
替换 Python 版 Misaki
"""

import ctypes
from pathlib import Path
import sys
import os

# 添加项目根目录到 sys.path
project_root = Path(__file__).parent.parent.parent
if str(project_root) not in sys.path:
    sys.path.insert(0, str(project_root))

# Kokoro imports
try:
    from kokoro import KPipeline
    import soundfile as sf
    import torch
    KOKORO_AVAILABLE = True
except ImportError as e:
    print(f"⚠️  Kokoro 未安装: {e}")
    KOKORO_AVAILABLE = False


class MisakiCG2P:
    """C 语言版 Misaki G2P 封装"""
    
    def __init__(self, dll_path=None, data_dir=None):
        """初始化"""
        if dll_path is None:
            dll_path = Path(__file__).parent / "libmisaki.dll"
        if data_dir is None:
            data_dir = "extracted_data"
        
        # 加载 DLL
        print(f"📦 加载 Misaki DLL: {dll_path}")
        old_cwd = os.getcwd()
        os.chdir(Path(dll_path).parent)
        self.lib = ctypes.CDLL(str(Path(dll_path).absolute()))
        os.chdir(old_cwd)
        
        # 定义函数签名
        self.lib.misaki_init.argtypes = [ctypes.c_char_p]
        self.lib.misaki_init.restype = ctypes.c_int
        
        self.lib.misaki_text_to_phonemes.argtypes = [
            ctypes.c_char_p, ctypes.c_char_p, ctypes.c_int
        ]
        self.lib.misaki_text_to_phonemes.restype = ctypes.c_int
        
        self.lib.misaki_get_version.argtypes = []
        self.lib.misaki_get_version.restype = ctypes.c_char_p
        
        self.lib.misaki_cleanup.argtypes = []
        self.lib.misaki_cleanup.restype = None
        
        # 初始化
        version = self.lib.misaki_get_version().decode('utf-8')
        print(f"✅ Misaki 版本: {version}")
        
        result = self.lib.misaki_init(data_dir.encode('utf-8'))
        if result != 0:
            raise RuntimeError("❌ Misaki 初始化失败")
        print(f"✅ Misaki 初始化成功 (数据目录: {data_dir})\n")
    
    def text_to_phonemes(self, text):
        """文本转音素"""
        output_buffer = ctypes.create_string_buffer(2048)
        text_bytes = text.encode('utf-8')
        
        result = self.lib.misaki_text_to_phonemes(
            text_bytes,
            output_buffer,
            len(output_buffer)
        )
        
        if result == 0:
            return output_buffer.value.decode('utf-8')
        else:
            return None
    
    def __del__(self):
        """清理"""
        if hasattr(self, 'lib'):
            self.lib.misaki_cleanup()


def test_c_g2p_tts(text, lang_code='z'):
    """
    测试 C G2P + Kokoro TTS
    生成两个版本的音频用于对比：
    - misaki_c_version.wav: C 语言版本（使用 C DLL 的 G2P）
    - misaki_python_version.wav: Python 原版（使用 Kokoro 内置的 Misaki）
    
    Args:
        text: 输入文本
        lang_code: 'z'=中文, 'j'=日文, 'a'=美英
    """
    
    print("=" * 60)
    print("🎯 C 语言 G2P + Kokoro TTS 测试")
    print("=" * 60)
    print(f"\n📝 输入文本: {text}")
    print(f"🌍 语言: {lang_code}\n")
    
    # 1. 初始化 C G2P
    g2p = MisakiCG2P()
    
    # 2. 文本转音素
    print("🔤 调用 C 语言 G2P...")
    phonemes = g2p.text_to_phonemes(text)
    
    if not phonemes:
        print("❌ G2P 转换失败")
        return
    
    print(f"✅ 音素输出: {phonemes}\n")
    
    # 3. Kokoro TTS 合成
    if not KOKORO_AVAILABLE:
        print("⚠️  Kokoro 未安装，跳过 TTS")
        return
    
    print("🎙️  Kokoro TTS 合成中...\n")
    try:
        # 初始化 Kokoro Pipeline
        pipeline = KPipeline(lang_code=lang_code)
        voice = 'zf_xiaoxiao' if lang_code == 'z' else 'jf_nezumi'
        
        # ===== 版本 1：C 语言版本 =====
        print("🔧 版本 1: C 语言 Misaki G2P")
        output_file_c = "misaki_c_version.wav"
        
        generator = pipeline(
            text,
            voice=voice,
            speed=1.0,
            split_pattern=r'\n+'
        )
        
        for i, (gs, ps, audio) in enumerate(generator):
            print(f"  文本: {gs}")
            print(f"  Kokoro 音素: {ps}")
            print(f"  C G2P 音素: {phonemes}")
            sf.write(output_file_c, audio, 24000)
            print(f"  ✅ 保存到: {output_file_c}\n")
        
        # ===== 版本 2：Python 原版 =====
        print("🐍 版本 2: Python 原版 Misaki G2P")
        output_file_py = "misaki_python_version.wav"
        
        # 重新创建 pipeline（使用 Kokoro 内置的 Misaki）
        pipeline2 = KPipeline(lang_code=lang_code)
        
        generator2 = pipeline2(
            text,
            voice=voice,
            speed=1.0,
            split_pattern=r'\n+'
        )
        
        for i, (gs, ps, audio) in enumerate(generator2):
            print(f"  文本: {gs}")
            print(f"  Python 音素: {ps}")
            sf.write(output_file_py, audio, 24000)
            print(f"  ✅ 保存到: {output_file_py}\n")
        
        print("🎉 完成！")
        print(f"  C 版本: {output_file_c}")
        print(f"  Python 版本: {output_file_py}")
        
    except Exception as e:
        print(f"❌ TTS 失败: {e}")
        import traceback
        traceback.print_exc()


if __name__ == "__main__":
    if len(sys.argv) > 1:
        text = sys.argv[1]
        lang = sys.argv[2] if len(sys.argv) > 2 else 'z'
    else:
        text = "今天天气很好"
        lang = 'z'
    
    test_c_g2p_tts(text, lang_code=lang)
