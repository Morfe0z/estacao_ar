// =====================================================================
// Diagnostico do envio ao ThingSpeak (rode uma vez e mande o print)
// Tenta enviar de 6 jeitos diferentes, com 20 s entre cada um, e mostra
// a resposta COMPLETA do servidor em cada tentativa.
// Grava ate 6 pontos de teste (campo 1 = numero do teste) no canal.
// Nao precisa de nenhum sensor ligado.
// =====================================================================
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

// Preencha antes de gravar (e NAO envie este arquivo preenchido para o GitHub)
const char* WIFI_NOME  = "NOME_DA_REDE";
const char* WIFI_SENHA = "SENHA_DA_REDE";
const char* CHAVE      = "SUA_WRITE_API_KEY";
const char* HOST       = "api.thingspeak.com";

void separador(int n, const char* titulo) {
  Serial.println();
  Serial.println("==================================================");
  Serial.print("TESTE ");
  Serial.print(n);
  Serial.print(": ");
  Serial.println(titulo);
  Serial.println("==================================================");
}

void mostrarResultado(int codigo, const String& corpo) {
  Serial.print("Codigo HTTP: ");
  Serial.println(codigo);
  Serial.println("Resposta:");
  Serial.println(corpo.length() ? corpo : String("(vazia)"));
  if (codigo == 200 && corpo.toInt() > 0) Serial.println(">>> ACEITO");
  else Serial.println(">>> RECUSADO");
}

// Testes com a biblioteca HTTPClient (a mesma da estacao)
void testeHttpClient(int n, const char* titulo, bool https, bool post, bool todosCampos) {
  separador(n, titulo);
  String campos = "field1=" + String(n);
  if (todosCampos) campos += "&field2=62.3&field3=1011.4&field4=0&field5=3&field6=3&field7=4.410&field8=69.9";
  String url = String(https ? "https://" : "http://") + HOST + "/update";
  if (!post) url += String("?api_key=") + CHAVE + "&" + campos;
  Serial.print("Endereco: ");
  Serial.println(url);

  WiFiClientSecure seguro;
  WiFiClient simples;
  HTTPClient http;
  http.setTimeout(10000);
  bool ok;
  if (https) { seguro.setInsecure(); ok = http.begin(seguro, url); }
  else ok = http.begin(simples, url);
  if (!ok) { Serial.println("Falhou ao iniciar a conexao"); return; }
  int codigo;
  if (post) {
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    String corpo = String("api_key=") + CHAVE + "&" + campos;
    Serial.print("Corpo: ");
    Serial.println(corpo);
    codigo = http.POST(corpo);
  } else {
    codigo = http.GET();
  }
  String resposta = codigo > 0 ? http.getString() : http.errorToString(codigo);
  http.end();
  mostrarResultado(codigo, resposta);
}

// Teste "na mao": manda exatamente o que o curl manda e mostra tudo que voltar
void testeManual(int n) {
  separador(n, "HTTPS escrito na mao, igual ao curl (mostra os cabecalhos da resposta)");
  WiFiClientSecure c;
  c.setInsecure();
  if (!c.connect(HOST, 443)) { Serial.println("Nao conectou na porta 443"); return; }
  String pedido = String("GET /update?api_key=") + CHAVE + "&field1=" + String(n) + " HTTP/1.1\r\n" +
                  "Host: " + HOST + "\r\n" +
                  "User-Agent: curl/8.4.0\r\n" +
                  "Accept: */*\r\n" +
                  "Connection: close\r\n\r\n";
  Serial.println("Pedido enviado:");
  Serial.print(pedido);
  c.print(pedido);
  unsigned long inicio = millis();
  Serial.println("Resposta completa:");
  while ((c.connected() || c.available()) && millis() - inicio < 10000) {
    while (c.available()) Serial.write(c.read());
    delay(5);
  }
  c.stop();
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  delay(1500);
  Serial.println();
  Serial.println("=== Diagnostico do ThingSpeak ===");
  Serial.print("Pacote ESP32: ");
  Serial.println(ESP.getSdkVersion());
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_NOME, WIFI_SENHA);
  Serial.print("Conectando no Wi-Fi");
  for (int i = 0; i < 60 && WiFi.status() != WL_CONNECTED; i++) { delay(500); Serial.print("."); }
  Serial.println();
  if (WiFi.status() != WL_CONNECTED) { Serial.println("Nao conectou no Wi-Fi. Confira nome e senha."); return; }
  Serial.print("Conectado. IP ");
  Serial.println(WiFi.localIP());
  IPAddress ip;
  if (WiFi.hostByName(HOST, ip)) { Serial.print("api.thingspeak.com = "); Serial.println(ip); }

  testeHttpClient(1, "HTTP simples, so o campo 1 (jeito da versao final)", false, false, false);
  delay(20000);
  testeHttpClient(2, "HTTPS, so o campo 1", true, false, false);
  delay(20000);
  testeHttpClient(3, "HTTPS, todos os campos (jeito da versao de campo)", true, false, true);
  delay(20000);
  testeHttpClient(4, "HTTPS com POST, todos os campos", true, true, true);
  delay(20000);
  testeManual(5);
  delay(20000);
  testeHttpClient(6, "HTTP simples, todos os campos", false, false, true);

  Serial.println();
  Serial.println("=== Fim. Role para cima e mande o print de todos os testes ===");
}

void loop() {}
