// Etapa 5: teste do BMP280 (temperatura e pressao) junto com o DHT22
// Ligacao BMP280: VIN -> 3V3 | GND -> GND | SCL -> GPIO22 | SDA -> GPIO21
// Ligacao DHT22: igual a Etapa 4 (dados no GPIO4)
// Bibliotecas: "Adafruit BMP280 Library" (+ "Adafruit BusIO") e "DHT sensor library"

#include <Wire.h>
#include <Adafruit_BMP280.h>
#include <DHT.h>

const int PINO_DHT = 4;
const int PINO_SDA = 21;
const int PINO_SCL = 22;

DHT dht(PINO_DHT, DHT22);
Adafruit_BMP280 bmp;
bool bmpOk = false;

// Le o registrador de identificacao (0xD0) do sensor no endereco I2C informado.
// 0x58 = BMP280, 0x60 = BME280, 0 = nao respondeu
uint8_t lerChipId(uint8_t endereco) {
  Wire.beginTransmission(endereco);
  Wire.write(0xD0);
  if (Wire.endTransmission() != 0) return 0;
  Wire.requestFrom((int)endereco, 1);
  if (Wire.available()) return Wire.read();
  return 0;
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println();
  Serial.println("=== Etapa 5: teste do BMP280 + DHT22 ===");

  Wire.begin(PINO_SDA, PINO_SCL);

  // 1) Procura dispositivos no barramento I2C
  Serial.println("Procurando dispositivos I2C...");
  int encontrados = 0;
  for (uint8_t endereco = 1; endereco < 127; endereco++) {
    Wire.beginTransmission(endereco);
    if (Wire.endTransmission() == 0) {
      Serial.print("  Dispositivo encontrado no endereco 0x");
      Serial.println(endereco, HEX);
      encontrados++;
    }
  }
  if (encontrados == 0) {
    Serial.println("  NENHUM dispositivo I2C encontrado! Confira SDA (D21), SCL (D22), VIN e GND.");
  }

  // 2) Identifica o chip e inicia o sensor
  const uint8_t enderecos[] = {0x76, 0x77};
  for (uint8_t i = 0; i < 2; i++) {
    uint8_t id = lerChipId(enderecos[i]);
    if (id == 0) continue;
    Serial.print("Chip no endereco 0x");
    Serial.print(enderecos[i], HEX);
    Serial.print(" tem ID 0x");
    Serial.print(id, HEX);
    if (id == 0x58) Serial.println(" -> e um BMP280");
    else if (id == 0x60) Serial.println(" -> e um BME280 (tambem mede umidade!)");
    else Serial.println(" -> ID desconhecido");

    if (bmp.begin(enderecos[i], id)) {
      bmpOk = true;
      break;
    }
  }
  if (!bmpOk) {
    Serial.println("ERRO: BMP280 nao iniciou. Confira as ligacoes.");
  }

  dht.begin();
  delay(2000);
}

void loop() {
  Serial.println("----------------------------------------");

  if (bmpOk) {
    Serial.print("BMP280  Temperatura: ");
    Serial.print(bmp.readTemperature(), 1);
    Serial.print(" C   |   Pressao: ");
    Serial.print(bmp.readPressure() / 100.0F, 1);
    Serial.println(" hPa");
  } else {
    Serial.println("BMP280  (sem leitura)");
  }

  float umidade = dht.readHumidity();
  float temperatura = dht.readTemperature();
  if (isnan(umidade) || isnan(temperatura)) {
    Serial.println("DHT22   ERRO de leitura");
  } else {
    Serial.print("DHT22   Temperatura: ");
    Serial.print(temperatura, 1);
    Serial.print(" C   |   Umidade: ");
    Serial.print(umidade, 1);
    Serial.println(" %");
  }

  delay(2500);
}
