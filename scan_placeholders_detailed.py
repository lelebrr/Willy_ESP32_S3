import os
import re
import sys

# Lista de padrões a buscar (case-insensitive)
patterns = [
    r'\bTODO\b',
    r'\bFIXME\b',
    r':::',  # Três pontos
    r'\bplaceholder\b',
    r'\bnot implemented\b',
    r'\bstub\b',
    r'\btemporário\b',
    r'\bdummy\b',
    r'\bmock\b',
    r'\bincompleto\b',
    r'\bem desenvolvimento\b',
    r'\bWIP\b',
    r'\bwork in progress\b',
    r'\bcoming soon\b',
    r'\bto be implemented\b',
    r'\bTBI\b',
    r'\bhack\b',
    r'\bquick fix\b',
    r'\btemporary\b',
    r'\bplaceholder text\b',
    r'\bexample\b',
    r'\bsample\b',
    r'\btest code\b',
    r'\bdebug code\b',
    r'\bremove me\b',
    r'\bdelete me\b',
    r'\bunused\b',
    r'\bdeprecated\b',
    r'\bnão funciona\b',
    r'\bquebrado\b',
    r'\bbugado\b',
    r'\bprecisa arrumar\b'
]

# Compilar regex combinado
combined_pattern = re.compile('|'.join(patterns), re.IGNORECASE)

# Extensões a focar
extensions = ['.cpp', '.h', '.ino', '.py', '.js']

def is_binary(file_path):
    """Verifica se o arquivo é binário tentando lê-lo como texto."""
    try:
        with open(file_path, 'rb') as f:
            chunk = f.read(1024)
            if b'\0' in chunk:
                return True
        return False
    except:
        return True

def scan_file(file_path, root_dir):
    """Escaneia um arquivo por placeholders."""
    relative_path = os.path.relpath(file_path, root_dir)
    try:
        with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
            lines = f.readlines()
            for line_num, line in enumerate(lines, 1):
                if combined_pattern.search(line):
                    print(f"{relative_path}:{line_num}: {line.rstrip()}")
    except Exception as e:
        print(f"Erro ao ler {relative_path}: {e}", file=sys.stderr)

def main():
    root_dir = os.getcwd()
    for dirpath, dirnames, filenames in os.walk(root_dir):
        # Ignorar diretórios como .git, etc., se necessário
        dirnames[:] = [d for d in dirnames if not d.startswith('.')]
        for filename in filenames:
            file_path = os.path.join(dirpath, filename)
            if any(filename.endswith(ext) for ext in extensions):
                if not is_binary(file_path):
                    scan_file(file_path, root_dir)

if __name__ == "__main__":
    main()