# Análise Paralela de Dados do Spotify com MPI

## 📋 Descrição

Esta aplicação implementa processamento paralelo usando **MPI (Message Passing Interface)** em C para analisar dados de músicas do Spotify. O programa realiza duas análises principais:

1. **Contagem de Palavras**: Conta a frequência de cada palavra presente nas letras das músicas
2. **Artistas com Mais Músicas**: Identifica os artistas com maior quantidade de músicas no dataset

## 🎯 Funcionalidades

### 1. Processamento Paralelo
- Distribui o processamento do arquivo CSV entre múltiplos processos MPI
- Cada processo trabalha em uma porção independente dos dados
- O processo mestre (rank 0) coordena e agrega os resultados

### 2. Análise de Palavras
- Extrai palavras das letras das músicas
- Normaliza texto (converte para minúsculas)
- Filtra palavras muito curtas (< 2 caracteres)
- Remove caracteres especiais e pontuação
- Conta frequência de cada palavra única

### 3. Análise de Artistas
- Identifica todos os artistas únicos
- Conta o número de músicas por artista
- Ordena por quantidade de músicas

### 4. Parser CSV Robusto
- Lida corretamente com campos entre aspas
- Suporta vírgulas dentro de campos delimitados por aspas
- Processa linhas de qualquer tamanho (até 20KB por linha)

## 🛠️ Requisitos

### Software Necessário
- **GCC** ou compilador C compatível
- **OpenMPI** ou **MPICH** (implementação MPI)
- **Make** para compilação

### Instalação no Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install build-essential
sudo apt-get install openmpi-bin openmpi-common libopenmpi-dev
```

### Instalação no Fedora/RedHat
```bash
sudo dnf install gcc make
sudo dnf install openmpi openmpi-devel
```

## 📦 Compilação

### Usando o Makefile
```bash
make clean    # Limpa compilações anteriores
make          # Compila o programa
```

### Compilação Manual
```bash
mpicc -Wall -g -O2 -o spotify_analyzer app.c
```

## 🚀 Execução

### Sintaxe Básica
```bash
mpirun -np <número_de_processos> ./spotify_analyzer
```

### Exemplos

**Execução sequencial (1 processo):**
```bash
mpirun -np 1 ./spotify_analyzer
```

**Execução paralela com 4 processos:**
```bash
mpirun -np 4 ./spotify_analyzer
```

**Execução paralela com 8 processos (se disponível):**
```bash
mpirun -np 8 ./spotify_analyzer
```

## 📊 Resultados de Performance

Com base nos testes realizados:

| Processos | Tempo de Execução | Speedup |
|-----------|-------------------|---------|
| 1         | ~124 segundos     | 1.0x    |
| 4         | ~68 segundos      | 1.83x   |
| 8         | ~40 segundos*     | 3.1x*   |

*Valores aproximados, dependem do hardware

## 📁 Estrutura do Projeto

```
trabalho spotify/
├── app.c                           # Código fonte principal
├── makefile                        # Script de compilação
├── spotify_millsongdata_novo.csv   # Dataset (64.42 MB)
├── spotify_analyzer                # Executável compilado
└── README.md                       # Esta documentação
```

## 🔧 Detalhes Técnicos

### Estruturas de Dados

```c
// Estrutura para contagem de palavras
typedef struct {
    char word[MAX_WORD_LEN];
    int count;
} WordCount;

