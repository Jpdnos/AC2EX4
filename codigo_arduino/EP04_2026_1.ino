/*
 * Projeto: ULA 4 Bits com Indicador de Próxima Instrução (--)
 * O símbolo -- aparece acima da instrução apontada pelo PC.
 */

const int leds[] = {13, 12, 11, 10}; 
String memoria[100];           
int registradores[4];          // [0]=PC, [1]=W, [2]=X, [3]=Y
int totalInstrucoes = 0;       
bool modoExecucao = false;

void setup() {
  Serial.begin(9600);
  for (int i = 0; i < 4; i++) pinMode(leds[i], OUTPUT);
  Serial.println("Aguardando carga do programa");
}

int hexToVal(char c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  return 0;
}

char valToHex(int v) {
  v &= 0xF; 
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

// --- FUNÇÃO DE DUMP COM INDICADOR MÓVEL ---
void realizarDump() {
  int pc = registradores[0];

  // 1. Imprime a linha do indicador (--)
  // "Memoria: |" tem 11 caracteres. O primeiro '--' deve alinhar com a instrução 0.
  Serial.print("           "); // 11 espaços para alinhar com o início do conteúdo da barra
  for (int i = 0; i < pc; i++) {
    Serial.print("      "); // 6 espaços para cada instrução já passada
  }
  Serial.println("--");

  // 2. Imprime a linha da Memória
  Serial.print("Memoria: | ");
  for (int i = 0; i < totalInstrucoes; i++) {
    Serial.print(memoria[i]);
    Serial.print(" | ");
  }
  Serial.println("    |"); // Espaço vazio no final

  // 3. Imprime a linha dos Registradores
  Serial.print("Registradores: | ");
  for (int i = 0; i < 4; i++) {
    Serial.print(valToHex(registradores[i]));
    Serial.print(" | ");
  }
  Serial.println("\n");
}

void loop() {
  if (Serial.available() > 0 && !modoExecucao) {
    String input = Serial.readString();
    input.trim();

    if (input.equalsIgnoreCase("S")) {
      if (totalInstrucoes > 0) {
        modoExecucao = true;
        registradores[0] = 0; 
      }
    } 
    else {
      // Carga do Vetor
      int len = input.length();
      for (int i = 0; i <= len - 3; i++) {
        String code = input.substring(i, i + 3);
        if (code.indexOf(' ') == -1) {
          memoria[totalInstrucoes++] = code;
          i += 2; 
        }
      }
      Serial.println("Carga do vetor:");
      realizarDump();
      Serial.println("Deseja executar o programa (S/N)?");
    }
  }

  if (modoExecucao) {
    if (registradores[0] < totalInstrucoes) {
      // Pequeno atraso antes de mostrar o dump da execução para dar tempo de ler
      if (registradores[0] == 0) delay(1000); 

      String instrucao = memoria[registradores[0]];
      
      int x = hexToVal(instrucao.charAt(0));
      int y = hexToVal(instrucao.charAt(1));
      int s = hexToVal(instrucao.charAt(2));

      int w = calcularULA(x, y, s);

      // Atualiza banco de registradores ANTES do Dump
      registradores[2] = x;    
      registradores[3] = y;    
      registradores[1] = w;    
      registradores[0]++; // Incrementa PC (indica a PRÓXIMA instrução)

      mostrarNosLeds(w);
      
      if (registradores[0] == 1) Serial.println("Após a execução da primeira instrução:");
      else if (registradores[0] == 2) Serial.println("Após a execução da segunda instrução:");
      else {
        Serial.print("Após a execução da instrução ");
        Serial.print(registradores[0]);
        Serial.println(":");
      }

      realizarDump();
      delay(4000);
    } 
    else {
      Serial.println("Programa concluido!");
      modoExecucao = false;
    }
  }
}