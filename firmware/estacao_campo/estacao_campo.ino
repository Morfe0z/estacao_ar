// =====================================================================
// Estacao de Monitoramento da Qualidade do Ar - VERSAO DE CAMPO
// ESP32 DevKit 30 pinos | core esp32 2.0.17
//
// Tudo da versao final, mais:
//  - FILA de envio: se a internet ou o ThingSpeak falharem, as leituras
//    ficam guardadas (ate 4 h) e sobem todas juntas quando voltar
//  - Envio por HTTPS (conexao criptografada)
//  - RECUPERACAO automatica: 10 min sem conseguir enviar -> reinicia o
//    Wi-Fi do zero; 30 min -> salva a fila na memoria flash e reinicia
//    o ESP32 (a fila e recarregada ao ligar, nada se perde)
//  - WATCHDOG: se o programa travar por 60 s, o ESP32 reinicia sozinho
//  - Pagina nova, com mais detalhes (rede, fila, reinicios, motivos)
//
// Ligacoes (iguais as da versao final):
//   BME280 : VIN -> 3V3 | GND -> GND | SDA -> GPIO21 | SCL -> GPIO22
//   DHT22  : perna 1 -> 3V3 | perna 2 -> GPIO4 (+ 4,7k ate 3V3) | perna 4 -> GND
//   PMS7003: TX -> GPIO16 | RX -> GPIO17 | VCC -> VIN | GND -> GND
//   MQ135  : VCC -> VIN | GND -> GND | AO -> divisor 4,7k/4,7k -> GPIO34
//
// Bibliotecas: "Adafruit BME280 Library" e "DHT sensor library" (Adafruit)
// Placa: "ESP32 Dev Module", Partition Scheme: "Default 4MB with spiffs"
// =====================================================================

#include <Wire.h>
#include <Adafruit_BME280.h>
#include <DHT.h>
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <LittleFS.h>
#include <esp_task_wdt.h>
#include <esp_arduino_version.h>
#include <esp_system.h>
#include <esp_timer.h>
#include <time.h>
#include <sys/time.h>

// =====================================================================
// CONFIGURACAO
// =====================================================================
// Nomes e senhas ficam no arquivo secrets.h, na mesma pasta deste .ino.
// Na primeira vez: copie secrets.exemplo.h para secrets.h e preencha.
// O secrets.h nunca vai para o GitHub (esta no .gitignore).
#include "secrets.h"
// =====================================================================

// ---------------- Pinos ----------------
const int PINO_SDA   = 21;
const int PINO_SCL   = 22;
const int PINO_DHT   = 4;
const int PMS_RX     = 16;
const int PMS_TX     = 17;
const int PINO_MQ135 = 34;

// ---------------- Tempos ----------------
const unsigned long INTERVALO_RELATORIO  = 5000;     // atualiza leituras a cada 5 s
const unsigned long INTERVALO_DHT        = 2500;
const unsigned long INTERVALO_MQ         = 100;
const unsigned long VALIDADE_DHT         = 10000;
const unsigned long VALIDADE_PMS         = 5000;
const unsigned long AQUECIMENTO_MQ       = 180000;
const unsigned long INTERVALO_HISTORICO  = 30000;    // um ponto no grafico a cada 30 s
const unsigned long INTERVALO_THINGSPEAK = 20000;    // uma leitura entra na fila a cada 20 s
const unsigned long INTERVALO_PEDIDO     = 16000;    // ThingSpeak: minimo 15 s entre pedidos
const unsigned long INTERVALO_RECONEXAO  = 30000;
const unsigned long FALHA_REINICIA_WIFI  = 600000;   // 10 min sem enviar -> reinicia o Wi-Fi
const unsigned long FALHA_REINICIA_ESP   = 1800000;  // 30 min sem enviar -> salva a fila e reinicia
const int WATCHDOG_SEG                   = 60;
const float FATOR_DIVISOR_MQ             = 2.0;
const float UMIDADE_ALTA                 = 85.0;

// ---------------- Objetos ----------------
Adafruit_BME280 bme;
DHT dht(PINO_DHT, DHT22);
WebServer servidor(80);
WiFiClientSecure clienteTls;

// ---------------- Leituras atuais ----------------
struct Leituras {
  float tBme = NAN, uBme = NAN, pressao = NAN;
  float tDht = NAN, uDht = NAN;
  float mqVolts = NAN;
  int pm1 = -1, pm25 = -1, pm10 = -1;
  bool bmeOk = false, dhtOk = false, pmsOk = false, mqOk = false;
  bool mqAquecendo = true;
};
Leituras atual;

// ---------------- Estado interno ----------------
bool bmeIniciado = false;

float dhtTemp = NAN, dhtUmid = NAN;
unsigned long dhtUltimaLeitura = 0, dhtUltimaTentativa = 0, dhtFalhas = 0;

uint8_t pmsBuf[32];
int pmsIdx = 0;
unsigned long pmsUltimoPacote = 0, pmsPacotesOk = 0, pmsPacotesRuins = 0;
uint32_t pmsSoma1 = 0, pmsSoma25 = 0, pmsSoma10 = 0, pmsQtd = 0;
bool pmsTemDado = false;
int pmsUlt1 = 0, pmsUlt25 = 0, pmsUlt10 = 0;

uint32_t mqSomaMv = 0, mqQtd = 0;
unsigned long mqUltimaAmostra = 0;

unsigned long ultimoRelatorio = 0, numeroLeitura = 0;
unsigned long ultimoHistorico = 0, ultimaGuardaEnvio = 0, ultimoPedido = 0, ultimaReconexao = 0;
unsigned long envioOk = 0, envioFalhas = 0, reiniciosWifi = 0;
unsigned long ultimoOk = 0;              // ultima vez que a fila estava em dia ou um envio deu certo
unsigned long ultimoEnvioOkEm = 0;       // millis do ultimo envio aceito (0 = nenhum)
unsigned long ultimoReinicioWifi = 0;
int ultimoCodigo = 0;
String ultimoErroEnvio = "";
bool fsOk = false;

// Contadores que sobrevivem a um reinicio por software
RTC_NOINIT_ATTR uint32_t rtcMarca;
RTC_NOINIT_ATTR uint32_t rtcReiniciosAuto;
const uint32_t MARCA_RTC = 0xEA5A2026;
const char* motivoReinicio = "";

// ---------------- Historico (para os graficos) ----------------
const int TAM_HISTORICO = 240;   // 240 pontos x 30 s = 2 horas
struct Ponto {
  uint32_t seg;
  float v[8];                    // mesma ordem dos campos do ThingSpeak
};
Ponto historico[TAM_HISTORICO];
int histInicio = 0, histQtd = 0;

// ---------------- Fila de envio ----------------
const int TAM_FILA = 720;        // 720 x 20 s = 4 horas
const int LOTE_MAX = 100;        // leituras por envio em lote
struct Registro {
  int64_t ms;                    // momento da leitura (relogio interno, em ms)
  float v[8];                    // NAN = sensor sem dado
};
Registro fila[TAM_FILA];
int filaInicio = 0, filaQtd = 0;
unsigned long filaDescartados = 0;
const int CASAS_CAMPO[8] = {1, 1, 1, 0, 0, 0, 3, 1};
const char* ARQUIVO_FILA = "/fila.bin";
const uint32_t MARCA_ARQUIVO = 0x46494C41;

int64_t agoraMs() { return esp_timer_get_time() / 1000; }

// String(valor, casas) do ESP32 completa com ESPACOS a esquerda numeros curtos
// (ex.: 3 vira " 3"). Um espaco no endereco faz o ThingSpeak responder
// "400 Bad Request" - foi o que parou o envio quando o PM caiu abaixo de 10.
String numeroTexto(float v, int casas) {
  String s = String(v, casas);
  s.trim();
  return s;
}
Registro &filaItem(int i) { return fila[(filaInicio + i) % TAM_FILA]; }

void filaAdicionar(const Registro &r) {
  if (filaQtd == TAM_FILA) {     // fila cheia: perde a leitura mais antiga
    filaInicio = (filaInicio + 1) % TAM_FILA;
    filaQtd--;
    filaDescartados++;
  }
  filaItem(filaQtd) = r;
  filaQtd++;
}
void filaRemoverInicio(int n) { filaInicio = (filaInicio + n) % TAM_FILA; filaQtd -= n; }
void filaRemoverFim() { filaQtd--; }

bool thingspeakConfigurado() {
  return strlen(WIFI_NOME) > 0 && strlen(THINGSPEAK_CHAVE) > 0;
}

// Valores atuais na ordem dos campos 1..8 do ThingSpeak
void valoresAtuais(float v[8]) {
  v[0] = atual.bmeOk ? atual.tBme : NAN;
  v[1] = atual.bmeOk ? atual.uBme : NAN;
  v[2] = atual.bmeOk ? atual.pressao : NAN;
  v[3] = atual.pmsOk ? atual.pm1 : NAN;
  v[4] = atual.pmsOk ? atual.pm25 : NAN;
  v[5] = atual.pmsOk ? atual.pm10 : NAN;
  v[6] = atual.mqOk ? atual.mqVolts : NAN;
  v[7] = atual.dhtOk ? atual.uDht : NAN;
}

