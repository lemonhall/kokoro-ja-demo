#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
从 jieba 提取 HMM 参数

HMM 用于未登录词识别（人名、地名等）
"""

import pickle
from pathlib import Path
import json

def load_jieba_hmm():
    """加载 jieba 的 HMM 参数"""
    jieba_finalseg = Path(".venv/Lib/site-packages/jieba/finalseg")
    
    # 加载三个 HMM 参数
    with open(jieba_finalseg / "prob_start.p", "rb") as f:
        prob_start = pickle.load(f)
    
    with open(jieba_finalseg / "prob_trans.p", "rb") as f:
        prob_trans = pickle.load(f)
    
    with open(jieba_finalseg / "prob_emit.p", "rb") as f:
        prob_emit = pickle.load(f)
    
    return prob_start, prob_trans, prob_emit

def main():
    print("🚀 从 jieba 提取 HMM 参数...\n")
    
    prob_start, prob_trans, prob_emit = load_jieba_hmm()
    
    print("📊 HMM 参数统计:")
    print(f"  - 初始概率 (prob_start): {len(prob_start)} 个状态")
    print(f"    状态: {list(prob_start.keys())}")
    print(f"    概率: {prob_start}")
    
    print(f"\n  - 转移概率 (prob_trans): {len(prob_trans)} 个转移")
    for state, trans in prob_trans.items():
        print(f"    {state} -> {list(trans.keys())}: {len(trans)} 个转移")
    
    print(f"\n  - 发射概率 (prob_emit): {len(prob_emit)} 个状态")
    for state, emit in prob_emit.items():
        print(f"    {state}: {len(emit)} 个字符")
    
    # 保存为 JSON（方便 C 代码读取）
    output_dir = Path("misaki_c_port/extracted_data/zh")
    output_dir.mkdir(parents=True, exist_ok=True)
    
    # 转换为可序列化的格式
    hmm_data = {
        "prob_start": prob_start,
        "prob_trans": {k: dict(v) for k, v in prob_trans.items()},
        "prob_emit": {k: dict(v) for k, v in prob_emit.items()}
    }
    
    output_file = output_dir / "hmm_model.json"
    print(f"\n💾 保存到: {output_file}")
    
    with open(output_file, 'w', encoding='utf-8') as f:
        json.dump(hmm_data, f, ensure_ascii=False, indent=2)
    
    print("\n✅ 完成！")
    
    # 显示示例
    print("\n📝 HMM 状态说明:")
    print("  B - 词的开始 (Begin)")
    print("  M - 词的中间 (Middle)")
    print("  E - 词的结束 (End)")
    print("  S - 单字词 (Single)")
    
    print("\n🎯 应用场景:")
    print("  当词典中找不到词时，使用 HMM 预测最可能的分词方式")
    print("  例如：'李小明' → B(李) M(小) E(明) → 识别为人名")

if __name__ == "__main__":
    main()
