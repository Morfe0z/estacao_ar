# KiCad K2b: esquemático, parte 2 — ligar as peças

Versão: **KiCad 10**. Agora as peças da K2a ganham as ligações. São **as mesmas ligações que já funcionam na protoboard** ([`etapa-02-mapa-de-ligacoes.md`](../etapas/etapa-02-mapa-de-ligacoes.md)), só que desenhadas.

## 1. A pinagem do seu ESP32 (copiada da foto, USB para baixo)

| Pino da barra | J1 (lado ESQUERDO) | J2 (lado DIREITO) |
|---|---|---|
| 1 (em cima) | EN | D23 |
| 2 | VP (GPIO36) | **D22 (SCL)** |
| 3 | VN (GPIO39) | TX0 |
| 4 | **D34 (MQ135)** | RX0 |
| 5 | D35 | **D21 (SDA)** |
| 6 | D32 | D19 |
| 7 | D33 | D18 |
| 8 | D25 | D5 |
| 9 | D26 | **TX2 (GPIO17) → RX do PMS** |
| 10 | D27 | **RX2 (GPIO16) ← TX do PMS** |
| 11 | D14 | **D4 (DHT22)** |
| 12 | D12 | D2 |
| 13 | D13 | D15 |
| 14 | **GND** | **GND** |
| 15 (embaixo) | **VIN (5 V)** | **3V3** |

No KiCad o **pino 1 do Conn_01x15 é o de cima**, então o número do pino na folha é exatamente a linha da tabela.
Em negrito estão os únicos pinos que usamos: 4 de J1 + 7 de J2.

## 2. Três ferramentas novas
| Tecla | Faz | Como usar |
|---|---|---|
| **W** | Fio (*wire*) | Aperte W com o mouse **na pontinha do pino**, clique para fazer curvas, **dois cliques** para terminar. Esc cancela. |
| **L** | Etiqueta (*label*) | Escreva o nome, OK, e encoste o **quadradinho da etiqueta na ponta de um fio**. Duas etiquetas com o **mesmo nome** estão ligadas entre si, mesmo sem fio desenhado entre elas. |
| **Q** | "Não conectado" (um **X** azul) | Clique na ponta de cada pino que **não** usamos. Diz ao KiCad "deixei solto de propósito". |

Como saber se ligou: a ponta de um pino solto mostra um **circulozinho**. Quando o fio encosta certo, o circulozinho some.
Onde um fio encontra outro no meio, aparece uma **bolinha** (junção): ela é necessária para ligar de verdade.

Por que usar etiquetas: em vez de puxar um fio comprido do ESP32 até cada borne (o que vira um emaranhado), fazemos um **fio curtinho** saindo do pino e colocamos uma etiqueta na ponta. Do outro lado, outro fio curtinho com uma etiqueta de mesmo nome. Fica limpo e fácil de conferir.

Os símbolos de alimentação (**GND**, **+5V**, **+3V3**) funcionam igual às etiquetas: todo GND está ligado a todo GND. Coloque mais com a tecla **P** sempre que precisar.

## 3. Ligações, uma peça de cada vez
Faça na ordem e salve (**Ctrl+S**) ao fim de cada bloco.

### 3.1 ESP32 lado esquerdo (J1)
| Pino | Ligar em |
|---|---|
| 4 (D34) | fio curto + etiqueta **`MQ135_ADC`** |
| 14 (GND) | símbolo **GND** |
| 15 (VIN) | símbolo **+5V** |
| todos os outros (1–3, 5–13) | **X** de não conectado (tecla Q) |

### 3.2 ESP32 lado direito (J2)
| Pino | Ligar em |
|---|---|
| 2 (D22) | etiqueta **`SCL`** |
| 5 (D21) | etiqueta **`SDA`** |
| 9 (TX2) | etiqueta **`PMS_RX`** (é o fio que **chega no RX** do PMS) |
| 10 (RX2) | etiqueta **`PMS_TX`** (é o fio que **sai do TX** do PMS) |
| 11 (D4) | etiqueta **`DHT_DADOS`** |
| 14 (GND) | símbolo **GND** |
| 15 (3V3) | símbolo **+3V3** |
| todos os outros (1, 3, 4, 6–8, 12, 13) | **X** de não conectado |

