"""
Kokoro TTS 简单示例
"""

from kokoro import KPipeline
import soundfile as sf

# 初始化（日文）
pipeline = KPipeline(lang_code='j')

# 生成语音
text = "こんにちは、世界。"
for i, (graphemes, phonemes, audio) in enumerate(pipeline(text, voice='jf_nezumi', speed=1)):
    print(f"文本: {graphemes}")
    print(f"音素: {phonemes}")
    sf.write(f'output_{i}.wav', audio, 24000)
    print(f"已保存: output_{i}.wav\n")
