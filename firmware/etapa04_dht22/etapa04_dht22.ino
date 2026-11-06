// Etapa 4: teste do DHT22 (temperatura e umidade)
// Ligacao: perna 1 -> 3V3 | perna 2 -> GPIO4 (+ resistor 4,7k ate 3V3) | perna 3 -> nada | perna 4 -> GND
// Biblioteca: "DHT sensor library" (Adafruit) + "Adafruit Unified Sensor"

#include <DHT.h>

const int PINO_DHT = 4;
DHT dht(PINO_DHT, DHT22);

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println();
  Serial.println("=== Etapa 4: teste do DHT22 ===");
  dht.begin();
  delay(2000);  // o DHT22 precisa de ~2 s apos ligar
}

void loop() {
  float umidade = dht.readHumidity();
  float temperatura = dht.readTemperature();  // em graus Celsius

  if (isnan(umidade) || isnan(temperatura)) {
    Serial.println("ERRO: falha ao ler o DHT22 (confira as ligacoes e o resistor)");
  } else {
    Serial.print("Temperatura: ");
    Serial.print(temperatura, 1);
    Serial.print(" C   |   Umidade: ");
    Serial.print(umidade, 1);
    Serial.println(" %");
  }

  delay(2500);  // o DHT22 so aceita uma leitura a cada 2 s
}
