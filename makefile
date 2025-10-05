# Define o compilador que queremos usar. Para MPI, usamos mpicc.
CC = mpicc

# Define as flags do compilador.
# -Wall: Mostra todos os avisos (warnings), o que é uma boa prática.
# -g:    Adiciona informações de debug ao executável (útil para depuradores como gdb).
# -O2:   Otimiza o código para velocidade (opcional, mas bom para desempenho).
CFLAGS = -Wall -g -O2

# Define o nome do arquivo executável de saída.
TARGET = spotify_analyzer

# Define o nome do arquivo-fonte C.
SRCS = app.c

# Regra principal e padrão: o que fazer quando você digita apenas "make".
# Ela depende da regra $(TARGET) para ser construída.
all: $(TARGET)

# Regra para construir o executável final.
# Ela "depende" do arquivo-fonte, ou seja, se o app.c mudar, esta regra será executada.
$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)
	@echo "Programa compilado com sucesso! Executável: $(TARGET)"

# Regra para limpar o diretório (remover o executável e outros arquivos compilados).
# É útil para começar uma compilação do zero.
clean:
	rm -f $(TARGET)
	@echo "Arquivos compilados foram removidos."

# Declara que "all" e "clean" não são nomes de arquivos reais.
.PHONY: all clean