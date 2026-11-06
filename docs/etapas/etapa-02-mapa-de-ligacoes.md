# Etapa 2: Mapa de ligações (planejamento, ainda sem montar)

Nesta etapa só **planejamos** onde cada fio vai. Ainda não monte nada: a montagem será feita
sensor por sensor nas próximas etapas, testando cada um antes de passar ao seguinte.

## O que ficou definido na Etapa 1
| Peça | O que temos | Consequência |
|---|---|---|
| ESP32 | DevKit de 30 pinos | Usaremos os nomes de pinos dessa placa (D4, D21, RX2...) |
| BMP280 | Chip retangular, 4 pinos, VIN | É **BMP280**: mede temperatura e pressão, **não mede umidade**. A umidade virá do DHT22 |
| DHT22 | 1 sensor solto, 4 pernas | Precisa de resistor externo de pull‑up: usaremos o de **4,7 kΩ** (funciona perfeitamente) |
| MQ135 | Módulo VCC/GND/DO/AO | Usaremos só o AO (analógico), com divisor de tensão |
| PMS7003 | Com adaptadora e cabo | Ligação por serial (UART) |
| Fonte | 5 V / 3 A | Mais que suficiente para a estação completa |

## Tabela de ligações

### PMS7003 (pela plaquinha adaptadora)
| Pino da adaptadora | Vai para no ESP32 | Observação |
|---|---|---|
| VCC | **VIN** (5 V) | O ventilador precisa de 5 V |
| GND | **GND** | |
| TX | **RX2** (GPIO16) | O TX de um vai no RX do outro (cruzado) |
| RX | **TX2** (GPIO17) | |
| SET, RESET, NC | **não ligar** | Já ficam ativos internamente |

### BMP280 (4 pinos)
| Pino do módulo | Vai para no ESP32 | Observação |
|---|---|---|
| VIN | **3V3** | Mesmo aceitando 5 V, ligar em 3,3 V é o mais seguro para o ESP32 |
| GND | **GND** | |
| SCL | **D22** (GPIO22) | Barramento I2C |
| SDA | **D21** (GPIO21) | Barramento I2C |

### DHT22 (sensor solto)
Segure o sensor com a **grade de furinhos virada para você** e as pernas para baixo. As pernas, da esquerda para a direita, são 1, 2, 3 e 4:

| Perna | Função | Vai para |
|---|---|---|
| 1 | VCC | **3V3** |
| 2 | DADOS | **D4** (GPIO4) **e** resistor de 4,7 kΩ até o **3V3** |
| 3 | não usada | não ligar |
| 4 | GND | **GND** |

### MQ135 (com divisor de tensão)
| Pino do módulo | Vai para | Observação |
|---|---|---|
| VCC | **VIN** (5 V) | O aquecedor precisa de 5 V |
| GND | **GND** | |
| DO | **não ligar** | Saída digital, não vamos usar |
| AO | **entrada do divisor** (abaixo) | **Nunca** direto no ESP32 |

Divisor de tensão com **dois resistores de 4,7 kΩ** iguais:

```
AO do MQ135 ──[ 4,7 kΩ ]──┬──[ 4,7 kΩ ]── GND
                          │
                          └──> D34 (GPIO34) do ESP32
```

Com dois resistores iguais a tensão cai pela metade: se o AO chegar a 5 V, o ESP32 recebe no máximo **2,5 V**, abaixo do limite de 3,3 V.
O GPIO34 foi escolhido porque é uma entrada analógica que continua funcionando com o Wi‑Fi ligado (pinos ADC2 deixam de funcionar com Wi‑Fi).

## Resumo dos pinos do ESP32 usados
| Pino ESP32 | Uso |
|---|---|
| VIN | 5 V para PMS7003 e MQ135 |
| 3V3 | 3,3 V para BMP280 e DHT22 (e o resistor de 4,7 kΩ) |
| GND | Todos os GND juntos (**terra comum é obrigatório**) |
| RX2 / GPIO16 | TX do PMS7003 |
| TX2 / GPIO17 | RX do PMS7003 |
| D21 / GPIO21 | SDA do BMP280 |
| D22 / GPIO22 | SCL do BMP280 |
| D4 / GPIO4 | Dados do DHT22 |
| D34 / GPIO34 | Meio do divisor do MQ135 |

## Pinos que vamos evitar (e por quê)
- **GPIO0, 2, 12, 15**: influenciam o modo de inicialização; um sensor ligado neles pode impedir a placa de ligar ou de ser gravada.
- **GPIO6 a 11**: ligados à memória interna (nem aparecem na placa de 30 pinos).
- **GPIO1 e 3 (TX0/RX0)**: usados pelo cabo USB para gravar e para o Monitor Serial.

## Sobre a alimentação
- **Durante os testes** (próximas etapas): tudo alimentado só pelo **cabo USB** do computador. O pino VIN entrega os ~5 V do USB para o PMS7003 e o MQ135.
- **Na estação final**: a fonte de 5 V / 3 A liga no **VIN** e no **GND** do ESP32.
- **Nunca ligue a fonte e o cabo USB ao mesmo tempo**, para não colocar duas fontes de 5 V uma contra a outra.

## Necessidade de material
Você precisa de **3 resistores de 4,7 kΩ** (1 para o DHT22 e 2 para o divisor). Os de 1 kΩ não serão usados.

## Adendo: placa adaptadora do PMS7003 (confirmado com foto)
- **P1** (encaixe preto 2x5): onde o sensor encaixa. Já encaixado.
- **P2** (conector branco + cabo de 8 fios): **não usado**, pode guardar.
- **P3** (4 pinos de 2,54 mm), na ordem: **TX, RX, VCC, GND**. É o que ligamos ao ESP32 com jumpers macho‑fêmea:
  TX → RX2/GPIO16 · RX → TX2/GPIO17 · VCC → VIN (5 V) · GND → GND
