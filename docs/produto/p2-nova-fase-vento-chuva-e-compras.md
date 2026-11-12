# P2: nova fase — placa expansora, vento, direção do vento e chuva (plano e compras)

Decisões da equipe (12/11/2026):
- **A placa de circuito própria (KiCad) fica de lado por enquanto.** O produto inicial usa uma **placa expansora (shield) para ESP32**.
- Entram **três medições novas**: **velocidade do vento** (anemômetro), **direção do vento** (cata-vento) e **chuva** (pluviômetro de báscula).
- Tudo continua no **ESP32** (nada de Arduino). As peças mecânicas saem da **impressora 3D**.

---

## 1. Placa expansora: atenção ao número de pinos ⚠️
A placa da foto é para ESP32 de **38 pinos** ("ESP32S 38P"). **O nosso ESP32 é de 30 pinos.** Se encaixar um de 30 num soquete de 38, sobram 4 furos de cada lado e **os nomes impressos na placa ficam errados**: é fácil ligar um sensor no pino errado.

**Compre a versão de 30 pinos** (procure por "placa expansora ESP32 30 pinos" / "ESP32 30P expansion board"). Ela é igual: soquete fêmea, uma fileira **G‑V‑S** (GND, alimentação, sinal) para cada pino, regulador, jack DC e conector USB.

Antes de comprar, meça no nosso ESP32 a **distância entre as duas fileiras de pinos** (centro a centro). Costuma ser **25,4 mm** ou **22,9 mm**, e o anúncio tem que dizer que aceita essa largura.

### Cuidados com a placa expansora
1. **Jumper 5 V / 3,3 V da fileira V:** em muitas dessas placas um jumper escolhe se a fileira "V" dá **5 V ou 3,3 V**. Os sensores que vão no 3,3 V (BME280, DHT22, AS5600, sensores hall) **não podem** receber 5 V na linha de sinal. Vamos usar o jumper em **3,3 V** e tirar os 5 V (PMS7003, MQ135) dos pinos **5V/VIN** da placa. Isso vai ser conferido com o multímetro antes de ligar qualquer sensor.
2. **Entrada de energia:** o jack DC dessas placas geralmente aceita **6,5 a 12 V** (passa por regulador). Se a fonte de 5 V entrar pelo jack, pode faltar tensão. Com a nossa fonte 5 V USB, entramos pelo **conector USB da placa** ou direto no pino 5V. Confirmar no anúncio da placa comprada.
3. **Resistores que estavam na protoboard** (pull-up 4,7 k do DHT22, divisor 4,7 k/4,7 k do MQ135 e os novos pull-ups) não têm lugar na expansora. Eles vão numa **plaquinha perfurada pequena** (5 × 7 cm) ou soldados **no próprio cabo**, protegidos com termo-retrátil.
4. **Jumpers Dupont soltam** com vibração e umidade. Dentro da caixa IP65 dá para usar, mas com **carcaça de conector com trava** (ou um ponto de cola quente) em cada um.

---

## 2. Anemômetro (velocidade do vento)
Como a equipe propôs: **conchas e estrutura impressas**, **ímã no eixo** e **sensor de efeito hall** contando as voltas.

### Ajuste importante no sensor hall
O **KY-003** usa o chip **A3144**, que pela folha de dados precisa de **4,5 V ou mais**. No 3,3 V do ESP32 ele pode funcionar mal. Se for alimentado em 5 V, o resistor de pull-up do módulo joga **5 V no pino do ESP32**, que aguenta só 3,3 V.

**Solução recomendada:** usar o **A3144 avulso** (o chip de 3 pernas, sem o módulo):
- VCC do A3144 → **5 V**; GND → GND;
- saída do A3144 → pino do ESP32 + **resistor 4,7 k para 3,3 V** (já temos 4,7 k).
A saída do A3144 é "coletor aberto": ela só puxa para o GND, então com o pull-up no 3,3 V o ESP32 nunca vê mais de 3,3 V. Se já tiverem o KY-003, dá para tirar o resistor do módulo com o ferro de solda e ligar do mesmo jeito.

### Medição e cálculo
- **1 ou 2 ímãs** de neodímio 6 × 3 mm no eixo (2 ímãs = 2 pulsos por volta, melhor em vento fraco).
- O ESP32 conta os pulsos por **interrupção** e calcula a cada 3 s:
  `voltas/s = pulsos / (ímãs × 3 s)` → `vento (m/s) = K × voltas/s`
