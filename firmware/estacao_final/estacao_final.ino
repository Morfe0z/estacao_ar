// =====================================================================
// Estacao de Monitoramento da Qualidade do Ar - VERSAO FINAL
// ESP32 DevKit 30 pinos | core esp32 2.0.17
//
// O que faz:
//  - Le BME280, DHT22, PMS7003 e MQ135 (igual a Etapa 8)
//  - Cria a propria rede Wi-Fi "EstacaoAr": conecte o celular/PC e abra
//    http://192.168.4.1 para ver os valores e os graficos (funciona sem internet)
//  - Se houver uma rede com internet configurada, envia os dados ao ThingSpeak
//  - Continua imprimindo o resumo no Monitor Serial (115200)
//
// Ligacoes:
//   BME280 : VIN -> 3V3 | GND -> GND | SDA -> GPIO21 | SCL -> GPIO22
//   DHT22  : perna 1 -> 3V3 | perna 2 -> GPIO4 (+ 4,7k ate 3V3) | perna 4 -> GND
//   PMS7003: TX -> GPIO16 | RX -> GPIO17 | VCC -> VIN | GND -> GND
//   MQ135  : VCC -> VIN | GND -> GND | AO -> divisor 4,7k/4,7k -> GPIO34
//
// Bibliotecas: "Adafruit BME280 Library" e "DHT sensor library" (Adafruit)
// =====================================================================

#include <Wire.h>
#include <Adafruit_BME280.h>
#include <DHT.h>
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>

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
const unsigned long INTERVALO_THINGSPEAK = 20000;    // o plano gratuito aceita no minimo 15 s
const unsigned long INTERVALO_RECONEXAO  = 30000;
const float FATOR_DIVISOR_MQ             = 2.0;

// ---------------- Objetos ----------------
Adafruit_BME280 bme;
DHT dht(PINO_DHT, DHT22);
WebServer servidor(80);

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
unsigned long ultimoHistorico = 0, ultimoEnvio = 0, ultimaReconexao = 0;
unsigned long envioOk = 0, envioFalhas = 0;
String ultimoErroEnvio = "";

// ---------------- Historico (para os graficos) ----------------
const int TAM_HISTORICO = 240;   // 240 pontos x 30 s = 2 horas
struct Ponto {
  uint32_t seg;
  float pm25, tBme, uBme, mq;
};
Ponto historico[TAM_HISTORICO];
int histInicio = 0, histQtd = 0;

void guardarHistorico() {
  int pos = (histInicio + histQtd) % TAM_HISTORICO;
  historico[pos].seg  = millis() / 1000;
  historico[pos].pm25 = atual.pmsOk ? atual.pm25 : NAN;
  historico[pos].tBme = atual.bmeOk ? atual.tBme : NAN;
  historico[pos].uBme = atual.bmeOk ? atual.uBme : NAN;
  historico[pos].mq   = atual.mqOk  ? atual.mqVolts : NAN;
  if (histQtd < TAM_HISTORICO) histQtd++;
  else histInicio = (histInicio + 1) % TAM_HISTORICO;
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
    Serial.println(" (pelo PM2.5)");
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
  if (strlen(WIFI_NOME) == 0) Serial.print("nao configurada");
  else if (WiFi.status() == WL_CONNECTED) Serial.print("conectada");
  else Serial.print("desconectada");
  Serial.print("  |  ThingSpeak: ");
  if (strlen(THINGSPEAK_CHAVE) == 0) Serial.println("nao configurado");
  else {
    Serial.print(envioOk);
    Serial.print(" envios ok, ");
    Serial.print(envioFalhas);
    Serial.println(" falhas");
  }
}

// =====================================================================
// ThingSpeak
// Campos: 1 temp BME | 2 umid BME | 3 pressao | 4 PM1.0 | 5 PM2.5
//         6 PM10 | 7 MQ135 (V) | 8 umid DHT22
// =====================================================================
void adicionarCampo(String &url, int campo, float valor, bool valido, int casas) {
  if (!valido || isnan(valor)) return;
  url += "&field";
  url += campo;
  url += "=";
  String numero = String(valor, casas);
  numero.trim();   // o ESP32 poe espacos na frente de numeros curtos (" 3"), e o ThingSpeak recusa com erro 400
  url += numero;
}

// Remove aspas, barras e quebras de linha (o texto vai dentro do JSON da pagina)
String textoSeguro(String t) {
  t.replace("\\", "/");
  t.replace("\"", "'");
  t.replace("\r", " ");
  t.replace("\n", " ");
  return t;
}