void guardarHistorico() {
  int pos = (histInicio + histQtd) % TAM_HISTORICO;
  historico[pos].seg = millis() / 1000;
  valoresAtuais(historico[pos].v);
  if (histQtd < TAM_HISTORICO) histQtd++;
  else histInicio = (histInicio + 1) % TAM_HISTORICO;
}

void guardarParaEnvio() {
  if (!thingspeakConfigurado()) return;
  if (millis() - ultimaGuardaEnvio < INTERVALO_THINGSPEAK) return;
  ultimaGuardaEnvio = millis();
  Registro r;
  r.ms = agoraMs();
  valoresAtuais(r.v);
  bool algum = false;
  for (int i = 0; i < 8; i++) if (!isnan(r.v[i])) algum = true;
  if (algum) filaAdicionar(r);
}

// ---------------- Fila na memoria flash (antes de reiniciar) ----------------
void salvarFila() {
  if (!fsOk || filaQtd == 0) return;
  File f = LittleFS.open(ARQUIVO_FILA, "w");
  if (!f) return;
  int64_t salvoEm = agoraMs();
  int32_t qtd = filaQtd;
  f.write((const uint8_t *)&MARCA_ARQUIVO, sizeof(MARCA_ARQUIVO));
  f.write((const uint8_t *)&qtd, sizeof(qtd));
  f.write((const uint8_t *)&salvoEm, sizeof(salvoEm));
  for (int i = 0; i < filaQtd; i++) f.write((const uint8_t *)&filaItem(i), sizeof(Registro));
  f.close();
  Serial.print("Fila: ");
  Serial.print(filaQtd);
  Serial.println(" leituras salvas na memoria flash");
}

void carregarFila() {
  if (!fsOk || !LittleFS.exists(ARQUIVO_FILA)) return;
  File f = LittleFS.open(ARQUIVO_FILA, "r");
  uint32_t marca = 0;
  int32_t qtd = 0;
  int64_t salvoEm = 0;
  bool ok = f && f.read((uint8_t *)&marca, sizeof(marca)) == sizeof(marca) && marca == MARCA_ARQUIVO
            && f.read((uint8_t *)&qtd, sizeof(qtd)) == sizeof(qtd) && qtd > 0 && qtd <= TAM_FILA
            && f.read((uint8_t *)&salvoEm, sizeof(salvoEm)) == sizeof(salvoEm);
  if (ok) {
    // o relogio interno recomeca do zero ao reiniciar: as leituras antigas ficam com tempo negativo
    int64_t base = agoraMs() - 5000;
    for (int i = 0; i < qtd; i++) {
      Registro r;
      if (f.read((uint8_t *)&r, sizeof(r)) != sizeof(r)) break;
      r.ms = base - (salvoEm - r.ms);
      filaAdicionar(r);
    }
    Serial.print("Fila: ");
    Serial.print(filaQtd);
    Serial.println(" leituras recuperadas da memoria flash");
  }
  if (f) f.close();
  LittleFS.remove(ARQUIVO_FILA);
}

// =====================================================================
// Leitura dos sensores
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

void processarMq() {
  if (millis() - mqUltimaAmostra < INTERVALO_MQ) return;
  mqUltimaAmostra = millis();
  mqSomaMv += analogReadMilliVolts(PINO_MQ135);
  mqQtd++;
}

const char* classificarPm25(int pm) {
  if (pm < 0)    return "---";
  if (pm <= 25)  return "BOA";
  if (pm <= 50)  return "MODERADA";
  if (pm <= 75)  return "RUIM";
  if (pm <= 125) return "MUITO RUIM";
  return "PESSIMA";
}

// Fecha o periodo de 5 s e atualiza "atual"
void atualizarLeituras() {
  // BME280
  atual.bmeOk = bmeIniciado;
  if (bmeIniciado) {
    atual.tBme = bme.readTemperature();
    atual.uBme = bme.readHumidity();
    atual.pressao = bme.readPressure() / 100.0F;
    if (isnan(atual.tBme) || isnan(atual.pressao) || atual.pressao < 300 || atual.pressao > 1100) {
      atual.bmeOk = false;
    }
  }

  // DHT22
  atual.dhtOk = dhtUltimaLeitura > 0 && (millis() - dhtUltimaLeitura < VALIDADE_DHT);
  atual.tDht = dhtTemp;
  atual.uDht = dhtUmid;

  // PMS7003 (media do periodo)
  if (pmsQtd > 0) {
    pmsUlt1  = pmsSoma1  / pmsQtd;
    pmsUlt25 = pmsSoma25 / pmsQtd;
    pmsUlt10 = pmsSoma10 / pmsQtd;
    pmsTemDado = true;
  }
  pmsSoma1 = pmsSoma25 = pmsSoma10 = pmsQtd = 0;
  atual.pmsOk = pmsTemDado && (millis() - pmsUltimoPacote < VALIDADE_PMS);
  atual.pm1 = pmsUlt1;
  atual.pm25 = pmsUlt25;
  atual.pm10 = pmsUlt10;

  // MQ135 (media do periodo)
  float mqMv = (mqQtd > 0) ? (float)mqSomaMv / mqQtd : NAN;
  mqSomaMv = mqQtd = 0;
  atual.mqOk = !isnan(mqMv) && mqMv > 20 && mqMv < 2800;
  atual.mqVolts = mqMv * FATOR_DIVISOR_MQ / 1000.0;
  atual.mqAquecendo = millis() < AQUECIMENTO_MQ;
}

bool umidadeAlta() {
  return atual.bmeOk && atual.uBme > UMIDADE_ALTA;
}

// =====================================================================
// Monitor Serial
// =====================================================================
void imprimirTempo(unsigned long s) {
  char texto[24];
  snprintf(texto, sizeof(texto), "%02lu:%02lu:%02lu", s / 3600, (s / 60) % 60, s % 60);
  Serial.print(texto);
}

void imprimirRelatorio() {
  numeroLeitura++;
  Serial.println();
  Serial.print("===== Leitura #");
  Serial.print(numeroLeitura);
  Serial.print("   (ligado ha ");
  imprimirTempo(millis() / 1000);
  Serial.println(") =====");

  Serial.print("Temperatura : ");
  if (atual.bmeOk) { Serial.print(atual.tBme, 1); Serial.print(" C (BME280)"); } else Serial.print("--- (BME280)");
  Serial.print("  |  ");
  if (atual.dhtOk) { Serial.print(atual.tDht, 1); Serial.println(" C (DHT22)"); } else Serial.println("--- (DHT22)");

  Serial.print("Umidade     : ");
  if (atual.bmeOk) { Serial.print(atual.uBme, 1); Serial.print(" % (BME280)"); } else Serial.print("--- (BME280)");
  Serial.print("  |  ");
  if (atual.dhtOk) { Serial.print(atual.uDht, 1); Serial.println(" % (DHT22)"); } else Serial.println("--- (DHT22)");

  Serial.print("Pressao     : ");
  if (atual.bmeOk) { Serial.print(atual.pressao, 1); Serial.println(" hPa"); } else Serial.println("---");

  Serial.print("Particulas  : ");
  if (atual.pmsOk) {
    Serial.print("PM1.0 = ");  Serial.print(atual.pm1);
    Serial.print("  |  PM2.5 = "); Serial.print(atual.pm25);
    Serial.print("  |  PM10 = ");  Serial.print(atual.pm10);
    Serial.println(" ug/m3");
    Serial.print("Qualidade   : ");
    Serial.print(classificarPm25(atual.pm25));
    Serial.print(" (pelo PM2.5)");
    if (umidadeAlta()) Serial.print("  [umidade alta: particulas podem estar superestimadas]");
    Serial.println();
  } else {
    Serial.println("--- (sem dados do PMS7003)");
  }

  Serial.print("Gases MQ135 : ");
  if (atual.mqOk) {
    Serial.print(atual.mqVolts, 2);
    Serial.print(" V");
    if (atual.mqAquecendo) Serial.print("  [aquecendo, valor ainda instavel]");
    Serial.println();
  } else {
    Serial.println("--- (leitura fora da faixa, confira o divisor)");
  }

  Serial.print("Status      : BME280 ");
  Serial.print(atual.bmeOk ? "ok" : "FALHA");
  Serial.print("  |  DHT22 ");
  Serial.print(atual.dhtOk ? "ok" : "FALHA");
  Serial.print("  |  PMS7003 ");
  Serial.print(atual.pmsOk ? "ok" : "FALHA");
  Serial.print("  |  MQ135 ");
  Serial.println(atual.mqOk ? "ok" : "FALHA");

  Serial.print("Rede        : pagina em http://");
  Serial.print(WiFi.softAPIP().toString());
  Serial.print("  |  internet: ");
  if (strlen(WIFI_NOME) == 0) Serial.println("nao configurada");
  else if (WiFi.status() == WL_CONNECTED) {
    Serial.print("conectada (IP ");
    Serial.print(WiFi.localIP().toString());
    Serial.print(", sinal ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm)");
  } else Serial.println("desconectada");

  if (thingspeakConfigurado()) {
    Serial.print("ThingSpeak  : ");
    Serial.print(envioOk);
    Serial.print(" envios ok, ");
    Serial.print(envioFalhas);
    Serial.print(" falhas  |  fila: ");
    Serial.print(filaQtd);
    Serial.print(" leituras esperando");
    if (ultimoErroEnvio.length() > 0) {
      Serial.print("  |  ultimo erro: ");
      Serial.print(ultimoErroEnvio);
    }
    Serial.println();
  } else {
    Serial.println("ThingSpeak  : nao configurado");
  }
}

