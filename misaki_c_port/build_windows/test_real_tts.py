"""
test_real_tts.py

真刀真枪测试：C 版本 vs Python 版本 Misaki G2P + ONNX TTS
生成两个音频文件进行对比
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

# 导入依赖
try:
    from misaki.zh import ZHG2P
    from kokoro import KPipeline
    import soundfile as sf
    import onnxruntime as ort
    DEPS_OK = True
except ImportError as e:
    print(f"❌ 依赖缺失: {e}")
    DEPS_OK = False


class MisakiCG2P:
    """C 语言版 Misaki G2P"""
    
    def __init__(self, dll_path=None, data_dir=None):
        if dll_path is None:
            dll_path = Path(__file__).parent / "libmisaki.dll"
        if data_dir is None:
            data_dir = "extracted_data"
        
        old_cwd = os.getcwd()
        os.chdir(Path(dll_path).parent)
        self.lib = ctypes.CDLL(str(Path(dll_path).absolute()))
        os.chdir(old_cwd)
        
        self.lib.misaki_init.argtypes = [ctypes.c_char_p]
        self.lib.misaki_init.restype = ctypes.c_int
        self.lib.misaki_text_to_phonemes.argtypes = [ctypes.c_char_p, ctypes.c_char_p, ctypes.c_int]
        self.lib.misaki_text_to_phonemes.restype = ctypes.c_int
        self.lib.misaki_cleanup.argtypes = []
        self.lib.misaki_cleanup.restype = None
        
        result = self.lib.misaki_init(data_dir.encode('utf-8'))
        if result != 0:
            raise RuntimeError("C Misaki 初始化失败")
    
    def text_to_phonemes(self, text):
        output_buffer = ctypes.create_string_buffer(2048)
        result = self.lib.misaki_text_to_phonemes(text.encode('utf-8'), output_buffer, len(output_buffer))
        return output_buffer.value.decode('utf-8') if result == 0 else None
    
    def __del__(self):
        if hasattr(self, 'lib'):
            self.lib.misaki_cleanup()


def synthesize_with_phonemes(phonemes, voice_name='zf_xiaoxiao', onnx_path=None):
    """
    使用音素直接调用 ONNX 模型合成
    """
    if onnx_path is None:
        onnx_path = project_root / "kokoro_latest.onnx"
    
    if not onnx_path.exists():
        print(f"❌ 模型文件不存在: {onnx_path}")
        return None
    
    # 初始化 pipeline 获取 vocab 和 voice
    pipeline = KPipeline(lang_code='z')
    vocab = pipeline.model.vocab
    
    # 音素转 input_ids
    input_ids = list(filter(lambda i: i is not None, map(lambda p: vocab.get(p), phonemes)))
    input_ids_with_special = [0, *input_ids, 0]  # 加 BOS/EOS
    
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


def main(text):
    if not DEPS_OK:
        print("请先安装依赖")
        return
    
    print("=" * 70)
    print("🎯 真刀真枪 TTS 对比测试")
    print("=" * 70)
    print(f"\n📝 输入文本: {text}\n")
    
    # 1. C 版本 G2P
    print("🔧 步骤 1: C 语言版 Misaki G2P")
    c_g2p = MisakiCG2P()
    c_phonemes = c_g2p.text_to_phonemes(text)
    print(f"   音素: {c_phonemes}\n")
    
    # 2. Python 版本 G2P
    print("🐍 步骤 2: Python 原版 Misaki G2P")
    py_g2p = ZHG2P()
    py_phonemes, _ = py_g2p(text)
    print(f"   音素: {py_phonemes}\n")
    
    # 3. C 版本 TTS
    print("🎤 步骤 3: C 版本音素 → ONNX TTS")
    c_audio = synthesize_with_phonemes(c_phonemes, 'zf_xiaoxiao')
    if c_audio is not None:
        sf.write('misaki_c_version.wav', c_audio, 24000)
        print(f"   ✅ 保存: misaki_c_version.wav ({len(c_audio)/24000:.2f}秒)\n")
    
    # 4. Python 版本 TTS
    print("🎤 步骤 4: Python 版本音素 → ONNX TTS")
    py_audio = synthesize_with_phonemes(py_phonemes, 'zf_xiaoxiao')
    if py_audio is not None:
        sf.write('misaki_python_version.wav', py_audio, 24000)
        print(f"   ✅ 保存: misaki_python_version.wav ({len(py_audio)/24000:.2f}秒)\n")
    
    # 5. 对比
    print("=" * 70)
    print("🎯 对比结果")
    print("=" * 70)
    print(f"C 版本音素:      {c_phonemes}")
    print(f"Python 版本音素: {py_phonemes}")
    print(f"\n音素是否相同: {'✅ 是' if c_phonemes.strip() == py_phonemes.strip() else '❌ 否'}")
    print(f"\n🎵 生成的音频文件:")
    print(f"   - misaki_c_version.wav      (C 语言版 G2P)")
    print(f"   - misaki_python_version.wav (Python 原版 G2P)")
    print(f"\n💡 请听听看哪个更自然！")
    print("=" * 70)


if __name__ == "__main__":
    text = sys.argv[1] if len(sys.argv) > 1 else "当人们说起人工智能时，通常指的是机器学习和深度学习技术"
    main(text)
