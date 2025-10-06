import pandas as pd

arquivo_entrada = 'spotify_millsongdata.csv'
arquivo_saida = 'spotify_millsongdata_novo.csv'

df = pd.read_csv(arquivo_entrada)

if 'text' in df.columns:
    print("Processando a coluna 'text'...")
    df['text'] = df['text'].str.replace('\n', ' ', regex=False)
    df['text'] = df['text'].str.replace('\r', ' ', regex=False)
    df['text'] = df['text'].str.replace(r'\s+', ' ', regex=True)
    
    df.to_csv(arquivo_saida, index=False, encoding='utf-8')
    print(f"Processo concluído! O arquivo '{arquivo_saida}' foi criado com sucesso.")

else:
    print(f"Erro: A coluna 'text' não foi encontrada no arquivo '{arquivo_entrada}'.")
    print(f"Colunas encontradas: {df.columns.tolist()}")
