# Projeto da estrutura física: estação ao ar livre (rascunho 2)

Desenho da versão externa, ajustado para quem tem **impressora 3D** e **sabe soldar** (seções 9 e 10).
As peças de loja continuam onde são melhores que peças impressas (a caixa vedada e os joelhos de PVC).

## 1. Visão geral: três partes separadas

```
            parede ou poste (face SUL, na sombra)
            │
            │   ┌─────────────────────┐
            ├───┤ A. CAIXA VEDADA     │  ESP32 + placa soldada
            │   │   (eletrônica)      │  (só cabos entram, por prensa-cabos)
            │   └──┬──────────┬───────┘
            │      │ cabo     │ cabo
            │   ┌──┴───┐   ┌──┴──────────┐
            ├───┤ B.   │   │ C. "U" DE   │  PMS7003 + MQ135
            │   │ABRIGO│   │   PVC       │  (aberturas para BAIXO)
            │   │PRATOS│   │             │
            │   └──────┘   └─────────────┘
            │   BME280 + DHT22
            │   (sombra e ventilação)
```

Por que separar:
- **A (eletrônica)** precisa ficar **seca**: caixa fechada.
- **B (temperatura/umidade)** precisa de **sombra e ar livre**, e longe de qualquer coisa que esquente (ESP32, MQ135, sol).
- **C (partículas e gases)** precisa de **ar de fora**, mas sem chuva entrando: aberturas viradas para baixo.

---

## 2. Parte A: caixa vedada da eletrônica
- **Caixa de passagem de PVC com vedação (IP65)**, cerca de **150 × 110 × 70 mm**.
- **Prensa-cabos** (PG7 ou PG9) para cada cabo que entra/sai: energia, abrigo de pratos, tubo de PVC. Prensa-cabo **por baixo** da caixa (a água escorre e não entra).
- Dentro: ESP32 na **placa soldada** (perfurada), fixada com espaçadores.
- **Um furo de 3 mm na parte mais baixa** da caixa (dreno contra condensação), ou um respiro com membrana.
- Plástico **não bloqueia** o Wi-Fi: a rede EstacaoAr continua alcançando ~20–40 m.

## 3. Parte B: abrigo de radiação (BME280 + DHT22)
É o abrigo clássico de estação meteorológica, feito com pratos de vaso:
- **6 a 8 pratos de vaso de plástico BRANCO**, ~15–20 cm de diâmetro (branco reflete o sol; outras cores esquentam e falseiam a temperatura).
- **3 barras roscadas M5 ou M6** (~25 cm) com porcas e **espaçadores de ~2 cm** entre os pratos, deixando o ar passar pelos lados.
- Prato de cima **inteiro** (teto); os outros com o **centro recortado** (furo de ~8 cm) para o ar circular.
- BME280 e DHT22 presos no meio da pilha, **sem encostar** nos pratos.
- Cabo de **4 vias** para o BME280 (até ~1 m funciona bem) e **3 vias** para o DHT22, entrando na caixa A pelo prensa-cabo.
- 🖨️ Com impressora 3D: existem modelos prontos de "Stevenson screen / radiation shield" para imprimir em PETG ou ASA **branco**.

## 4. Parte C: "U" de PVC (PMS7003 + MQ135)
Desenho usado pelo projeto aberto **Sensor.Community** (milhares de estações no mundo):
- **2 joelhos (curvas) de PVC de esgoto de 75 mm**, encaixados formando um **U invertido**, com as **duas aberturas para baixo**.
- O ar entra por baixo; a chuva não.
- Tela de mosquiteiro nas duas aberturas (contra insetos).
- **PMS7003** preso dentro de um lado, com a entrada e a saída de ar livres. **MQ135** do outro lado.
- O calor do MQ135 fica aqui, **longe do abrigo B**.
- 🖨️ Com impressora 3D: dá para imprimir um suporte interno para o PMS7003 e o MQ135.

## 5. Energia
O ponto mais delicado da versão externa é a **queda de tensão no cabo**:
- Cabo USB comprido e fino **não serve**: um cabo USB comum de 3 m pode perder ~0,6 V, e o ESP32 começa a reiniciar.
- **Opção recomendada:** fonte USB **dentro de casa/laboratório** + cabo **paralelo 2 × 0,75 mm²** (ou mais grosso) até a caixa A, ligado no **VIN e GND** (com a polaridade conferida no multímetro, como na Etapa 10).
  Até **10 m** a perda fica em ~0,2 V, aceitável.
- Se a fonte precisar ficar do lado de fora, ela tem que ficar **dentro de uma caixa vedada** também.

