#!/usr/bin/env python3
"""
对比 Python 版本和 C 版本的 G2P 输出质量

用法:
    python compare_g2p.py "你好世界"
    python compare_g2p.py "Hello world"
"""

import sys
import subprocess
from pathlib import Path


def get_python_g2p(text: str) -> str:
    """获取 Python 版本的 G2P 结果"""
    try:
        # 尝试运行 Python 版 misaki
        result = subprocess.run(
            ["python3", "-c", f"from misaki import text_to_phonemes; print(text_to_phonemes('{text}'))"],
            capture_output=True,
            text=True,
            timeout=5
        )
        
        if result.returncode == 0:
            return result.stdout.strip()
        else:
            return f"错误: {result.stderr.strip()}"
    except Exception as e:
        return f"错误: {e}"


def get_c_g2p(text: str) -> tuple[str, str]:
    """获取 C 版本的 G2P 结果
    
    返回: (音素序列, 完整输出)
    """
    c_binary = Path("misaki_c_port/build/misaki")
    
    if not c_binary.exists():
        return ("未编译", "C 版本程序未找到，请先编译:\n  cd misaki_c_port/build && cmake .. && make")
    
    try:
        result = subprocess.run(
            [str(c_binary), text],
            capture_output=True,
            text=True,
            check=True
        )
        
        # 提取音素序列
        lines = result.stdout.split("\n")
        phonemes = ""
        for line in lines:
            if "音素序列:" in line:
                phonemes = line.split("音素序列:")[1].strip()
                break
        
        return (phonemes, result.stdout)
    except subprocess.CalledProcessError as e:
        return ("错误", e.stderr)
    except Exception as e:
        return ("错误", str(e))


def compare(text: str):
    """对比两个版本的输出"""
    print("=" * 80)
    print(f"📝 测试文本: {text}")
    print("=" * 80)
    print()
    
    # Python 版本
    print("🐍 Python 版本 (misaki):")
    print("-" * 80)
    py_result = get_python_g2p(text)
    print(f"音素: {py_result}")
    print()
    
    # C 版本
    print("⚡ C 版本 (misaki_c_port):")
    print("-" * 80)
    c_phonemes, c_output = get_c_g2p(text)
    print(f"音素: {c_phonemes}")
    print()
    
    # 对比
    print("📊 对比结果:")
    print("-" * 80)
    if py_result == c_phonemes:
        print("✅ 完全一致！")
    else:
        print("⚠️  存在差异:")
        print(f"  Python: {py_result}")
        print(f"  C 版本: {c_phonemes}")
        
        # 计算相似度（简单字符匹配）
        py_set = set(py_result.split())
        c_set = set(c_phonemes.split())
        
        if py_set and c_set:
            common = py_set & c_set
            similarity = len(common) / max(len(py_set), len(c_set)) * 100
            print(f"  相似度: {similarity:.1f}%")
            
            if py_set - c_set:
                print(f"  Python 独有: {py_set - c_set}")
            if c_set - py_set:
                print(f"  C 独有: {c_set - py_set}")
    
    print()
    print("=" * 80)
    print()


def main():
    if len(sys.argv) < 2:
        print("用法: python compare_g2p.py <文本>")
        print("\n示例:")
        print('  python compare_g2p.py "你好世界"')
        print('  python compare_g2p.py "Hello world"')
        print('  python compare_g2p.py "こんにちは"')
        sys.exit(1)
    
    text = sys.argv[1]
    compare(text)


if __name__ == "__main__":
    main()
