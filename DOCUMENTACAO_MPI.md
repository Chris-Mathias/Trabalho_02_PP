# Documentação Técnica - Implementação MPI

## Visão Geral da Arquitetura

Este documento detalha a implementação paralela usando MPI (Message Passing Interface) para processamento de dados do Spotify.

## Padrão de Paralelização: Master-Worker

A aplicação utiliza o padrão **Master-Worker** (também conhecido como Manager-Worker):

- **1 Processo Mestre** (rank 0): Coordena o trabalho e agrega resultados
- **N-1 Processos Trabalhadores** (rank 1 a N-1): Processam dados em paralelo

## Fluxo de Comunicação MPI

```
                    ┌─────────────────┐
                    │  Processo 0     │
                    │   (Mestre)      │
                    └────────┬────────┘
                             │
            ┌────────────────┼────────────────┐
            │                │                │
            ▼                ▼                ▼
    ┌──────────────┐ ┌──────────────┐ ┌──────────────┐
    │  Processo 1  │ │  Processo 2  │ │  Processo 3  │
    │ (Trabalhador)│ │ (Trabalhador)│ │ (Trabalhador)│
    └──────┬───────┘ └──────┬───────┘ └──────┬───────┘
           │                │                │
           └────────────────┼────────────────┘
                            │
                            ▼
                    ┌─────────────────┐
                    │  Processo 0     │
                    │   (Agregação)   │
                    └─────────────────┘
```

## Etapas de Processamento

### 1. Inicialização (Todos os Processos)

```c
MPI_Init(&argc, &argv);
MPI_Comm_rank(MPI_COMM_WORLD, &rank);  // Identifica o rank do processo
MPI_Comm_size(MPI_COMM_WORLD, &nprocs); // Total de processos
```

### 2. Processo Mestre (rank == 0)

#### 2.1 Leitura do Arquivo
```c
FILE *file = fopen(filename, "rb");
fseek(file, 0, SEEK_END);
long file_size = ftell(file);
// Lê todo o arquivo na memória
```

#### 2.2 Divisão dos Dados
```c
size_t chunk_size = data_size / nprocs;
// Divide o arquivo em chunks aproximadamente iguais
// Ajusta para não cortar linhas no meio
```

#### 2.3 Distribuição para Trabalhadores
```c
for (int i = 1; i < nprocs; i++) {
    MPI_Send(&actual_size, 1, MPI_UNSIGNED_LONG, i, 0, MPI_COMM_WORLD);
    MPI_Send(chunk, actual_size, MPI_CHAR, i, 1, MPI_COMM_WORLD);
}
```

**Tags MPI usadas:**
- Tag 0: Tamanho do chunk
- Tag 1: Dados do chunk

#### 2.4 Processamento Local
O mestre também processa seu próprio chunk de dados.

#### 2.5 Recepção e Agregação
```c
for (int i = 1; i < nprocs; i++) {
    // Recebe contagem de palavras
    MPI_Recv(&recv_word_count, 1, MPI_INT, i, 2, MPI_COMM_WORLD, &status);
    MPI_Recv(recv_words, ..., MPI_BYTE, i, 3, MPI_COMM_WORLD, &status);
    
    // Recebe contagem de artistas
    MPI_Recv(&recv_artist_count, 1, MPI_INT, i, 4, MPI_COMM_WORLD, &status);
    MPI_Recv(recv_artists, ..., MPI_BYTE, i, 5, MPI_COMM_WORLD, &status);
    
    // Mescla os resultados
}
```

**Tags MPI usadas:**
- Tag 2: Número de palavras
- Tag 3: Dados das palavras (WordCount array)
- Tag 4: Número de artistas
- Tag 5: Dados dos artistas (ArtistCount array)

### 3. Processos Trabalhadores (rank > 0)

#### 3.1 Recepção de Dados
```c
MPI_Recv(&chunk_size, 1, MPI_UNSIGNED_LONG, 0, 0, MPI_COMM_WORLD, &status);
MPI_Recv(chunk, chunk_size, MPI_CHAR, 0, 1, MPI_COMM_WORLD, &status);
```

#### 3.2 Processamento Local
Cada trabalhador processa seu chunk independentemente:
- Parse do CSV
- Extração de palavras
- Contagem de artistas

#### 3.3 Envio de Resultados
```c
MPI_Send(&word_count, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);
MPI_Send(words, word_count * sizeof(WordCount), MPI_BYTE, 0, 3, MPI_COMM_WORLD);
MPI_Send(&artist_count, 1, MPI_INT, 0, 4, MPI_COMM_WORLD);
MPI_Send(artists, artist_count * sizeof(ArtistCount), MPI_BYTE, 0, 5, MPI_COMM_WORLD);
```

### 4. Finalização

```c
MPI_Finalize();  // Todos os processos finalizam o MPI
```

## Algoritmos de Processamento

### Parser CSV com Suporte a Aspas

```c
char* extract_csv_field(char **ptr) {
    // Verifica se o campo começa com aspas
    if (*start == '"') {
        // Procura pela aspa de fechamento
        // Lida com aspas escapadas ("")
    } else {
        // Campo simples, procura pela vírgula
    }
}
```

**Casos tratados:**
- Campos simples: `Artist Name,Song Title`
- Campos com aspas: `"Artist Name","Song Title"`
- Campos com vírgulas: `"Artist, The","Title, A"`
- Aspas escapadas: `"He said ""Hello"""` → `He said "Hello"`

### Contagem de Palavras

