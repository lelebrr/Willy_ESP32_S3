import os
import re
import sys

sys.stdout.reconfigure(encoding='utf-8')

# Extensões de arquivos de código a serem verificados
EXTENSIONS = ['.cpp', '.h', '.ino', '.py', '.js']

# Palavras-chave indicando placeholders (case insensitive)
KEYWORDS = ['todo', 'fixme', ':::', 'placeholder', 'not implemented', 'stub']

def scan_placeholders(directory):
    for root, dirs, files in os.walk(directory):
        for file in files:
            if any(file.endswith(ext) for ext in EXTENSIONS):
                filepath = os.path.join(root, file)
                try:
                    with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
                        lines = f.readlines()
                        for line_num, line in enumerate(lines, start=1):
                            line_lower = line.lower()
                            if any(keyword in line_lower for keyword in KEYWORDS):
                                print(f"{filepath}:{line_num}: {line.rstrip()}")
                except (UnicodeDecodeError, IOError):
                    # Ignorar arquivos binários ou que não podem ser lidos como texto
                    continue

if __name__ == "__main__":
    current_directory = os.getcwd()
    scan_placeholders(current_directory)