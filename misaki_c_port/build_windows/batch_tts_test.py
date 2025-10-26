"""
批量TTS对比测试 - 优化版
C版本 vs Python版本 G2P + ONNX TTS
常驻内存，批量生成
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


class GlobalTTSEngine:
    """全局TTS引擎 - 常驻内存"""
    
    def __init__(self):
        print("🚀 初始化全局TTS引擎...")
        
        # 1. 初始化C版本G2P
        dll_path = Path(__file__).parent / "libmisaki.dll"
        os.chdir(dll_path.parent)
        self.c_lib = ctypes.CDLL(str(dll_path.absolute()))
        
        self.c_lib.misaki_init.argtypes = [ctypes.c_char_p]
        self.c_lib.misaki_init.restype = ctypes.c_int
        self.c_lib.misaki_text_to_phonemes_lang.argtypes = [ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_int]
        self.c_lib.misaki_text_to_phonemes_lang.restype = ctypes.c_int
        
        result = self.c_lib.misaki_init(b"extracted_data")
        if result != 0:
            raise RuntimeError("C Misaki 初始化失败")
        print("   ✅ C版本G2P已加载")
        
        # 2. 初始化Python版本G2P
        self.py_g2p = ZHG2P()
        print("   ✅ Python版本G2P已加载")
        
        # 3. 初始化ONNX模型
        onnx_path = project_root / "kokoro_latest.onnx"
        if not onnx_path.exists():
            raise FileNotFoundError(f"模型文件不存在: {onnx_path}")
        
        self.onnx_session = ort.InferenceSession(str(onnx_path))
        print("   ✅ ONNX模型已加载")
        
        # 4. 初始化Kokoro Pipeline（获取vocab和voice）
        self.pipeline = KPipeline(lang_code='z')
        self.vocab = self.pipeline.model.vocab
        
        # 5. 预加载语音嵌入
        self.voice_name = 'zf_xiaoxiao'
        self.voices_tensor = self.pipeline.load_voice(self.voice_name)
        print(f"   ✅ 语音嵌入已加载 ({self.voice_name})")
        
        print("✅ 全局TTS引擎初始化完成\n")
    
    def c_g2p(self, text):
        """C版本G2P"""
        output_buffer = ctypes.create_string_buffer(2048)
        result = self.c_lib.misaki_text_to_phonemes_lang(
            text.encode('utf-8'), 
            b'zh',  # 强制中文
            output_buffer, 
            len(output_buffer)
        )
        return output_buffer.value.decode('utf-8') if result == 0 else None
    
    def py_g2p_convert(self, text):
        """Python版本G2P"""
        phonemes, _ = self.py_g2p(text)
        return phonemes
    
    def synthesize(self, phonemes):
        """音素转音频（复用ONNX session）"""
        # 音素转 input_ids
        input_ids = list(filter(lambda i: i is not None, map(lambda p: self.vocab.get(p), phonemes)))
        input_ids_with_special = [0, *input_ids, 0]
        
        # 动态选择语音帧
        frame_index = min(len(input_ids) - 1, self.voices_tensor.shape[0] - 1)
        ref_s = self.voices_tensor[frame_index, 0, :].unsqueeze(0).numpy()
        
        # ONNX推理
        inputs = {
            'input_ids': np.array([input_ids_with_special], dtype=np.int64),
            'ref_s': ref_s,
            'speed': np.array(1.0, dtype=np.float64)
        }
        
        outputs = self.onnx_session.run(None, inputs)
        waveform = outputs[0]
        
        # 归一化
        if np.max(np.abs(waveform)) > 1.0:
            waveform = waveform / np.max(np.abs(waveform)) * 0.95
        
        return waveform
    
    def cleanup(self):
        """清理资源"""
        if hasattr(self, 'c_lib'):
            self.c_lib.misaki_cleanup()


# 测试语料 - 批量处理
TEST_SENTENCES = [
    "我可以理解你的想法",
    "今天天气真不错",
    "明天我们去爬山",
    "北京市朝阳区",
    "中国人民解放军",
    "音乐让人心情愉悦",
    "心情不好需要调整",
    "新颖的设计理念",
    "经营一家餐厅",
    "时间过得真快啊",
    "这个问题很复杂",
    "我喜欢吃火锅",
    "他说的有道理",
    "你的建议很好",
    "我们一起努力吧",
    "知道吃饭的时候",
    "市场上正在杀猪",
    "太阳从东方升起",
    "月亮在天上挂着",
    "春天来了万物复苏",
]


def batch_test(engine, output_dir="batch_output"):
    """批量测试"""
    output_path = Path(output_dir)
    output_path.mkdir(exist_ok=True)
    
    print("="*70)
    print("🎯 批量TTS对比测试")
    print("="*70)
    print(f"📊 测试语料: {len(TEST_SENTENCES)} 句\n")
    
    for idx, text in enumerate(TEST_SENTENCES, 1):
        print(f"[{idx}/{len(TEST_SENTENCES)}] {text}")
        
        # 1. C版本G2P
        c_phonemes = engine.c_g2p(text)
        
        # 2. Python版本G2P
        py_phonemes = engine.py_g2p_convert(text)
        
        # 3. 对比音素
        match = "✅" if c_phonemes == py_phonemes else "❌"
        print(f"   C:  {c_phonemes}")
        print(f"   Py: {py_phonemes}")
        print(f"   {match} {'相同' if match == '✅' else '不同'}")
        
        # 4. 生成音频
        c_audio = engine.synthesize(c_phonemes)
        py_audio = engine.synthesize(py_phonemes)
        
        # 5. 保存音频
        c_filename = output_path / f"{idx:02d}_c_{text[:10]}.wav"
        py_filename = output_path / f"{idx:02d}_py_{text[:10]}.wav"
        
        sf.write(str(c_filename), c_audio, 24000)
        sf.write(str(py_filename), py_audio, 24000)
        
        print(f"   💾 已保存: {c_filename.name} / {py_filename.name}\n")
    
    print("="*70)
    print(f"✅ 批量测试完成！")
    print(f"📁 输出目录: {output_path.absolute()}")
    print(f"🎵 共生成: {len(TEST_SENTENCES) * 2} 个音频文件")
    print("="*70)


def main():
    if not DEPS_OK:
        print("❌ 请先安装依赖")
        return
    
    # 初始化全局引擎（常驻内存）
    engine = GlobalTTSEngine()
    
    try:
        # 批量测试
        batch_test(engine)
    finally:
        # 清理
        engine.cleanup()
        print("\n✅ 资源已清理")


if __name__ == "__main__":
    main()
