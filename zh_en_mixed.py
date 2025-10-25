# pip install misaki[zh]
# 1️⃣  Initalize a pipeline
from kokoro import KPipeline
import soundfile as sf
import torch


# 🇨🇳 'z' => Mandarin Chinese: pip install misaki[zh]
pipeline = KPipeline(lang_code='z') # <= make sure lang_code matches voice

text = '今天的presentation很成功'

# 4️⃣ Generate, display, and save audio files in a loop.
generator = pipeline(
    text, voice='zf_xiaoxiao', # <= change voice here
    speed=1, split_pattern=r'\n+'
)

for i, (gs, ps, audio) in enumerate(generator):
    print(i)  # i => index
    print(gs) # gs => graphemes/text
    print(ps) # ps => phonemes
    sf.write(f'{i}.wav', audio, 24000) # save each audio file