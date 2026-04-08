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
    if not os.path.exists(ARQ_FONTE):
        print(f"[ERRO] Arquivo '{ARQ_FONTE}' não encontrado.")
        return

    instrucoes_geradas = []
    x_atual = "0"
    y_atual = "0"

    with open(ARQ_FONTE, "r", encoding="utf-8") as f:
        for linha in f:
            # Remove espacos e o ponto e virgula
            linha_limpa = linha.strip().replace(";", "")
            
            # Ignora linhas totalmente vazias ou os marcadores de inicio/fim
            if not linha_limpa or linha_limpa.lower() in ["inicio:", "fim."]:
                continue
            # Processa as atribuicoes (X=, Y=, W=)
            if "=" in linha_limpa:
                partes = linha_limpa.split("=")
                
                # Ignora linhas malformadas como "X=" ou "W=" sem valor
                if len(partes) < 2 or not partes[1].strip():
                    continue

                var = partes[0].strip().upper()
                val = partes[1].strip()
                if var == "X":
                    # Se for invalido (X=G, X=12), ignora e mantem o X anterior
                    if eh_hex_valido(val):
                        x_atual = val.upper()
                elif var == "Y":
                    # Se for invalido, ignora e mantem o Y anterior
                    if eh_hex_valido(val):
                        y_atual = val.upper()
                elif var == "W":
                    # Busca case-insensitive no dicionario de mnemônicos.
                    # Percorre todos os pares (mnemonico, codigo) e armazena o codigo
                    # do mnemônico que bater com val (ignorando maiusculas/minusculas).
                    # Se nao encontrar nenhum, cod_s permanece None e a linha será ignorada.
                    cod_s = None
                    for mnem, codigo in MNEMONICOS.items():
                        if mnem.lower() == val.lower():
                            cod_s = codigo
                            break
                    if cod_s:
                        # Gera a instrucao final de 3 caracteres (X + Y + S)
                        instrucao = f"{x_atual}{y_atual}{cod_s}"
                        instrucoes_geradas.append(instrucao)

    # Gravacao do arquivo .hex
    if instrucoes_geradas:
        with open(ARQ_HEX, "w", encoding="utf-8") as f_out:
            for item in instrucoes_geradas:
                f_out.write(item + "\n")
        print(f" Gerado '{ARQ_HEX}' com {len(instrucoes_geradas)} instruções.")
    else:
        print(" Nenhuma instrucao valida encontrada.")

if __name__ == "__main__":
    compilar()