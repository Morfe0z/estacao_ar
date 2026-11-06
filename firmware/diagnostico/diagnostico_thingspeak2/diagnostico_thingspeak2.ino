// =====================================================================
// Diagnostico 2: o mesmo envio, mas com a rede propria "EstacaoAr" ligada
// junto (como na estacao). Mostra o DNS e o endereco usado em cada teste.
// Grava ate 4 pontos de teste (campo 1 = 11, 12, 13, 14) no canal.
// =====================================================================
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

// Preencha antes de gravar (e NAO envie este arquivo preenchido para o GitHub)
const char* WIFI_NOME  = "NOME_DA_REDE";
const char* WIFI_SENHA = "SENHA_DA_REDE";
const char* CHAVE      = "SUA_WRITE_API_KEY";
const char* HOST       = "api.thingspeak.com";

WebServer servidor(80);
WiFiClientSecure clienteGlobal;   // igual a estacao: um cliente so, reaproveitado

void mostrarRede() {
  Serial.print("DNS 1: "); Serial.print(WiFi.dnsIP(0));
  Serial.print("  |  DNS 2: "); Serial.println(WiFi.dnsIP(1));
  IPAddress ip;
  if (WiFi.hostByName(HOST, ip)) { Serial.print("api.thingspeak.com = "); Serial.println(ip); }
  else Serial.println("api.thingspeak.com = NAO RESOLVEU");
}

void enviar(int n, const char* titulo, bool jeitoDaEstacao) {
  Serial.println();
  Serial.println("==================================================");
  Serial.print("TESTE "); Serial.print(n); Serial.print(": "); Serial.println(titulo);
  Serial.println("==================================================");
  mostrarRede();
  String url = String("https://") + HOST + "/update?api_key=" + CHAVE + "&field1=" + String(n) +
               "&field2=62.3&field3=1011.4&field4=0&field5=3&field6=3&field7=4.410&field8=69.9";
  WiFiClientSecure clienteNovo;
  clienteNovo.setInsecure();
  HTTPClient http;
  bool ok;
  if (jeitoDaEstacao) {
    http.setConnectTimeout(8000);
    http.setTimeout(8000);
    ok = http.begin(clienteGlobal, url);
    const char* cab[] = {"Date"};
    http.collectHeaders(cab, 1);
  } else {
    http.setTimeout(10000);
    ok = http.begin(clienteNovo, url);
  }
  if (!ok) { Serial.println("Falhou ao iniciar a conexao"); return; }
  int codigo = http.GET();
  String resposta = codigo > 0 ? http.getString() : http.errorToString(codigo);
  http.end();
  Serial.print("Codigo HTTP: "); Serial.println(codigo);
  Serial.println("Resposta:"); Serial.println(resposta);
  Serial.println((codigo == 200 && resposta.toInt() > 0) ? ">>> ACEITO" : ">>> RECUSADO");
}

void setup() {
  Serial.begin(115200);
  delay(1500);
  Serial.println();
  Serial.println("=== Diagnostico 2 do ThingSpeak (com a rede EstacaoAr ligada) ===");
  clienteGlobal.setInsecure();
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP("EstacaoAr", "troque-esta-senha");
  WiFi.begin(WIFI_NOME, WIFI_SENHA);
  Serial.print("Conectando no Wi-Fi");
  for (int i = 0; i < 60 && WiFi.status() != WL_CONNECTED; i++) { delay(500); Serial.print("."); }
  Serial.println();
  if (WiFi.status() != WL_CONNECTED) { Serial.println("Nao conectou no Wi-Fi."); return; }
  Serial.print("Conectado. IP "); Serial.println(WiFi.localIP());
  servidor.on("/", []() { servidor.send(200, "text/plain", "ok"); });
  servidor.begin();

  enviar(11, "rede propria ligada, cliente novo", false);
  delay(20000);
  enviar(12, "rede propria ligada, jeito da estacao (cliente reaproveitado)", true);
  delay(20000);
  enviar(13, "jeito da estacao, segunda vez com o mesmo cliente", true);
  delay(20000);
  Serial.println();
  Serial.println("Desligando a rede propria e repetindo...");
  WiFi.softAPdisconnect(true);
  delay(2000);
  enviar(14, "rede propria DESLIGADA, jeito da estacao", true);
  Serial.println();
  Serial.println("=== Fim. Mande o print de todos os testes ===");
}

void loop() { servidor.handleClient(); }
