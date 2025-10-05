# 🎵 Trabalho Spotify - Análise Paralela com MPI

## 📋 Resumo do Projeto

Este projeto implementa uma aplicação paralela usando **MPI (Message Passing Interface)** em C para processar e analisar dados de músicas do Spotify. A aplicação realiza duas análises principais:

1. **Contagem de Palavras nas Letras**: Identifica e conta as palavras mais frequentes em todas as letras
2. **Artistas com Mais Músicas**: Determina quais artistas têm mais músicas no dataset

## 🎯 Objetivos Alcançados

✅ **Processamento Paralelo**: Implementação completa usando MPI  
✅ **Divisão de Trabalho**: Distribuição eficiente dos dados entre processos  
✅ **Agregação de Resultados**: Combinação correta dos resultados parciais  
✅ **Parser CSV Robusto**: Tratamento adequado de campos com vírgulas e aspas  
✅ **Performance Medida**: Comparação de tempos com diferentes números de processos  
✅ **Documentação Completa**: README, documentação técnica e scripts auxiliares  

## 📊 Resultados Obtidos

### Dataset Processado
- **Arquivo**: `spotify_millsongdata_novo.csv`
- **Tamanho**: 64.42 MB
- **Músicas**: ~57,650 músicas
- **Artistas únicos**: 645
- **Palavras únicas**: 81,881

### Top 10 Artistas com Mais Músicas
1. Donna Summer - 191 músicas
2. Gordon Lightfoot - 189 músicas
3. Bob Dylan - 188 músicas
4. George Strait - 188 músicas
5. Alabama - 187 músicas
6. Cher - 187 músicas
7. Loretta Lynn - 187 músicas
8. Reba Mcentire - 187 músicas
9. Chaka Khan - 186 músicas
10. Dean Martin - 186 músicas

### Top 10 Palavras Mais Frequentes
1. the - 498,091 ocorrências
2. you - 495,850 ocorrências
3. to - 297,068 ocorrências
4. and - 294,723 ocorrências
5. it - 219,630 ocorrências
6. me - 202,521 ocorrências
7. my - 171,007 ocorrências
8. in - 168,031 ocorrências
9. of - 140,084 ocorrências
10. that - 135,087 ocorrências

### Performance Paralela

| Processos | Tempo (s) | Speedup | Eficiência |
|-----------|-----------|---------|------------|
| 1         | 124.08    | 1.00x   | 100%       |
| 4         | 67.58     | 1.84x   | 46%        |

**Análise de Performance:**
- Com 4 processos, obtivemos um speedup de 1.84x
- A eficiência de 46% é razoável considerando:
  - Overhead de comunicação MPI
  - Leitura serial do arquivo
  - Agregação serial dos resultados
  - Balanceamento de carga não-perfeito

## 🛠️ Tecnologias Utilizadas

- **Linguagem**: C (padrão C99/C11)
- **Paralelização**: MPI (OpenMPI)
- **Compilador**: GCC com mpicc wrapper
- **Build System**: Make
- **Sistema Operacional**: Linux (WSL Ubuntu)

## 📁 Estrutura de Arquivos

```
trabalho spotify/
├── app.c                           # Código fonte principal (implementação MPI)
├── makefile                        # Script de compilação automática
├── README.md                       # Documentação principal do usuário
├── DOCUMENTACAO_MPI.md             # Documentação técnica detalhada
├── run_analysis.sh                 # Script para executar análise facilmente
├── test_performance.sh             # Script para testes de performance
├── spotify_millsongdata_novo.csv   # Dataset de entrada (64.42 MB)
├── spotify_analyzer                # Executável compilado
└── PROJETO_COMPLETO.md            # Este arquivo (resumo geral)
```

## 🚀 Como Usar

### 1. Compilar
```bash
make clean
make
```

### 2. Executar (forma simples)
```bash
# Com número padrão de processos (4)
./run_analysis.sh

# Com número específico de processos
./run_analysis.sh 8
```

### 3. Executar (forma direta)
```bash
# Com 1 processo (sequencial)
mpirun -np 1 ./spotify_analyzer

# Com 4 processos (paralelo)
mpirun -np 4 ./spotify_analyzer

# Com 8 processos (se disponível)
mpirun -np 8 ./spotify_analyzer
```

### 4. Testar Performance
```bash
./test_performance.sh
```

## 🔧 Características Técnicas

### Padrão de Paralelização
- **Master-Worker (Manager-Worker)**
- Processo mestre (rank 0) coordena e agrega
- Processos trabalhadores (rank > 0) processam dados

### Comunicação MPI
- **Ponto-a-Ponto**: `MPI_Send` e `MPI_Recv`
- **Síncrona**: Mensagens bloqueantes para simplicidade
- **Tags**: Diferentes tags para tipos de dados (0-5)

### Estruturas de Dados
```c
typedef struct {
    char word[MAX_WORD_LEN];
    int count;
} WordCount;

typedef struct {
    char artist[MAX_ARTIST_LEN];
    int count;
} ArtistCount;
```

### Algoritmos Implementados

1. **Parser CSV**: Extração correta de campos com aspas e vírgulas
2. **Tokenização**: Separação de palavras das letras
3. **Normalização**: Conversão para minúsculas e remoção de pontuação
4. **Contagem**: Uso de arrays dinâmicos com busca linear
5. **Agregação**: Merge de resultados parciais dos trabalhadores
6. **Ordenação**: QuickSort (qsort) para ranking final

## 📈 Análise de Complexidade

