#!/bin/bash

# Script para executar a análise e salvar os resultados completos

echo "╔════════════════════════════════════════════════════════════╗"
echo "║     Análise de Dados do Spotify com MPI                   ║"
echo "╚════════════════════════════════════════════════════════════╝"
echo ""

# Verificar se o executável existe
if [ ! -f "./spotify_analyzer" ]; then
    echo "⚠️  Executável não encontrado. Compilando..."
    make clean > /dev/null 2>&1
    make
    
    if [ $? -ne 0 ]; then
        echo "❌ Erro na compilação!"
        exit 1
    fi
    echo "✅ Compilação bem-sucedida!"
    echo ""
fi

# Definir número de processos (padrão: 4)
NP=${1:-4}

echo "📊 Configuração:"
echo "   - Processos MPI: $NP"
echo "   - Arquivo: spotify_millsongdata_novo.csv"
echo ""

# Nome do arquivo de saída
OUTPUT_FILE="resultados_analise_$(date +%Y%m%d_%H%M%S).txt"

echo "⏳ Executando análise..."
echo ""

# Executar e mostrar na tela e salvar no arquivo
mpirun -np $NP ./spotify_analyzer 2>&1 | tee "$OUTPUT_FILE"

# Verificar se executou com sucesso
if [ ${PIPESTATUS[0]} -eq 0 ]; then
    echo ""
    echo "✅ Análise concluída com sucesso!"
    echo "📄 Resultados salvos em: $OUTPUT_FILE"
    echo ""
    
    # Extrair estatísticas chave
    echo "📈 Resumo Rápido:"
    grep -E "Total de artistas|Total de palavras|Tempo de execução" "$OUTPUT_FILE" | sed 's/^/   /'
else
    echo ""
    echo "❌ Erro durante a execução!"
    exit 1
fi