- Guardamos a **média de 10 min** (padrão meteorológico) e a **rajada** (maior valor de 3 s).
- **K precisa ser calibrado**, porque depende do tamanho das conchas. Duas formas:
  1. **Anemômetro portátil** de referência (~R$ 60–100) lado a lado no vento (mais fácil);
  2. **Carro** em dia sem vento, anemômetro para fora da janela, a 10, 20, 30 e 40 km/h.
- **Rolamentos 608ZZ** (de skate) no eixo: **lavar e tirar a graxa** de fábrica, senão a concha não gira com vento fraco.

---

## 3. Direção do vento (o que faltava)
**Recomendação: sensor magnético de ângulo AS5600** com um **ímã diametral** na ponta do eixo do cata-vento.
- Mede o ângulo **sem contato** (nada gasta), com resolução muito fina (0,1°), e funciona em 3,3 V.
- O módulo tem uma **saída analógica (OUT)** proporcional ao ângulo: vamos ler essa saída num pino **ADC1** do ESP32. A leitura analógica aguenta bem o cabo de 1–3 m até o alto do mastro. O I2C nessa distância costuma dar erro.
- Na instalação, gira-se o cata-vento para o **norte** (bússola do celular) e o programa grava esse ângulo como zero.
- Mostramos em graus e também em **16 direções** (N, NNE, NE…).

Alternativa mais "clássica" (usada em estações comerciais): **8 reed switches + 8 resistores** em círculo, lidos por um único pino analógico. Dá 8 ou 16 direções, mas são muito mais soldas e peças. **O AS5600 é mais simples e mais preciso.**

O cata-vento (aleta, contrapeso, eixo) é impresso e usa os mesmos **rolamentos 608ZZ**.

---

## 4. Pluviômetro de báscula (chuva)
Como a equipe propôs: **funil + gangorra (báscula) impressos**, um pulso a cada tombo.

### Sensor do tombo
O **sensor de inclinação de bolinha (SW-520D)** funciona, mas a bolinha **quica** e gera vários pulsos falsos em cada tombo, e oxida com a umidade.
**Recomendação: usar o mesmo sistema do anemômetro**: um **ímã na báscula** e um **A3144** (ou um reed switch) fixo na base. Cada tombo passa o ímã na frente do sensor: um pulso limpo, sem contato. Assim a lista de compras fica menor (mesma peça nos dois).

### Dimensionamento (para o desenho 3D)
- Resolução usual: **0,2 mm de chuva por tombo**.
- Volume por tombo = 0,2 mm × área do funil. Com funil de **Ø 160 mm** (área 201 cm²): **≈ 4,0 mL por tombo**. Com **Ø 110 mm** (95 cm²): **≈ 1,9 mL**.
- A báscula é **calibrada com uma seringa**: pingar devagar e ajustar o parafuso batente até tombar no volume certo. Conferência final: 100 mL derramados devagar = número de tombos esperado.
- Tela no funil (folhas e insetos) e **furos de dreno** embaixo da báscula.

---

## 5. Pinos do ESP32 (proposta)
| Sensor | Pino | Observação |
|---|---|---|
| DHT22 | GPIO4 | já em uso |
| PMS7003 | GPIO16 / GPIO17 | já em uso |
| BME280 | GPIO21 / GPIO22 (I2C) | já em uso |
| MQ135 | GPIO34 | já em uso (ADC1) |
| **Anemômetro (A3144)** | **GPIO27** | interrupção, pull-up 4,7 k para 3,3 V |
| **Pluviômetro (A3144)** | **GPIO26** | interrupção, pull-up 4,7 k para 3,3 V |
| **Direção (AS5600 OUT)** | **GPIO35** | ADC1 (o ADC2 não funciona com Wi-Fi ligado) |

## 6. ThingSpeak: os 8 campos já estão ocupados
O canal atual usa os **8 campos** (PM1, PM2.5, PM10, temperatura, umidade, pressão, DHT22, MQ135). A conta gratuita permite **4 canais**, então a saída é criar um **segundo canal "Vento e chuva"**: velocidade média, rajada, direção, chuva (mm na hora) e chuva acumulada no dia. O firmware envia para os dois. A página Wi-Fi própria mostra tudo junto.

## 7. Onde instalar cada parte
- **Anemômetro e cata-vento:** no **alto de um mastro**, acima de telhados e obstáculos próximos. O padrão oficial é 10 m, mas para o nosso uso basta ficar **o mais livre possível**, pelo menos ~1 m acima de qualquer coisa num raio de alguns metros.
- **Pluviômetro:** **nivelado** (bolha de nível), boca do funil livre. Distância de paredes e árvores de pelo menos **2× a altura** delas. Nada de beiral pingando dentro.
- **Abrigo do BME280/DHT22 e o "U" de PVC:** continuam como no desenho da estrutura externa (1,5–3 m, sombra).

