# Etapa 8: Estação integrada (todos os sensores juntos)

**Nenhum fio muda.** Todos os sensores ficam como estão desde a Etapa 7.
Este programa junta tudo o que testamos: lê os 4 sensores ao mesmo tempo, sem que um atrapalhe o outro,
e imprime um **resumo a cada 5 segundos**.

## O que o programa faz
- **BME280**: temperatura, umidade e pressão (sensor de referência).
- **DHT22**: temperatura e umidade (segunda opinião). Leituras falhas são descartadas; vale a última boa por até 10 s.
- **PMS7003**: confere cada pacote e mostra a **média** do período (PM1.0, PM2.5, PM10) e uma classificação do ar pelo PM2.5.
- **MQ135**: média de ~50 amostras por período, em volts. Avisa enquanto está aquecendo (3 primeiros minutos).
- **Status**: mostra se cada sensor está ok ou em falha, e conta falhas/pacotes ruins.

Classificação usada para o PM2.5 (µg/m³): até 25 BOA · até 50 MODERADA · até 75 RUIM · até 125 MUITO RUIM · acima PÉSSIMA.
São faixas de referência para **média de 24 h**; aqui servem só como indicação para a leitura do momento.

## Gravar e testar
1. **File → New**, cole [`firmware/etapa08_estacao_integrada/etapa08_estacao_integrada.ino`](../../firmware/etapa08_estacao_integrada/etapa08_estacao_integrada.ino), salve como `etapa08_estacao_integrada` e grave.
2. **Serial Monitor** em **115200**.

Esperado:
```
=== Estacao de Qualidade do Ar - Etapa 8 (integrada) ===
BME280  : iniciado
DHT22   : iniciado
PMS7003 : porta serial iniciada
MQ135   : entrada analogica iniciada
Primeiro resumo em 5 segundos...

===== Leitura #1   (ligado ha 00:00:05) =====
Temperatura : 24.5 C (BME280)  |  25.4 C (DHT22)
Umidade     : 44.2 % (BME280)  |  49.3 % (DHT22)
Pressao     : 1013.6 hPa
Particulas  : PM1.0 = 62  |  PM2.5 = 92  |  PM10 = 101 ug/m3  (media de 5 pacotes)
Qualidade   : MUITO RUIM (pelo PM2.5)
Gases MQ135 : 0.78 V  [aquecendo, valor ainda instavel]
Status      : BME280 ok  |  DHT22 ok (0 falhas)  |  PMS7003 ok (0 pacotes ruins)  |  MQ135 ok
```

## Teste de robustez (opcional, mas recomendado)
Com o programa rodando, **desconecte só o fio TX do PMS7003** por uns 10 segundos e reconecte.
- Enquanto desconectado: a linha de partículas deve mostrar `--- (sem dados do PMS7003)` e o status `PMS7003 FALHA`.
- Ao reconectar: tudo volta a `ok` sozinho, **sem reiniciar o ESP32**.
(Mexer só nesse fio de dados com a placa ligada é seguro; **não** mexa em fios de VCC/VIN/GND com ela ligada.)

## Problemas comuns
| Sintoma | Causa provável | Solução |
|---|---|---|
| Algum sensor com `FALHA` desde o início | Fio solto desde a última etapa | Desligar, conferir aquele sensor pela etapa dele |
| `DHT22 ok` mas o número de falhas sobe rápido | Mau contato no fio de dados | Apertar o jumper do GPIO4 |
| ESP32 reinicia sozinho | Consumo alto no USB | Outra porta USB, de preferência atrás do gabinete |
