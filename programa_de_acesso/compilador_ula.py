import os

# Configuracoes de arquivos
ARQ_FONTE = "testeula.ula"
ARQ_HEX = "testeula.hex"

# Tabela de mnemonicos 
MNEMONICOS = {
    "CopiaA": "0", "CopiaB": "1", "AxB": "2", "nAxnB": "3",
    "AeBn": "4", "nB": "5", "nAonB": "6", "nA": "7",
    "AonB": "8", "UmL": "9", "ZeroL": "A", "AeB": "B",
    "nAeB": "C", "AenB": "D", "AoB": "E", "nAenB": "F",
}

def eh_hex_valido(val):
    # Valida se o valor e um unico digito hexadecimal (0-F).
    return len(val) == 1 and val.upper() in "0123456789ABCDEF"

def compilar():
    # Verifica se o arquivo fonte existe antes de tentar abri-lo
    if not os.path.exists(ARQ_FONTE):
        print(f"ERRO Arquivo '{ARQ_FONTE}' nao encontrado.")
        return

    instrucoes_geradas = []  # lista que acumulara as instrucoes no formato XYW
    x_atual = "0"            # valor corrente de X (atualizado a cada X=<val> no arquivo fonte)
    y_atual = "0"            # valor corrente de Y (atualizado a cada Y=<val> no arquivo fonte)
    linhas_com_erro = []     # acumula (num_linha, conteudo, motivo)

    with open(ARQ_FONTE, "r", encoding="utf-8") as f:
        for num_linha, linha in enumerate(f, start=1):  # enumerate para rastrear numero da linha
            # Remove espacos e o ponto e virgula
            linha_limpa = linha.strip().replace(";", "")
            
            # Ignora linhas totalmente vazias ou os marcadores de inicio/fim
            if not linha_limpa:
                conteudo_original = linha.strip()
                if conteudo_original:  # tinha algo (ex: ";"), mas virou vazio apos limpeza
                    linhas_com_erro.append((num_linha, linha.rstrip(), "conteudo invalido (apenas ';' ou espacos)"))
                else:
                    linhas_com_erro.append((num_linha, "(vazia)", "linha vazia"))
                continue
            if linha_limpa.lower() in ["inicio:", "fim."]:
                continue
            # Verifica se uma atribuicao (X=, Y=, W=)
            if "=" in linha_limpa:
                # Divide a linha em [variavel, valor] pelo sinal de '='
                partes = linha_limpa.split("=",1)
                
                # Ignora linhas malformadas como "X=" ou "W=" sem valor
                # len retorna o tamanho da string, entao se for menor que 2, 
                # significa que nao tem o valor apos o "="
                # O strip() remove espacos, entao se o valor for apenas espacos,
                # o strip() resultara em string vazia, e a linha sera ignorada
                if len(partes) < 2 or not partes[1].strip():
                    linhas_com_erro.append((num_linha, linha.rstrip(), "atribuicao sem valor (ex: 'X=')"))
                    continue
        
                var = partes[0].strip().upper() # variavel: X, Y ou W (converte para maiusculo)
                val = partes[1].strip() # Valor atribuido a variavel (hexadecimal : 0-F ou mnemonico: CopiaA, AxB, etc)
                if var == "X":
                    # Se for invalido (X=G, X=12), ignora e mantem o X anterior
                    if eh_hex_valido(val):
                        x_atual = val.upper()
                    else:
                        linhas_com_erro.append((num_linha, linha.rstrip(), f"valor hex invalido para X: '{val}'"))
                elif var == "Y":
                    # Se for invalido, ignora e mantem o Y anterior
                    if eh_hex_valido(val):
                        y_atual = val.upper()
                    else:
                        linhas_com_erro.append((num_linha, linha.rstrip(), f"valor hex invalido para Y: '{val}'"))
                elif var == "W":
                    # Busca case-insensitive no dicionario de mnemonicos.
                    # Percorre todos os pares (mnemonico, codigo) e armazena o codigo
                    # do mnemonico que bater com val (ignorando maiusculas/minusculas).
                    # Se nao encontrar nenhum, cod_s permanece None e a linha sera ignorada.
                    cod_s = None
                    for mnem, codigo in MNEMONICOS.items():
                        if mnem == val:
                            cod_s = codigo
                            break #  mnemonico encontrado, encerra o loop
                    if cod_s:
                        # Gera a instrucao final de 3 caracteres (X + Y + S)
                        instrucao = f"{x_atual}{y_atual}{cod_s}"
                        instrucoes_geradas.append(instrucao)
                    else:
                        linhas_com_erro.append((num_linha, linha.rstrip(), f"mnemonico desconhecido: '{val}'"))
                else:
                    linhas_com_erro.append((num_linha, linha.rstrip(), f"variavel desconhecida: '{var}'"))
            else:
                linhas_com_erro.append((num_linha, linha.rstrip(), "linha sem '=' e nao reconhecida"))

     # Relatorio de erros: exibe todas as linhas problematicas encontradas durante a compilacao
    if linhas_com_erro:
        print(f"\n{'='*50}")
        print(f" {len(linhas_com_erro)} linha(s) com problema encontrada(s):")  # quantidade total de erros
        print(f"{'='*50}")
        for num, conteudo, motivo in linhas_com_erro:  # desempacota a tupla (num_linha, conteudo, motivo)
            print(f"  Linha {num:>4}: {motivo}")   # num:>4 alinha o numero a direita em 4 espacos
            print(f"           >> {conteudo}")      # exibe o conteudo original da linha com erro
        print(f"{'='*50}\n")
    # Gravacao do arquivo .hex
    if instrucoes_geradas:
        with open(ARQ_HEX, "w", encoding="utf-8") as f_out:
            for item in instrucoes_geradas:
                f_out.write(item + "\n")
        print(f" Gerado '{ARQ_HEX}' com {len(instrucoes_geradas)} instrucoes.")
    else:
        print(" Nenhuma instrucao valida encontrada.")
        return -1

if __name__ == "__main__":
    compilar()
