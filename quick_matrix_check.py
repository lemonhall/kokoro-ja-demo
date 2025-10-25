import struct

path = '.venv/Lib/site-packages/unidic/dicdir/matrix.bin'
with open(path, 'rb') as f:
    header = f.read(4)
    left_size, right_size = struct.unpack('<HH', header)
    print(f'Matrix size: {left_size} x {right_size}')
    print(f'Total matrix elements: {left_size * right_size}')
    print(f'Expected file size: {4 + left_size * right_size * 2} bytes')
    
    costs = struct.unpack('<20h', f.read(40))
    print(f'First 20 costs: {costs[:20]}')