// =====================================================================
// Relogio (necessario para mandar leituras atrasadas com a hora certa)
// =====================================================================
bool relogioOk() {
  return time(nullptr) > 1700000000;
}

// Acerta o relogio pelo cabecalho "Date" da resposta do servidor,
// caso o NTP esteja bloqueado na rede. Ex.: "Thu, 09 Oct 2026 14:05:00 GMT"
void acertarRelogio(const String &d) {
  if (relogioOk() || d.length() < 25) return;
  int dia, ano, h, m, s;
  char mes[4] = {0};
  if (sscanf(d.c_str(), "%*3s, %d %3s %d %d:%d:%d", &dia, mes, &ano, &h, &m, &s) != 6) return;
  const char *nomes = "JanFebMarAprMayJunJulAugSepOctNovDec";
  const char *p = strstr(nomes, mes);
  if (!p || (p - nomes) % 3 != 0) return;
  int mesN = (p - nomes) / 3 + 1;
  int y = ano - (mesN <= 2);
  int era = y / 400;
  unsigned yoe = y - era * 400;
  unsigned doy = (153 * (mesN + (mesN > 2 ? -3 : 9)) + 2) / 5 + dia - 1;
  unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
  long dias = era * 146097L + (long)doe - 719468L;
  struct timeval tv = {(time_t)(dias * 86400L + h * 3600L + m * 60L + s), 0};
  settimeofday(&tv, nullptr);
  Serial.println("Relogio: acertado pela resposta do ThingSpeak");
}

// =====================================================================
// ThingSpeak
// Campos: 1 temp BME | 2 umid BME | 3 pressao | 4 PM1.0 | 5 PM2.5
//         6 PM10 | 7 MQ135 (V) | 8 umid DHT22
// =====================================================================
// Remove aspas, barras e quebras de linha (o texto vai dentro do JSON da pagina)
String textoSeguro(String t) {
  t.replace("\\", "/");
  t.replace("\"", "'");
  t.replace("\r", " ");
  t.replace("\n", " ");
  return t;
}

bool registrarResultado(int codigo, const String &resposta, bool aceito) {
  ultimoCodigo = codigo;
  if (aceito) {
    envioOk++;
    ultimoErroEnvio = "";
    ultimoEnvioOkEm = millis();
    ultimoOk = millis();
    return true;
  }
  envioFalhas++;
  if (codigo <= 0)      ultimoErroEnvio = "sem resposta do servidor (codigo " + String(codigo) + ")";
  else if (codigo == 200) ultimoErroEnvio = "ThingSpeak recusou (confira a chave e o numero do canal)";
  else                  ultimoErroEnvio = "servidor respondeu codigo " + String(codigo) + ": " + textoSeguro(resposta.substring(0, 80));
  Serial.print("ThingSpeak: falha no envio - ");
  Serial.println(ultimoErroEnvio);
  return false;
}

// Envia uma leitura pelo jeito simples (a hora e a do momento do envio)
bool enviarUma(const Registro &r) {
  String url = "https://api.thingspeak.com/update?api_key=";
  url += THINGSPEAK_CHAVE;
  for (int i = 0; i < 8; i++) {
    if (isnan(r.v[i])) continue;
    url += "&field";
    url += (i + 1);
    url += "=";
    url += numeroTexto(r.v[i], CASAS_CAMPO[i]);
  }
  HTTPClient http;
  http.setConnectTimeout(8000);
  http.setTimeout(8000);
  if (!http.begin(clienteTls, url)) return registrarResultado(-100, "", false);
  const char *cabecalhos[] = {"Date"};
  http.collectHeaders(cabecalhos, 1);
  int codigo = http.GET();
  String resposta = (codigo > 0) ? http.getString() : "";
  if (codigo > 0) acertarRelogio(http.header("Date"));
  http.end();
  // O ThingSpeak responde com o numero da entrada (> 0) quando aceita
  return registrarResultado(codigo, resposta, codigo == 200 && resposta.toInt() > 0);
}

// Envia as "qtd" leituras mais antigas da fila de uma vez, cada uma com a sua hora
bool enviarLote(int qtd) {
  time_t agoraS = time(nullptr);
  int64_t agoraM = agoraMs();
  String corpo;
  corpo.reserve(qtd * 140 + 80);
  corpo = "{\"write_api_key\":\"";
  corpo += THINGSPEAK_CHAVE;
  corpo += "\",\"updates\":[";
  for (int i = 0; i < qtd; i++) {
    const Registro &r = filaItem(i);
    time_t t = agoraS - (time_t)((agoraM - r.ms) / 1000);
    struct tm tmv;
    gmtime_r(&t, &tmv);
    char data[24];
    strftime(data, sizeof(data), "%Y-%m-%dT%H:%M:%SZ", &tmv);
    if (i > 0) corpo += ",";
    corpo += "{\"created_at\":\"";
    corpo += data;
    corpo += "\"";
    for (int k = 0; k < 8; k++) {
      if (isnan(r.v[k])) continue;
      corpo += ",\"field";
      corpo += (k + 1);
      corpo += "\":";
      corpo += numeroTexto(r.v[k], CASAS_CAMPO[k]);
    }
    corpo += "}";
  }
  corpo += "]}";

  String url = "https://api.thingspeak.com/channels/";
  url += THINGSPEAK_CANAL;
  url += "/bulk_update.json";
  HTTPClient http;
  http.setConnectTimeout(8000);
  http.setTimeout(15000);
  if (!http.begin(clienteTls, url)) return registrarResultado(-100, "", false);
  http.addHeader("Content-Type", "application/json");
  int codigo = http.POST(corpo);
  String resposta = (codigo > 0) ? http.getString() : "";
  http.end();
  bool aceito = (codigo == 200 || codigo == 202) && resposta.indexOf("true") >= 0;
  if (aceito) {
    Serial.print("ThingSpeak: lote de ");
    Serial.print(qtd);
    Serial.println(" leituras atrasadas enviado");
  }
  return registrarResultado(codigo, resposta, aceito);
}

void enviarThingSpeak() {
  if (!thingspeakConfigurado() || filaQtd == 0) return;
  if (WiFi.status() != WL_CONNECTED) return;
  if (millis() - ultimoPedido < INTERVALO_PEDIDO) return;
  ultimoPedido = millis();
  esp_task_wdt_reset();

  // Em dia (1 leitura recente) ou sem relogio: manda a mais nova do jeito simples.
  // Atrasado e com relogio: manda as mais antigas em lote, com a hora de cada uma.
  bool lotePossivel = relogioOk() && strlen(THINGSPEAK_CANAL) > 0;
  bool emDia = filaQtd == 1 && agoraMs() - filaItem(0).ms < 60000;
  if (emDia || !lotePossivel) {
    if (enviarUma(filaItem(filaQtd - 1))) filaRemoverFim();
    return;
  }
  int qtd = min(filaQtd, LOTE_MAX);
  if (enviarLote(qtd)) filaRemoverInicio(qtd);
  // se falhar, as leituras continuam na fila e o lote e tentado de novo no proximo pedido
}

// =====================================================================
// Wi-Fi e recuperacao automatica
// =====================================================================
void ligarWifi() {
  if (strlen(WIFI_NOME) > 0) WiFi.mode(WIFI_AP_STA);
  else WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_NOME, AP_SENHA);
  if (strlen(WIFI_NOME) > 0) {
    WiFi.setAutoReconnect(true);
    WiFi.begin(WIFI_NOME, WIFI_SENHA);
    ultimaReconexao = millis();
  }
}

void manterWifi() {
  if (strlen(WIFI_NOME) == 0) return;
  if (WiFi.status() == WL_CONNECTED) return;
  if (millis() - ultimaReconexao < INTERVALO_RECONEXAO) return;
  ultimaReconexao = millis();
  Serial.print("Wi-Fi: tentando conectar em \"");
  Serial.print(WIFI_NOME);
  Serial.println("\"...");
  WiFi.disconnect();
  WiFi.begin(WIFI_NOME, WIFI_SENHA);
}