void enviarThingSpeak() {
  if (strlen(THINGSPEAK_CHAVE) == 0 || strlen(WIFI_NOME) == 0) return;
  if (millis() - ultimoEnvio < INTERVALO_THINGSPEAK) return;
  ultimoEnvio = millis();
  if (WiFi.status() != WL_CONNECTED) return;

  String url = "http://api.thingspeak.com/update?api_key=";
  url += THINGSPEAK_CHAVE;
  adicionarCampo(url, 1, atual.tBme, atual.bmeOk, 1);
  adicionarCampo(url, 2, atual.uBme, atual.bmeOk, 1);
  adicionarCampo(url, 3, atual.pressao, atual.bmeOk, 1);
  adicionarCampo(url, 4, atual.pm1, atual.pmsOk, 0);
  adicionarCampo(url, 5, atual.pm25, atual.pmsOk, 0);
  adicionarCampo(url, 6, atual.pm10, atual.pmsOk, 0);
  adicionarCampo(url, 7, atual.mqVolts, atual.mqOk, 3);
  adicionarCampo(url, 8, atual.uDht, atual.dhtOk, 1);

  HTTPClient http;
  http.setTimeout(4000);
  http.begin(url);
  int codigo = http.GET();
  String resposta = (codigo > 0) ? http.getString() : "";
  http.end();

  // O ThingSpeak responde com o numero da entrada (> 0) quando aceita
  if (codigo == 200 && resposta.toInt() > 0) {
    envioOk++;
    ultimoErroEnvio = "";
  } else {
    envioFalhas++;
    if (codigo == 200)     ultimoErroEnvio = "ThingSpeak recusou (chave errada ou envio rapido demais)";
    else if (codigo <= 0)  ultimoErroEnvio = "sem resposta do servidor (codigo " + String(codigo) + ")";
    else                   ultimoErroEnvio = "servidor respondeu codigo " + String(codigo) + ": " + textoSeguro(resposta.substring(0, 80));
    Serial.print("ThingSpeak: falha no envio - ");
    Serial.println(ultimoErroEnvio);
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

// =====================================================================
// Pagina web
// =====================================================================
String numeroJson(float v, int casas) {
  if (isnan(v)) return "null";
  return String(v, casas);
}

void responderDados() {
  String j = "{";
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
  j += "\"ok\":{\"bme\":" + String(atual.bmeOk ? "true" : "false");
  j += ",\"dht\":" + String(atual.dhtOk ? "true" : "false");
  j += ",\"pms\":" + String(atual.pmsOk ? "true" : "false");
  j += ",\"mq\":"  + String(atual.mqOk ? "true" : "false") + "},";
  j += "\"ligadoSeg\":" + String(millis() / 1000) + ",";
  j += "\"internet\":\"";
  if (strlen(WIFI_NOME) == 0) j += "nao configurada";
  else if (WiFi.status() == WL_CONNECTED) j += "conectada";
  else j += "desconectada";
  j += "\",\"thingspeak\":\"";
  if (strlen(THINGSPEAK_CHAVE) == 0) j += "nao configurado";
  else {
    j += String(envioOk) + " envios ok, " + String(envioFalhas) + " falhas";
    if (ultimoErroEnvio.length() > 0) j += " (ultimo erro: " + ultimoErroEnvio + ")";
  }
  j += "\"}";
  servidor.send(200, "application/json", j);
}

void responderHistorico() {
  String j = "{\"agora\":" + String(millis() / 1000) + ",\"pontos\":[";
  for (int i = 0; i < histQtd; i++) {
    const Ponto &p = historico[(histInicio + i) % TAM_HISTORICO];
    if (i > 0) j += ",";
    j += "[" + String(p.seg) + "," + numeroJson(p.pm25, 0) + "," + numeroJson(p.tBme, 1) + ","
         + numeroJson(p.uBme, 1) + "," + numeroJson(p.mq, 2) + "]";
  }
  j += "]}";
  servidor.send(200, "application/json", j);
}

void responderCsv() {
  String c = "segundos_ligado,pm2_5_ugm3,temperatura_C,umidade_pct,mq135_V\n";
  for (int i = 0; i < histQtd; i++) {
    const Ponto &p = historico[(histInicio + i) % TAM_HISTORICO];
    c += String(p.seg) + "," + (isnan(p.pm25) ? String("") : String(p.pm25, 0)) + ","
         + (isnan(p.tBme) ? String("") : String(p.tBme, 1)) + ","
         + (isnan(p.uBme) ? String("") : String(p.uBme, 1)) + ","
         + (isnan(p.mq) ? String("") : String(p.mq, 2)) + "\n";
  }
  servidor.sendHeader("Content-Disposition", "attachment; filename=estacao.csv");
  servidor.send(200, "text/csv", c);
}

const char PAGINA[] PROGMEM = R"HTML(<!DOCTYPE html>
<html lang="pt-BR"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Estacao de Qualidade do Ar</title>
<style>
body{font-family:system-ui,sans-serif;margin:0;background:#f4f6f8;color:#1d2733}
header{background:#1f6f5c;color:#fff;padding:14px 16px}
header h1{margin:0;font-size:1.2em}
header small{opacity:.85}
main{padding:12px;max-width:900px;margin:auto}
.grade{display:grid;grid-template-columns:repeat(auto-fit,minmax(150px,1fr));gap:10px}
.cartao{background:#fff;border-radius:10px;padding:12px;box-shadow:0 1px 3px rgba(0,0,0,.08)}
.rotulo{font-size:.8em;color:#5b6b7a}
.valor{font-size:1.7em;font-weight:600;margin-top:4px}
.sub{font-size:.8em;color:#5b6b7a;margin-top:2px}
#qualidade{font-size:1.3em;font-weight:700;padding:10px;border-radius:10px;text-align:center;margin:10px 0;color:#fff;background:#888}
h2{font-size:1em;margin:18px 0 8px}
canvas{width:100%;height:160px;background:#fff;border-radius:10px;box-shadow:0 1px 3px rgba(0,0,0,.08)}
.status{font-size:.85em;line-height:1.6}
.ok{color:#1a7f37}.falha{color:#c62828;font-weight:600}
a{color:#1f6f5c}
</style></head><body>
<header><h1>Estação de Qualidade do Ar</h1><small id="ligado">carregando...</small></header>
<main>
<div id="qualidade">---</div>
<div class="grade">
 <div class="cartao"><div class="rotulo">PM2.5</div><div class="valor" id="pm25">--</div><div class="sub">µg/m³</div></div>
 <div class="cartao"><div class="rotulo">PM10</div><div class="valor" id="pm10">--</div><div class="sub">µg/m³ · PM1.0: <span id="pm1">--</span></div></div>
 <div class="cartao"><div class="rotulo">Temperatura</div><div class="valor" id="tBme">--</div><div class="sub">°C · DHT22: <span id="tDht">--</span></div></div>
 <div class="cartao"><div class="rotulo">Umidade</div><div class="valor" id="uBme">--</div><div class="sub">% · DHT22: <span id="uDht">--</span></div></div>
 <div class="cartao"><div class="rotulo">Pressão</div><div class="valor" id="pressao">--</div><div class="sub">hPa</div></div>
 <div class="cartao"><div class="rotulo">Gases (MQ135)</div><div class="valor" id="mq">--</div><div class="sub" id="mqSub">V</div></div>
</div>
<h2>PM2.5 (µg/m³) - últimas 2 horas</h2><canvas id="gPm"></canvas>
<h2>Temperatura (°C)</h2><canvas id="gT"></canvas>
<h2>Umidade (%)</h2><canvas id="gU"></canvas>
<h2>Gases MQ135 (V)</h2><canvas id="gMq"></canvas>
<h2>Situação</h2>
<div class="cartao status" id="status">--</div>
<p><a href="/csv">Baixar histórico (CSV)</a></p>
</main>
<script>
const cores={"BOA":"#2e7d32","MODERADA":"#f9a825","RUIM":"#ef6c00","MUITO RUIM":"#c62828","PESSIMA":"#6a1b9a"};
function txt(id,v){document.getElementById(id).textContent=(v===null||v===undefined)?"--":v;}
function st(nome,ok){return nome+': <span class="'+(ok?'ok':'falha')+'">'+(ok?'ok':'FALHA')+'</span>';}
function tempo(s){const h=Math.floor(s/3600),m=Math.floor(s/60)%60;return h+'h '+m+'min';}
async function atualizar(){
 try{
  const d=await (await fetch('/dados')).json();
  txt('pm25',d.pm25);txt('pm10',d.pm10);txt('pm1',d.pm1);
  txt('tBme',d.tBme);txt('tDht',d.tDht);txt('uBme',d.uBme);txt('uDht',d.uDht);
  txt('pressao',d.pressao);txt('mq',d.mq);
  txt('mqSub',d.mqAquecendo?'V · aquecendo':'V');
  const q=document.getElementById('qualidade');
  q.textContent='Qualidade do ar: '+d.qualidade;q.style.background=cores[d.qualidade]||'#888';
  txt('ligado','ligada há '+tempo(d.ligadoSeg));
  document.getElementById('status').innerHTML=st('BME280',d.ok.bme)+' · '+st('DHT22',d.ok.dht)+' · '+st('PMS7003',d.ok.pms)+' · '+st('MQ135',d.ok.mq)
   +'<br>Internet: '+d.internet+'<br>ThingSpeak: '+d.thingspeak;
 }catch(e){txt('ligado','sem conexão com a estação');}
}
function grafico(id,pontos,col,cor){
 const c=document.getElementById(id),w=c.clientWidth,h=c.clientHeight,r=window.devicePixelRatio||1;
 c.width=w*r;c.height=h*r;const g=c.getContext('2d');g.scale(r,r);g.clearRect(0,0,w,h);
 const v=pontos.filter(p=>p[col]!==null);
 g.fillStyle='#5b6b7a';g.font='11px sans-serif';
 if(v.length<2){g.fillText('aguardando dados (1 ponto a cada 30 s)',10,h/2);return;}
 let mn=Math.min(...v.map(p=>p[col])),mx=Math.max(...v.map(p=>p[col]));
 if(mx-mn<1e-6){mx+=1;mn-=1;}
 const t0=v[0][0],t1=v[v.length-1][0],e=40,dir=8,top=8,base=h-18;
 const X=t=>e+(t-t0)/(t1-t0||1)*(w-e-dir),Y=y=>base-(y-mn)/(mx-mn)*(base-top);
 g.strokeStyle='#e3e8ee';g.beginPath();g.moveTo(e,top);g.lineTo(e,base);g.lineTo(w-dir,base);g.stroke();
 g.fillText(mx.toFixed(1),2,top+8);g.fillText(mn.toFixed(1),2,base);
 g.fillText('-'+Math.round((t1-t0)/60)+' min',e,h-4);g.fillText('agora',w-dir-30,h-4);
 g.strokeStyle=cor;g.lineWidth=2;g.beginPath();
 v.forEach((p,i)=>{i?g.lineTo(X(p[0]),Y(p[col])):g.moveTo(X(p[0]),Y(p[col]));});g.stroke();
}
async function historico(){
 try{
  const d=await (await fetch('/historico')).json();
  grafico('gPm',d.pontos,1,'#c62828');grafico('gT',d.pontos,2,'#ef6c00');
  grafico('gU',d.pontos,3,'#1565c0');grafico('gMq',d.pontos,4,'#6a1b9a');
 }catch(e){}
}
atualizar();historico();setInterval(atualizar,5000);setInterval(historico,30000);
</script></body></html>)HTML";

void responderPagina() {
  servidor.send_P(200, "text/html", PAGINA);
}

// =====================================================================
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println();
  Serial.println("=== Estacao de Qualidade do Ar - VERSAO FINAL ===");

  Wire.begin(PINO_SDA, PINO_SCL);
  if (bme.begin(0x76, &Wire) || bme.begin(0x77, &Wire)) {
    bmeIniciado = true;
    Serial.println("BME280  : iniciado");
  } else {
    Serial.println("BME280  : NAO encontrado (confira SDA/SCL/VIN/GND)");
  }

  dht.begin();
  Serial.println("DHT22   : iniciado");

  Serial2.begin(9600, SERIAL_8N1, PMS_RX, PMS_TX);
  Serial.println("PMS7003 : porta serial iniciada");

  analogReadResolution(12);
  analogSetPinAttenuation(PINO_MQ135, ADC_11db);
  Serial.println("MQ135   : entrada analogica iniciada");

  // Wi-Fi: rede propria sempre; rede com internet se configurada
  if (strlen(WIFI_NOME) > 0) {
    WiFi.mode(WIFI_AP_STA);
  } else {
    WiFi.mode(WIFI_AP);
  }
  WiFi.softAP(AP_NOME, AP_SENHA);
  Serial.print("Wi-Fi   : rede propria \"");
  Serial.print(AP_NOME);
  Serial.print("\" criada. Pagina em http://");
  Serial.println(WiFi.softAPIP().toString());

  if (strlen(WIFI_NOME) > 0) {
    WiFi.setAutoReconnect(true);
    WiFi.begin(WIFI_NOME, WIFI_SENHA);
    ultimaReconexao = millis();
    Serial.print("Wi-Fi   : conectando em \"");
    Serial.print(WIFI_NOME);
    Serial.println("\" para enviar ao ThingSpeak...");
  } else {
    Serial.println("Wi-Fi   : rede com internet nao configurada (ThingSpeak desligado)");
  }

  servidor.on("/", responderPagina);
  servidor.on("/dados", responderDados);
  servidor.on("/historico", responderHistorico);
  servidor.on("/csv", responderCsv);
  servidor.begin();
  Serial.println("Pagina  : servidor iniciado");

  Serial.println("Primeiro resumo em 5 segundos...");
  ultimoRelatorio = millis();
  ultimoHistorico = millis();
}

void loop() {
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

  manterWifi();
  enviarThingSpeak();
}
