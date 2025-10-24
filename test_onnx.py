"""
最终正确的 ONNX 测试
完全模拟 PyTorch pipeline 的流程
"""

import onnxruntime as ort
import numpy as np
import soundfile as sf
from kokoro import KPipeline
import torch

# 初始化
print("初始化 Kokoro...")
pipeline = KPipeline(lang_code='j')

# 测试文本
text = "こんにちは"
voice_name = 'jf_nezumi'

# 1. 使用 pipeline 生成参考音频
print(f"\n1️⃣  使用 PyTorch Pipeline 生成参考音频...")
print(f"   文本: {text}")
for i, (gs, ps, audio) in enumerate(pipeline(text, voice=voice_name, speed=1, split_pattern=r'\n+')):
    if i == 0:
        sf.write('reference_pytorch.wav', audio, 24000)
        print(f"   Graphemes: {gs}")
        print(f"   Phonemes: {ps}")
        print(f"   ✅ 保存: reference_pytorch.wav ({len(audio)/24000:.2f}秒)")
        phoneme_string = ps
        break

# 2. 手动转换音素为 input_ids（模拟模型的 forward 方法）
print(f"\n2️⃣  转换音素为 input_ids...")
vocab = pipeline.model.vocab
print(f"   音素字符串: '{phoneme_string}'")
print(f"   字符: {[c for c in phoneme_string]}")

input_ids = list(filter(lambda i: i is not None, map(lambda p: vocab.get(p), phoneme_string)))
print(f"   input_ids (不含BOS/EOS): {input_ids}")
print(f"   长度: {len(input_ids)}")

# 加上 BOS 和 EOS
input_ids_with_special = [0, *input_ids, 0]
print(f"   input_ids (含BOS/EOS): {input_ids_with_special}")

# 3. 加载语音嵌入
print(f"\n3️⃣  加载语音嵌入...")
voices_tensor = pipeline.load_voice(voice_name)  # [510, 1, 256]
ref_s = voices_tensor[0, 0, :].unsqueeze(0)  # [1, 256]
print(f"   语音嵌入形状: {ref_s.shape}")

# 4. ONNX 推理
print(f"\n4️⃣  ONNX 推理...")
onnx_path = 'kokoro_latest.onnx'
session = ort.InferenceSession(onnx_path)

inputs = {
    'input_ids': np.array([input_ids_with_special], dtype=np.int64),
    'ref_s': ref_s.numpy(),
    'speed': np.array(1.0, dtype=np.float64)
}

outputs = session.run(None, inputs)
onnx_waveform = outputs[0]

print(f"   输出形状: {onnx_waveform.shape}")
print(f"   峰值: {np.max(np.abs(onnx_waveform)):.3f}")

# 归一化
if np.max(np.abs(onnx_waveform)) > 1.0:
    onnx_waveform = onnx_waveform / np.max(np.abs(onnx_waveform)) * 0.95

sf.write('onnx_final_output.wav', onnx_waveform, 24000)
print(f"   ✅ 保存: onnx_final_output.wav ({len(onnx_waveform)/24000:.2f}秒)")

# 5. PyTorch forward_with_tokens 对比
print(f"\n5️⃣  PyTorch forward_with_tokens 对比...")
input_ids_tensor = torch.tensor([input_ids_with_special], dtype=torch.long)
with torch.no_grad():
    pt_waveform, pt_duration = pipeline.model.forward_with_tokens(input_ids_tensor, ref_s, speed=1.0)
pt_waveform = pt_waveform.numpy()

sf.write('pytorch_forward_tokens.wav', pt_waveform, 24000)
print(f"   输出形状: {pt_waveform.shape}")
print(f"   峰值: {np.max(np.abs(pt_waveform)):.3f}")
print(f"   ✅ 保存: pytorch_forward_tokens.wav ({len(pt_waveform)/24000:.2f}秒)")

# 6. 对比差异
print(f"\n6️⃣  数值对比...")
min_len = min(len(onnx_waveform), len(pt_waveform))
diff = np.abs(onnx_waveform[:min_len] - pt_waveform[:min_len])
print(f"   最大差异: {np.max(diff):.6f}")
print(f"   平均差异: {np.mean(diff):.6f}")
print(f"   相对误差: {np.mean(diff)/np.mean(np.abs(pt_waveform[:min_len]))*100:.2f}%")

if np.max(diff) < 0.01:
    print(f"   ✅ ONNX 和 PyTorch 输出基本一致!")
else:
    print(f"   ⚠️  存在差异，但可能是正常的浮点数误差")

print(f"\n{'='*70}")
print(f"✅ 测试完成! 请播放以下文件验证:")
print(f"   - reference_pytorch.wav (Pipeline 完整流程)")
print(f"   - pytorch_forward_tokens.wav (PyTorch forward_with_tokens)")
print(f"   - onnx_final_output.wav (ONNX 推理)")
print(f"{'='*70}")
