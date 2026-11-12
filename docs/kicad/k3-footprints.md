# KiCad K3: footprints (o desenho real dos furos de cada peça)

Versão: **KiCad 10**. No esquemático cada peça é um símbolo; na placa ela vira um **footprint**: as ilhas de cobre e os furos, no tamanho e na distância reais da peça. Se o footprint estiver errado, a peça não entra nos furos.

## 1. O que comprar (antes de soldar, não antes de desenhar)
| Peça | Quantidade | Observação |
|---|---|---|
| Borne de parafuso **KF301, passo 5,0 mm, 2 vias** | 5 | 1 para a energia + 2 para o BME280 (2+2) + 2 para o PMS7003 (2+2) |
| Borne de parafuso **KF301, passo 5,0 mm, 3 vias** | 2 | DHT22 e MQ135 |
| Capacitor eletrolítico **470 µF / 16 V** (ou 25 V) | 1 (compre 2) | O desenho usa o tamanho mais comum: **8 mm de diâmetro, pernas a 3,5 mm**. Ao comprar, confira e me diga |
| Barra de pinos **fêmea** 1×15, passo 2,54 mm | 2 | Se vocês não tiverem; pode ser uma barra 1×40 cortada |

Os KF301 de 2 vias encaixam lado a lado (têm um trilho na lateral): dois de 2 vias viram um de 4 vias.

## 2. Footprints escolhidos
| Peça | Footprint (biblioteca:nome) | Por quê |
|---|---|---|
| J1, J2 (ESP32) | `Connector_PinSocket_2.54mm:PinSocket_1x15_P2.54mm_Vertical` | barra fêmea de 15 pinos, passo 2,54 mm |
| J3 (energia) | `TerminalBlock_Phoenix:TerminalBlock_Phoenix_MKDS-1,5-2-5.08_1x02_P5.08mm_Horizontal` | borne de 2 vias, passo 5 mm (o KF301 tem a mesma furação) |
| J5, J7 (DHT22, MQ135) | `…MKDS-1,5-3-5.08_1x03_P5.08mm_Horizontal` | borne de 3 vias |
| J4, J6 (BME280, PMS7003) | `…MKDS-1,5-4-5.08_1x04_P5.08mm_Horizontal` | 4 vias = dois KF301 de 2 vias lado a lado |
| R1, R2, R3 | `Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal` | resistor comum de 1/4 W deitado, furos a 10,16 mm |
| C1 | `Capacitor_THT:CP_Radial_D8.0mm_P3.50mm` | eletrolítico em pé, 8 mm, pernas a 3,5 mm |

Sobre o passo 5,0 × 5,08 mm: o KF301 tem 5,0 mm e o footprint 5,08 mm. Em 4 pinos a diferença soma 0,24 mm: as pernas entram sem forçar.

## 3. Usar o arquivo pronto
O arquivo [`hardware/kicad/placa_estacao.kicad_sch`](../../hardware/kicad/placa_estacao.kicad_sch) já vem com todos os footprints preenchidos (e os textos de alimentação arrumados).
1. Feche o KiCad.
2. Substitua o `placa_estacao.kicad_sch` da pasta `Documents/placa_estacao/` por este.
3. Abra pelo AppImage → **Schematic Editor**.

## 4. Conferir no KiCad (é aqui que se aprende)
1. **Tools → Assign Footprints…** (ou o ícone com um chip e um pezinho na barra de cima).
   - Abre uma janela com 3 colunas: bibliotecas à esquerda, **peças no meio** (cada linha mostra `J1 - Esp32_esquerda : Connector_PinSocket…`), footprints à direita.
   - Confira se todas as 12 peças têm um footprint depois dos dois-pontos.
2. Clique numa linha do meio (por exemplo C1) e depois no ícone de **lupa/visualizar footprint** (*View selected footprint*): aparece o desenho das ilhas. Faça isso pelo menos para C1, J4 e J1.
3. Feche com **OK**, rode o **ERC** de novo (**Inspect → Electrical Rules Checker → Run ERC**): deve continuar **0 erros**.
4. **Ctrl+S**.

Se quiser trocar um footprint no futuro (por exemplo, se o capacitor comprado for de 10 mm): passe o mouse na peça, **E**, campo **Footprint**, clique no ícone de livros e escolha outro.

## 5. Me mande
- [ ] Print da janela **Assign Footprints** com as 12 peças.
- [ ] Print do ERC (0 erros).
- [ ] Se já tiver: as **barras fêmea** (têm ou precisam comprar?) e, quando comprar o capacitor, o diâmetro e a distância entre as pernas.

Na **K4** passamos para o **Editor de Placa**: o KiCad traz as peças do esquemático, definimos o tamanho da placa, as regras para placa caseira (trilhas largas, ilhas grandes) e desenhamos as trilhas.
