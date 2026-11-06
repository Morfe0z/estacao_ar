// Etapa 7: teste do MQ135 (gases / qualidade do ar)
// Ligacao: VCC -> VIN (5 V) | GND -> GND | DO -> nada
//          AO -> resistor 4,7k -> ponto do meio -> resistor 4,7k -> GND
//          ponto do meio -> GPIO34
// O divisor reduz a tensao pela metade: o ESP32 nunca recebe mais que 2,5 V.

const int PINO_MQ135 = 34;
const float FATOR_DIVISOR = 2.0;  // dois resistores iguais: tensao no AO = 2 x tensao no pino
const int AMOSTRAS = 20;          // media de varias leituras para reduzir ruido

unsigned long inicio;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println();
  Serial.println("=== Etapa 7: teste do MQ135 ===");
  Serial.println("O sensor precisa aquecer: as leituras estabilizam depois de alguns minutos.");

  analogReadResolution(12);                         // 0 a 4095
  analogSetPinAttenuation(PINO_MQ135, ADC_11db);   // faixa de ~0 a 3,1 V
  inicio = millis();
}

void loop() {
  uint32_t somaMv = 0;
  uint32_t somaBruto = 0;
  for (int i = 0; i < AMOSTRAS; i++) {
    somaMv += analogReadMilliVolts(PINO_MQ135);
    somaBruto += analogRead(PINO_MQ135);
    delay(10);
  }
  float mvPino = somaMv / (float)AMOSTRAS;
  int bruto = somaBruto / AMOSTRAS;
  float vSaidaAO = mvPino * FATOR_DIVISOR / 1000.0;
  unsigned long segundos = (millis() - inicio) / 1000;

  Serial.print("Tempo: ");
  Serial.print(segundos);
  Serial.print(" s  |  Leitura bruta: ");
  Serial.print(bruto);
  Serial.print("  |  GPIO34: ");
  Serial.print(mvPino, 0);
  Serial.print(" mV  |  Saida AO do MQ135: ");
  Serial.print(vSaidaAO, 2);
  Serial.println(" V");

  if (mvPino > 2800) {
    Serial.println("ATENCAO: tensao no GPIO34 acima do esperado. Desconecte o USB e confira o divisor!");
  }

  delay(2000);
}
