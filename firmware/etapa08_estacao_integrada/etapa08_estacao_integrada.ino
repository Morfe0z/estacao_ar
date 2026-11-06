// Etapa 8: estacao integrada - todos os sensores juntos
// ESP32 DevKit 30 pinos | core esp32 2.0.17
//
// Ligacoes (iguais as etapas anteriores):
//   BME280 : VIN -> 3V3 | GND -> GND | SDA -> GPIO21 | SCL -> GPIO22
//   DHT22  : perna 1 -> 3V3 | perna 2 -> GPIO4 (+ 4,7k ate 3V3) | perna 4 -> GND
//   PMS7003: TX -> GPIO16 | RX -> GPIO17 | VCC -> VIN | GND -> GND
//   MQ135  : VCC -> VIN | GND -> GND | AO -> divisor 4,7k/4,7k -> GPIO34
//
// Bibliotecas: "Adafruit BME280 Library" e "DHT sensor library" (Adafruit)

#include <Wire.h>
#include <Adafruit_BME280.h>
#include <DHT.h>

// ---------------- Pinos ----------------
const int PINO_SDA   = 21;
const int PINO_SCL   = 22;
const int PINO_DHT   = 4;
const int PMS_RX     = 16;  // recebe o TX do PMS7003
const int PMS_TX     = 17;  // envia para o RX do PMS7003
const int PINO_MQ135 = 34;

// ---------------- Configuracoes ----------------
const unsigned long INTERVALO_RELATORIO = 5000;    // imprime um resumo a cada 5 s
const unsigned long INTERVALO_DHT       = 2500;    // o DHT22 aceita 1 leitura a cada 2 s
const unsigned long INTERVALO_MQ        = 100;     // amostra do MQ135 a cada 100 ms
const unsigned long VALIDADE_DHT        = 10000;   // leitura do DHT22 vale por 10 s
const unsigned long VALIDADE_PMS        = 5000;    // sem pacote do PMS por 5 s = falha
const unsigned long AQUECIMENTO_MQ      = 180000;  // MQ135 aquecendo nos primeiros 3 min
const float FATOR_DIVISOR_MQ            = 2.0;     // dois resistores iguais de 4,7k

// ---------------- Objetos ----------------
Adafruit_BME280 bme;
DHT dht(PINO_DHT, DHT22);

// ---------------- Estado dos sensores ----------------
bool bmeOk = false;

float dhtTemp = NAN, dhtUmid = NAN;
unsigned long dhtUltimaLeitura = 0;   // momento da ultima leitura VALIDA
unsigned long dhtUltimaTentativa = 0;
unsigned long dhtFalhas = 0;

uint8_t pmsBuf[32];
int pmsIdx = 0;
unsigned long pmsUltimoPacote = 0;
unsigned long pmsPacotesOk = 0, pmsPacotesRuins = 0;
uint32_t pmsSoma1 = 0, pmsSoma25 = 0, pmsSoma10 = 0, pmsQtd = 0;   // acumulado do periodo
uint16_t pms1 = 0, pms25 = 0, pms10 = 0;                          // media do ultimo periodo
bool pmsTemDado = false;

uint32_t mqSomaMv = 0, mqQtd = 0;
unsigned long mqUltimaAmostra = 0;

unsigned long ultimoRelatorio = 0;
unsigned long numeroLeitura = 0;