```c
void add_word(WordCount **words, int *word_count, int *word_capacity, const char *word) {
    // 1. Busca linear para encontrar palavra existente
    for (int i = 0; i < *word_count; i++) {
        if (strcmp((*words)[i].word, word) == 0) {
            (*words)[i].count++;
            return;
        }
    }
    
    // 2. Se não encontrou, adiciona nova palavra
    // 3. Redimensiona array se necessário (capacidade * 2)
}
```

**Complexidade:**
- Melhor caso: O(1) - palavra encontrada na primeira posição
- Pior caso: O(n) - palavra não existe ou está no final
- Média: O(n/2)

**Otimização futura:** Usar hash table para O(1) médio

### Agregação de Resultados (Reduce)

```c
// Para cada resultado recebido de um trabalhador
for (int j = 0; j < recv_word_count; j++) {
    int found = 0;
    // Procura se a palavra já existe no resultado global
    for (int k = 0; k < word_count; k++) {
        if (strcmp(words[k].word, recv_words[j].word) == 0) {
            words[k].count += recv_words[j].count;  // Soma as contagens
            found = 1;
            break;
        }
    }
    if (!found) {
        // Adiciona nova palavra ao resultado global
    }
}
```

## Análise de Performance

### Speedup Teórico

```
Speedup = T(1) / T(p)
```

Onde:
- T(1) = tempo com 1 processo
- T(p) = tempo com p processos

### Lei de Amdahl

```
Speedup ≤ 1 / (s + (1-s)/p)
```

Onde:
- s = fração serial do programa (leitura arquivo, agregação)
- p = número de processos

### Eficiência Paralela

```
Eficiência = Speedup / p
```

Idealmente, Eficiência = 1.0 (speedup linear)

### Overhead de Comunicação

```
T_total = T_computação + T_comunicação

T_comunicação = T_send + T_recv + T_serialização
```

**Fatores que afetam:**
- Tamanho das mensagens MPI
- Latência da rede (se distribuído)
- Número de mensagens trocadas

## Balanceamento de Carga

### Estratégia Atual: Divisão Estática

```c
size_t chunk_size = data_size / nprocs;
```

**Vantagens:**
- Simples de implementar
- Baixo overhead
- Previsível

**Desvantagens:**
- Não considera heterogeneidade dos dados
- Algumas linhas têm muito mais texto que outras
- Possível desbalanceamento

### Melhorias Futuras

1. **Divisão por Linhas**: Distribuir número igual de linhas (não bytes)
2. **Work Stealing**: Processos ociosos pegam trabalho de processos ocupados
3. **Divisão Dinâmica**: Mestre distribui trabalho sob demanda

## Gargalos e Otimizações

### Gargalos Identificados

1. **Leitura Serial do Arquivo**
   - Apenas o mestre lê o arquivo
   - Todos aguardam a leitura terminar
   
2. **Agregação Serial**
   - Mestre processa resultados sequencialmente
   - Busca linear O(n) para cada palavra/artista

3. **Memória**
   - Arquivo completo carregado na memória
   - Múltiplas cópias dos dados

### Otimizações Aplicadas

✅ **Parser CSV Eficiente**: Lida corretamente com campos complexos
✅ **Arrays Dinâmicos**: Crescem conforme necessário (capacidade * 2)
✅ **Filtragem de Palavras**: Remove palavras muito curtas
✅ **Normalização**: Converte tudo para minúsculas
✅ **Divisão Inteligente**: Não corta linhas no meio

### Otimizações Futuras

⚠️ **Uso de Hash Tables**: Reduzir busca de O(n) para O(1)
⚠️ **MPI I/O**: Leitura paralela do arquivo
⚠️ **Compressão**: Reduzir tamanho das mensagens MPI
⚠️ **Pipeline**: Sobrepor comunicação com computação
⚠️ **Reduce Coletivo**: Usar MPI_Reduce em vez de reduce manual

## Recursos MPI Utilizados

### Funções de Inicialização
- `MPI_Init()`: Inicializa ambiente MPI
- `MPI_Comm_rank()`: Obtém rank do processo
- `MPI_Comm_size()`: Obtém número total de processos
- `MPI_Finalize()`: Finaliza ambiente MPI

### Funções de Comunicação Ponto-a-Ponto
- `MPI_Send()`: Envia dados (bloqueante)
- `MPI_Recv()`: Recebe dados (bloqueante)

### Funções de Temporização
- `MPI_Wtime()`: Obtém tempo atual em segundos (precisão alta)

### Tipos de Dados MPI
- `MPI_CHAR`: Caracteres
- `MPI_INT`: Inteiros
- `MPI_UNSIGNED_LONG`: Unsigned long
- `MPI_BYTE`: Bytes brutos (para estruturas)

### Controle de Erros
- `MPI_Abort()`: Aborta todos os processos em caso de erro

## Considerações de Escalabilidade

### Escalabilidade Forte (Strong Scaling)
- **Tamanho do problema fixo**
- Aumentar número de processos
- Objetivo: Reduzir tempo de execução

### Escalabilidade Fraca (Weak Scaling)
- **Tamanho por processo fixo**
- Aumentar problema proporcionalmente aos processos
- Objetivo: Manter tempo constante

### Limites de Escalabilidade

Para este problema:
- **Limite inferior**: 1 processo (sequencial)
- **Limite superior prático**: ~número de linhas / 1000
- **Fator limitante**: Overhead de comunicação vs. benefício

## Referências

- MPI Forum: https://www.mpi-forum.org/
- OpenMPI Documentation: https://www.open-mpi.org/doc/
- Tutorial MPI: https://mpitutorial.com/
- Lei de Amdahl: https://en.wikipedia.org/wiki/Amdahl%27s_law

---

**Nota**: Esta documentação técnica serve como referência para entender a implementação paralela e possíveis melhorias futuras.
