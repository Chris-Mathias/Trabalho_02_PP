import ollama
import pandas as pd

def classificar_sentimento(letra):
    prompt = f"""
    Classifique o sentimento da seguinte letra de música como 'Positiva', 'Neutra' ou 'Negativa'.
    Responda apenas com uma dessas palavras.

    Letra:
    {letra}
    """
    
    response = ollama.chat(
        model="gemma3:1b",  
        messages=[{"role": "user", "content": prompt}]
    )
    
    resposta = response["message"]["content"].strip().lower()
    
    if "positiva" in resposta:
        return "Positiva"
    elif "negativa" in resposta:
        return "Negativa"
    else:
        return "Neutra"

df = pd.read_csv('spotify_millsongdata_novo.csv')

df = df.dropna(subset=['text'])
df = df[df['text'].str.strip() != '']

print(f"Total de músicas encontradas: {len(df)}")

if 'link' in df.columns:
    df = df.drop(columns=['link'])

contagem = {"Positiva": 0, "Neutra": 0, "Negativa": 0}

print("\nIniciando classificação de sentimentos...")
for idx, row in df.iterrows():
    letra = row['text']
    artista = row.get('artist', 'Desconhecido')
    musica = row.get('song', 'Desconhecida')
    
    classe = classificar_sentimento(letra)
    contagem[classe] += 1
    
    # Adicionar a classificação no DataFrame
    df.at[idx, 'sentimento'] = classe
    
    # Printar cada música e seu resultado
    print(f"[{idx + 1}/{len(df)}] {artista} - {musica}: {classe}")
    
    # Salvar o CSV a cada música classificada
    df.to_csv('spotify_millsongdata_com_sentimento.csv', index=False)
    
    # Criar arquivo de resumo a cada 10 músicas
    if (idx + 1) % 10 == 0:
        with open("resumo_sentimentos.txt", "w", encoding='utf-8') as f:
            f.write("Análise de Sentimentos das Músicas\n\n")
            f.write(f"Total de músicas analisadas: {idx + 1}\n\n")
            f.write("Contagem por sentimento:\n")
            for sentimento, total in contagem.items():
                porcentagem = (total / (idx + 1)) * 100
                f.write(f"  {sentimento}: {total} ({porcentagem:.2f}%)\n")
        print(f"    → Resumo atualizado ({idx + 1} músicas processadas)")

print("\nClassificação concluída!")
print("\nArquivo salvo: spotify_millsongdata_com_sentimento.csv")

print("\nResumo final:")
for k, v in contagem.items():
    print(f"{k}: {v} ({v/len(df)*100:.2f}%)")

with open("resumo_sentimentos.txt", "w", encoding='utf-8') as f:
    f.write("Análise de Sentimentos das Músicas\n\n")
    f.write(f"Total de músicas analisadas: {len(df)}\n\n")
    f.write("Contagem por sentimento:\n")
    for sentimento, total in contagem.items():
        porcentagem = (total / len(df)) * 100
        f.write(f"  {sentimento}: {total} ({porcentagem:.2f}%)\n")

print("\nArquivo resumo salvo: resumo_sentimentos.txt")