// Estrutura para contagem de artistas
typedef struct {
    char artist[MAX_ARTIST_LEN];
    int count;
} ArtistCount;
```

### Constantes Configuráveis

- `MAX_WORD_LEN`: 100 caracteres (tamanho máximo de uma palavra)
- `MAX_ARTIST_LEN`: 200 caracteres (tamanho máximo do nome de artista)
- `MAX_LINE_LEN`: 20000 caracteres (tamanho máximo de linha do CSV)
- `TOP_N`: 20 (número de resultados exibidos no top ranking)
- `INITIAL_CAPACITY`: 10000 (capacidade inicial dos arrays dinâmicos)

### Fluxo de Execução

1. **Inicialização MPI**: Todos os processos inicializam o ambiente MPI
2. **Processo Mestre (rank 0)**:
   - Lê o arquivo CSV completo
   - Divide os dados em chunks aproximadamente iguais
   - Envia cada chunk para um processo trabalhador
   - Processa seu próprio chunk
   - Recebe e agrega resultados de todos os trabalhadores
   - Ordena e exibe os resultados finais
3. **Processos Trabalhadores (rank > 0)**:
   - Recebem um chunk de dados
   - Processam o chunk localmente
   - Enviam resultados de volta ao mestre
4. **Finalização**: Todos os processos finalizam o ambiente MPI

## 📈 Exemplo de Saída

```
=== Análise Paralela de Dados do Spotify com MPI ===
Número de processos: 4
Arquivo: spotify_millsongdata_novo.csv

Tamanho do arquivo: 64.42 MB
Processando dados em paralelo...

================================================
RESULTADOS DA ANÁLISE
================================================

--- Top 20 Artistas com Mais Músicas ---
  1. Donna Summer                                191 músicas
  2. Gordon Lightfoot                            189 músicas
  3. Bob Dylan                                   188 músicas
  4. George Strait                               188 músicas
  5. Alabama                                     187 músicas
  ...

--- Top 20 Palavras Mais Frequentes ---
  1. the                                498091 ocorrências
  2. you                                495850 ocorrências
  3. to                                 297068 ocorrências
  4. and                                294723 ocorrências
  5. it                                 219630 ocorrências
  ...

================================================
Estatísticas:
  - Total de artistas únicos: 645
  - Total de palavras únicas: 81881
  - Tempo de execução: 67.578 segundos
================================================
```

## 🐛 Troubleshooting

### Erro: "mpicc: command not found"
- Instale o OpenMPI: `sudo apt-get install openmpi-bin libopenmpi-dev`

### Erro: "There are not enough slots available"
- Use menos processos ou adicione `--oversubscribe`:
  ```bash
  mpirun --oversubscribe -np 8 ./spotify_analyzer
  ```

### Arquivo CSV não encontrado
- Certifique-se de que `spotify_millsongdata_novo.csv` está no mesmo diretório

### Warnings de compilação
- Os warnings sobre variáveis não utilizadas (`song`, `link`) são intencionais
- Esses campos são extraídos mas não usados nas análises atuais

## 🔮 Melhorias Futuras

- [ ] Implementar análise de sentimentos das letras
- [ ] Adicionar suporte para múltiplos arquivos CSV
- [ ] Criar visualizações gráficas dos resultados
- [ ] Implementar filtro de stop words (palavras comuns)
- [ ] Adicionar análise de n-gramas (pares/trios de palavras)
- [ ] Salvar resultados em arquivo JSON/CSV
- [ ] Implementar balanceamento dinâmico de carga
- [ ] Adicionar suporte para processamento de arquivos muito grandes (> 1GB)

## 📝 Formato do CSV

O arquivo CSV deve ter o seguinte formato:

```csv
artist,song,link,text
"Artist Name","Song Title","URL","Lyrics text here..."
```

**Campos:**
- `artist`: Nome do artista
- `song`: Título da música
- `link`: URL da letra (não utilizado atualmente)
- `text`: Letra completa da música

## 👥 Autor

Desenvolvido como trabalho acadêmico para demonstrar programação paralela com MPI em C.

## 📄 Licença

Este projeto é de código aberto para fins educacionais.

## 🙏 Agradecimentos

- Dataset baseado no Million Song Dataset
- Implementação MPI usando OpenMPI
- Universidade/Instituição de ensino

---

**Nota**: Este projeto foi desenvolvido para fins educacionais e demonstração de técnicas de programação paralela com MPI.