Atenção à troca TX/RX: o TX do PMS vai no **RX2** do ESP32 e vice-versa (igual à protoboard). Com as etiquetas `PMS_TX` e `PMS_RX` com o nome **do lado do PMS**, basta ligar etiqueta igual em pino igual do borne.

### 3.3 Energia (J3) e capacitor (C1)
| Peça | Pino | Ligar em |
|---|---|---|
| J3 ENERGIA | 1 | **+5V** (fio vermelho da fonte) |
| J3 ENERGIA | 2 | **GND** (fio preto) |
| C1 470 µF | **+** (lado com o sinal "+") | **+5V** |
| C1 470 µF | − | **GND** |

O capacitor eletrolítico **tem polaridade**: invertido, ele estoura. No desenho e na solda, o "+" vai no 5 V.

### 3.4 BME280 (J4)
A ordem dos bornes copia a ordem dos pinos da plaquinha do BME280, para o cabo ir reto.
**Ainda falta você confirmar essa ordem**; a mais comum é `VIN GND SCL SDA`, que é a usada abaixo. Se a sua for diferente, só muda a coluna "Ligar em".

| Pino de J4 | Pino do BME280 | Ligar em |
|---|---|---|
| 1 | VIN | **+3V3** |
| 2 | GND | **GND** |
| 3 | SCL | etiqueta **`SCL`** |
| 4 | SDA | etiqueta **`SDA`** |

### 3.5 DHT22 (J5) e resistor R1
| Peça | Pino | Ligar em |
|---|---|---|
| J5 | 1 | **+3V3** (perna 1 do DHT22) |
| J5 | 2 | etiqueta **`DHT_DADOS`** (perna 2) |
| J5 | 3 | **GND** (perna 4; a perna 3 fica sem ligar no sensor) |
| R1 4,7k | um lado | **+3V3** |
| R1 4,7k | outro lado | etiqueta **`DHT_DADOS`** |

### 3.6 PMS7003 (J6)
Mesma ordem do conector P3 da plaquinha adaptadora (**TX, RX, VCC, GND**), para o cabo ir reto:

| Pino de J6 | Pino do P3 | Ligar em |
|---|---|---|
| 1 | TX | etiqueta **`PMS_TX`** |
| 2 | RX | etiqueta **`PMS_RX`** |
| 3 | VCC | **+5V** |
| 4 | GND | **GND** |

### 3.7 MQ135 (J7) e divisor R2/R3
Mesma ordem do módulo (VCC, GND, … AO); o DO não é usado, então o borne tem só 3 vias:

| Peça | Pino | Ligar em |
|---|---|---|
| J7 | 1 | **+5V** (VCC do módulo) |
| J7 | 2 | **GND** |
| J7 | 3 | etiqueta **`MQ135_AO`** (AO do módulo) |
| R2 4,7k | um lado | etiqueta **`MQ135_AO`** |
| R2 4,7k | outro lado | etiqueta **`MQ135_ADC`** |
| R3 4,7k | um lado | etiqueta **`MQ135_ADC`** |
| R3 4,7k | outro lado | **GND** |

Assim o desenho fica igual ao divisor que já usamos: `AO ──R2── MQ135_ADC (D34) ──R3── GND`.
**Nunca** ligue `MQ135_AO` direto no ESP32: são até 5 V.

## 4. Conferência visual
1. Aperte **Home** para ver a folha toda.
2. Procure **circulozinhos** sobrando na ponta de pinos: cada um é um pino esquecido (ou sem fio, ou sem X).
3. **Clique em uma etiqueta** (por exemplo `SDA`): o KiCad destaca todas as partes ligadas a ela. Confira se aparecem as duas pontas certas (J2 pino 5 e J4 pino 4).
4. Faça o mesmo com `SCL`, `PMS_TX`, `PMS_RX`, `DHT_DADOS`, `MQ135_AO`, `MQ135_ADC`.

## 5. Me mande
- [ ] Print da folha inteira (Home).
- [ ] A **ordem dos pinos do seu BME280** (o que está escrito na plaquinha), para confirmar a seção 3.4.

Na **K2c** rodamos a verificação automática (ERC): o KiCad procura sozinho pinos esquecidos, curtos entre etiquetas e alimentação sem fonte.
