# Firmware (programas do ESP32)

Todos os programas são sketches da **Arduino IDE** (uma pasta com um `.ino` do mesmo nome).

## Qual programa usar

| Pasta | Etapa | O que faz | Situação |
|---|---|---|---|
| [`etapa03_teste_esp32`](etapa03_teste_esp32/) | 3 | Pisca o LED e mostra os dados do chip | testado |
| [`etapa04_dht22`](etapa04_dht22/) | 4 | Lê só o DHT22 | testado |
| [`etapa05_bmp280`](etapa05_bmp280/) | 5 | Procura o sensor I2C e identifica BMP280/BME280 | testado (descobriu o BME280) |
| [`etapa05b_bme280`](etapa05b_bme280/) | 5 | BME280 + DHT22 | testado |
| [`etapa06_pms7003`](etapa06_pms7003/) | 6 | Lê o PMS7003 pela UART2 | testado |
| [`etapa07_mq135`](etapa07_mq135/) | 7 | Lê o MQ135 pelo divisor no GPIO34 | testado |
| [`etapa08_estacao_integrada`](etapa08_estacao_integrada/) | 8 | Os 4 sensores juntos, resumo a cada 5 s | testado |
| [`estacao_final`](estacao_final/) | 9 | + rede Wi-Fi própria com página e envio ao ThingSpeak | testado |
| [`estacao_campo`](estacao_campo/) | 10b | Versão robusta: fila de 4 h, HTTPS, recuperação automática, watchdog | **em uso** |
| [`estacao_campo_redes`](estacao_campo_redes/) | 10b | Igual à de campo, com lista de várias redes (inclusive eduroam) | compila; falta testar no hardware |
| [`diagnostico/`](diagnostico/) | — | Testes do envio ao ThingSpeak (usados no incidente de 09/10) | ferramentas |

## Configurar senhas e chaves (secrets.h)

Este repositório é **público**, então nenhuma senha fica nos programas. Os sketches que usam rede
(`estacao_final`, `estacao_campo`, `estacao_campo_redes`) leem a configuração de um arquivo `secrets.h`:

1. Baixe a pasta do sketch (botão **Code → Download ZIP** na página principal do repositório).
2. Dentro da pasta do sketch, **copie** `secrets.exemplo.h` e renomeie a cópia para **`secrets.h`**.
3. Abra o `.ino` na Arduino IDE (**File → Open**). O `secrets.h` aparece numa aba ao lado.
4. Preencha na aba `secrets.h`:
   - `AP_SENHA`: senha da rede própria da estação (mínimo 8 caracteres);
   - `WIFI_NOME` e `WIFI_SENHA` (ou a lista `REDES` na versão de várias redes);
   - `THINGSPEAK_CHAVE`: a **Write API Key** do canal;
   - `THINGSPEAK_CANAL`: o Channel ID (versão de campo).
5. Grave normalmente.

O `secrets.h` está no [`.gitignore`](../.gitignore): mesmo que alguém use git nesta pasta, ele não é enviado ao GitHub.

## Arduino IDE
- Pacote de placas: **esp32 by Espressif Systems** (testado nas versões 2.0.17 e 3.3.x).
- Placa: **ESP32 Dev Module**. Porta: `/dev/ttyACM0` no Linux, `COMx` no Windows (driver CH343).
- Partition Scheme: **Default 4MB with spiffs** (se der "Sketch too big", use **Huge APP**).
- Monitor Serial: **115200 baud**.

## Bibliotecas
| Biblioteca (Library Manager) | Usada a partir da |
|---|---|
| DHT sensor library (Adafruit) + Adafruit Unified Sensor | Etapa 4 |
| Adafruit BMP280 Library | Etapa 5 (só o teste do BMP280) |
| Adafruit BME280 Library (+ Adafruit BusIO) | Etapa 5b em diante |

Wi-Fi, servidor web, HTTPS, LittleFS e watchdog já vêm no pacote do ESP32.

## Envio ao ThingSpeak
8 campos, a cada 20 s:

| Campo | Valor |
|---|---|
| 1 | Temperatura (°C, BME280) |
| 2 | Umidade (%, BME280) |
| 3 | Pressão (hPa) |
| 4 | PM1.0 (µg/m³) |
| 5 | PM2.5 (µg/m³) |
| 6 | PM10 (µg/m³) |
| 7 | MQ135 (V) |
| 8 | Umidade DHT22 (%) |
