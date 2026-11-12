# Projeto KiCad da placa principal

Feito no **KiCad 10**. Guias passo a passo em [`docs/kicad`](../../docs/kicad/).

| Arquivo | Conteúdo |
|---|---|
| `placa_estacao.kicad_sch` | Esquemático completo (K2b): ESP32 em duas barras de 15 pinos, bornes de energia, BME280, DHT22, PMS7003 e MQ135, pull-up de 4,7 kΩ do DHT22, divisor 4,7 kΩ / 4,7 kΩ do MQ135, capacitor de 470 µF no VIN e PWR_FLAG para a verificação ERC. |

Redes do esquemático: `+5V`, `+3V3`, `GND`, `SDA` (GPIO21), `SCL` (GPIO22), `PMS_TX` (→ GPIO16/RX2), `PMS_RX` (← GPIO17/TX2), `DHT_DADOS` (GPIO4), `MQ135_AO` (saída do módulo, até 5 V) e `MQ135_ADC` (meio do divisor → GPIO34).

Para abrir: copie este arquivo para a pasta do projeto `placa_estacao` (junto do `.kicad_pro`) e abra pelo KiCad.
