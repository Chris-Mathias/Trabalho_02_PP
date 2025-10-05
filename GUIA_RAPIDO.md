# 🚀 Guia Rápido - Análise Spotify MPI

## ⚡ Comandos Essenciais

### Compilar
```bash
make
```

### Executar
```bash
# Forma simples (4 processos por padrão)
./run_analysis.sh

# Forma simples com N processos
./run_analysis.sh 8

# Forma direta
mpirun -np 4 ./spotify_analyzer
```

### Testar Performance
```bash
./test_performance.sh
```

### Limpar
```bash
make clean
```

## 📊 Interpretando Resultados

### Speedup
```
Speedup = Tempo(1 processo) / Tempo(N processos)
```
**Ideal**: Speedup = N (linear)  
**Bom**: Speedup > 0.7 * N  
**Aceitável**: Speedup > 0.5 * N  

### Eficiência
```
Eficiência = Speedup / N
```
**Ideal**: 100%  
**Bom**: > 70%  
**Aceitável**: > 50%  

## 🔧 Solução de Problemas Comuns

### "mpicc not found"
```bash
sudo apt-get install openmpi-bin libopenmpi-dev
```

### "Not enough slots"
```bash
# Adicione --oversubscribe
mpirun --oversubscribe -np 8 ./spotify_analyzer
```

### Arquivo CSV não encontrado
```bash
# Certifique-se de estar no diretório correto
cd "/mnt/d/Estudos C/trabalho spotify"
ls spotify_millsongdata_novo.csv
```

### Recompilar após mudanças
```bash
make clean && make
```

## 📈 Benchmarks de Referência

| Processos | Tempo Esperado | Speedup |
|-----------|----------------|---------|
| 1         | ~120 segundos  | 1.0x    |
| 2         | ~70 segundos   | 1.7x    |
| 4         | ~65 segundos   | 1.8x    |

*Tempos podem variar conforme hardware*

## 📝 Arquivos Importantes

| Arquivo | Descrição |
|---------|-----------|
| `app.c` | Código fonte principal |
| `makefile` | Script de compilação |
| `spotify_millsongdata_novo.csv` | Dataset de entrada |
| `spotify_analyzer` | Executável compilado |
| `README.md` | Documentação completa |
| `DOCUMENTACAO_MPI.md` | Detalhes técnicos |

## 🎯 Checklist de Uso

- [ ] Compilar o programa
- [ ] Verificar se o CSV está presente
- [ ] Executar com 1 processo (baseline)
- [ ] Executar com múltiplos processos
- [ ] Comparar tempos de execução
- [ ] Verificar resultados salvos

## 💡 Dicas

1. **Primeira execução**: Use 1 processo para ter baseline
2. **Otimizar**: Teste com 2, 4, 8 processos
3. **Hardware**: Mais processos que CPUs pode degradar performance
4. **Resultados**: São salvos automaticamente com timestamp
5. **Comparação**: Use diff para verificar consistência entre execuções

## 🔍 Analisando Saída

A saída contém:
- ✅ **Top 20 Artistas**: Ordenados por número de músicas
- ✅ **Top 20 Palavras**: Ordenadas por frequência
- ✅ **Estatísticas**: Total de artistas/palavras únicas
- ✅ **Performance**: Tempo de execução total

## 📞 Onde Obter Ajuda

1. **README.md**: Documentação completa do usuário
2. **DOCUMENTACAO_MPI.md**: Detalhes técnicos da implementação
3. **PROJETO_COMPLETO.md**: Visão geral do projeto
4. **Código comentado**: `app.c` tem comentários explicativos

---

**Versão**: 1.0  
**Atualizado**: Outubro 2025
