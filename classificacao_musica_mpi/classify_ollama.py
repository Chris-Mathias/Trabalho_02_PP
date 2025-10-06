#!/usr/bin/env python3
"""
classify_ollama.py

Usage:
  python3 classify_ollama.py --text "some lyrics..." 
  python3 classify_ollama.py --file /tmp/lyrics.txt

Outputs a single word on stdout: "positivo", "negativo" ou "neutro"
"""

import argparse
import sys
import os
from typing import Optional

try:
    from ollama import chat
except Exception as e:
    sys.stderr.write("Erro: não foi possível importar 'ollama'. Instale com `pip install ollama`.\n")
    sys.exit(2)

def classify_text(text: str) -> str:
    """
    Envia `text` para o modelo gemma3:1b via Ollama e pede a classificação.
    Retorna 'positivo', 'negativo' ou 'neutro'.
    """
    system_prompt = (
        "Você é um classificador de sentimento. "
        "Receberá a letra de uma música e deve responder com EXATAMENTE uma palavra: "
        "'positivo', 'negativo' ou 'neutro'. "
        "Responda apenas essa palavra, sem explicações, sem pontuação extra."
    )

    user_prompt = f"Classifique o seguinte texto quanto ao sentimento (positivo/negativo/neutro):\n\n\"\"\"\n{text}\n\"\"\""

    try:
        resp = chat(model='gemma3:1b', messages=[
            {'role': 'system', 'content': system_prompt},
            {'role': 'user', 'content': user_prompt}
        ])
    except Exception as e:
        sys.stderr.write(f"Erro ao consultar Ollama: {e}\n")
        sys.exit(3)

    content = ""
    if isinstance(resp, dict) and 'message' in resp:
        message = resp['message']
        if isinstance(message, dict) and 'content' in message:
            content = message['content']
    else:
        try:
            content = resp.message.content
        except Exception:
            content = str(resp)

    content = content.strip().splitlines()[0].strip().lower()

    map_ok = {
        'positive': 'positivo',
        'negative': 'negativo',
        'neutral': 'neutro',
        'positivo': 'positivo',
        'negativo': 'negativo',
        'positiva': 'positivo',
        'negativa': 'negativo',
        'neutro': 'neutro',
        'neutra': 'neutro'
    }

    token = content.split()[0] if content else ''
    resultado = map_ok.get(token, None)

    if resultado is None:
        if 'positivo' in content or 'positive' in content:
            resultado = 'positivo'
        elif 'negativo' in content or 'negative' in content:
            resultado = 'negativo'
        else:
            resultado = 'neutro'

    print(resultado)
    return resultado

def main():
    ap = argparse.ArgumentParser()
    group = ap.add_mutually_exclusive_group(required=True)
    group.add_argument('--text', type=str, help='Texto a classificar (pode conter espaços).')
    group.add_argument('--file', type=str, help='Caminho para arquivo contendo o texto (UTF-8).')
    args = ap.parse_args()

    if args.file:
        if not os.path.exists(args.file):
            sys.stderr.write(f"Arquivo não encontrado: {args.file}\n")
            sys.exit(4)
        with open(args.file, 'r', encoding='utf-8') as f:
            text = f.read()
    else:
        text = args.text

    classify_text(text)

if __name__ == '__main__':
    main()