### Tempo de Execução
- **Leitura do arquivo**: O(n) - serial, apenas mestre
- **Divisão dos dados**: O(n) - serial, apenas mestre
- **Processamento paralelo**: O(n/p) - onde p = número de processos
- **Agregação**: O(m * k) - onde m = palavras únicas, k = processos
- **Ordenação**: O(m log m) - QuickSort

### Espaço
- **Arquivo**: O(n) - mestre mantém cópia completa
- **Resultados locais**: O(m) - cada processo
- **Resultados agregados**: O(m) - apenas mestre

## 🎓 Conceitos de Programação Paralela Aplicados

1. **Paralelismo de Dados**: Mesmo processamento em diferentes porções dos dados
2. **Decomposição de Domínio**: Divisão espacial dos dados
3. **Comunicação Coletiva**: Gather implícito (manual via loops)
4. **Sincronização**: MPI_Recv bloqueante garante ordem
5. **Escalabilidade**: Testado com 1, 2, 4 processos

## 🔍 Insights dos Dados

### Sobre as Músicas
- Dataset contém músicas predominantemente em inglês
- Artistas clássicos dominam o top rankings
- Diversidade de gêneros (rock, country, pop, etc.)

### Sobre as Palavras
- Palavras mais comuns são artigos e pronomes (stop words)
- "love" aparece 94,070 vezes - tema dominante em músicas
- Vocabulário rico com mais de 81 mil palavras únicas

### Sobre os Artistas
- Donna Summer lidera com 191 músicas
- Top 20 artistas têm entre 181-191 músicas cada
- Representação de diferentes décadas e estilos

## 🚧 Limitações Conhecidas

1. **Memória**: Arquivo completo carregado na RAM (limite ~1-2 GB)
2. **Busca Linear**: O(n) para encontrar palavras/artistas
3. **Balanceamento**: Divisão por bytes, não por complexidade
4. **Stop Words**: Palavras comuns não são filtradas
5. **I/O Serial**: Apenas mestre lê o arquivo

## 🔮 Melhorias Futuras Propostas

### Performance
- [ ] Usar hash tables para busca O(1)
- [ ] Implementar MPI I/O para leitura paralela
- [ ] Balanceamento dinâmico de carga
- [ ] Pipeline para sobrepor comunicação/computação
- [ ] Usar MPI_Reduce coletivo

### Funcionalidades
- [ ] Filtro de stop words configurável
- [ ] Análise de n-gramas (pares/trios de palavras)
- [ ] Análise de sentimentos das letras
- [ ] Exportar resultados para JSON/CSV
- [ ] Visualizações gráficas (word clouds, gráficos)

### Robustez
- [ ] Tratamento de erros mais completo
- [ ] Validação de entrada
- [ ] Suporte a diferentes encodings
- [ ] Processamento streaming para arquivos gigantes
- [ ] Checkpointing para recuperação de falhas

## 📚 Referências e Recursos

### MPI
- [MPI Forum Official](https://www.mpi-forum.org/)
- [OpenMPI Documentation](https://www.open-mpi.org/doc/)
- [MPI Tutorial](https://mpitutorial.com/)

### Conceitos
- [Amdahl's Law](https://en.wikipedia.org/wiki/Amdahl%27s_law)
- [Gustafson's Law](https://en.wikipedia.org/wiki/Gustafson%27s_law)
- [Parallel Programming Patterns](https://patterns.eecs.berkeley.edu/)

### Dataset
- [Million Song Dataset](http://millionsongdataset.com/)

## 🧪 Como Testar

### Teste Básico
```bash
make
mpirun -np 4 ./spotify_analyzer
```

### Teste de Correção
```bash
# Executar com 1 e 4 processos e comparar resultados
mpirun -np 1 ./spotify_analyzer > result_1.txt
mpirun -np 4 ./spotify_analyzer > result_4.txt
diff result_1.txt result_4.txt
# Os resultados devem ser idênticos (exceto tempo)
```

### Teste de Performance
```bash
./test_performance.sh
# Analisa resultados em performance_results.txt
```

### Teste de Escalabilidade
```bash
for np in 1 2 4 8; do
    echo "Testing with $np processes"
    mpirun --oversubscribe -np $np ./spotify_analyzer | grep "Tempo"
done
```

## 💡 Lições Aprendidas

1. **MPI é poderoso mas complexo**: Requer cuidado com sincronização e deadlocks
2. **Comunicação tem custo**: Overhead pode limitar speedup
3. **Balanceamento importa**: Distribuição desigual reduz eficiência
4. **Parser CSV é desafiador**: Campos com aspas e vírgulas complicam
5. **Agregação é gargalo**: Fase serial limita escalabilidade

## 🎯 Conclusão

Este projeto demonstra com sucesso a aplicação de programação paralela usando MPI para processamento de grandes volumes de dados textuais. A implementação alcançou:

- ✅ **Funcionalidade completa**: Ambas as análises funcionando corretamente
- ✅ **Paralelização efetiva**: Speedup de 1.84x com 4 processos
- ✅ **Código robusto**: Parser CSV adequado e tratamento de casos especiais
- ✅ **Documentação abrangente**: Documentação técnica e de usuário

O projeto serve como excelente exemplo didático de:
- Programação paralela com MPI em C
- Processamento de dados textuais em larga escala
- Análise de performance de aplicações paralelas
- Boas práticas de documentação e estruturação de código

---

**Desenvolvido como trabalho acadêmico**  
**Data**: Outubro de 2025  
**Tecnologias**: C, MPI, OpenMPI, Linux  
