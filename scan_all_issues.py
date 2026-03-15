#!/usr/bin/env python3
import os
import re
from pathlib import Path

# Diretórios a escanear
DIRS_TO_SCAN = ['src', 'include']

# Padrões para placeholders e TODOs
PLACEHOLDER_PATTERNS = [
    r'\bTODO\b', r'\bFIXME\b', r':::', r'\bplaceholder\b', r'\bnot implemented\b',
    r'\bstub\b', r'\btemporário\b', r'\bdummy\b', r'\bmock\b', r'\bincompleto\b',
    r'\bem desenvolvimento\b', r'\bWIP\b', r'\bwork in progress\b', r'\bcoming soon\b',
    r'\bto be implemented\b', r'\bTBI\b', r'\bhack\b', r'\bquick fix\b',
    r'\btemporary\b', r'\bplaceholder text\b', r'\bexample\b', r'\bsample\b',
    r'\btest code\b', r'\bdebug code\b', r'\bremove me\b', r'\bdelete me\b',
    r'\bunused\b', r'\bdeprecated\b'
]

# Padrões para código problemático
PROBLEMATIC_PATTERNS = [
    r'\bnão funciona\b', r'\bquebrado\b', r'\bbugado\b', r'\bprecisa arrumar\b',
    r'\bdeprecated\b', r'\bunused variable\b', r'\bdead code\b', r'\bmagic number\b',
    r'\bhardcoded\b'
]

# Padrões para warnings comuns
WARNING_PATTERNS = [
    r'\bunused\b', r'\bnot used\b', r'\bcomparação de tipos diferentes\b',
    r'\btype mismatch\b', r'\bvariable not initialized\b', r'\bnull pointer\b'
]

# Padrões para otimizações
OPTIMIZATION_PATTERNS = [
    r'\bcódigo duplicado\b', r'\bduplicate code\b', r'\bfunção longa\b',
    r'\blong function\b', r'\bcomplex function\b'
]

def scan_file(file_path):
    issues = []
    try:
        with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
            lines = f.readlines()
            for line_num, line in enumerate(lines, 1):
                line_lower = line.lower().strip()

                # Verificar placeholders e TODOs
                for pattern in PLACEHOLDER_PATTERNS:
                    if re.search(pattern, line_lower, re.IGNORECASE):
                        issues.append({
                            'file': file_path,
                            'line': line_num,
                            'description': f"Placeholder/TODO encontrado: {pattern}",
                            'content': line.strip()
                        })

                # Verificar código problemático
                for pattern in PROBLEMATIC_PATTERNS:
                    if re.search(pattern, line_lower, re.IGNORECASE):
                        issues.append({
                            'file': file_path,
                            'line': line_num,
                            'description': f"Código problemático: {pattern}",
                            'content': line.strip()
                        })

                # Verificar warnings comuns
                for pattern in WARNING_PATTERNS:
                    if re.search(pattern, line_lower, re.IGNORECASE):
                        issues.append({
                            'file': file_path,
                            'line': line_num,
                            'description': f"Possível warning: {pattern}",
                            'content': line.strip()
                        })

                # Verificar otimizações
                for pattern in OPTIMIZATION_PATTERNS:
                    if re.search(pattern, line_lower, re.IGNORECASE):
                        issues.append({
                            'file': file_path,
                            'line': line_num,
                            'description': f"Possível otimização: {pattern}",
                            'content': line.strip()
                        })

                # Verificar comentários desnecessários (linhas comentadas que parecem código)
                if line.strip().startswith('//') or line.strip().startswith('/*'):
                    # Heurística: se contém ; ou { ou } ou =, pode ser código comentado
                    if any(char in line for char in [';', '{', '}', '=']):
                        issues.append({
                            'file': file_path,
                            'line': line_num,
                            'description': "Comentário desnecessário (possível código morto)",
                            'content': line.strip()
                        })

                # Verificar funções longas (mais de 50 linhas consecutivas sem quebra)
                # Isso é simplificado; em um arquivo real, precisaria de análise de sintaxe
                # Aqui, apenas marcar se a linha contém '{' e contar linhas até '}'
                # Mas isso é complexo; por simplicidade, procurar por funções com muitas linhas
                # Usar uma heurística: se a linha tem 'void' ou 'int' seguido de '(', pode ser função
                if re.search(r'\b(void|int|float|char|bool)\s+\w+\s*\(', line):
                    # Contar linhas da função (simplificado: até encontrar '}')
                    # Isso não é preciso, mas para demonstração
                    pass  # Implementar se necessário

    except Exception as e:
        issues.append({
            'file': file_path,
            'line': 0,
            'description': f"Erro ao ler arquivo: {str(e)}",
            'content': ""
        })

    return issues

def main():
    all_issues = []
    for dir_name in DIRS_TO_SCAN:
        if os.path.exists(dir_name):
            for root, dirs, files in os.walk(dir_name):
                for file in files:
                    if file.endswith(('.cpp', '.h', '.c', '.hpp', '.ino')):  # Arquivos de código
                        file_path = os.path.join(root, file)
                        issues = scan_file(file_path)
                        all_issues.extend(issues)

    # Ordenar por arquivo e linha
    all_issues.sort(key=lambda x: (x['file'], x['line']))

    # Salvar resultados em arquivo
    with open('scan_results.txt', 'w', encoding='utf-8') as f:
        if all_issues:
            f.write(f"Encontrados {len(all_issues)} issues:\n")
            for issue in all_issues:
                f.write(f"Arquivo: {issue['file']}\n")
                f.write(f"Linha: {issue['line']}\n")
                f.write(f"Descrição: {issue['description']}\n")
                f.write(f"Conteúdo: {issue['content']}\n")
                f.write("-" * 50 + "\n")
        else:
            f.write("Nenhum issue encontrado.\n")

    print(f"Resultados salvos em scan_results.txt. Total de issues: {len(all_issues)}")

if __name__ == "__main__":
    main()