// =====================================================================
// PMS7003: le os bytes recebidos e acumula cada pacote valido
// Pacote: 32 bytes, comeca com 0x42 0x4D, termina com soma de verificacao
// =====================================================================
void processarPms() {
  while (Serial2.available()) {
    uint8_t b = Serial2.read();

    if (pmsIdx == 0) {
      if (b == 0x42) pmsBuf[pmsIdx++] = b;
      continue;
    }
    if (pmsIdx == 1 && b != 0x4D) {
      pmsIdx = (b == 0x42) ? 1 : 0;
      continue;
    }

    pmsBuf[pmsIdx++] = b;
    if (pmsIdx < 32) continue;
    pmsIdx = 0;

    uint16_t comprimento = (pmsBuf[2] << 8) | pmsBuf[3];
    uint16_t soma = 0;
    for (int i = 0; i < 30; i++) soma += pmsBuf[i];
    uint16_t verificacao = (pmsBuf[30] << 8) | pmsBuf[31];
    if (comprimento != 28 || soma != verificacao) {
      pmsPacotesRuins++;
      continue;
    }

    pmsSoma1  += (pmsBuf[10] << 8) | pmsBuf[11];
    pmsSoma25 += (pmsBuf[12] << 8) | pmsBuf[13];
    pmsSoma10 += (pmsBuf[14] << 8) | pmsBuf[15];
    pmsQtd++;
    pmsPacotesOk++;
    pmsUltimoPacote = millis();
  }
}

// =====================================================================
// DHT22: tenta ler a cada 2,5 s e guarda a ultima leitura valida
// =====================================================================
void processarDht() {
  if (millis() - dhtUltimaTentativa < INTERVALO_DHT) return;
  dhtUltimaTentativa = millis();

  float u = dht.readHumidity();
  float t = dht.readTemperature();
  if (isnan(u) || isnan(t)) {
    dhtFalhas++;
    return;
  }
  dhtUmid = u;
  dhtTemp = t;
  dhtUltimaLeitura = millis();
}

// =====================================================================
// MQ135: acumula amostras para tirar a media no relatorio
// =====================================================================
void processarMq() {
  if (millis() - mqUltimaAmostra < INTERVALO_MQ) return;
  mqUltimaAmostra = millis();
  mqSomaMv += analogReadMilliVolts(PINO_MQ135);
  mqQtd++;
}

// =====================================================================
// Classificacao do PM2.5 (faixas de referencia para media de 24 h,
// usadas aqui so como indicacao para a leitura instantanea)
// =====================================================================
const char* classificarPm25(uint16_t pm) {
  if (pm <= 25)  return "BOA";
  if (pm <= 50)  return "MODERADA";
  if (pm <= 75)  return "RUIM";
  if (pm <= 125) return "MUITO RUIM";
  return "PESSIMA";
}

void imprimirTempoLigado() {
  unsigned long s = millis() / 1000;
  char texto[24];
  snprintf(texto, sizeof(texto), "%02lu:%02lu:%02lu", s / 3600, (s / 60) % 60, s % 60);
  Serial.print(texto);
}

