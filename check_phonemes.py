from kokoro import KPipeline

p = KPipeline(lang_code='j')

tests = [
    'こんにちは',
    'こんにち',
    'こん',
    'にち',
    'ち',
    'して',
    'て',
    'ます',
    'ました',
    'た'
]

for text in tests:
    result = list(p(text, voice='jf_nezumi', speed=1))
    phonemes = result[0][1]
    print(f"{text:10s} -> {phonemes}")
