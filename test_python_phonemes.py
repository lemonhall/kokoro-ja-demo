from misaki import text_to_phonemes

words = ['心情', '新颖', '经营', '音乐']

print("=" * 60)
print("Python版本 (Misaki) 音素输出")
print("=" * 60)

for w in words:
    result = text_to_phonemes(w, language='zh')
    print(f'{w}: {result}')
