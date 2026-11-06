// Etapa 3: teste do ESP32 sozinho (sem nenhum sensor ligado)
// - Pisca o LED azul da placa (GPIO2)
// - Mostra informações do chip no Monitor Serial (115200 baud)

const int LED_PLACA = 2;  // LED azul embutido na DevKit de 30 pinos

void setup() {
  Serial.begin(115200);
  delay(1000);  // tempo para o Monitor Serial abrir

  pinMode(LED_PLACA, OUTPUT);

  Serial.println();
  Serial.println("=== Etapa 3: teste do ESP32 ===");
  Serial.print("Modelo do chip: ");
  Serial.println(ESP.getChipModel());
  Serial.print("Revisao: ");
  Serial.println(ESP.getChipRevision());
  Serial.print("Nucleos: ");
  Serial.println(ESP.getChipCores());
  Serial.print("Frequencia (MHz): ");
  Serial.println(ESP.getCpuFreqMHz());
  Serial.print("Memoria flash (MB): ");
  Serial.println(ESP.getFlashChipSize() / (1024 * 1024));
  Serial.println("Se voce esta lendo isto, a placa esta funcionando!");
}

void loop() {
  digitalWrite(LED_PLACA, HIGH);
  Serial.println("LED aceso");
  delay(1000);

  digitalWrite(LED_PLACA, LOW);
  Serial.println("LED apagado");
  delay(1000);
}
