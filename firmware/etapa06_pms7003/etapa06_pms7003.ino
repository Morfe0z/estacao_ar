// Etapa 6: teste do PMS7003 (material particulado PM1.0 / PM2.5 / PM10)
// Ligacao (placa adaptadora, conector P3): TX -> GPIO16 | RX -> GPIO17 | VCC -> VIN (5 V) | GND -> GND
// Nao precisa de biblioteca: o pacote de dados do sensor e lido e conferido aqui mesmo.

const int PMS_RX = 16;  // recebe o TX do PMS7003
const int PMS_TX = 17;  // envia para o RX do PMS7003

struct PmsDados {
  uint16_t pm1_0;   // ug/m3 (atmosferico)
  uint16_t pm2_5;   // ug/m3 (atmosferico)
  uint16_t pm10;    // ug/m3 (atmosferico)
  uint16_t n0_3;    // particulas > 0,3 um em 0,1 L de ar
  uint16_t n2_5;    // particulas > 2,5 um em 0,1 L de ar
};

uint8_t buf[32];
int idx = 0;
unsigned long pacotesOk = 0;
unsigned long pacotesRuins = 0;

// Le os bytes que chegaram do sensor. Devolve true quando um pacote completo e valido foi lido.
// Pacote do PMS7003: 32 bytes, comeca com 0x42 0x4D, termina com a soma de verificacao.
bool lerPms(PmsDados &d) {
  while (Serial2.available()) {
    uint8_t b = Serial2.read();

    if (idx == 0) {
      if (b == 0x42) buf[idx++] = b;
      continue;
    }
    if (idx == 1 && b != 0x4D) {
      idx = (b == 0x42) ? 1 : 0;
      continue;
    }

    buf[idx++] = b;
    if (idx < 32) continue;
    idx = 0;

    uint16_t comprimento = (buf[2] << 8) | buf[3];
    uint16_t soma = 0;
    for (int i = 0; i < 30; i++) soma += buf[i];
    uint16_t verificacao = (buf[30] << 8) | buf[31];

    if (comprimento != 28 || soma != verificacao) {
      pacotesRuins++;
      continue;
    }

    d.pm1_0 = (buf[10] << 8) | buf[11];
    d.pm2_5 = (buf[12] << 8) | buf[13];
    d.pm10  = (buf[14] << 8) | buf[15];
    d.n0_3  = (buf[16] << 8) | buf[17];
    d.n2_5  = (buf[22] << 8) | buf[23];
    pacotesOk++;
    return true;
  }
  return false;
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println();
  Serial.println("=== Etapa 6: teste do PMS7003 ===");
  Serial2.begin(9600, SERIAL_8N1, PMS_RX, PMS_TX);
  Serial.println("Aguardando dados do sensor (as leituras estabilizam depois de ~30 s)...");
}

unsigned long ultimoDado = 0;

void loop() {
  PmsDados d;
  if (lerPms(d)) {
    ultimoDado = millis();
    Serial.print("PM1.0: ");
    Serial.print(d.pm1_0);
    Serial.print("  |  PM2.5: ");
    Serial.print(d.pm2_5);
    Serial.print("  |  PM10: ");
    Serial.print(d.pm10);
    Serial.print(" ug/m3   |  >0,3um: ");
    Serial.print(d.n0_3);
    Serial.print("  >2,5um: ");
    Serial.print(d.n2_5);
    Serial.print("   [ok: ");
    Serial.print(pacotesOk);
    Serial.print(" / ruins: ");
    Serial.print(pacotesRuins);
    Serial.println("]");
  }

  if (millis() - ultimoDado > 5000) {
    Serial.println("Nenhum dado do PMS7003 ha 5 s: confira TX->GPIO16, VCC no VIN e se a ventoinha esta girando.");
    ultimoDado = millis();
  }
}
