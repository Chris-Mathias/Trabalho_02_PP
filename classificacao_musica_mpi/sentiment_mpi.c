#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define MAXLINE 65536
#define MAXFIELDS 256
#define TMPDIR "/tmp"

int parse_csv_line(const char *line, char **fields, int maxfields) {
    const char *p = line;
    int field = 0;
    while (*p && field < maxfields) {
        if (*p == '"') {
            p++;
            const char *start = p;
            char *buf = malloc(MAXLINE);
            if (!buf) return -1;
            int idx = 0;
            while (*p) {
                if (*p == '"') {
                    if (*(p+1) == '"') {
                        buf[idx++] = '"';
                        p += 2;
                    } else {
                        p++;
                        break;
                    }
                } else {
                    buf[idx++] = *p;
                    p++;
                }
            }
            buf[idx] = '\0';
            fields[field++] = buf;
            while (*p && *p != ',') p++;
            if (*p == ',') p++;
        } else {
            const char *start = p;
            const char *comma = strchr(p, ',');
            size_t len = comma ? (size_t)(comma - start) : strlen(start);
            char *buf = malloc(len + 1);
            if (!buf) return -1;
            strncpy(buf, start, len);
            buf[len] = '\0';
            fields[field++] = buf;
            if (comma) p = comma + 1;
            else p += len;
        }
    }
    return field;
}