void verificarRecuperacao() {
  if (!thingspeakConfigurado()) return;
  if (filaQtd == 0) ultimoOk = millis();   // nada esperando: tudo em dia
  unsigned long semEnviar = millis() - ultimoOk;

  if (semEnviar >= FALHA_REINICIA_ESP) {
    Serial.println("Recuperacao: 30 min sem conseguir enviar, reiniciando o ESP32");
    salvarFila();
    rtcReiniciosAuto++;
    delay(200);
    ESP.restart();
  }
  if (semEnviar >= FALHA_REINICIA_WIFI && millis() - ultimoReinicioWifi >= FALHA_REINICIA_WIFI) {
    Serial.println("Recuperacao: 10 min sem conseguir enviar, reiniciando o Wi-Fi do zero");
    ultimoReinicioWifi = millis();
    reiniciosWifi++;
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    delay(500);
    ligarWifi();
  }
}

const char* descreverReinicio(esp_reset_reason_t r) {
  switch (r) {
    case ESP_RST_POWERON:  return "ligada na energia ou botão EN";
    case ESP_RST_EXT:      return "botão EN";
    case ESP_RST_SW:       return "reinício automático (sem conseguir enviar)";
    case ESP_RST_PANIC:    return "erro no programa";
    case ESP_RST_INT_WDT:
    case ESP_RST_TASK_WDT:
    case ESP_RST_WDT:      return "travou e o watchdog reiniciou sozinho";
    case ESP_RST_BROWNOUT: return "queda de tensão na alimentação";
    case ESP_RST_DEEPSLEEP:return "saiu do modo de economia de energia";
    default:               return "outro motivo";
  }
}

// =====================================================================
// Pagina web
// =====================================================================
String numeroJson(float v, int casas) {
  if (isnan(v)) return "null";
  return numeroTexto(v, casas);
}

void responderDados() {
  String j;
  j.reserve(1200);
  j = "{";
  j += "\"tBme\":"    + (atual.bmeOk ? numeroJson(atual.tBme, 1) : String("null")) + ",";
  j += "\"uBme\":"    + (atual.bmeOk ? numeroJson(atual.uBme, 1) : String("null")) + ",";
  j += "\"pressao\":" + (atual.bmeOk ? numeroJson(atual.pressao, 1) : String("null")) + ",";
  j += "\"tDht\":"    + (atual.dhtOk ? numeroJson(atual.tDht, 1) : String("null")) + ",";
  j += "\"uDht\":"    + (atual.dhtOk ? numeroJson(atual.uDht, 1) : String("null")) + ",";
  j += "\"pm1\":"     + (atual.pmsOk ? String(atual.pm1)  : String("null")) + ",";
  j += "\"pm25\":"    + (atual.pmsOk ? String(atual.pm25) : String("null")) + ",";
  j += "\"pm10\":"    + (atual.pmsOk ? String(atual.pm10) : String("null")) + ",";
  j += "\"qualidade\":\"" + String(atual.pmsOk ? classificarPm25(atual.pm25) : "---") + "\",";
  j += "\"mq\":"      + (atual.mqOk ? numeroJson(atual.mqVolts, 2) : String("null")) + ",";
  j += "\"mqAquecendo\":" + String(atual.mqAquecendo ? "true" : "false") + ",";
  j += "\"umidadeAlta\":" + String(umidadeAlta() ? "true" : "false") + ",";
  j += "\"ok\":{\"bme\":" + String(atual.bmeOk ? "true" : "false");
  j += ",\"dht\":" + String(atual.dhtOk ? "true" : "false");
  j += ",\"pms\":" + String(atual.pmsOk ? "true" : "false");
  j += ",\"mq\":"  + String(atual.mqOk ? "true" : "false") + "},";
  j += "\"ligadoSeg\":" + String(millis() / 1000) + ",";
  j += "\"reinicio\":\"" + String(motivoReinicio) + "\",";
  j += "\"reiniciosAuto\":" + String(rtcReiniciosAuto) + ",";
  j += "\"reiniciosWifi\":" + String(reiniciosWifi) + ",";
  j += "\"pmsRuins\":" + String(pmsPacotesRuins) + ",";
  j += "\"memoriaKb\":" + String(ESP.getFreeHeap() / 1024) + ",";
  j += "\"wifi\":{\"configurada\":" + String(strlen(WIFI_NOME) > 0 ? "true" : "false");
  j += ",\"conectada\":" + String(WiFi.status() == WL_CONNECTED ? "true" : "false");
  j += ",\"nome\":\"" + textoSeguro(String(WIFI_NOME)) + "\"";
  j += ",\"ip\":\"" + (WiFi.status() == WL_CONNECTED ? WiFi.localIP().toString() : String("")) + "\"";
  j += ",\"sinal\":" + (WiFi.status() == WL_CONNECTED ? String(WiFi.RSSI()) : String("null")) + "},";
  j += "\"ts\":{\"configurado\":" + String(thingspeakConfigurado() ? "true" : "false");
  j += ",\"ok\":" + String(envioOk);
  j += ",\"falhas\":" + String(envioFalhas);
  j += ",\"fila\":" + String(filaQtd);
  j += ",\"descartados\":" + String(filaDescartados);
  j += ",\"ultimoOkSeg\":" + (ultimoEnvioOkEm ? String((millis() - ultimoEnvioOkEm) / 1000) : String("null"));
  j += ",\"relogio\":" + String(relogioOk() ? "true" : "false");
  j += ",\"erro\":\"" + ultimoErroEnvio + "\"}";
  j += "}";
  servidor.send(200, "application/json", j);
}

void responderHistorico() {
  String j;
  j.reserve(histQtd * 60 + 40);
  j = "{\"agora\":" + String(millis() / 1000) + ",\"pontos\":[";
  for (int i = 0; i < histQtd; i++) {
    const Ponto &p = historico[(histInicio + i) % TAM_HISTORICO];
    if (i > 0) j += ",";
    j += "[" + String(p.seg);
    for (int k = 0; k < 8; k++) j += "," + numeroJson(p.v[k], CASAS_CAMPO[k]);
    j += "]";
  }
  j += "]}";
  servidor.send(200, "application/json", j);
}

void responderCsv() {
  String c = "segundos_ligado,temperatura_C,umidade_pct,pressao_hPa,pm1_ugm3,pm2_5_ugm3,pm10_ugm3,mq135_V,umidade_dht22_pct\n";
  for (int i = 0; i < histQtd; i++) {
    const Ponto &p = historico[(histInicio + i) % TAM_HISTORICO];
    c += String(p.seg);
    for (int k = 0; k < 8; k++) c += "," + (isnan(p.v[k]) ? String("") : numeroTexto(p.v[k], CASAS_CAMPO[k]));
    c += "\n";
  }
  servidor.sendHeader("Content-Disposition", "attachment; filename=estacao.csv");
  servidor.send(200, "text/csv", c);
}

const char PAGINA[] PROGMEM = R"HTML(<!DOCTYPE html>
<html lang="pt-BR"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Estação de Qualidade do Ar</title>
<style>
/* Painel de instrumento: resumo no topo, mostradores em grade, graficos empilhados com cursor */
:root{
  --bg:#eef2f3; --surface:#ffffff; --ink:#15232b; --muted:#5d6f78; --line:#d9e1e4;
  --accent:#1d6a7a; --accent-soft:#d7ecef;
  --ok:#2e7d32; --mod:#c99a06; --ruim:#e46a12; --mruim:#c62828; --pess:#6a1b9a;
  --pm1:#7aa7b3; --pm25:#c62828; --pm10:#8d6e63; --temp:#e46a12; --umid:#1565c0; --umid2:#7fa6d9; --press:#4e5d9c; --gas:#7b3fa0;
  --font-ui:system-ui,-apple-system,"Segoe UI",Roboto,sans-serif;
  --font-num:ui-monospace,"SFMono-Regular",Menlo,Consolas,"Roboto Mono",monospace;
}
@media (prefers-color-scheme: dark){
  :root{
    --bg:#0f181c; --surface:#172429; --ink:#e3ecef; --muted:#93a6ad; --line:#2a3a40;
    --accent:#5fb6c6; --accent-soft:#1d3a41;
    --pm1:#8fbcc7; --pm25:#ff6b6b; --pm10:#c2a093; --temp:#ff9a4d; --umid:#64a8ff; --umid2:#a5c4ee; --press:#9aa8ec; --gas:#c18be0;
    color-scheme:dark;
  }
}
*{box-sizing:border-box}
body{margin:0;background:var(--bg);color:var(--ink);font-family:var(--font-ui);font-size:14px;line-height:1.45}
.pagina{max-width:1100px;margin:0 auto;padding-inline:16px;padding-block:16px 28px;display:grid;gap:14px}
.num{font-family:var(--font-num);font-variant-numeric:tabular-nums}

