// Etapa 5b: o sensor e um BME280 (ID 0x60) -> agora lendo tambem a umidade dele
// Ligacao: igual a Etapa 5 (VIN -> 3V3 | GND -> GND | SCL -> GPIO22 | SDA -> GPIO21) + DHT22 no GPIO4
// Bibliotecas: "Adafruit BME280 Library" (+ "Adafruit Unified Sensor", "Adafruit BusIO") e "DHT sensor library"

#include <Wire.h>
#include <Adafruit_BME280.h>
#include <DHT.h>

const int PINO_DHT = 4;
const int PINO_SDA = 21;
const int PINO_SCL = 22;

DHT dht(PINO_DHT, DHT22);
Adafruit_BME280 bme;
bool bmeOk = false;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println();
  Serial.println("=== Etapa 5b: teste do BME280 + DHT22 ===");

  Wire.begin(PINO_SDA, PINO_SCL);

  if (bme.begin(0x76, &Wire)) {
    bmeOk = true;
    Serial.println("BME280 iniciado no endereco 0x76");
  } else if (bme.begin(0x77, &Wire)) {
    bmeOk = true;
    Serial.println("BME280 iniciado no endereco 0x77");
  } else {
    Serial.println("ERRO: BME280 nao iniciou. Confira as ligacoes.");
  }

  dht.begin();
  delay(2000);
}

void loop() {
  Serial.println("----------------------------------------------------------");

  if (bmeOk) {
    Serial.print("BME280  Temperatura: ");
    Serial.print(bme.readTemperature(), 1);
    Serial.print(" C  |  Umidade: ");
    Serial.print(bme.readHumidity(), 1);
    Serial.print(" %  |  Pressao: ");
    Serial.print(bme.readPressure() / 100.0F, 1);
    Serial.println(" hPa");
  } else {
    Serial.println("BME280  (sem leitura)");
  }

  float umidade = dht.readHumidity();
  float temperatura = dht.readTemperature();
  if (isnan(umidade) || isnan(temperatura)) {
    Serial.println("DHT22   ERRO de leitura");
  } else {
    Serial.print("DHT22   Temperatura: ");
    Serial.print(temperatura, 1);
    Serial.print(" C  |  Umidade: ");
    Serial.print(umidade, 1);
    Serial.println(" %");
  }

  delay(2500);
}
