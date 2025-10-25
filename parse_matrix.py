"""
解析 MeCab matrix.bin 文件

MeCab 的 matrix.bin 格式：
- 前 4 字节：left_size (左词性数量，short)
- 接着 4 字节：right_size (右词性数量，short)
- 后面是 left_size x right_size 的成本矩阵（每个 short，2字节）

cost = matrix[left_id][right_id]
"""

import struct
import os

def parse_matrix_bin(file_path):
    """解析 matrix.bin 文件"""
    
    with open(file_path, 'rb') as f:
        # 读取前 4 字节：left_size 和 right_size (各 2 字节，short)
        header = f.read(4)
        left_size, right_size = struct.unpack('<HH', header)
        
        print(f"📊 Matrix 信息:")
        print(f"  - Left size (前接词性数): {left_size}")
        print(f"  - Right size (后接词性数): {right_size}")
        print(f"  - 矩阵大小: {left_size} x {right_size}")
        print(f"  - 预期数据大小: {left_size * right_size * 2} 字节\n")
        
        # 读取矩阵数据
        matrix_size = left_size * right_size
        matrix_data = f.read(matrix_size * 2)  # 每个元素 2 字节
        
        if len(matrix_data) != matrix_size * 2:
            print(f"⚠️ 警告: 数据大小不匹配!")
            print(f"  预期: {matrix_size * 2} 字节")
            print(f"  实际: {len(matrix_data)} 字节")
            return None
        
        # 解析为 short 数组
        costs = struct.unpack(f'<{matrix_size}h', matrix_data)
        
        # 转换为 numpy 矩阵 (left_size x right_size)
        matrix = np.array(costs, dtype=np.int16).reshape(left_size, right_size)
        
        return matrix, left_size, right_size

def analyze_matrix(matrix):
    """分析矩阵统计信息"""
    print(f"📈 矩阵统计:")
    print(f"  - 最小成本: {matrix.min()}")
    print(f"  - 最大成本: {matrix.max()}")
    print(f"  - 平均成本: {matrix.mean():.2f}")
    print(f"  - 中位数: {np.median(matrix):.2f}\n")
    
    # 显示矩阵的一小部分
    print(f"🔍 矩阵前 10x10 预览:")
    print(matrix[:10, :10])
    print()

def export_simplified_matrix(matrix, left_size, right_size, output_file):
    """导出简化的文本格式矩阵（用于 C 代码）"""
    
    # 采样：只保留部分常用词性组合
    # 可以通过统计最常见的词性 ID 来优化
    sample_size = min(100, left_size, right_size)
    
    with open(output_file, 'w', encoding='utf-8') as f:
        f.write(f"# MeCab Transition Cost Matrix (Simplified)\n")
        f.write(f"# Original size: {left_size} x {right_size}\n")
        f.write(f"# Sampled size: {sample_size} x {sample_size}\n")
        f.write(f"# Format: left_id right_id cost\n\n")
        
        count = 0
        for i in range(min(sample_size, left_size)):
            for j in range(min(sample_size, right_size)):
                cost = matrix[i, j]
                f.write(f"{i}\t{j}\t{cost}\n")
                count += 1
        
        print(f"✅ 导出了 {count} 个词性转移成本")

if __name__ == "__main__":
    import sys
    
    # matrix.bin 路径
    matrix_path = ".venv/Lib/site-packages/unidic/dicdir/matrix.bin"
    
    if not os.path.exists(matrix_path):
        print(f"❌ 找不到 matrix.bin: {matrix_path}")
        sys.exit(1)
    
    print(f"📖 解析 matrix.bin: {matrix_path}\n")
    
    # 解析矩阵
    result = parse_matrix_bin(matrix_path)
    if result is None:
        sys.exit(1)
    
    matrix, left_size, right_size = result
    
    # 分析矩阵
    analyze_matrix(matrix)
    
    # 导出简化版本
    output_file = "extracted_data/ja/transition_matrix.txt"
    export_simplified_matrix(matrix, left_size, right_size, output_file)
    
    print(f"\n✅ 完成！简化矩阵已保存到: {output_file}")
