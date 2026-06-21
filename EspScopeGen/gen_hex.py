# Convert the fixed HTML to a hex byte array and insert it into EspScopeGen.ino
import os
import re

script_dir = os.path.dirname(os.path.abspath(__file__))
html_path = os.path.join(script_dir, 'page_fixed.html')
ino_path = os.path.join(script_dir, 'EspScopeGen.ino')

with open(html_path, 'r', encoding='utf-8') as f:
    html = f.read()

byte_array = html.encode('utf-8')
print(f'Byte count: {len(byte_array)}')

# Generate the C++ hex array lines
hex_lines = ['const uint8_t INDEX_HTML[] PROGMEM = {']
for i in range(0, len(byte_array), 16):
    chunk = byte_array[i:i+16]
    hex_lines.append('  ' + ', '.join(f'0x{b:02X}' for b in chunk))
hex_lines.append('};')
hex_lines.append(f'const size_t INDEX_HTML_LEN = {len(byte_array)};\n')

array_content = '\n'.join(hex_lines)

# Read current EspScopeGen.ino content
with open(ino_path, 'r', encoding='utf-8') as f:
    ino_content = f.read()

start_marker = '// ===== INDEX_HTML START ====='
end_marker = '// ===== INDEX_HTML END ====='

if start_marker in ino_content and end_marker in ino_content:
    # Replace content between markers
    pattern = re.escape(start_marker) + r'[\s\S]*?' + re.escape(end_marker)
    replacement = f"{start_marker}\n{array_content}{end_marker}"
    new_ino_content = re.sub(pattern, replacement, ino_content)
    
    with open(ino_path, 'w', encoding='utf-8') as f:
        f.write(new_ino_content)
    print('EspScopeGen.ino updated successfully with new HTML array!')
else:
    print('Error: Could not find HTML markers in EspScopeGen.ino.')
