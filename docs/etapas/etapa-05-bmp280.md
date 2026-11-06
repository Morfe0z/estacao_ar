# Etapa 5: Segundo sensor, BMP280 (temperatura e pressão)

> **Descoberta durante o teste:** o módulo era na verdade um **BME280** (o programa leu o ID `0x60`), que mede também a umidade.
> Para ele, use a biblioteca **Adafruit BME280 Library** e o programa [`firmware/etapa05b_bme280/`](../../firmware/etapa05b_bme280/etapa05b_bme280.ino).
> As ligações são exatamente as mesmas desta página. Nas etapas seguintes o BME280 é a referência de temperatura e umidade.

O DHT22 **continua ligado** como na Etapa 4. Agora acrescentamos o BMP280.
Ele usa o barramento **I2C**: só 2 fios de dados (SDA e SCL), além de alimentação.

## 5.1 Instalar a biblioteca
1. **Sketch → Include Library → Manage Libraries...**
2. Pesquise **Adafruit BMP280 Library** e instale.
3. Quando perguntar sobre dependências, clique em **Install all** (instala a "Adafruit BusIO").

## 5.2 Montar (com o USB DESCONECTADO)
| Pino do BMP280 | Liga em |
|---|---|
| VIN | **3V3** do ESP32 (a mesma fileira que já alimenta o DHT22) |
| GND | **GND** do ESP32 |
| SCL | **D22 / GPIO22** |
| SDA | **D21 / GPIO21** |

Cuidado para não trocar **SDA** e **SCL**: se trocar, nada queima, mas o sensor não é encontrado.

## 5.3 Conferir antes de ligar (multímetro em continuidade)
1. **3V3 com GND**: **não pode apitar**.
2. **SDA do módulo com D21**: tem que apitar.
3. **SCL do módulo com D22**: tem que apitar.

## 5.4 Gravar e testar
1. **File → New** (sketch novo!), cole [`firmware/etapa05_bmp280/etapa05_bmp280.ino`](../../firmware/etapa05_bmp280/etapa05_bmp280.ino), salve como `etapa05_bmp280`.
2. Conecte o USB e clique na **seta (Upload)**.
3. **Serial Monitor** em **115200**, aperte **EN** na placa para ver o começo.

Esperado:
```
=== Etapa 5: teste do BMP280 + DHT22 ===
Procurando dispositivos I2C...
  Dispositivo encontrado no endereco 0x76
Chip no endereco 0x76 tem ID 0x58 -> e um BMP280
----------------------------------------
BMP280  Temperatura: 26.1 C   |   Pressao: 1008.4 hPa
DHT22   Temperatura: 25.6 C   |   Umidade: 50.4 %
```
- O endereço pode ser **0x76** ou **0x77**; os dois servem.
- A **pressão** depende da altitude: ~1013 hPa ao nível do mar, e cai cerca de **12 hPa a cada 100 m** de altitude.
- A temperatura do BMP280 costuma ficar um pouco diferente da do DHT22 (até ~1 °C). Normal.

**Teste de vida:** levante a placa ~1 m: a pressão cai ~0,1 hPa. Ou coloque o dedo sobre o chip: a temperatura do BMP280 sobe.

## Problemas comuns
| Sintoma | Causa provável | Solução |
|---|---|---|
| `NENHUM dispositivo I2C encontrado` | SDA/SCL trocados ou soltos, ou falta VIN/GND | Refazer 5.2 e 5.3 |
| Encontrou o endereço, mas `BMP280 nao iniciou` | Mau contato | Apertar os jumpers e apertar EN |
| Pressão absurda (ex.: 0 ou 1100+ fixo) | Sensor com defeito ou mau contato | Me mande o print |
| `Adafruit_BMP280.h: No such file` | Biblioteca não instalada | Passo 5.1 |