void free_fields(char **fields, int n) {
    for (int i = 0; i < n; ++i) {
        if (fields[i]) free(fields[i]);
    }
}

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Uso: %s input.csv output.csv\n", argv[0]);
        return 1;
    }

    const char *input_csv = argv[1];
    const char *output_csv = argv[2];

    MPI_Init(&argc, &argv);
    int rank, nprocs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    FILE *fin = NULL;
    if (rank == 0) {
        fin = fopen(input_csv, "r");
        if (!fin) {
            fprintf(stderr, "Erro ao abrir %s: %s\n", input_csv, strerror(errno));
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }

    char header[MAXLINE];
    int text_col_index = -1;
    int artist_col_index = -1;
    int song_col_index = -1;
    char **headers = NULL;
    int num_header_fields = 0;
    long total_rows = 0;

    if (rank == 0) {
        if (!fgets(header, sizeof(header), fin)) {
            fprintf(stderr, "Arquivo vazio ou erro ao ler cabeçalho.\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        size_t hlen = strlen(header);
        if (hlen && (header[hlen-1] == '\n' || header[hlen-1] == '\r')) {
            header[hlen-1] = '\0';
            hlen--;
            if (hlen && header[hlen-1] == '\r') { header[hlen-1] = '\0'; hlen--; }
        }
        headers = malloc(sizeof(char*) * MAXFIELDS);
        num_header_fields = parse_csv_line(header, headers, MAXFIELDS);
        for (int i = 0; i < num_header_fields; ++i) {
            if (strcasecmp(headers[i], "text") == 0) text_col_index = i;
            if (strcasecmp(headers[i], "artist") == 0) artist_col_index = i;
            if (strcasecmp(headers[i], "song") == 0) song_col_index = i;
        }
        if (text_col_index < 0) {
            fprintf(stderr, "Coluna 'text' não encontrada no cabeçalho.\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        if (artist_col_index < 0) {
            fprintf(stderr, "Coluna 'artist' não encontrada no cabeçalho.\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        if (song_col_index < 0) {
            fprintf(stderr, "Coluna 'song' não encontrada no cabeçalho.\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        char line[MAXLINE];
        total_rows = 0;
        while (fgets(line, sizeof(line), fin)) {
            total_rows++;
        }
        fclose(fin);
    }

    MPI_Bcast(&text_col_index, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&artist_col_index, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&song_col_index, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&num_header_fields, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&total_rows, 1, MPI_LONG, 0, MPI_COMM_WORLD);

    if (rank != 0) {
        headers = malloc(sizeof(char*) * MAXFIELDS);
        for (int i = 0; i < MAXFIELDS; ++i) headers[i] = NULL;
    }
    for (int i = 0; i < num_header_fields; ++i) {
        int len = 0;
        if (rank == 0) len = headers[i] ? (int)strlen(headers[i]) : 0;
        MPI_Bcast(&len, 1, MPI_INT, 0, MPI_COMM_WORLD);
        if (len > 0) {
            if (rank != 0) headers[i] = malloc(len + 1);
            MPI_Bcast(headers[i], len, MPI_CHAR, 0, MPI_COMM_WORLD);
            if (rank != 0) headers[i][len] = '\0';
        } else {
            if (rank != 0) {
                free(headers[i]);
                headers[i] = NULL;
            }
        }
    }

    fin = fopen(input_csv, "r");
    if (!fin) {
        fprintf(stderr, "[rank %d] Erro ao abrir %s: %s\n", rank, input_csv, strerror(errno));
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    char tmpbuf[MAXLINE];
    if (!fgets(tmpbuf, sizeof(tmpbuf), fin)) {
        fprintf(stderr, "Erro lendo cabeçalho (segunda vez)\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    char out_tmpname[1024];
    snprintf(out_tmpname, sizeof(out_tmpname), "%s/sent_part_rank_%d.csv", TMPDIR, rank);
    FILE *fout = fopen(out_tmpname, "w");
    if (!fout) {
        fprintf(stderr, "[rank %d] Erro ao abrir saída %s: %s\n", rank, out_tmpname, strerror(errno));
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    for (int i = 0; i < num_header_fields; ++i) {
        if (i) fprintf(fout, ",");
        if (headers[i]) {
            int need_quote = (strchr(headers[i], ',') != NULL);
            if (need_quote) fprintf(fout, "\"%s\"", headers[i]);
            else fprintf(fout, "%s", headers[i]);
        }
    }
    fprintf(fout, ",sentimento\n");

    long cnt_pos = 0, cnt_neg = 0, cnt_neu = 0, cnt_total = 0;

    long line_index = 0;
    while (fgets(tmpbuf, sizeof(tmpbuf), fin)) {
        if ((line_index % nprocs) != rank) {
            line_index++;
            continue;
        }

        char *fields[MAXFIELDS] = {0};
        int nf = parse_csv_line(tmpbuf, fields, MAXFIELDS);
        if (nf <= 0) {
            line_index++;
            free_fields(fields, nf);
            continue;
        }

        char *text_field = NULL;
    char *artist_field = (artist_col_index < nf) ? fields[artist_col_index] : "";
    char *song_field = (song_col_index < nf) ? fields[song_col_index] : "";
    if (text_col_index < nf) text_field = fields[text_col_index];
    else text_field = "";

        char tmpfile[1024];
        snprintf(tmpfile, sizeof(tmpfile), "%s/sent_rank%d_line%ld.txt", TMPDIR, rank, line_index);
        FILE *tf = fopen(tmpfile, "w");
        if (!tf) {
            fprintf(stderr, "[rank %d] Erro escrevendo tmpfile %s\n", rank, tmpfile);
            tf = fopen(tmpfile, "w");
            if (!tf) { }
        }
        if (tf) {
            fprintf(tf, "%s\n", text_field ? text_field : "");
            fclose(tf);
        }

        char cmd[2048];
        snprintf(cmd, sizeof(cmd), "python3 classify_ollama.py --file \"%s\"", tmpfile);

        FILE *pp = popen(cmd, "r");
        char sentiment[128] = {0};
        if (pp) {
            if (fgets(sentiment, sizeof(sentiment), pp)) {
                size_t sl = strlen(sentiment);
                while (sl > 0 && (sentiment[sl-1] == '\n' || sentiment[sl-1] == '\r')) {
                    sentiment[--sl] = '\0';
                }
            } else {
                strcpy(sentiment, "neutro");
            }
            int rc = pclose(pp);
            (void)rc;
        } else {
            strcpy(sentiment, "neutro");
        }

        printf("[rank %d] Artista: %s | Música: %s | Sentimento: %s\n", rank, artist_field, song_field, sentiment);

        if (strcmp(sentiment, "positivo") == 0) cnt_pos++;
        else if (strcmp(sentiment, "negativo") == 0) cnt_neg++;
        else cnt_neu++;

        for (int i = 0; i < nf; ++i) {
            if (i) fprintf(fout, ",");
            if (fields[i]) {
                int need_quote = (strchr(fields[i], ',') != NULL || strchr(fields[i], '"') != NULL);
                if (need_quote) {
                    fprintf(fout, "\"");
                    for (char *c = fields[i]; *c; ++c) {
                        if (*c == '"') fprintf(fout, "\"\"");
                        else fputc(*c, fout);
                    }
                    fprintf(fout, "\"");
                } else {
                    fprintf(fout, "%s", fields[i]);
                }
            }
        }
        fprintf(fout, ",%s\n", sentiment);

        free_fields(fields, nf);

        remove(tmpfile);

        cnt_total++;
        if (cnt_total % 2 == 0) {
            long sum_pos = 0, sum_neg = 0, sum_neu = 0, sum_total = 0;
            MPI_Reduce(&cnt_pos, &sum_pos, 1, MPI_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
            MPI_Reduce(&cnt_neg, &sum_neg, 1, MPI_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
            MPI_Reduce(&cnt_neu, &sum_neu, 1, MPI_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
            MPI_Reduce(&cnt_total, &sum_total, 1, MPI_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
            if (rank == 0) {
                printf("\n[global] Resumo após %ld músicas: Positivo = %ld, Negativo = %ld, Neutro = %ld\n\n",
                       sum_total, sum_pos, sum_neg, sum_neu);
            }
        }
        line_index++;
    }

    fclose(fin);
    fclose(fout);

    long total_pos = 0, total_neg = 0, total_neu = 0, total_all = 0;
    MPI_Reduce(&cnt_pos, &total_pos, 1, MPI_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&cnt_neg, &total_neg, 1, MPI_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&cnt_neu, &total_neu, 1, MPI_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&cnt_total, &total_all, 1, MPI_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

    MPI_Barrier(MPI_COMM_WORLD);

    if (rank == 0) {
        FILE *fout_final = fopen(output_csv, "w");
        if (!fout_final) {
            fprintf(stderr, "Erro ao criar %s: %s\n", output_csv, strerror(errno));
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        char headerline[MAXLINE];
        char partname[1024];
        snprintf(partname, sizeof(partname), "%s/sent_part_rank_%d.csv", TMPDIR, 0);
        FILE *p0 = fopen(partname, "r");
        if (!p0) {
            fprintf(stderr, "Erro ao abrir parte %s\n", partname);
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        if (!fgets(headerline, sizeof(headerline), p0)) {
            fprintf(stderr, "Erro ler header da parte %s\n", partname);
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        fputs(headerline, fout_final);
        while (fgets(headerline, sizeof(headerline), p0)) fputs(headerline, fout_final);
        fclose(p0);

        for (int r = 1; r < nprocs; ++r) {
            snprintf(partname, sizeof(partname), "%s/sent_part_rank_%d.csv", TMPDIR, r);
            FILE *pr = fopen(partname, "r");
            if (!pr) continue;
            if (!fgets(headerline, sizeof(headerline), pr)) { fclose(pr); continue; }
            while (fgets(headerline, sizeof(headerline), pr)) fputs(headerline, fout_final);
            fclose(pr);
            remove(partname);
        }
        fclose(fout_final);

        printf("Total linhas processadas: %ld\n", total_all);
        printf("Positivo: %ld (%.2f%%)\n", total_pos, total_all ? (100.0 * total_pos / total_all) : 0.0);
        printf("Negativo: %ld (%.2f%%)\n", total_neg, total_all ? (100.0 * total_neg / total_all) : 0.0);
        printf("Neutro:   %ld (%.2f%%)\n", total_neu, total_all ? (100.0 * total_neu / total_all) : 0.0);
        printf("Arquivo final escrito: %s\n", output_csv);
    }

    for (int i = 0; i < num_header_fields; ++i) if (headers[i]) free(headers[i]);
    free(headers);

    MPI_Finalize();
    return 0;
}
