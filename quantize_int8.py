"""
Kokoro ONNX 模型 INT8 量化

将 ~310MB 的模型压缩到 ~80MB
速度提升 2-4x，质量损失最小
"""

from onnxruntime.quantization import quantize_dynamic, QuantType
import argparse
import os


def quantize_model_int8(input_model, output_model=None):
    """
    INT8 动态量化
    
    Args:
        input_model: 输入 ONNX 模型路径
        output_model: 输出量化模型路径
    """
    if output_model is None:
        base, ext = os.path.splitext(input_model)
        output_model = f"{base}_int8{ext}"
    
    print("=" * 70)
    print("Kokoro ONNX INT8 量化")
    print("=" * 70)
    
    input_size = os.path.getsize(input_model) / (1024 * 1024)
    print(f"\n📊 输入模型:")
    print(f"   文件: {input_model}")
    print(f"   大小: {input_size:.2f} MB")
    
    print(f"\n⚙️  开始量化...")
    print(f"   类型: INT8 动态量化")
    print(f"   这会压缩权重到 int8，推理时动态转换")
    
    try:
        quantize_dynamic(
            model_input=input_model,
            model_output=output_model,
            weight_type=QuantType.QInt8
            # 可以指定要排除的操作类型
            # op_types_to_quantize=['Conv', 'MatMul']
        )
        
        output_size = os.path.getsize(output_model) / (1024 * 1024)
        compression_ratio = (1 - output_size / input_size) * 100
        
        print(f"\n✅ 量化成功!")
        print(f"\n📊 输出模型:")
        print(f"   文件: {os.path.abspath(output_model)}")
        print(f"   大小: {output_size:.2f} MB")
        print(f"   压缩: {compression_ratio:.1f}% (从 {input_size:.2f} MB 到 {output_size:.2f} MB)")
        
        print(f"\n💡 性能预期:")
        print(f"   - 大小: ~{compression_ratio:.0f}% 减少")
        print(f"   - 速度: 2-4x 提升 (取决于硬件)")
        print(f"   - 质量: 对 Kokoro 影响很小")
        
        return True
        
    except Exception as e:
        print(f"\n❌ 量化失败: {e}")
        import traceback
        traceback.print_exc()
        return False


def main():
    parser = argparse.ArgumentParser(description='量化 Kokoro ONNX 模型到 INT8')
    parser.add_argument(
        'input',
        nargs='?',
        default='kokoro_latest.onnx',
        help='输入 ONNX 模型路径'
    )
    parser.add_argument(
        '--output',
        help='输出量化模型路径 (默认: 输入文件名_int8.onnx)'
    )
    
    args = parser.parse_args()
    
    if not os.path.exists(args.input):
        print(f"❌ 文件不存在: {args.input}")
        print(f"\n💡 使用方法:")
        print(f"   python quantize_int8.py [输入模型.onnx]")
        return
    
    success = quantize_model_int8(args.input, args.output)
    
    if success:
        print("\n" + "=" * 70)
        print("🎉 完成!")
        print("=" * 70)
        print(f"\n📝 下一步:")
        print(f"  测试量化模型: python test_onnx_inference.py {args.output or args.input.replace('.onnx', '_int8.onnx')}")


if __name__ == "__main__":
    main()
