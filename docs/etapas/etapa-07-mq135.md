# Etapa 7: Último sensor, MQ135 (gases)

Os outros três sensores **continuam ligados**. O MQ135 detecta gases como CO₂, amônia, álcool, fumaça e solventes.
Ele dá uma **tensão** (não um número pronto): quanto mais gás, maior a tensão na saída AO.

⚠️ Esta é a etapa mais delicada: o MQ135 funciona a **5 V** e a saída AO pode chegar a 5 V, mas o ESP32 só aceita até **3,3 V**.
Por isso vamos montar o **divisor de tensão** e **medir com o multímetro antes** de ligar no ESP32.

Não precisa de biblioteca.

## 7.1 Montar o MQ135 e o divisor (USB DESCONECTADO), SEM ligar no GPIO34 ainda
| Pino do MQ135 | Liga em |
|---|---|
| VCC | **VIN** do ESP32 (5 V) |
| GND | **GND** do ESP32 |
| DO | **nada** |
| AO | ponta de um resistor de **4,7 kΩ** (R1) |

Divisor na protoboard:
```
AO ──[ R1 4,7 kΩ ]──● PONTO DO MEIO ──[ R2 4,7 kΩ ]── GND
                    │
                    └── (por enquanto NÃO ligue no GPIO34)
```
- R1: uma ponta na fileira do **AO**, outra numa fileira livre (o **ponto do meio**).
- R2: uma ponta no **ponto do meio**, outra no **GND**.

## 7.2 Conferir sem energia (multímetro em continuidade)
1. **VIN com GND**: não pode apitar.
2. **AO com GPIO34**: **não pode apitar** (o AO nunca vai direto no ESP32).

## 7.3 Medir a tensão com energia (multímetro em tensão contínua, escala 20 V)
1. Conecte o USB. O LED do módulo MQ135 acende e ele começa a **esquentar** (normal, é o aquecedor interno; não toque na parte metálica).
2. Ponta **preta** no **GND**, ponta **vermelha** no **ponto do meio**: anote o valor. **Precisa estar abaixo de 2,5 V.**
3. Ponta vermelha no **AO**: deve ser aproximadamente o **dobro** do valor anterior.

Se o ponto do meio der **acima de 2,6 V**, **não continue**: desconecte e me mande uma foto da montagem.

## 7.4 Ligar no ESP32
1. **Desconecte o USB.**
2. Ligue um jumper do **ponto do meio** até o **GPIO34 / D34**.

## 7.5 Gravar e testar
1. **File → New**, cole [`firmware/etapa07_mq135/etapa07_mq135.ino`](../../firmware/etapa07_mq135/etapa07_mq135.ino), salve como `etapa07_mq135` e grave.
2. **Serial Monitor** em **115200**.

Esperado (uma linha a cada ~2 s):
```
=== Etapa 7: teste do MQ135 ===
Tempo: 2 s  |  Leitura bruta: 1240  |  GPIO34: 1050 mV  |  Saida AO do MQ135: 2.10 V
Tempo: 4 s  |  Leitura bruta: 1180  |  GPIO34: 1000 mV  |  Saida AO do MQ135: 2.00 V
```
- O valor do **GPIO34** deve ser parecido com o que você mediu no ponto do meio no passo 7.3.
- Nos primeiros minutos o valor **cai devagar** enquanto o sensor aquece. É normal.
- Num sensor novo, as leituras só ficam realmente estáveis depois de **24 a 48 horas** ligado (queima inicial). Para o teste de hoje, alguns minutos bastam.

**Teste de vida (depois de uns 5 minutos):**
- Passe um pouco de **álcool em gel** na mão e aproxime a mão a uns 5 cm do sensor. A tensão deve **subir bem** e depois voltar devagar.
- Ou sopre devagar na direção dele: sobe um pouco (CO₂ e umidade do ar expirado).

## Problemas comuns
| Sintoma | Causa provável | Solução |
|---|---|---|
| GPIO34 sempre ~0 mV | Ponto do meio não chega no GPIO34, ou AO solto | Conferir 7.1 e 7.4 |
| GPIO34 sempre ~3100 mV ou aviso de ATENÇÃO | Divisor errado ou AO direto no ESP32 | **Desconectar na hora** e refazer 7.1–7.3 |
| ESP32 reinicia sozinho | Consumo alto no USB (aquecedor + ventoinha) | Usar outra porta USB do computador, de preferência atrás do gabinete |
| Não reage ao álcool | Sensor ainda frio | Esperar 5–10 minutos e repetir |
