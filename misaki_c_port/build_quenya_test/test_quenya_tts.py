"""
test_quenya_tts.py

🧝 昆雅语 (Quenya) TTS 测试脚本
使用 C 版本 Misaki G2P + ONNX TTS 生成精灵语音频

星光照耀我们相遇之时！ Elen síla lúmenn' omentielvo!
"""

import ctypes
from pathlib import Path
import sys
import os
import numpy as np

# 添加项目根目录到 sys.path
project_root = Path(__file__).parent.parent.parent
if str(project_root) not in sys.path:
    sys.path.insert(0, str(project_root))

print(f"Project root: {project_root}")
print(f"sys.path: {sys.path[:3]}")

# 导入依赖
try:
    import torch
    from kokoro import KPipeline
    import soundfile as sf
    import onnxruntime as ort
    DEPS_OK = True
except ImportError as e:
    print(f"❌ 依赖缺失: {e}")
    print(f"当前目录: {os.getcwd()}")
    DEPS_OK = False


class MisakiCG2P:
    """C 语言版 Misaki G2P - 支持昆雅语"""
    
    def __init__(self, dll_path=None, data_dir=None, force_lang='qya'):
        if dll_path is None:
            dll_path = Path(__file__).parent / "libmisaki.dll"
        if data_dir is None:
            data_dir = "extracted_data"
        
        self.force_lang = force_lang
        
        # 切换工作目录加载 DLL
        old_cwd = os.getcwd()
        os.chdir(Path(dll_path).parent)
        self.lib = ctypes.CDLL(str(Path(dll_path).absolute()))
        os.chdir(old_cwd)
        
        # 初始化函数
        self.lib.misaki_init.argtypes = [ctypes.c_char_p]
        self.lib.misaki_init.restype = ctypes.c_int
        
        # 使用带语言参数的API
        self.lib.misaki_text_to_phonemes_lang.argtypes = [
            ctypes.c_char_p,  # text
            ctypes.c_char_p,  # lang
            ctypes.c_char_p,  # output_buffer
            ctypes.c_int      # buffer_size
        ]
        self.lib.misaki_text_to_phonemes_lang.restype = ctypes.c_int
        
        # 清理函数
        self.lib.misaki_cleanup.argtypes = []
        self.lib.misaki_cleanup.restype = None
        
        # 初始化
        result = self.lib.misaki_init(data_dir.encode('utf-8'))
        if result != 0:
            raise RuntimeError("C Misaki 初始化失败")
        
        print(f"✅ Misaki C 初始化成功 (语言: {force_lang})")
    
    def text_to_phonemes(self, text):
        """将文本转换为音素"""
        output_buffer = ctypes.create_string_buffer(2048)
        result = self.lib.misaki_text_to_phonemes_lang(
            text.encode('utf-8'), 
            self.force_lang.encode('utf-8'),
            output_buffer, 
            len(output_buffer)
        )
        
        if result != 0:
            return None
        
        return output_buffer.value.decode('utf-8')
    
    def __del__(self):
        if hasattr(self, 'lib'):
            self.lib.misaki_cleanup()


def synthesize_with_phonemes(phonemes, voice_name='af_sarah', onnx_path=None):
    """
    使用音素直接调用 ONNX 模型合成音频
    
    参数:
        phonemes: 音素字符串（空格分隔）
        voice_name: 语音预设名称（推荐用女声：af_sarah, af_sky, bf_emma）
        onnx_path: ONNX 模型路径
    """
    if onnx_path is None:
        onnx_path = project_root / "kokoro_latest.onnx"
    
    if not onnx_path.exists():
        print(f"❌ 模型文件不存在: {onnx_path}")
        return None
    
    # 初始化 pipeline 获取 vocab
    pipeline = KPipeline(lang_code='z')
    vocab = pipeline.model.vocab
    
    # 音素转 input_ids
    phoneme_list = phonemes.split()
    input_ids = list(filter(lambda i: i is not None, map(lambda p: vocab.get(p), phoneme_list)))
    input_ids_with_special = [0, *input_ids, 0]  # 加 BOS/EOS
    
    print(f"  音素列表: {phoneme_list}")
    print(f"  转换后 ID: {input_ids[:20]}..." if len(input_ids) > 20 else f"  转换后 ID: {input_ids}")
    
    # 加载语音嵌入（动态帧选择）
    voices_tensor = pipeline.load_voice(voice_name)
    frame_index = min(len(input_ids) - 1, voices_tensor.shape[0] - 1)
    ref_s = voices_tensor[frame_index, 0, :].unsqueeze(0).numpy()
    
    # ONNX 推理
    session = ort.InferenceSession(str(onnx_path))
    inputs = {
        'input_ids': np.array([input_ids_with_special], dtype=np.int64),
        'ref_s': ref_s,
        'speed': np.array(1.0, dtype=np.float64)
    }
    
    outputs = session.run(None, inputs)
    waveform = outputs[0]
    
    # 归一化
    if np.max(np.abs(waveform)) > 1.0:
        waveform = waveform / np.max(np.abs(waveform)) * 0.95
    
    return waveform


def main(text, voice='af_sarah'):
    if not DEPS_OK:
        print("请先安装依赖: pip install kokoro soundfile onnxruntime")
        return
    
    print("=" * 70)
    print("🧝 昆雅语 (Quenya) TTS 测试")
    print("   Aiya! Elen síla lúmenn' omentielvo!")
    print("=" * 70)
    print(f"\n📝 昆雅语文本: {text}")
    print(f"🎤 使用语音: {voice}\n")
    
    # 1. C 版本 Quenya G2P
    print("🔧 步骤 1: Quenya G2P 转换")
    g2p = MisakiCG2P(force_lang='qya')
    phonemes = g2p.text_to_phonemes(text)
    
    if not phonemes:
        print("❌ G2P 转换失败")
        return
    
    print(f"  IPA 音素: /{phonemes}/\n")
    
    # 2. ONNX TTS 合成
    print("🎵 步骤 2: ONNX TTS 合成")
    audio = synthesize_with_phonemes(phonemes, voice)
    
    if audio is not None:
        # 输出到脚本同级目录
        script_dir = Path(__file__).parent
        output_file = script_dir / 'quenya_tts_output.wav'
        sf.write(str(output_file), audio, 24000)
        duration = len(audio) / 24000
        print(f"  ✅ 保存: {output_file.name} ({duration:.2f}秒)\n")
        
        print("=" * 70)
        print("🎉 完成！")
        print(f"🎵 生成的昆雅语音频: {output_file}")
        print("💡 现在你可以听听精灵的语言了！")
        print("=" * 70)
    else:
        print("❌ TTS 合成失败")


if __name__ == "__main__":
    # 昆雅语测试文本
    quenya_texts = [
        "Quenya",              # 昆雅语
        "Eldar",               # 精灵
        "Valar",               # 维拉（神）
        "Elen síla lúmenn' omentielvo",  # 星光照耀我们相遇之时
        "Namárië",             # 告别
        "Arda",                # 世界
        "Ilúvatar",            # 至高神
        "Laurelin",            # 金树
        "Telperion"            # 银树
    ]
    
    # 从命令行参数或使用默认文本
    if len(sys.argv) > 1:
        text = sys.argv[1]
    else:
        # 使用经典问候语
        text = quenya_texts[3]  # "Elen síla lúmenn' omentielvo"
    
    # 可选：指定语音
    voice = sys.argv[2] if len(sys.argv) > 2 else 'af_sarah'
    
    main(text, voice)