/* Cabecalho */
.topo{display:flex;flex-wrap:wrap;gap:10px 16px;align-items:center;justify-content:space-between}
.topo h1{margin:0;font-size:1.25rem;font-weight:700;letter-spacing:-.01em;text-wrap:balance}
.topo .sub{color:var(--muted);font-size:.85rem}
.estado{display:inline-flex;align-items:center;gap:6px;padding:3px 10px;border-radius:999px;font-size:.8rem;font-weight:600;background:var(--accent-soft);color:var(--ink)}
.estado::before{content:"";width:8px;height:8px;border-radius:50%;background:var(--muted)}
.estado.online::before{background:var(--ok)}
.estado.offline::before{background:var(--mruim)}
.periodos{display:inline-flex;border:1px solid var(--line);border-radius:8px;overflow:hidden;background:var(--surface)}
.periodos button{font:inherit;font-size:.85rem;border:0;background:transparent;color:var(--ink);padding:6px 12px;cursor:pointer}
.periodos button+button{border-left:1px solid var(--line)}
.periodos button[aria-pressed="true"]{background:var(--accent);color:var(--surface);font-weight:600}
.periodos button:focus-visible{outline:2px solid var(--accent);outline-offset:-2px}

/* Resumo da qualidade do ar */
.resumo{background:var(--surface);border:1px solid var(--line);border-radius:12px;padding:16px;display:grid;grid-template-columns:minmax(0,1fr) minmax(0,1.4fr);gap:18px;align-items:center}
.resumo .rotulo{font-size:.75rem;text-transform:uppercase;letter-spacing:.06em;color:var(--muted)}
.resumo .grande{display:flex;align-items:baseline;gap:8px;margin-top:2px}
.resumo .grande .num{font-size:2.8rem;font-weight:500;line-height:1}
.resumo .grande .un{color:var(--muted)}
.classe{display:inline-block;margin-top:8px;padding:4px 12px;border-radius:6px;font-weight:700;color:#fff;background:var(--muted)}
.escala{position:relative;margin-top:6px}
.escala .faixas{display:flex;height:12px;border-radius:6px;overflow:hidden}
.escala .faixas span{display:block;height:100%}
.escala .marcador{position:absolute;top:-5px;width:3px;height:22px;background:var(--ink);border-radius:2px;transform:translateX(-1px);transition:left .4s}
.escala .marcas{display:flex;justify-content:space-between;font-size:.7rem;color:var(--muted);margin-top:4px}
.aviso{margin-top:10px;font-size:.85rem;color:var(--muted)}
.aviso strong{color:var(--ink)}

/* Mostradores */
.grade{display:grid;grid-template-columns:repeat(4,minmax(0,1fr));gap:10px}
@media (max-width:860px){.grade{grid-template-columns:repeat(2,minmax(0,1fr))}}
.mostrador{background:var(--surface);border:1px solid var(--line);border-radius:10px;padding:12px 14px;display:grid;gap:4px;min-width:0}
.mostrador .nome{font-size:.8rem;color:var(--muted)}
.mostrador .valor{display:flex;align-items:baseline;gap:6px}
.mostrador .valor .num{font-size:1.6rem;font-weight:500}
.mostrador .valor .un{color:var(--muted);font-size:.85rem}
.tend{font-size:.8rem;font-family:var(--font-num)}
.tend.sobe{color:var(--ruim)} .tend.desce{color:var(--accent)} .tend.igual{color:var(--muted)}
.mostrador .faixa{font-size:.75rem;color:var(--muted);font-family:var(--font-num);font-variant-numeric:tabular-nums}

/* Graficos */
.bloco{background:var(--surface);border:1px solid var(--line);border-radius:12px;padding:12px 14px 10px;min-width:0}
.bloco h2{margin:0 0 6px;font-size:.95rem;font-weight:600;display:flex;flex-wrap:wrap;gap:4px 14px;align-items:baseline}
.legenda{display:inline-flex;flex-wrap:wrap;gap:4px 12px;font-size:.78rem;font-weight:400;color:var(--muted)}
.legenda i{display:inline-block;width:12px;height:3px;border-radius:2px;margin-right:5px;vertical-align:middle}
.grafico{position:relative}
.grafico canvas{display:block;width:100%;height:190px;touch-action:pan-y}
.dica{position:absolute;pointer-events:none;background:var(--ink);color:var(--bg);font-size:.75rem;padding:6px 8px;border-radius:6px;white-space:nowrap;font-family:var(--font-num);transform:translate(-50%,-100%);top:0}
.dupla{display:grid;grid-template-columns:repeat(auto-fit,minmax(min(100%,420px),1fr));gap:14px}

/* Saude da estacao */
.saude{display:grid;grid-template-columns:repeat(auto-fill,minmax(160px,1fr));gap:10px 18px}
.saude div{display:grid;gap:2px}
.saude .k{font-size:.75rem;color:var(--muted);text-transform:uppercase;letter-spacing:.05em}
.saude .v{font-family:var(--font-num);font-size:1rem}
.rodape{font-size:.8rem;color:var(--muted);display:flex;flex-wrap:wrap;gap:6px 16px;justify-content:space-between}
.rodape a{color:var(--accent)}
.erro{background:var(--surface);border:1px solid var(--mruim);border-left-width:4px;border-radius:8px;padding:10px 12px}
@media (max-width:640px){
  .resumo{grid-template-columns:minmax(0,1fr)}
  .resumo .grande .num{font-size:2.3rem}
}
@media (prefers-reduced-motion:reduce){.escala .marcador{transition:none}}
.chips{display:flex;flex-wrap:wrap;gap:6px}
.chip{display:inline-flex;align-items:center;gap:6px;padding:3px 10px;border-radius:999px;border:1px solid var(--line);font-size:.8rem}
.chip::before{content:"";width:8px;height:8px;border-radius:50%;background:var(--ok)}
.chip.falha{border-color:var(--mruim);font-weight:600}
.chip.falha::before{background:var(--mruim)}
.saude .v.alerta{color:var(--mruim)}
</style></head><body>
<div class="pagina">
  <header class="topo">
    <div>
      <h1>Estação de Qualidade do Ar</h1>
      <div class="sub"><span class="estado" id="estado">conectando</span> <span id="ultimo">--</span></div>
    </div>
    <div class="chips" id="chips"></div>
  </header>
  <div class="erro" id="erro" hidden></div>
  <section class="resumo" aria-label="Qualidade do ar agora">
    <div>
      <div class="rotulo">PM2.5 agora, média de 5 s</div>
      <div class="grande"><span class="num" id="pmAgora">--</span><span class="un">µg/m³</span></div>
      <span class="classe" id="classe">---</span>
    </div>
    <div>
      <div class="rotulo">Escala usada pela estação, PM2.5</div>
      <div class="escala">
        <div class="faixas" id="faixas"></div>
        <div class="marcador" id="marcador" style="left:0%"></div>
        <div class="marcas num"><span>0</span><span>25</span><span>50</span><span>75</span><span>125</span><span>150+</span></div>
      </div>
      <div class="aviso" id="aviso">--</div>
    </div>
  </section>
  <section class="grade" id="mostradores" aria-label="Leituras atuais"></section>
  <section class="bloco">
    <h2>Partículas, últimas 2 horas <span class="legenda"><span><i style="background:var(--pm1)"></i>PM1.0</span><span><i style="background:var(--pm25)"></i>PM2.5</span><span><i style="background:var(--pm10)"></i>PM10</span><span>tracejado: limite "BOA" (25)</span></span></h2>
    <div class="grafico"><canvas id="gPm"></canvas></div>
  </section>
  <div class="dupla">
    <section class="bloco">
      <h2>Temperatura <span class="legenda"><span><i style="background:var(--temp)"></i>BME280 (°C)</span></span></h2>
      <div class="grafico"><canvas id="gTemp"></canvas></div>
    </section>
    <section class="bloco">
      <h2>Umidade <span class="legenda"><span><i style="background:var(--umid)"></i>BME280</span><span><i style="background:var(--umid2)"></i>DHT22</span><span>(%)</span></span></h2>
      <div class="grafico"><canvas id="gUmid"></canvas></div>
    </section>
    <section class="bloco">
      <h2>Pressão <span class="legenda"><span><i style="background:var(--press)"></i>hPa</span></span></h2>
      <div class="grafico"><canvas id="gPress"></canvas></div>
    </section>
    <section class="bloco">
      <h2>Gases (MQ135) <span class="legenda"><span><i style="background:var(--gas)"></i>tensão (V), mais alto = mais gases</span></span></h2>
      <div class="grafico"><canvas id="gGas"></canvas></div>
    </section>
  </div>
  <section class="bloco">
    <h2>Situação da estação</h2>
    <div class="saude">
      <div><span class="k">Internet</span><span class="v" id="sNet">--</span></div>
      <div><span class="k">Sinal do Wi-Fi</span><span class="v" id="sSinal">--</span></div>
      <div><span class="k">ThingSpeak</span><span class="v" id="sTs">--</span></div>
      <div><span class="k">Último envio aceito</span><span class="v" id="sUltimo">--</span></div>
      <div><span class="k">Fila esperando envio</span><span class="v" id="sFila">--</span></div>
      <div><span class="k">Último erro de envio</span><span class="v" id="sErro">--</span></div>
      <div><span class="k">Ligada há</span><span class="v" id="sLigada">--</span></div>
      <div><span class="k">Motivo do último início</span><span class="v" id="sMotivo">--</span></div>
      <div><span class="k">Reinícios automáticos</span><span class="v" id="sReinicios">--</span></div>
      <div><span class="k">Memória livre</span><span class="v" id="sMem">--</span></div>
    </div>
  </section>
  <footer class="rodape">
    <span id="atualizado">--</span>
    <a href="/csv">Baixar as últimas 2 horas (CSV)</a>
  </footer>
</div>
<script>
const PERIODOS = {"2h": {bucket: 30, seg: 7200}};
const CLASSES = [
  {ate: 25,       nome: "BOA",        cor: "--ok"},
  {ate: 50,       nome: "MODERADA",   cor: "--mod"},
  {ate: 75,       nome: "RUIM",       cor: "--ruim"},
  {ate: 125,      nome: "MUITO RUIM", cor: "--mruim"},
  {ate: Infinity, nome: "PÉSSIMA",    cor: "--pess"}
];


const MOSTRADORES = [
  {id: "pm25", campo: 5, nome: "PM2.5",            un: "µg/m³", casas: 0, chave: "pm25"},
  {id: "pm10", campo: 6, nome: "PM10",             un: "µg/m³", casas: 0, chave: "pm10"},
  {id: "pm1",  campo: 4, nome: "PM1.0",            un: "µg/m³", casas: 0, chave: "pm1"},
  {id: "temp", campo: 1, nome: "Temperatura",      un: "°C",    casas: 1, chave: "tBme"},
  {id: "umid", campo: 2, nome: "Umidade (BME280)", un: "%",     casas: 1, chave: "uBme"},
  {id: "umd2", campo: 8, nome: "Umidade (DHT22)",  un: "%",     casas: 1, chave: "uDht"},
  {id: "pres", campo: 3, nome: "Pressão",          un: "hPa",   casas: 1, chave: "pressao"},
  {id: "gas",  campo: 7, nome: "Gases (MQ135)",    un: "V",     casas: 2, chave: "mq"}
];
const cssVar = n => getComputedStyle(document.documentElement).getPropertyValue(n).trim();
const $ = id => document.getElementById(id);
let periodo = "2h";
let graficos = {};

function classificar(pm) {
  if (pm === null) return null;
  return CLASSES.find(c => pm <= c.ate);
}
function num(v) {
  if (v === null || v === undefined || v === "") return null;
  const n = parseFloat(v);
  return isFinite(n) ? n : null;
}
function fmt(v, casas) {
  return v === null ? "--" : v.toLocaleString("pt-BR", {minimumFractionDigits: casas, maximumFractionDigits: casas});
}
function duracao(seg) {
  seg = Math.max(0, Math.round(seg));
  if (seg < 90) return seg + " s";
  if (seg < 5400) return Math.round(seg / 60) + " min";
  if (seg < 172800) return (seg / 3600).toFixed(1).replace(".", ",") + " h";
  return (seg / 86400).toFixed(1).replace(".", ",") + " dias";
}
function horaCurta(d) { return d.toLocaleTimeString("pt-BR", {hour: "2-digit", minute: "2-digit"}); }
function dataHora(d) { return d.toLocaleString("pt-BR", {day: "2-digit", month: "2-digit", hour: "2-digit", minute: "2-digit"}); }


function mostrarErro(msg) {
  const e = $("erro");
  e.hidden = !msg;
  e.textContent = msg || "";
}
// ---------------- Mostradores ----------------
function montarMostradores() {
  $("mostradores").innerHTML = MOSTRADORES.map(m =>
    '<div class="mostrador"><div class="nome">' + m.nome + '</div>' +
    '<div class="valor"><span class="num" id="v_' + m.id + '">--</span><span class="un">' + m.un + '</span></div>' +
    '<div class="tend igual" id="t_' + m.id + '"></div>' +
    '<div class="faixa" id="f_' + m.id + '">mín -- · méd -- · máx --</div></div>').join("");
}

function mediaJanela(linhas, campo, desde, ate) {
  const v = linhas.filter(l => l.t >= desde && l.t <= ate && l[campo] !== null).map(l => l[campo]);
  return v.length ? v.reduce((a, b) => a + b, 0) / v.length : null;
}

function montarEscala() {
  $("faixas").innerHTML = CLASSES.map(c => '<span style="flex:1;background:var(' + c.cor + ')"></span>').join("");
}
function posicaoEscala(pm) {
  // 5 faixas de largura igual na barra: 0-25, 25-50, 50-75, 75-125, 125-150+
  const pontos = [0, 25, 50, 75, 125, 150];
  pm = Math.min(Math.max(pm, 0), 150);
  for (let i = 0; i < 5; i++) {
    if (pm <= pontos[i + 1]) return (i + (pm - pontos[i]) / (pontos[i + 1] - pontos[i])) * 20;
  }
  return 100;
}
// ---------------- Graficos ----------------
function passoBonito(faixa, alvo) {
  const bruto = faixa / Math.max(alvo, 1);
  const pot = Math.pow(10, Math.floor(Math.log10(bruto)));
  const n = bruto / pot;
  return (n < 1.5 ? 1 : n < 3 ? 2 : n < 7 ? 5 : 10) * pot;
}

function criarGrafico(idCanvas, series, opcoes) {
  const canvas = $(idCanvas);
  const caixa = canvas.parentElement;
  const dica = document.createElement("div");
  dica.className = "dica";
  dica.hidden = true;
  caixa.appendChild(dica);
  const g = {canvas, dica, series, opcoes, linhas: [], t0: 0, t1: 1, cursor: null};

  function desenhar() {
    const r = window.devicePixelRatio || 1;
    const w = canvas.clientWidth, h = canvas.clientHeight;
    if (!w) return;
    canvas.width = w * r; canvas.height = h * r;
    const c = canvas.getContext("2d");
    c.setTransform(r, 0, 0, r, 0, 0);
    c.clearRect(0, 0, w, h);
    const esq = 44, dir = 10, cima = 10, baixo = h - 22;
    g.area = {esq, dir: w - dir, cima, baixo};
    const corTexto = cssVar("--muted"), corGrade = cssVar("--line");
    c.font = "11px " + cssVar("--font-num");

    const valores = [];
    for (const s of series) for (const l of g.linhas) if (l[s.campo] !== null) valores.push(l[s.campo]);
    for (const ref of (opcoes.referencias || [])) if (valores.length) valores.push(ref.v);
    if (valores.length < 2) {
      c.fillStyle = corTexto;
      c.fillText("sem dados neste período", esq, (cima + baixo) / 2);
      return;
    }
    let mn = Math.min(...valores), mx = Math.max(...valores);
    if (opcoes.zero) mn = Math.min(0, mn);
    if (mx - mn < (opcoes.faixaMin || 1)) { const m = (mx + mn) / 2, f = (opcoes.faixaMin || 1) / 2; mn = m - f; mx = m + f; if (opcoes.zero) mn = Math.max(0, mn); }
    const passo = passoBonito(mx - mn, 4);
    mn = Math.floor(mn / passo) * passo; mx = Math.ceil(mx / passo) * passo;
    g.mn = mn; g.mx = mx;
    const X = t => esq + (t - g.t0) / (g.t1 - g.t0) * (w - dir - esq);
    const Y = v => baixo - (v - mn) / (mx - mn) * (baixo - cima);
    g.X = X; g.Y = Y;

    // grade horizontal e rotulos do eixo Y
    c.strokeStyle = corGrade; c.lineWidth = 1; c.fillStyle = corTexto; c.textAlign = "right"; c.textBaseline = "middle";
    const casasEixo = passo < 1 ? (passo < 0.1 ? 2 : 1) : 0;
    for (let v = mn; v <= mx + passo / 2; v += passo) {
      const y = Math.round(Y(v)) + 0.5;
      c.beginPath(); c.moveTo(esq, y); c.lineTo(w - dir, y); c.stroke();
      c.fillText(v.toFixed(casasEixo).replace(".", ","), esq - 6, y);
    }
    // eixo do tempo
    c.textAlign = "center"; c.textBaseline = "top";
    const span = (g.t1 - g.t0) / 1000;
    const passosT = [600, 1800, 3600, 7200, 10800, 21600, 43200, 86400];
    const largura = w - esq - dir;
    const alvo = Math.max(2, Math.floor(largura / 90));
    const pt = passosT.find(p => span / p <= alvo) || 86400;
    const fuso = new Date().getTimezoneOffset() * 60;
    let t = Math.ceil((g.t0 / 1000 - fuso) / pt) * pt + fuso;
    for (; t * 1000 <= g.t1; t += pt) {
      const x = X(t * 1000), d = new Date(t * 1000);
      c.strokeStyle = corGrade; c.beginPath(); c.moveTo(Math.round(x) + 0.5, cima); c.lineTo(Math.round(x) + 0.5, baixo); c.stroke();
      const rot = pt >= 86400 ? d.toLocaleDateString("pt-BR", {day: "2-digit", month: "2-digit"})
                : (d.getHours() === 0 && span > 86400) ? d.toLocaleDateString("pt-BR", {day: "2-digit", month: "2-digit"}) : horaCurta(d);
      c.fillText(rot, x, baixo + 5);
    }
    // linhas de referencia
    for (const ref of (opcoes.referencias || [])) {
      if (ref.v < mn || ref.v > mx) continue;
      c.save(); c.setLineDash([5, 4]); c.strokeStyle = cssVar(ref.cor); c.lineWidth = 1.2;
      c.beginPath(); c.moveTo(esq, Y(ref.v)); c.lineTo(w - dir, Y(ref.v)); c.stroke(); c.restore();
    }
    // series (interrompe a linha onde ha buraco nos dados)
    const limiteBuraco = PERIODOS[periodo].bucket * 3 * 1000;
    series.forEach((s, i) => {
      const cor = cssVar(s.cor);
      let ant = null;
      c.strokeStyle = cor; c.lineWidth = s.fina ? 1.4 : 2; c.lineJoin = "round";
      c.beginPath();
      for (const l of g.linhas) {
        const v = l[s.campo];
        if (v === null) { ant = null; continue; }
        const x = X(l.t), y = Y(v);
        if (ant && l.t - ant.t <= limiteBuraco) c.lineTo(x, y); else c.moveTo(x, y);
        ant = l;
      }
      c.stroke();
      // area suave sob a serie principal
      if (i === 0 && opcoes.area) {
        c.save(); c.globalAlpha = 0.10; c.fillStyle = cor;
        let seg = [];
        const fechar = () => {
          if (seg.length > 1) {
            c.beginPath(); c.moveTo(X(seg[0].t), baixo);
            for (const p of seg) c.lineTo(X(p.t), Y(p[s.campo]));
            c.lineTo(X(seg[seg.length - 1].t), baixo); c.closePath(); c.fill();
          }
          seg = [];
        };
        for (const l of g.linhas) {
          if (l[s.campo] === null || (seg.length && l.t - seg[seg.length - 1].t > limiteBuraco)) fechar();
          if (l[s.campo] !== null) seg.push(l);
        }
        fechar(); c.restore();
      }
      // ponto final destacado
      for (let k = g.linhas.length - 1; k >= 0; k--) {
        const l = g.linhas[k];
        if (l[s.campo] !== null) {
          c.fillStyle = cor; c.beginPath(); c.arc(X(l.t), Y(l[s.campo]), 3.2, 0, 7); c.fill();
          break;
        }
      }
    });
    // cursor
    if (g.cursor !== null) {
      const l = g.linhas[g.cursor];
      const x = X(l.t);
      c.strokeStyle = cssVar("--ink"); c.globalAlpha = 0.35; c.beginPath(); c.moveTo(x, cima); c.lineTo(x, baixo); c.stroke(); c.globalAlpha = 1;
      for (const s of series) if (l[s.campo] !== null) {
        c.fillStyle = cssVar("--surface"); c.strokeStyle = cssVar(s.cor); c.lineWidth = 2;
        c.beginPath(); c.arc(x, Y(l[s.campo]), 4, 0, 7); c.fill(); c.stroke();
      }
    }
  }

  function mover(ev) {
    if (!g.linhas.length || !g.X) return;
    const rect = canvas.getBoundingClientRect();
    const x = ev.clientX - rect.left;
    let melhor = 0, dist = Infinity;
    g.linhas.forEach((l, i) => { const d = Math.abs(g.X(l.t) - x); if (d < dist) { dist = d; melhor = i; } });
    g.cursor = melhor;
    const l = g.linhas[melhor];
    const partes = series.filter(s => l[s.campo] !== null).map(s => s.nome + " " + fmt(l[s.campo], s.casas));
    dica.textContent = dataHora(l.t) + "  " + (partes.join(" · ") || "sem dado");
    dica.hidden = false;
    const px = Math.min(Math.max(g.X(l.t), 90), rect.width - 90);
    dica.style.left = px + "px";
    dica.style.top = "0px";
    desenhar();
  }
  function sair() { g.cursor = null; dica.hidden = true; desenhar(); }
  canvas.addEventListener("pointermove", mover);
  canvas.addEventListener("pointerdown", mover);
  canvas.addEventListener("pointerleave", sair);

  g.atualizar = (linhas, t0, t1) => { g.linhas = linhas; g.t0 = t0; g.t1 = t1; g.cursor = null; dica.hidden = true; desenhar(); };
  g.desenhar = desenhar;
  return g;
}

function montarGraficos() {
  graficos.pm = criarGrafico("gPm", [
    {campo: 5, nome: "PM2.5", cor: "--pm25", casas: 0},
    {campo: 6, nome: "PM10", cor: "--pm10", casas: 0, fina: true},
    {campo: 4, nome: "PM1.0", cor: "--pm1", casas: 0, fina: true}
  ], {zero: true, area: true, faixaMin: 10, referencias: [{v: 25, cor: "--ok"}]});
  graficos.temp = criarGrafico("gTemp", [{campo: 1, nome: "°C", cor: "--temp", casas: 1}], {area: true, faixaMin: 2});
  graficos.umid = criarGrafico("gUmid", [
    {campo: 2, nome: "BME", cor: "--umid", casas: 1},
    {campo: 8, nome: "DHT", cor: "--umid2", casas: 1, fina: true}
  ], {area: true, faixaMin: 5, referencias: [{v: 85, cor: "--ruim"}]});
  graficos.press = criarGrafico("gPress", [{campo: 3, nome: "hPa", cor: "--press", casas: 1}], {faixaMin: 2});
  graficos.gas = criarGrafico("gGas", [{campo: 7, nome: "V", cor: "--gas", casas: 2}], {area: true, faixaMin: 0.1});
}


let pontos = [];
function atualizarTendencias() {
  // tendencia: media dos ultimos 5 min comparada com a de 55-65 min atras (do historico de 2 h)
  const fim = pontos.length ? pontos[pontos.length - 1].t : null;
  for (const m of MOSTRADORES) {
    const t = $("t_" + m.id);
    let agora = null, antes = null;
    if (fim) {
      agora = mediaJanela(pontos, m.campo, new Date(fim - 5 * 60000), fim);
      antes = mediaJanela(pontos, m.campo, new Date(fim - 65 * 60000), new Date(fim - 55 * 60000));
    }
    if (agora !== null && antes !== null) {
      const d = agora - antes, limiar = Math.pow(10, -m.casas) * (m.casas ? 1 : 2);
      t.className = "tend " + (Math.abs(d) < limiar ? "igual" : d > 0 ? "sobe" : "desce");
      t.textContent = (Math.abs(d) < limiar ? "estável" : (d > 0 ? "▲ " : "▼ ") + fmt(Math.abs(d), m.casas)) + " na última hora";
    } else {
      t.className = "tend igual";
      t.textContent = "tendência: aguardando 1 h de dados";
    }
    const vals = pontos.map(l => l[m.campo]).filter(v => v !== null);
    $("f_" + m.id).textContent = vals.length
      ? "mín " + fmt(Math.min(...vals), m.casas) + " · méd " + fmt(vals.reduce((a, b) => a + b, 0) / vals.length, m.casas) + " · máx " + fmt(Math.max(...vals), m.casas)
      : "aguardando dados (1 ponto a cada 30 s)";
  }
  const vals = pontos.map(l => l[5]).filter(v => v !== null);
  return vals.length ? vals.reduce((a, b) => a + b, 0) / vals.length : null;
}

function chip(nome, ok) { return '<span class="chip' + (ok ? '' : ' falha') + '">' + nome + (ok ? '' : ': falha') + '</span>'; }
function definir(id, texto, alerta) { const e = $(id); e.textContent = texto; e.classList.toggle("alerta", !!alerta); }

async function atualizar() {
  try {
    const d = await (await fetch("/dados", {cache: "no-store"})).json();
    mostrarErro("");
    const pm = d.pm25;
    $("pmAgora").textContent = fmt(pm, 0);
    const c = classificar(pm);
    const cl = $("classe");
    cl.textContent = c ? c.nome : "sem dados";
    cl.style.background = c ? "var(" + c.cor + ")" : "var(--muted)";
    $("marcador").style.left = (pm === null ? 0 : posicaoEscala(pm)) + "%";
    for (const m of MOSTRADORES) $("v_" + m.id).textContent = fmt(d[m.chave], m.casas);
    $("v_temp").parentElement.parentElement.querySelector(".nome").textContent = "Temperatura" + (d.tDht !== null ? " (DHT22: " + fmt(d.tDht, 1) + " °C)" : "");
    $("v_gas").parentElement.parentElement.querySelector(".nome").textContent = "Gases (MQ135)" + (d.mqAquecendo ? ", aquecendo" : "");
    const media = atualizarTendencias();
    let aviso = 'Média de PM2.5 nas últimas 2 h: <strong class="num">' + fmt(media, 0) + '</strong> µg/m³ · referência da OMS para 24 h: 15 µg/m³';
    if (d.umidadeAlta) aviso += '<br><strong>Umidade acima de 85 %:</strong> as partículas podem estar superestimadas (gotículas de água também são contadas).';
    $("aviso").innerHTML = aviso;

    $("chips").innerHTML = chip("BME280", d.ok.bme) + chip("DHT22", d.ok.dht) + chip("PMS7003", d.ok.pms) + chip("MQ135", d.ok.mq);
    const est = $("estado");
    const enviando = d.ts.configurado && d.ts.ultimoOkSeg !== null && d.ts.ultimoOkSeg < 120;
    est.className = "estado " + (!d.ts.configurado ? "" : enviando ? "online" : "offline");
    est.textContent = !d.ts.configurado ? "só rede própria" : enviando ? "enviando" : "sem enviar";
    $("ultimo").textContent = "ligada há " + duracao(d.ligadoSeg);

    definir("sNet", !d.wifi.configurada ? "não configurada" : d.wifi.conectada ? d.wifi.nome + " (" + d.wifi.ip + ")" : "desconectada de " + d.wifi.nome, d.wifi.configurada && !d.wifi.conectada);
    definir("sSinal", d.wifi.sinal === null ? "--" : d.wifi.sinal + " dBm (" + (d.wifi.sinal > -60 ? "bom" : d.wifi.sinal > -75 ? "razoável" : "fraco") + ")", d.wifi.sinal !== null && d.wifi.sinal <= -75);
    definir("sTs", d.ts.configurado ? d.ts.ok + " envios ok, " + d.ts.falhas + " falhas" : "não configurado");
    definir("sUltimo", d.ts.ultimoOkSeg === null ? "nenhum ainda" : "há " + duracao(d.ts.ultimoOkSeg), d.ts.configurado && (d.ts.ultimoOkSeg === null || d.ts.ultimoOkSeg > 300));
    definir("sFila", d.ts.fila + (d.ts.fila === 1 ? " leitura" : " leituras") + " (≈ " + duracao(d.ts.fila * 20) + ")" + (d.ts.descartados ? " · " + d.ts.descartados + " perdidas" : ""), d.ts.fila > 3);
    definir("sErro", d.ts.erro || "nenhum", !!d.ts.erro);
    definir("sLigada", duracao(d.ligadoSeg));
    definir("sMotivo", d.reinicio);
    definir("sReinicios", d.reiniciosAuto + " do ESP32 · " + d.reiniciosWifi + " do Wi-Fi", d.reiniciosAuto > 0);
    definir("sMem", d.memoriaKb + " kB");
    $("atualizado").textContent = "Atualizado às " + horaCurta(new Date()) + " · atualiza sozinho a cada 5 s";
  } catch (e) {
    mostrarErro("Sem conexão com a estação. Confira se o aparelho está na rede EstacaoAr.");
  }
}

async function historico() {
  try {
    const d = await (await fetch("/historico", {cache: "no-store"})).json();
    const agora = Date.now();
    pontos = d.pontos.map(p => {
      const l = {t: new Date(agora - (d.agora - p[0]) * 1000)};
      for (let c = 1; c <= 8; c++) l[c] = p[c];
      return l;
    });
    for (const k in graficos) graficos[k].atualizar(pontos, agora - 7200000, agora);
    atualizarTendencias();
  } catch (e) {}
}

montarMostradores();
montarEscala();
montarGraficos();
let redesenho = null;
window.addEventListener("resize", () => { clearTimeout(redesenho); redesenho = setTimeout(() => { for (const k in graficos) graficos[k].desenhar(); }, 150); });
historico().then(atualizar);
setInterval(atualizar, 5000);
setInterval(historico, 30000);
</script></body></html>)HTML";

