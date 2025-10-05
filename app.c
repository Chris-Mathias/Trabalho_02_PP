#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_WORD_LEN 100
#define MAX_ARTIST_LEN 200
#define MAX_LINE_LEN 20000
#define TOP_N 20
#define INITIAL_CAPACITY 10000

// Estrutura para armazenar a contagem de palavras
typedef struct {
    char word[MAX_WORD_LEN];
    int count;
} WordCount;

// Estrutura para armazenar a contagem de músicas por artista
typedef struct {
    char artist[MAX_ARTIST_LEN];
    int count;
} ArtistCount;

// Funções de comparação para o qsort
int compare_word_count(const void *a, const void *b) {
    return ((WordCount *)b)->count - ((WordCount *)a)->count;
}

int compare_artist_count(const void *a, const void *b) {
    return ((ArtistCount *)b)->count - ((ArtistCount *)a)->count;
}

// Função para extrair campos de CSV (lidando com campos entre aspas)
char* extract_csv_field(char **ptr) {
    if (*ptr == NULL || **ptr == '\0') return NULL;
    
    char *start = *ptr;
    char *field_start = start;
    
    // Se o campo começa com aspas
    if (*start == '"') {
        field_start = ++start; // Pula a aspa inicial
        
        while (*start) {
            if (*start == '"') {
                if (*(start + 1) == '"') {
                    // Aspas duplas escapadas
                    start += 2;
                } else {
                    // Fim do campo entre aspas
                    *start = '\0';
                    start++;
                    if (*start == ',') start++;
                    *ptr = start;
                    return field_start;
                }
            } else {
                start++;
            }
        }
    } else {
        // Campo sem aspas
        while (*start && *start != ',') {
            start++;
        }
        if (*start == ',') {
            *start = '\0';
            *ptr = start + 1;
        } else {
            *ptr = start;
        }
        return field_start;
    }
    
    *ptr = start;
    return field_start;
}

// Função para adicionar ou incrementar palavra
void add_word(WordCount **words, int *word_count, int *word_capacity, const char *word) {
    if (strlen(word) < 2 || strlen(word) >= MAX_WORD_LEN) return;
    
    // Procura se a palavra já existe
    for (int i = 0; i < *word_count; i++) {
        if (strcmp((*words)[i].word, word) == 0) {
            (*words)[i].count++;
            return;
        }
    }
    
    // Adiciona nova palavra
    if (*word_count >= *word_capacity) {
        *word_capacity *= 2;
        *words = realloc(*words, *word_capacity * sizeof(WordCount));
    }
    strncpy((*words)[*word_count].word, word, MAX_WORD_LEN - 1);
    (*words)[*word_count].word[MAX_WORD_LEN - 1] = '\0';
    (*words)[*word_count].count = 1;
    (*word_count)++;
}

// Função para adicionar ou incrementar artista
void add_artist(ArtistCount **artists, int *artist_count, int *artist_capacity, const char *artist) {
    if (strlen(artist) == 0 || strlen(artist) >= MAX_ARTIST_LEN) return;
    
    // Procura se o artista já existe
    for (int i = 0; i < *artist_count; i++) {
        if (strcmp((*artists)[i].artist, artist) == 0) {
            (*artists)[i].count++;
            return;
        }
    }
    
    // Adiciona novo artista
    if (*artist_count >= *artist_capacity) {
        *artist_capacity *= 2;
        *artists = realloc(*artists, *artist_capacity * sizeof(ArtistCount));
    }
    strncpy((*artists)[*artist_count].artist, artist, MAX_ARTIST_LEN - 1);
    (*artists)[*artist_count].artist[MAX_ARTIST_LEN - 1] = '\0';
    (*artists)[*artist_count].count = 1;
    (*artist_count)++;
}

