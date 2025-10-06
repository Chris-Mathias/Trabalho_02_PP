#!/bin/bash

if [ ! -f "./spotify_analyzer" ]; then
    echo "Executável não encontrado. Compilando..."
    make clean > /dev/null 2>&1
    make
    
    if [ $? -ne 0 ]; then
        echo "Erro na compilação!"
        exit 1
    fi
    echo "Compilação bem-sucedida!"
    echo ""
fi

NP=${1:-4}

OUTPUT_FILE="resultados_analise_$(date +%Y%m%d_%H%M%S).txt"

mpirun -np $NP ./spotify_analyzer 2>&1 | tee "$OUTPUT_FILE"

if [ ${PIPESTATUS[0]} -eq 0 ]; then
    echo ""
    echo "Resultados salvos em: $OUTPUT_FILE"
    echo ""
else
    echo ""
    echo "Erro durante a execução!"
    exit 1
fi