void responderPagina() {
  servidor.send_P(200, "text/html", PAGINA);
}

// =====================================================================
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println();
  Serial.println("=== Estacao de Qualidade do Ar - VERSAO DE CAMPO ===");

  esp_reset_reason_t razao = esp_reset_reason();
  motivoReinicio = descreverReinicio(razao);
  if (rtcMarca != MARCA_RTC || razao == ESP_RST_POWERON) {
    rtcMarca = MARCA_RTC;
    rtcReiniciosAuto = 0;
  }
  Serial.print("Inicio  : ");
  Serial.println(motivoReinicio);

  Wire.begin(PINO_SDA, PINO_SCL);
  if (bme.begin(0x76, &Wire) || bme.begin(0x77, &Wire)) {
    bmeIniciado = true;
    Serial.println("BME280  : iniciado");
  } else {
    Serial.println("BME280  : NAO encontrado (confira SDA/SCL/VIN/GND)");
  }

  dht.begin();
  Serial.println("DHT22   : iniciado");

  Serial2.setRxBufferSize(1024);   // folga enquanto o envio ao ThingSpeak ocupa o processador
  Serial2.begin(9600, SERIAL_8N1, PMS_RX, PMS_TX);
  Serial.println("PMS7003 : porta serial iniciada");

  analogReadResolution(12);
  analogSetPinAttenuation(PINO_MQ135, ADC_11db);
  Serial.println("MQ135   : entrada analogica iniciada");

  fsOk = LittleFS.begin(true);
  Serial.println(fsOk ? "Memoria : pronta para guardar a fila" : "Memoria : FALHOU (a fila nao sobrevive a reinicios)");
  carregarFila();

  ligarWifi();
  Serial.print("Wi-Fi   : rede propria \"");
  Serial.print(AP_NOME);
  Serial.print("\" criada. Pagina em http://");
  Serial.println(WiFi.softAPIP().toString());
  if (strlen(WIFI_NOME) > 0) {
    Serial.print("Wi-Fi   : conectando em \"");
    Serial.print(WIFI_NOME);
    Serial.println("\" para enviar ao ThingSpeak...");
  } else {
    Serial.println("Wi-Fi   : rede com internet nao configurada (ThingSpeak desligado)");
  }
  if (strlen(WIFI_NOME) > 0 && strlen(THINGSPEAK_CHAVE) > 0 && strlen(THINGSPEAK_CANAL) == 0) {
    Serial.println("Aviso   : THINGSPEAK_CANAL vazio, leituras atrasadas nao poderao ser enviadas em lote");
  }

  clienteTls.setInsecure();        // HTTPS sem conferir certificado (dados nao sao sigilosos)
  configTime(0, 0, "pool.ntp.org", "time.google.com");

  servidor.on("/", responderPagina);
  servidor.on("/dados", responderDados);
  servidor.on("/historico", responderHistorico);
  servidor.on("/csv", responderCsv);
  servidor.begin();
  Serial.println("Pagina  : servidor iniciado");

