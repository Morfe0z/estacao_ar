# Dados medidos

[`thingspeak_2026-10-08_a_2026-10-20.csv`](thingspeak_2026-10-08_a_2026-10-20.csv): exportação do canal do ThingSpeak,
de 08/10/2026 22h43 a 20/10/2026 17h39 (UTC), cerca de 47 mil leituras, uma a cada ~20 s.

| Coluna | Conteúdo |
|---|---|
| `created_at` | data e hora (UTC) |
| `entry_id` | número da leitura |
| `field1` | temperatura (°C, BME280) |
| `field2` | umidade (%, BME280) |
| `field3` | pressão (hPa) |
| `field4` / `field5` / `field6` | PM1.0 / PM2.5 / PM10 (µg/m³) |
| `field7` | MQ135 (V, índice relativo, ainda sem calibração) |
| `field8` | umidade do DHT22 (%) |

Cuidados ao analisar:
- Até 09/10 a estação ficou no laboratório (PM2.5 interno alto, ~90 µg/m³); a partir da noite de 09/10, fora do laboratório (ar bem mais limpo, PM perto de 0).
- Em 09/10 por volta das 23h (UTC) o MQ135 teve mau contato na protoboard e algumas leituras chegaram a ~4,9 V: descarte-as (`field7` > 4,5).
- Buracos em 09/10 de manhã: é o incidente do erro 400 (ver [solução de problemas](../docs/solucao-de-problemas.md)).
- O DHT22 lê cerca de 8 pontos de umidade acima do BME280; o BME280 é a referência.