## 6. Onde instalar
- Altura de **1,5 a 3 m** do chão, **afastada pelo menos 1 m da parede** (use um braço/suporte), longe de exaustores, chaminés e da rua imediata (a não ser que seja isso que se quer medir).
- Face **SUL** da construção (no Brasil o sol passa pelo norte, então o lado sul fica mais tempo na sombra).
- Nada de chão de asfalto ou telha logo abaixo (esquentam e sobem ar quente).

## 7. Ajustes de software previstos para a versão externa
- **Aviso de umidade alta:** acima de ~85 % de umidade o PMS7003 **superestima** as partículas (as gotículas de água também são contadas). O programa vai marcar essas leituras.
- **Calibração do MQ135** (Etapa 11), depois da queima de fim de semana.

## 8. Lista de compras (estimativa)
| Item | Qtd |
|---|---|
| Caixa de passagem PVC IP65 ~150×110×70 mm | 1 |
| Prensa-cabos PG7/PG9 | 4 |
| Pratos de vaso plástico **brancos** 15–20 cm | 6–8 |
| Barra roscada M5/M6 + porcas + espaçadores | 3 barras |
| Joelho PVC esgoto 75 mm (87° ou 90°) | 2 |
| Tela mosquiteiro (retalho) | 1 |
| Placa perfurada (ilhada) ~7 × 9 cm | 1 |
| Barras de pinos fêmea (para encaixar o ESP32, sem soldar nele) | 2 × 15 pinos |
| Cabo 4 vias (~1 m) e 3 vias (~1 m), tipo manga ou cabo de alarme | 1 de cada |
| Cabo paralelo 2 × 0,75 mm² (comprimento até a tomada) | conforme o local |
| Suporte/braço de fixação, abraçadeiras, parafusos inox | — |

## 9. O que imprimir em 3D
**Material: PETG ou ASA, na cor BRANCA.** Nunca PLA ao ar livre: ele amolece ao sol (~55–60 °C) e racha com o UV.
Sugestão de impressão: 3–4 perímetros, 20–30 % de preenchimento.

| Peça | Imprimir? | Por quê |
|---|---|---|
| **B. Abrigo de radiação** | **Sim** (substitui os pratos de vaso) | Fica mais leve e regular. Procure no Printables/Thingiverse por "radiation shield" ou "Stevenson screen" (anéis de ~120 mm, 6–8 andares). Use as 3 barras roscadas M5 para montar a pilha. |
| Suporte do BME280 + DHT22 dentro do abrigo | **Sim** | Mantém os dois sensores no centro, sem encostar nas paredes. |
| **C. Suporte interno do PMS7003 e MQ135** | **Sim** | Encaixa dentro do U de PVC; o PMS7003 mede 50 × 38 × 21 mm, deixar a entrada e a saída de ar livres. |
| U de PVC 75 mm | **Não** (comprar) | Barato, resistente ao sol e à chuva por anos. |
| **A. Caixa da eletrônica** | **Não** (comprar IP65) | Caixa impressa raramente fica estanque de verdade. |
| Placa de montagem dentro da caixa A | **Sim** | Base para parafusar a placa soldada com espaçadores. |

## 10. Placa soldada (substitui a protoboard)
A protoboard **não serve ao ar livre**: os contatos oxidam e soltam com a vibração e a umidade. A versão final usa uma placa perfurada soldada:
- **ESP32 encaixado em barras de pinos fêmea** (não soldar o ESP32 direto): se ele queimar, troca sem dessoldar nada.
- Ficam **na placa**: o resistor **4,7 k de pull-up do DHT22** e o **divisor 4,7 k / 4,7 k do MQ135**, exatamente como já testamos na protoboard.
- Cada cabo externo chega num **borne de parafuso** (KF301, passo 5 mm) ou conector JST-XH, para desmontar sem solda:
  - Energia: 2 vias (VIN, GND)
  - BME280: 4 vias (3V3, GND, SDA, SCL)
  - DHT22: 3 vias (3V3, GND, dados)
  - PMS7003: 4 vias (VIN, GND, TX, RX)
  - MQ135: 3 vias (VIN, GND, AO)
- **Capacitor de 470 µF / 16 V** entre VIN e GND, perto da entrada de energia: segura os picos do Wi-Fi e da ventoinha do PMS, que ficam piores com cabo comprido.
- Depois de soldar: conferir **cada ligação com o multímetro (continuidade)** e procurar curto entre VIN, 3V3 e GND **antes** de encaixar o ESP32.

Itens a mais na lista de compras: filamento PETG/ASA branco; bornes KF301 de 2 e 3 vias (ou JST-XH); capacitor eletrolítico 470 µF 16 V; estanho e espaçadores M3.

## Próximos passos
1. **Desenho da placa soldada, furo a furo** (onde vai cada fio e componente), para conferir antes de soldar.
2. Escolha e ajuste dos modelos para imprimir (abrigo e suportes).
3. Etapa 11: calibração do MQ135, depois da queima do fim de semana.