---

## 8. Lista de compras da nova fase
Preços aproximados de Mercado Livre/lojas de eletrônica (novembro/2026), só para ter uma ideia.

### Eletrônica
| Item | Qtd | ~Preço | Obs. |
|---|---|---|---|
| **Placa expansora ESP32 de 30 pinos** | 1 (+1 reserva) | R$ 25–35 cada | conferir a largura entre fileiras |
| **Sensor hall A3144** (avulso, 3 pernas) | 5 | R$ 10 o pacote | 2 em uso + reservas |
| **Módulo AS5600** com ímã diametral | 2 | R$ 20–30 cada | 1 em uso + 1 reserva; conferir que vem o ímã |
| **Ímãs de neodímio** 6 × 3 mm | 10 | R$ 15 | anemômetro e báscula |
| Placa perfurada 5 × 7 cm | 2 | R$ 10 | resistores e emendas |
| Carcaças de conector Dupont com trava (2, 3 e 4 vias) | 1 kit | R$ 20 | ou conectores JST-XH |
| Cabo manga blindado 4 vias (alarme) | 10 m | R$ 30 | mastro → caixa (anemômetro, direção, chuva) |
| Termo-retrátil sortido | 1 kit | R$ 15 | |
| Sensor de inclinação SW-520D | 2 | R$ 5 | **só se** quiserem testar a ideia original da báscula |

### Mecânica
| Item | Qtd | ~Preço | Obs. |
|---|---|---|---|
| **Rolamento 608ZZ** (skate) | 6 | R$ 20 | 2 anemômetro, 2 cata-vento, 2 reserva |
| **Barra roscada inox M8** ou eixo inox Ø 8 mm | 1 m | R$ 25 | eixos (cortar) |
| Porcas e arruelas inox M8 | 1 pacote | R$ 15 | |
| Parafusos inox M3 (10–20 mm) + porcas | 1 kit | R$ 25 | fixar peças impressas e placas |
| Insertos roscados M3 de latão (para plástico, aplicados com ferro de solda) | 30 | R$ 20 | rosca firme nas peças impressas |
| **Mastro**: tubo galvanizado ou eletroduto de 3/4" a 1", 1,5–2 m | 1 | R$ 40–80 | onde ficam vento e direção |
| Abraçadeiras tipo "U" para tubo | 2–4 | R$ 20 | prender mastro e peças |
| Nível de bolha pequeno | 1 | R$ 10 | nivelar o pluviômetro |
| **Filamento PETG ou ASA branco** | 2 kg | R$ 100–140/kg | abrigo, conchas, aleta, funil, báscula, suportes |

### Calibração
| Item | Qtd | ~Preço | Obs. |
|---|---|---|---|
| Anemômetro portátil digital | 1 | R$ 60–100 | referência para o K (ou usar o método do carro) |
| Seringa de 10 mL | 2 | R$ 5 | calibrar a báscula |

### Continua valendo da lista anterior
Caixa IP65 (o tamanho vai ser recalculado para a placa expansora, que é menor que os 100 × 75 mm da placa própria), prensa-cabos PG7/PG9 (agora **5 ou 6**: energia, abrigo, "U", e o cabo do mastro), joelhos de PVC 75 mm, tela mosquiteiro, cabo 2 × 0,75 mm² para a energia, capacitor 470 µF/16 V.

**Itens que não precisam ser comprados:** resistores 4,7 k (já temos), multímetro, fonte 5 V 3 A.

---

## 9. Ordem de trabalho proposta
1. **Comprar** (lista acima) e terminar a **P1** (medidas).
2. **Bancada, um sensor por vez** (como fizemos nas Etapas 4–7): A3144 com ímã na mão → AS5600 girando o ímã → expansora com os sensores antigos.
3. **Desenho 3D** (OpenSCAD): anemômetro → cata-vento → pluviômetro → abrigo → suportes. Primeiro uma **peça de teste** de cada encaixe (rolamento, ímã, sensor).
4. **Calibração**: K do anemômetro, mL por tombo da báscula, norte do cata-vento.
5. Firmware com as três medições novas e o segundo canal do ThingSpeak.
6. Montagem e teste de 48 h na bancada, depois instalação.
