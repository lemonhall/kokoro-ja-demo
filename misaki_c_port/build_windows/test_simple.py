"""简化测试 - 只显示音素"""
import ctypes
from pathlib import Path
import sys

def test_simple(text):
    dll_path = Path(__file__).parent / "libmisaki.dll"
    import os
    os.chdir(dll_path.parent)
    lib = ctypes.CDLL(str(dll_path.absolute()))
    
    lib.misaki_init.argtypes = [ctypes.c_char_p]
    lib.misaki_init.restype = ctypes.c_int
    lib.misaki_text_to_phonemes.argtypes = [ctypes.c_char_p, ctypes.c_char_p, ctypes.c_int]
    lib.misaki_text_to_phonemes.restype = ctypes.c_int
    lib.misaki_cleanup.argtypes = []
    lib.misaki_cleanup.restype = None
    
    lib.misaki_init(b"extracted_data")
    output_buffer = ctypes.create_string_buffer(1024)
    result = lib.misaki_text_to_phonemes(text.encode('utf-8'), output_buffer, len(output_buffer))
    
    if result == 0:
        phonemes = output_buffer.value.decode('utf-8')
        print(f"Text: {text}")
        print(f"Hex: {phonemes.encode('utf-8').hex()}")
        # 解码hex显示
        import binascii
        phonemes_bytes = phonemes.encode('utf-8')
        print(f"Phonemes ({len(phonemes)} chars, {len(phonemes_bytes)} bytes)")
    else:
        print(f"Failed for: {text}")
    
    lib.misaki_cleanup()

if __name__ == "__main__":
    for word in ["心情", "新颖", "经营", "音乐"]:
        test_simple(word)