// =====================================================================
// Relatorio a cada 5 s
// =====================================================================
void imprimirRelatorio() {
  numeroLeitura++;

  // fecha o periodo do PMS7003
  if (pmsQtd > 0) {
    pms1  = pmsSoma1  / pmsQtd;
    pms25 = pmsSoma25 / pmsQtd;
    pms10 = pmsSoma10 / pmsQtd;
    pmsTemDado = true;
  }
  uint32_t pmsQtdPeriodo = pmsQtd;
  pmsSoma1 = pmsSoma25 = pmsSoma10 = pmsQtd = 0;
  bool pmsOk = pmsTemDado && (millis() - pmsUltimoPacote < VALIDADE_PMS);

  // fecha o periodo do MQ135
  float mqMv = (mqQtd > 0) ? (float)mqSomaMv / mqQtd : NAN;
  mqSomaMv = mqQtd = 0;
  float mqVolts = mqMv * FATOR_DIVISOR_MQ / 1000.0;
  bool mqOk = !isnan(mqMv) && mqMv > 20 && mqMv < 2800;

  bool dhtOk = dhtUltimaLeitura > 0 && (millis() - dhtUltimaLeitura < VALIDADE_DHT);

  Serial.println();
  Serial.print("===== Leitura #");
  Serial.print(numeroLeitura);
  Serial.print("   (ligado ha ");
  imprimirTempoLigado();
  Serial.println(") =====");

  // Temperatura e umidade
  Serial.print("Temperatura : ");
  if (bmeOk) { Serial.print(bme.readTemperature(), 1); Serial.print(" C (BME280)"); }
  else Serial.print("--- (BME280)");
  Serial.print("  |  ");
  if (dhtOk) { Serial.print(dhtTemp, 1); Serial.println(" C (DHT22)"); }
  else Serial.println("--- (DHT22)");

  Serial.print("Umidade     : ");
  if (bmeOk) { Serial.print(bme.readHumidity(), 1); Serial.print(" % (BME280)"); }
  else Serial.print("--- (BME280)");
  Serial.print("  |  ");
  if (dhtOk) { Serial.print(dhtUmid, 1); Serial.println(" % (DHT22)"); }
  else Serial.println("--- (DHT22)");

  Serial.print("Pressao     : ");
  if (bmeOk) { Serial.print(bme.readPressure() / 100.0F, 1); Serial.println(" hPa"); }
  else Serial.println("---");

  // Particulas
  Serial.print("Particulas  : ");
  if (pmsOk) {
    Serial.print("PM1.0 = ");  Serial.print(pms1);
    Serial.print("  |  PM2.5 = "); Serial.print(pms25);
    Serial.print("  |  PM10 = ");  Serial.print(pms10);
    Serial.print(" ug/m3  (media de ");
    Serial.print(pmsQtdPeriodo);
    Serial.println(" pacotes)");
    Serial.print("Qualidade   : ");
    Serial.print(classificarPm25(pms25));
    Serial.println(" (pelo PM2.5)");
  } else {
    Serial.println("--- (sem dados do PMS7003)");
  }

  // Gases
  Serial.print("Gases MQ135 : ");
  if (mqOk) {
    Serial.print(mqVolts, 2);
    Serial.print(" V");
    if (millis() < AQUECIMENTO_MQ) Serial.print("  [aquecendo, valor ainda instavel]");
    Serial.println();
  } else {
    Serial.println("--- (leitura fora da faixa, confira o divisor)");
  }

  // Situacao de cada sensor
  Serial.print("Status      : BME280 ");
  Serial.print(bmeOk ? "ok" : "FALHA");
  Serial.print("  |  DHT22 ");
  Serial.print(dhtOk ? "ok" : "FALHA");
  Serial.print(" (");
  Serial.print(dhtFalhas);
  Serial.print(" falhas)  |  PMS7003 ");
  Serial.print(pmsOk ? "ok" : "FALHA");
  Serial.print(" (");
  Serial.print(pmsPacotesRuins);
  Serial.print(" pacotes ruins)  |  MQ135 ");
  Serial.println(mqOk ? "ok" : "FALHA");
}

// =====================================================================
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println();
  Serial.println("=== Estacao de Qualidade do Ar - Etapa 8 (integrada) ===");

  // BME280
  Wire.begin(PINO_SDA, PINO_SCL);
  if (bme.begin(0x76, &Wire) || bme.begin(0x77, &Wire)) {
    bmeOk = true;
    Serial.println("BME280  : iniciado");
  } else {
    Serial.println("BME280  : NAO encontrado (confira SDA/SCL/VIN/GND)");
  }

  // DHT22
  dht.begin();
  Serial.println("DHT22   : iniciado");

  // PMS7003
  Serial2.begin(9600, SERIAL_8N1, PMS_RX, PMS_TX);
  Serial.println("PMS7003 : porta serial iniciada");

  // MQ135
  analogReadResolution(12);
  analogSetPinAttenuation(PINO_MQ135, ADC_11db);
  Serial.println("MQ135   : entrada analogica iniciada");

  Serial.println("Primeiro resumo em 5 segundos...");
  ultimoRelatorio = millis();
}

void loop() {
  processarPms();
  processarDht();
  processarMq();

  if (millis() - ultimoRelatorio >= INTERVALO_RELATORIO) {
    ultimoRelatorio = millis();
    imprimirRelatorio();
  }
}
