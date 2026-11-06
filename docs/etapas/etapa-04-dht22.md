# Etapa 4: Primeiro sensor, DHT22 (temperatura e umidade)

Começamos pelo DHT22 porque é o mais simples: 3 fios e 1 resistor.
**Só o DHT22 é ligado nesta etapa.** Os outros sensores continuam desconectados do ESP32.

## 4.0 Antes de tudo: tornar a permissão da porta permanente
O `chmod` da Etapa 3 era temporário e se perde quando a placa é desconectada (e nesta etapa vamos desconectar).
1. **Saia da sessão do Ubuntu e entre de novo.**
2. No Terminal, rode `groups`: precisa aparecer **dialout**.
A partir daí a gravação funciona sempre, sem `chmod`.

## 4.1 Instalar a biblioteca do DHT22
1. Na IDE: **Sketch → Include Library → Manage Libraries...**
2. Pesquise **DHT sensor library** e instale a da **Adafruit**.
3. Se a IDE perguntar sobre dependências, clique em **Install all** (instala também a "Adafruit Unified Sensor").
   Se não perguntar, pesquise **Adafruit Unified Sensor** e instale também.

## 4.2 Montar (com o USB DESCONECTADO)
Segure o DHT22 com a **grade de furinhos virada para você** e as pernas para baixo. Da esquerda para a direita: pernas 1, 2, 3, 4.

| Perna do DHT22 | Liga em |
|---|---|
| 1 (VCC) | **3V3** do ESP32 |
| 2 (DADOS) | **D4 / GPIO4** do ESP32 |
| 3 | **nada** |
| 4 (GND) | **GND** do ESP32 |

E o resistor de **4,7 kΩ**: uma ponta na fileira da **perna 2**, a outra na fileira do **3V3**.

```
3V3 ──┬──────────────── perna 1 (VCC)
      │
   [4,7 kΩ]
      │
D4 ───┴──────────────── perna 2 (DADOS)
                        perna 3 (nada)
GND ─────────────────── perna 4 (GND)
```

## 4.3 Conferir antes de ligar (multímetro em continuidade)
1. **3V3 com GND**: **não pode apitar**.
2. **Perna 2 com D4**: **tem que apitar** (confirma o fio de dados).
3. **Perna 4 com GND**: **tem que apitar**.

## 4.4 Gravar e testar
1. Conecte o USB. Confira **Tools → Port → /dev/ttyACM0**.
2. Abra [`firmware/etapa04_dht22/etapa04_dht22.ino`](../../firmware/etapa04_dht22/etapa04_dht22.ino) (ou cole num sketch novo) e clique na **seta (Upload)**.
3. **Serial Monitor** em **115200**.

Esperado (uma linha a cada 2,5 s):
```
=== Etapa 4: teste do DHT22 ===
Temperatura: 24.3 C   |   Umidade: 58.1 %
Temperatura: 24.3 C   |   Umidade: 58.2 %
```
Valores razoáveis em ambiente interno: **15 a 35 °C** e **20 a 90 %**.

**Teste de vida:** sopre devagar perto do sensor (ou segure-o entre os dedos). A umidade deve subir em alguns segundos e depois voltar.

## Problemas comuns
| Sintoma | Causa provável | Solução |
|---|---|---|
| `ERRO: falha ao ler o DHT22` em todas as linhas | Fio de dados fora do D4, resistor ausente, ou sensor virado | Refazer 4.2 e 4.3 |
| Erro de compilação `DHT.h: No such file` | Biblioteca não instalada | Passo 4.1 |
| Valores absurdos (ex.: -40 °C ou 0 %) | Mau contato na protoboard | Apertar os jumpers |
| Sensor esquentando | VCC e GND invertidos | **Desligar na hora** e conferir as pernas 1 e 4 |
