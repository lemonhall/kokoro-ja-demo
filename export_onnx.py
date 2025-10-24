"""
Kokoro TTS 最新版本导出到 ONNX

使用 Kokoro 内置的 KModelForONNX 包装器进行导出
支持最新的 Kokoro 模型
"""

import torch
import torch.onnx
from kokoro.model import KModel, KModelForONNX
import os
import argparse


def export_kokoro_to_onnx(
    repo_id='hexgrad/Kokoro-82M',
    output_path='kokoro_latest.onnx',
    opset_version=17,
    seq_length=50
):
    """
    导出 Kokoro 模型到 ONNX
    
    Args:
        repo_id: HuggingFace 模型仓库 ID
        output_path: 输出 ONNX 文件路径
        opset_version: ONNX opset 版本
        seq_length: 测试序列长度
    """
    print("=" * 70)
    print("Kokoro TTS -> ONNX 导出工具")
    print("=" * 70)
    
    # 1. 加载模型
    print(f"\n🔧 加载模型...")
    print(f"   仓库: {repo_id}")
    
    try:
        # disable_complex=True 避免 STFT 使用复数（ONNX 不支持）
        kmodel = KModel(repo_id=repo_id, disable_complex=True)
        kmodel.eval()
        print(f"✅ 模型加载成功")
        print(f"   词汇表大小: {len(kmodel.vocab)}")
    except Exception as e:
        print(f"❌ 模型加载失败: {e}")
        return False
    
    # 2. 创建 ONNX 包装器
    print(f"\n📦 准备 ONNX 导出...")
    onnx_model = KModelForONNX(kmodel)
    onnx_model.eval()
    
    # 3. 准备虚拟输入
    batch_size = 1
    n_vocab = len(kmodel.vocab)
    
    # input_ids: [batch, seq_len] 包含 BOS 和 EOS
    input_ids = torch.randint(1, n_vocab, (batch_size, seq_length), dtype=torch.long)
    input_ids = torch.cat([
        torch.zeros(batch_size, 1, dtype=torch.long),  # BOS
        input_ids,
        torch.zeros(batch_size, 1, dtype=torch.long),  # EOS
    ], dim=1)
    
    # ref_s: [batch, 256] - 语音参考嵌入
    ref_s = torch.randn(batch_size, 256)
    
    # speed: scalar
    speed = 1.0
    
    print(f"   输入形状:")
    print(f"     input_ids: {input_ids.shape}")
    print(f"     ref_s: {ref_s.shape}")
    print(f"     speed: {speed}")
    
    # 4. 导出到 ONNX
    print(f"\n⚙️  执行 ONNX 导出...")
    print(f"   输出: {output_path}")
    print(f"   Opset: {opset_version}")
    print(f"   注意: 这可能需要几分钟...")
    
    try:
        # 定义动态轴
        dynamic_axes = {
            'input_ids': {0: 'batch', 1: 'seq_length'},
            'ref_s': {0: 'batch'},
            'waveform': {0: 'time'},
            'duration': {0: 'seq_length'}
        }
        
        # 导出（不使用 dynamo 因为 LSTM 不支持）
        with torch.no_grad():
            torch.onnx.export(
                onnx_model,
                (input_ids, ref_s, speed),
                output_path,
                export_params=True,
                opset_version=opset_version,
                do_constant_folding=True,
                input_names=['input_ids', 'ref_s', 'speed'],
                output_names=['waveform', 'duration'],
                dynamic_axes=dynamic_axes,
                verbose=False
            )
        
        file_size = os.path.getsize(output_path) / (1024 * 1024)
        print(f"\n✅ 导出成功!")
        print(f"   文件: {os.path.abspath(output_path)}")
        print(f"   大小: {file_size:.2f} MB")
        
        return True
        
    except Exception as e:
        print(f"\n❌ 导出失败: {e}")
        print(f"\n可能的原因:")
        print(f"  - STFT 层问题 (尝试使用 disable_complex=True)")
        print(f"  - 动态操作不兼容")
        print(f"  - PyTorch/ONNX 版本问题")
        
        import traceback
        traceback.print_exc()
        return False


def verify_onnx(onnx_path):
    """验证 ONNX 模型"""
    print(f"\n🔍 验证 ONNX 模型...")
    
    try:
        import onnx
        import onnxruntime as ort
        
        # 检查模型
        model = onnx.load(onnx_path)
        onnx.checker.check_model(model)
        print("✅ 模型结构验证通过")
        
        # 加载到 ONNX Runtime
        session = ort.InferenceSession(onnx_path)
        print(f"✅ ONNX Runtime 加载成功")
        
        # 打印信息
        print("\n📊 模型信息:")
        print("  输入:")
        for inp in session.get_inputs():
            print(f"    {inp.name}: {inp.shape} ({inp.type})")
        print("  输出:")
        for out in session.get_outputs():
            print(f"    {out.name}: {out.shape} ({out.type})")
        
        return True
        
    except ImportError:
        print("⚠️  需要安装: pip install onnx onnxruntime")
        return False
    except Exception as e:
        print(f"❌ 验证失败: {e}")
        return False


def main():
    parser = argparse.ArgumentParser(description='导出 Kokoro TTS 到 ONNX')
    parser.add_argument(
        '--repo-id', 
        default='hexgrad/Kokoro-82M',
        help='HuggingFace 模型仓库 ID'
    )
    parser.add_argument(
        '--output', 
        default='kokoro_latest.onnx',
        help='输出文件路径'
    )
    parser.add_argument(
        '--opset', 
        type=int, 
        default=17,
        help='ONNX opset 版本'
    )
    parser.add_argument(
        '--seq-length',
        type=int,
        default=50,
        help='测试序列长度'
    )
    
    args = parser.parse_args()
    
    # 导出
    success = export_kokoro_to_onnx(
        repo_id=args.repo_id,
        output_path=args.output,
        opset_version=args.opset,
        seq_length=args.seq_length
    )
    
    # 验证
    if success:
        verify_onnx(args.output)
        
        print("\n" + "=" * 70)
        print("🎉 完成!")
        print("=" * 70)
        print(f"\n📝 下一步:")
        print(f"  1. 测试推理: python test_onnx_inference.py")
        print(f"  2. INT8 量化: python quantize_int8.py")
        print(f"  3. FP16 转换: python convert_fp16.py")
        print(f"\n💡 提示:")
        print(f"  - INT8 量化: ~326MB -> ~80MB (推荐移动端)")
        print(f"  - FP16 转换: ~326MB -> ~163MB (中端设备)")


if __name__ == "__main__":
    # 检查依赖
    try:
        import onnx
        import onnxruntime
    except ImportError:
        print("⚠️  缺少依赖，请先安装:")
        print("   uv add onnx onnxruntime")
        exit(1)
    
    main()