#if ESP_ARDUINO_VERSION_MAJOR >= 3
  // core esp32 3.x: o watchdog ja vem ligado, so ajusta o tempo
  esp_task_wdt_config_t configWdt = {};
  configWdt.timeout_ms = WATCHDOG_SEG * 1000;
  configWdt.idle_core_mask = 0;
  configWdt.trigger_panic = true;
  esp_task_wdt_reconfigure(&configWdt);
#else
  // core esp32 2.x
  esp_task_wdt_init(WATCHDOG_SEG, true);
#endif
  esp_task_wdt_add(NULL);
  Serial.println("Watchdog: ligado (reinicia se travar por 60 s)");

  Serial.println("Primeiro resumo em 5 segundos...");
  ultimoRelatorio = millis();
  ultimoHistorico = millis();
  ultimaGuardaEnvio = millis();
  ultimoOk = millis();
  ultimoReinicioWifi = millis();
}

void loop() {
  esp_task_wdt_reset();
  servidor.handleClient();
  processarPms();
  processarDht();
  processarMq();

  if (millis() - ultimoRelatorio >= INTERVALO_RELATORIO) {
    ultimoRelatorio = millis();
    atualizarLeituras();
    imprimirRelatorio();
  }

  if (millis() - ultimoHistorico >= INTERVALO_HISTORICO) {
    ultimoHistorico = millis();
    guardarHistorico();
  }

  guardarParaEnvio();
  manterWifi();
  enviarThingSpeak();
  verificarRecuperacao();
}