// Função para processar uma porção das linhas do CSV
void process_chunk(char *chunk, size_t chunk_size, WordCount **words, int *word_count, int *word_capacity, ArtistCount **artists, int *artist_count, int *artist_capacity) {
    char *line_start = chunk;
    char *line_end;

    while (line_start < chunk + chunk_size && *line_start) {
        line_end = strchr(line_start, '\n');
        if (line_end == NULL) {
            line_end = chunk + chunk_size;
        }
        
        size_t line_len = line_end - line_start;
        if (line_len > 0 && line_len < MAX_LINE_LEN) {
            char line[MAX_LINE_LEN];
            strncpy(line, line_start, line_len);
            line[line_len] = '\0';
            
            // Remove \r se existir
            if (line_len > 0 && line[line_len - 1] == '\r') {
                line[line_len - 1] = '\0';
            }
            
            char *ptr = line;
            
            // Extrair campos: artist, song, link, text
            char *artist_name = extract_csv_field(&ptr);
            char *song = extract_csv_field(&ptr);
            char *link = extract_csv_field(&ptr);
            char *text = extract_csv_field(&ptr);
            
            if (artist_name != NULL && strlen(artist_name) > 0) {
                // Adicionar artista
                add_artist(artists, artist_count, artist_capacity, artist_name);
                
                // Processar texto (letra da música)
                if (text != NULL && strlen(text) > 0) {
                    // Tokenizar o texto em palavras
                    char *word = strtok(text, " \t\n\r,.-?!\"'()[]{}:;/\\");
                    while (word != NULL) {
                        // Converter para minúsculas
                        for (int i = 0; word[i]; i++) {
                            word[i] = tolower((unsigned char)word[i]);
                        }
                        
                        // Verificar se a palavra tem pelo menos uma letra
                        int has_alpha = 0;
                        for (int i = 0; word[i]; i++) {
                            if (isalpha((unsigned char)word[i])) {
                                has_alpha = 1;
                                break;
                            }
                        }
                        
                        if (has_alpha && strlen(word) >= 2) {
                            add_word(words, word_count, word_capacity, word);
                        }
                        
                        word = strtok(NULL, " \t\n\r,.-?!\"'()[]{}:;/\\");
                    }
                }
            }
        }
        
        line_start = line_end + 1;
    }
}
int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);
    int rank, nprocs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    double start_time = MPI_Wtime();
    char *filename = "spotify_millsongdata_novo.csv";
    
    // Variáveis para armazenar resultados
    int word_capacity = INITIAL_CAPACITY, artist_capacity = INITIAL_CAPACITY;
    int word_count = 0, artist_count = 0;
    WordCount *words = malloc(word_capacity * sizeof(WordCount));
    ArtistCount *artists = malloc(artist_capacity * sizeof(ArtistCount));
    
    // --- Lógica do Mestre ---
    if (rank == 0) {
        printf("=== Análise Paralela de Dados do Spotify com MPI ===\n");
        printf("Número de processos: %d\n", nprocs);
        printf("Arquivo: %s\n\n", filename);
        
        FILE *file = fopen(filename, "rb");
        if (!file) {
            fprintf(stderr, "Erro ao abrir o arquivo %s\n", filename);
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        // Obter tamanho do arquivo
        fseek(file, 0, SEEK_END);
        long file_size = ftell(file);
        fseek(file, 0, SEEK_SET);
        
        printf("Tamanho do arquivo: %.2f MB\n", file_size / (1024.0 * 1024.0));
        
        // Ler todo o arquivo
        char *file_content = malloc(file_size + 1);
        size_t bytes_read = fread(file_content, 1, file_size, file);
        file_content[bytes_read] = '\0';
        fclose(file);
        
        // Pular cabeçalho
        char *data_start = strchr(file_content, '\n');
        if (data_start) data_start++;
        else data_start = file_content;
        
        size_t data_size = bytes_read - (data_start - file_content);
        
        // Dividir o trabalho entre os processos
        size_t chunk_size = data_size / nprocs;
        
        // Enviar chunks para os trabalhadores
        for (int i = 1; i < nprocs; i++) {
            size_t start_pos = i * chunk_size;
            size_t end_pos = (i == nprocs - 1) ? data_size : (i + 1) * chunk_size;
            
            // Ajustar para não cortar no meio de uma linha
            if (end_pos < data_size) {
                while (end_pos < data_size && data_start[end_pos] != '\n') {
                    end_pos++;
                }
                if (end_pos < data_size) end_pos++;
            }
            
            size_t actual_size = end_pos - start_pos;
            
            MPI_Send(&actual_size, 1, MPI_UNSIGNED_LONG, i, 0, MPI_COMM_WORLD);
            MPI_Send(data_start + start_pos, actual_size, MPI_CHAR, i, 1, MPI_COMM_WORLD);
        }
        
        // Processar o chunk do mestre
        size_t master_size = (nprocs > 1) ? chunk_size : data_size;
        if (nprocs > 1 && master_size < data_size) {
            while (master_size < data_size && data_start[master_size] != '\n') {
                master_size++;
            }
            if (master_size < data_size) master_size++;
        }
        
        printf("Processando dados em paralelo...\n");
        process_chunk(data_start, master_size, &words, &word_count, &word_capacity, 
                     &artists, &artist_count, &artist_capacity);
        
        free(file_content);
        
        // Receber e agregar resultados dos trabalhadores
        for (int i = 1; i < nprocs; i++) {
            MPI_Status status;
            int recv_word_count, recv_artist_count;
            
            // Receber palavras
            MPI_Recv(&recv_word_count, 1, MPI_INT, i, 2, MPI_COMM_WORLD, &status);
            if (recv_word_count > 0) {
                WordCount *recv_words = malloc(recv_word_count * sizeof(WordCount));
                MPI_Recv(recv_words, recv_word_count * sizeof(WordCount), MPI_BYTE, i, 3, 
                        MPI_COMM_WORLD, &status);
                
                // Mesclar palavras
                for (int j = 0; j < recv_word_count; j++) {
                    int found = 0;
                    for (int k = 0; k < word_count; k++) {
                        if (strcmp(words[k].word, recv_words[j].word) == 0) {
                            words[k].count += recv_words[j].count;
                            found = 1;
                            break;
                        }
                    }
                    if (!found) {
                        if (word_count >= word_capacity) {
                            word_capacity *= 2;
                            words = realloc(words, word_capacity * sizeof(WordCount));
                        }
                        words[word_count++] = recv_words[j];
                    }
                }
                free(recv_words);
            }
            
            // Receber artistas
            MPI_Recv(&recv_artist_count, 1, MPI_INT, i, 4, MPI_COMM_WORLD, &status);
            if (recv_artist_count > 0) {
                ArtistCount *recv_artists = malloc(recv_artist_count * sizeof(ArtistCount));
                MPI_Recv(recv_artists, recv_artist_count * sizeof(ArtistCount), MPI_BYTE, i, 5, 
                        MPI_COMM_WORLD, &status);
                
                // Mesclar artistas
                for (int j = 0; j < recv_artist_count; j++) {
                    int found = 0;
                    for (int k = 0; k < artist_count; k++) {
                        if (strcmp(artists[k].artist, recv_artists[j].artist) == 0) {
                            artists[k].count += recv_artists[j].count;
                            found = 1;
                            break;
                        }
                    }
                    if (!found) {
                        if (artist_count >= artist_capacity) {
                            artist_capacity *= 2;
                            artists = realloc(artists, artist_capacity * sizeof(ArtistCount));
                        }
                        artists[artist_count++] = recv_artists[j];
                    }
                }
                free(recv_artists);
            }
        }

        double end_time = MPI_Wtime();
        
        // Ordenar e exibir resultados
        qsort(words, word_count, sizeof(WordCount), compare_word_count);
        qsort(artists, artist_count, sizeof(ArtistCount), compare_artist_count);

        printf("\n================================================\n");
        printf("RESULTADOS DA ANÁLISE\n");
        printf("================================================\n\n");
        
        printf("--- Top %d Artistas com Mais Músicas ---\n", TOP_N);
        for(int i = 0; i < TOP_N && i < artist_count; i++) {
            printf("%3d. %-40s %6d músicas\n", i + 1, artists[i].artist, artists[i].count);
        }

        printf("\n--- Top %d Palavras Mais Frequentes ---\n", TOP_N);
        for(int i = 0; i < TOP_N && i < word_count; i++) {
            printf("%3d. %-30s %10d ocorrências\n", i + 1, words[i].word, words[i].count);
        }
        
        printf("\n================================================\n");
        printf("Estatísticas:\n");
        printf("  - Total de artistas únicos: %d\n", artist_count);
        printf("  - Total de palavras únicas: %d\n", word_count);
        printf("  - Tempo de execução: %.3f segundos\n", end_time - start_time);
        printf("================================================\n");
        
        free(words);
        free(artists);

    } 
    // --- Lógica do Trabalhador ---
    else {
        MPI_Status status;
        size_t chunk_size;

        MPI_Recv(&chunk_size, 1, MPI_UNSIGNED_LONG, 0, 0, MPI_COMM_WORLD, &status);
        char *chunk = malloc(chunk_size + 1);
        MPI_Recv(chunk, chunk_size, MPI_CHAR, 0, 1, MPI_COMM_WORLD, &status);
        chunk[chunk_size] = '\0';
        
        // Processar o chunk
        process_chunk(chunk, chunk_size, &words, &word_count, &word_capacity, 
                     &artists, &artist_count, &artist_capacity);
        
        free(chunk);
        
        // Enviar resultados de volta para o mestre
        MPI_Send(&word_count, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);
        if (word_count > 0) {
            MPI_Send(words, word_count * sizeof(WordCount), MPI_BYTE, 0, 3, MPI_COMM_WORLD);
        }
        MPI_Send(&artist_count, 1, MPI_INT, 0, 4, MPI_COMM_WORLD);
        if (artist_count > 0) {
            MPI_Send(artists, artist_count * sizeof(ArtistCount), MPI_BYTE, 0, 5, MPI_COMM_WORLD);
        }
        
        free(words);
        free(artists);
    }

    MPI_Finalize();
    return 0;
}