"""
Kokoro ONNX 模型 FP16 转换

将 ~310MB 的模型压缩到 ~155MB
在支持 FP16 的硬件上速度更快
"""

from onnxconverter_common import float16
import onnx
import argparse
import os


def convert_model_fp16(input_model, output_model=None):
    """
    FP16 转换
    
    Args:
        input_model: 输入 ONNX 模型路径
        output_model: 输出 FP16 模型路径
    """
    if output_model is None:
        base, ext = os.path.splitext(input_model)
        output_model = f"{base}_fp16{ext}"
    
    print("=" * 70)
    print("Kokoro ONNX FP16 转换")
    print("=" * 70)
    
    input_size = os.path.getsize(input_model) / (1024 * 1024)
    print(f"\n📊 输入模型:")
    print(f"   文件: {input_model}")
    print(f"   大小: {input_size:.2f} MB")
    
    print(f"\n⚙️  开始转换...")
    print(f"   类型: FP32 -> FP16")
    print(f"   这会将所有权重从 32位 转换为 16位浮点数")
    
    try:
        # 加载模型
        model = onnx.load(input_model)
        
        # 转换为 FP16
        model_fp16 = float16.convert_float_to_float16(model)
        
        # 保存
        onnx.save(model_fp16, output_model)
        
        output_size = os.path.getsize(output_model) / (1024 * 1024)
        compression_ratio = (1 - output_size / input_size) * 100
        
        print(f"\n✅ 转换成功!")
        print(f"\n📊 输出模型:")
        print(f"   文件: {os.path.abspath(output_model)}")
        print(f"   大小: {output_size:.2f} MB")
        print(f"   压缩: {compression_ratio:.1f}% (从 {input_size:.2f} MB 到 {output_size:.2f} MB)")
        
        print(f"\n💡 性能预期:")
        print(f"   - 大小: ~50% 减少")
        print(f"   - 速度: 在支持 FP16 的 GPU/NPU 上更快")
        print(f"   - 质量: 通常影响很小")
        print(f"\n⚠️  注意:")
        print(f"   - CPU 上可能没有加速效果")
        print(f"   - GPU/NPU (如 ARM Mali, Adreno) 会有明显加速")
        
        return True
        
    except Exception as e:
        print(f"\n❌ 转换失败: {e}")
        import traceback
        traceback.print_exc()
        return False


def main():
    parser = argparse.ArgumentParser(description='转换 Kokoro ONNX 模型到 FP16')
    parser.add_argument(
        'input',
        nargs='?',
        default='kokoro_latest.onnx',
        help='输入 ONNX 模型路径'
    )
    parser.add_argument(
        '--output',
        help='输出 FP16 模型路径 (默认: 输入文件名_fp16.onnx)'
    )
    
    args = parser.parse_args()
    
    if not os.path.exists(args.input):
        print(f"❌ 文件不存在: {args.input}")
        print(f"\n💡 使用方法:")
        print(f"   python convert_fp16.py [输入模型.onnx]")
        return
    
    success = convert_model_fp16(args.input, args.output)
    
    if success:
        print("\n" + "=" * 70)
        print("🎉 完成!")
        print("=" * 70)
        print(f"\n📝 下一步:")
        print(f"  测试 FP16 模型: python test_onnx_inference.py {args.output or args.input.replace('.onnx', '_fp16.onnx')}")


if __name__ == "__main__":
    try:
        import onnxconverter_common
    except ImportError:
        print("❌ 缺少依赖，请先安装:")
        print("   uv add onnxconverter-common")
        exit(1)
    
    main()
