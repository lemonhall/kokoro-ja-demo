# pip install misaki[ja]
# 1️⃣  Initalize a pipeline
from kokoro import KPipeline
import soundfile as sf
import torch


# 🇯🇵 'j' => Japanese: pip install misaki[ja]
pipeline = KPipeline(lang_code='j') # <= make sure lang_code matches voice

text = '「もしおれがただ偶然、そしてこうしようというつもりでなくここに立っているのなら、ちょっとばかり絶望するところだな」と、そんなことが彼の頭に思い浮かんだ。'
# 4️⃣ Generate, display, and save audio files in a loop.
generator = pipeline(
    text, voice='jf_nezumi', # <= change voice here
    speed=1, split_pattern=r'\n+'
)

for i, (gs, ps, audio) in enumerate(generator):
    print(i)  # i => index
    print(gs) # gs => graphemes/text
    print(ps) # ps => phonemes
    sf.write(f'{i}.wav', audio, 24000) # save each audio file