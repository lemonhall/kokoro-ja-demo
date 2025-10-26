hexes = [
    'c99569e286926e74c995cab069e28697c58b',
    'c99569e286926e69e28693c58b', 
    '74c99569e28692c58b69e28697c58b',
    '69e286926e79c99be28698'
]
words = ['心情', '新颖', '经营', '音乐']

print("\nC版本（当前）:")
for w, h in zip(words, hexes):
    ipa = bytes.fromhex(h).decode('utf-8')
    print(f'  {w}: {ipa}')

print("\nPython版本（目标）:")
print('  心情: ɕi→nʨʰi↗ŋ')
print('  新颖: ɕi→ni↓ŋ')
print('  经营: ʨi→ŋi↗ŋ')
print('  音乐: i→nɥe↘')
