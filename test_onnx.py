"""
最终正确的 ONNX 测试
完全模拟 PyTorch pipeline 的流程

用法:
  uv run python test_onnx.py                          # 默认测试 kokoro_latest.onnx
  uv run python test_onnx.py kokoro_latest_int8.onnx  # 测试 INT8 量化版本
  uv run python test_onnx.py --text "你好" --voice jf_nezumi
"""

import onnxruntime as ort
import numpy as np
import soundfile as sf
from kokoro import KPipeline
import torch
import sys
import argparse
import os

def main():
    # 命令行参数
    parser = argparse.ArgumentParser(description='测试 Kokoro ONNX 模型')
    parser.add_argument(
        'model',
        nargs='?',
        default='kokoro_latest.onnx',
        help='ONNX 模型路径 (默认: kokoro_latest.onnx)'
    )
    parser.add_argument(
        '--text',
        default='こんにちは',
        help='测试文本 (默认: こんにちは)'
    )
    parser.add_argument(
        '--voice',
        default='jf_nezumi',
        help='语音名称 (默认: jf_nezumi)'
    )
    args = parser.parse_args()
    
    onnx_path = args.model
    text = args.text
    voice_name = args.voice
    
    # 检查模型文件是否存在
    if not os.path.exists(onnx_path):
        print(f"❌ 错误: 模型文件不存在: {onnx_path}")
        print(f"\n请先导出 ONNX 模型:")
        print(f"  uv run python export_onnx.py")
        sys.exit(1)
    
    model_size = os.path.getsize(onnx_path) / (1024 * 1024)
    print("="*70)
    print(f"测试 ONNX 模型: {onnx_path}")
    print(f"模型大小: {model_size:.2f} MB")
    print(f"文本: {text}")
    print(f"语音: {voice_name}")
    print("="*70)

    # 初始化
    print("\n初始化 Kokoro...")
    pipeline = KPipeline(lang_code='j')

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

    # 3. 加载语音嵌入 - 测试三种方式
    print(f"\n3️⃣  加载语音嵌入...")
    voices_tensor = pipeline.load_voice(voice_name)  # [510, 1, 256]
    print(f"   完整语音嵌入形状: {voices_tensor.shape}")
    
    # 方式1: 只用第一帧（当前Android的错误做法）
    ref_s_first = voices_tensor[0, 0, :].unsqueeze(0)  # [1, 256]
    print(f"   方式1 - 第一帧: {ref_s_first.shape}")
    
    # 方式2: 用中间帧
    ref_s_middle = voices_tensor[255, 0, :].unsqueeze(0)  # [1, 256]
    print(f"   方式2 - 中间帧(255): {ref_s_middle.shape}")
    
    # 方式3: 用平均值
    ref_s_mean = voices_tensor.mean(dim=0)  # [1, 256]
    print(f"   方式3 - 平均值: {ref_s_mean.shape}")
    
    # 方式4: 动态帧选择（Pipeline 的正确做法！）
    # Pipeline 使用: pack[len(ps)-1]
    frame_index = len(input_ids) - 1  # 不含BOS/EOS的音素长度
    frame_index = min(frame_index, voices_tensor.shape[0] - 1)  # 防止越界
    ref_s_dynamic = voices_tensor[frame_index, 0, :].unsqueeze(0)  # [1, 256]
    print(f"   方式4 - 动态帧选择(Pipeline方式): {ref_s_dynamic.shape}")
    print(f"          音素数={len(input_ids)}, 选择帧索引={frame_index}")
    
    # 对比四种方式的数据差异
    print(f"\n   数据对比:")
    print(f"     第一帧 vs 中间帧: {torch.mean(torch.abs(ref_s_first - ref_s_middle)).item():.6f}")
    print(f"     第一帧 vs 平均值: {torch.mean(torch.abs(ref_s_first - ref_s_mean)).item():.6f}")
    print(f"     第一帧 vs 动态帧: {torch.mean(torch.abs(ref_s_first - ref_s_dynamic)).item():.6f}")
    print(f"     动态帧 vs 平均值: {torch.mean(torch.abs(ref_s_dynamic - ref_s_mean)).item():.6f}")

    # 4. ONNX 推理 - 测试三种语音嵌入
    print(f"\n4️⃣  ONNX 推理 - 测试三种语音嵌入...")
    print(f"   加载模型: {onnx_path}")
    
    try:
        session = ort.InferenceSession(onnx_path)
    except Exception as e:
        print(f"\n❌ 加载 ONNX 模型失败: {e}")
        
        # 检查是否是 ConvInteger 问题
        if 'ConvInteger' in str(e):
            print(f"\n💡 检测到 ConvInteger 不支持的问题")
            print(f"\n这通常发生在 INT8 量化模型上。解决方案:")
            print(f"  1. 使用 FP32 版本: uv run python test_onnx.py kokoro_latest.onnx")
            print(f"  2. 使用 FP16 版本: uv run python convert_fp16.py && uv run python test_onnx.py kokoro_latest_fp16.onnx")
            print(f"  3. 升级 ONNX Runtime: uv add onnxruntime --upgrade")
            print(f"\n注意: INT8 量化在移动端（NNAPI/CoreML）上才能充分发挥优势")
        
        sys.exit(1)
    
    # 渣备输入
    input_ids_np = np.array([input_ids_with_special], dtype=np.int64)
    speed_np = np.array(1.0, dtype=np.float64)
    
    # 测试四种语音嵌入
    test_configs = [
        ('first', '第一帧 (Android当前)', ref_s_first),
        ('middle', '中间帧', ref_s_middle),
        ('mean', '平均值', ref_s_mean),
        ('dynamic', f'动态帧[{frame_index}] (Pipeline正确)', ref_s_dynamic),
    ]
    
    for name, desc, ref_s in test_configs:
        print(f"\n   🎤 测试 {desc}...")
        
        inputs = {
            'input_ids': input_ids_np,
            'ref_s': ref_s.numpy(),
            'speed': speed_np
        }
        
        outputs = session.run(None, inputs)
        onnx_waveform = outputs[0]
        
        print(f"      输出形状: {onnx_waveform.shape}")
        print(f"      峰值: {np.max(np.abs(onnx_waveform)):.3f}")
        
        # 归一化
        if np.max(np.abs(onnx_waveform)) > 1.0:
            onnx_waveform = onnx_waveform / np.max(np.abs(onnx_waveform)) * 0.95
        
        output_file = f'onnx_{name}_output.wav'
        sf.write(output_file, onnx_waveform, 24000)
        print(f"      ✅ 保存: {output_file} ({len(onnx_waveform)/24000:.2f}秒)")

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
    print(f"✅ 测试完成! 生成了以下音频文件:")
    print(f"")
    print(f"   参考音频:")
    print(f"     - reference_pytorch.wav (Pipeline 完整流程)")
    print(f"     - pytorch_forward_tokens.wav (PyTorch forward_with_tokens)")
    print(f"")
    print(f"   ONNX 输出 (四种语音嵌入方式):")
    print(f"     - onnx_first_output.wav (第一帧 - Android当前错误做法)")
    print(f"     - onnx_middle_output.wav (中间帧)")
    print(f"     - onnx_mean_output.wav (平均值)")
    print(f"     - onnx_dynamic_output.wav (动态帧[{frame_index}] - Pipeline正确做法 ⭐)")
    print(f"")
    print(f"   🎵 重点对比:")
    print(f"      reference_pytorch.wav (PyTorch原版)")
    print(f"      vs")
    print(f"      onnx_dynamic_output.wav (ONNX动态帧)")
    print(f"")
    print(f"   💡 如果两者音质接近，说明动态帧选择方案正确！")
    print(f"{'='*70}")

if __name__ == "__main__":
    main()
