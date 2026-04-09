/*
 * Projeto: ULA 4 Bits com Indicador de Próxima Instrução (--)
 * O símbolo -- aparece acima da instrucao apontada pelo PC.
 *  
 * Banco de Registradores - vetor com 4 posições:
 *
 * PC (Program Counter/Banco de Registradores): Índice da próxima instrução a ser executada no vetor memória.
 * Comeca em 0 e e incrementado a cada instrucao executada.
 * 
 * W :Resultado da ultima operacao executada pela ULA.
 * 
 * X :Operando A (primeiro argumento) da última instrução executada.
 *
 * Y :Operando B (segundo argumento) da última instrução executada.
 */
 


const int leds[] = {13, 12, 11, 10}; 
String memoria[100];           
int registradores[4];          // [0]=PC, [1]=W, [2]=X, [3]=Y
int totalInstrucoes = 0;       
bool modoExecucao = false;

void setup() {
  Serial.begin(9600);
  // Configura os 4 pinos dos LEDs como saida (pinos 13, 12, 11, 10)
  for (int i = 0; i < 4; i++) pinMode(leds[i], OUTPUT);
  Serial.println("Aguardando carga do programa");
}

// Converte um caractere hex ('0'-'F') para seu valor inteiro (0-15)
int hexToVal(char c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  return 0; // caractere inválido retorna 0
}

// Converte um valor inteiro (0-15) para seu caractere hex ('0'-'F')
char valToHex(int v) {
  v &= 0xF; // garante que o valor esta entre 0 e 15
  if (v < 10) return v + '0';
  return (v - 10) + 'A';
}

void mostrarNosLeds(int valor) {
  for (int i = 0; i < 4; i++) {
    digitalWrite(leds[i], (valor >> (3 - i)) & 0x01);
  }
}
int calcularULA(int x, int y, int s) {
  int nx = (~x) & 0xF;
  int ny = (~y) & 0xF;
  switch (s) {
    case 0x0: return x;                
    case 0x1: return y;                
    case 0x2: return (x ^ y) & 0xF;    
    case 0x3: return (nx ^ ny) & 0xF;  
    case 0x4: return (~(x & y)) & 0xF; 
    case 0x5: return ny;               
    case 0x6: return (nx | ny) & 0xF;  
    case 0x7: return nx;               
    case 0x8: return (x | ny) & 0xF;   
    case 0x9: return 0xF;              
    case 0xA: return 0x0;              
    case 0xB: return (x & y) & 0xF;    
    case 0xC: return (nx & y) & 0xF;   
    case 0xD: return (x & ny) & 0xF;   
    case 0xE: return (x | y) & 0xF;    
    case 0xF: return (nx & ny) & 0xF;  
    default: return 0;
  }
}

// FUNCAO DE DUMP COM O INDICADOR  
// para exibir o estado da maquina atual no formato abaixo
//            --
// Memoria: | C6B | A3E |     |
// Registradores: | 1 | 4 | C | 6 |

void realizarDump() {
  int pc = registradores[0];

  // Imprime a linha do indicador (--)
  // "Memoria: |" tem 11 caracteres. O primeiro '--' deve alinhar com a instrução 0.
  Serial.print("           "); //  espaços para alinhar com o inicio do conteudo da barra
  for (int i = 0; i < pc; i++) {
    Serial.print("      "); // espacos para cada instrucao ja passada
  }
  Serial.println("--");

  // Imprime a linha da memoria : exibe apenas as posicoes carregadas
  Serial.print("Memoria: | ");
  for (int i = 0; i < totalInstrucoes; i++) {
    Serial.print(memoria[i]);
    Serial.print(" | ");
  }
  Serial.println("    |"); // Espaco vazio no final

  // Imprime a linha dos registradores : PC, W, X e Y convertidos para hex
  Serial.print("Registradores: | ");
  for (int i = 0; i < 4; i++) {
    Serial.print(valToHex(registradores[i]));
    Serial.print(" | ");
  }
  Serial.println("\n");
}

void loop() {
  // Verifica se há dados na serial E se ainda nao estamos em modo de execução
  // (durante a execucao e ignorado entradas seriais)
  if (Serial.available() > 0 && !modoExecucao) {

    // Le toda a string recebida de uma vez e remove espaços/quebras de linha nas bordas
    String input = Serial.readString();
    input.trim();

    // Usuario respondeu "S" para iniciar a execução 
    if (input.equalsIgnoreCase("S")) {
      if (totalInstrucoes > 0) {
        modoExecucao = true;   // ativa o modo de execução no próximo ciclo do loop
        registradores[0] = 0; // garante que o PC começa do inicio da memoria
      }
    }

    // Usuario enviou instruções para carregar na memoria 
    else {
      int len = input.length();

      // Percorre a string recebida de 3 em 3 caracteres procurando instrucoes validas
      for (int i = 0; i <= len - 3; i++) {
        String code = input.substring(i, i + 3); // extrai a instrucao

        // So armazena se nao houver espaço dentro dos 3 caracteres
        // (espaco indicaria que pegamos partes de duas instruções separadas)
        if (code.indexOf(' ') == -1) {
          memoria[totalInstrucoes++] = code; // salva instrução na proxima posicao livre
          i += 2; // avança mais 2 (o for já avança 1), pulando os outros chars dessa instrução
        }
      }

      // Exibe o estado da memoria logo apos a carga para o usuário confirmar
      Serial.println("Carga do vetor:");
      realizarDump();

      // Aguarda confirmacao do usuário 
      Serial.println("Deseja executar o programa (S/N)?");
    }
  }

  // Modo de execucao ativo: processa uma instrução por ciclo do loop 
  if (modoExecucao) {

    // Ainda existem instrucoes a executar (PC nao ultrapassou o total carregado)
    if (registradores[0] < totalInstrucoes) {

      // Delay para o usuario ler a mensagem ,antes de começar o dump
      if (registradores[0] == 0) delay(1000);

      // Busca na memoria a instrucao apontada pelo PC
      String instrucao = memoria[registradores[0]];

      // Decodifica os tres algarismos da instrução: X (operando A), Y (operando B), S (operação)
      int x = hexToVal(instrucao.charAt(0));
      int y = hexToVal(instrucao.charAt(1));
      int s = hexToVal(instrucao.charAt(2));

      // Executa a operacao na ULA e obtém o resultado W
      int w = calcularULA(x, y, s);

      // Atualiza o banco de registradores com os valores da instrução
      registradores[2] = x;    // X: operando A usado
      registradores[3] = y;    // Y: operando B usado
      registradores[1] = w;    // W: resultado da ULA
      registradores[0]++;      // PC: aponta para a proxima instrução

      // Resultado nos leds
      mostrarNosLeds(w);

      // Mensagem indicando qual instrucao acabou de ser executada
      if (registradores[0] == 1)
        Serial.println("Apos a execucao da primeira instrucao:");
      else if (registradores[0] == 2)
        Serial.println("Apos a execucao da segunda instrucao:");
      else {
        Serial.print("Apos a execucao da instrucao ");
        Serial.print(registradores[0]);
        Serial.println(":");
      }

      // Exibe a memoria e os registradores apos a execucao da instrucao
      realizarDump();

      // Pausa de 4 segundos para acompanhar o estado dos LEDs e do dump
      delay(4000);
    }

    // PC chegou ao fim: todas as instrucoes foram executadas
    else {
      Serial.println("Programa concluido!");
      modoExecucao = false; // volta ao modo de carga para um provavel novo programa
    }
  }
}
