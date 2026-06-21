# Convert the fixed HTML to a hex byte array for page.h
import os
script_dir = os.path.dirname(os.path.abspath(__file__))
with open(os.path.join(script_dir, 'page_fixed.html'), 'r', encoding='utf-8') as f:
    html = f.read()

byte_array = html.encode('utf-8')
print(f'Byte count: {len(byte_array)}')

with open(os.path.join(script_dir, 'page.h'), 'w', encoding='ascii') as f:
    f.write('const uint8_t INDEX_HTML[] PROGMEM = {\n')
    lines_out = []
    for i in range(0, len(byte_array), 16):
        chunk = byte_array[i:i+16]
        lines_out.append('  ' + ', '.join(f'0x{b:02X}' for b in chunk))
    f.write(',\n'.join(lines_out))
    f.write('\n};\n')
    f.write(f'const size_t INDEX_HTML_LEN = {len(byte_array)};\n')

print('page.h written!')
