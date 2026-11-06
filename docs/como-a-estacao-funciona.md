# Como a estação de qualidade do ar funciona

> **Nota:** este texto foi escrito em 07/10/2026, no começo da montagem. Depois dele, o "BMP280" se revelou um **BME280**
> (mede também umidade), o PMS7003 e o MQ135 foram montados e testados, e o programa final ganhou página Wi-Fi e envio ao ThingSpeak.
> A situação atual está no [README](../README.md); a teoria de cada sensor continua valendo.

Este documento explica o funcionamento da estação inteira: o que cada peça mede, como ela mede,
como conversa com o ESP32, como a energia chega a cada parte e o que o programa faz com os dados.

Ele descreve o projeto **completo**, mas nem tudo já foi montado. A situação em 07/10/2026 é:

| Parte | Situação |
|---|---|
| Inventário e mapa de ligações (Etapas 1 e 2) | Concluído |
| ESP32 gravado e testado (Etapa 3) | Concluído |
| DHT22 (Etapa 4) | Montado e funcionando (25,6 °C, 50 %) |
| BMP280 (Etapa 5) | Guia e programa prontos, aguardando o teste |
| PMS7003 | **Ainda vai ser construído** (só planejado no mapa) |
| MQ135 | **Ainda vai ser construído** (só planejado no mapa) |
| Programa final da estação | **Ainda vai ser construído** |

Tudo o que aparece aqui como "planejado" segue o mapa da
[Etapa 2](etapas/etapa-02-mapa-de-ligacoes.md). Os detalhes finais serão definidos e testados, um passo
de cada vez, na montagem.

---

## 1. Visão geral: a estação em uma figura

```
                    ┌────────────────────────────────────────┐
  Ar ambiente ──►   │  SENSORES                              │
                    │                                        │
                    │  PMS7003  partículas (PM1, PM2.5, PM10)│──UART (serial)──┐
                    │  MQ135    gases (índice relativo)      │──analógico──────┤
                    │  DHT22    temperatura e umidade        │──1 fio digital──┤
                    │  BMP280   pressão e temperatura        │──I2C (2 fios)───┤
                    └────────────────────────────────────────┘                 │
                                                                               ▼
                                                                  ┌─────────────────────┐
  Fonte 5 V (ou USB) ──────────────────────────────────────────►  │  ESP32              │
                                                                  │  lê, confere, faz   │
                                                                  │  médias e mostra    │
                                                                  └──────────┬──────────┘
                                                                             ▼
                                                            Monitor Serial (hoje)
                                                            Wi‑Fi / painel (possível depois)
```

A ideia é simples: **quatro sensores** observam o ar, cada um com um tipo de sinal elétrico diferente,
e o **ESP32** é o "cérebro" que conversa com todos, transforma os sinais em números com unidade
(°C, %, hPa, µg/m³) e entrega o resultado.

Cada sensor cobre uma parte do quadro:

- **Poluição por partículas** (fumaça, poeira, fuligem): PMS7003. É a medida principal da estação.
- **Gases** (poluentes, odores, ambiente fechado sem ventilação): MQ135.
- **Condições do ar** (temperatura, umidade, pressão): DHT22 e BMP280. Além de interessantes por si
  só, elas ajudam a **interpretar** as outras medidas (explicado na seção 7).

---

## 2. O ESP32: o cérebro da estação

### O que tem dentro
A placa é uma **ESP32 DevKit de 30 pinos** com o módulo **ESP32‑WROOM‑32** (chip ESP32‑D0WD‑V3).
Dentro dela há:

- **Processador de 2 núcleos** a até 240 MHz. É muito mais rápido do que a estação precisa.
- **4 MB de memória flash**, onde fica gravado o programa. Ele continua lá mesmo sem energia.
- **Wi‑Fi e Bluetooth** embutidos, que permitem, no futuro, mandar os dados pela rede.
- **Periféricos de comunicação** prontos em hardware: 3 portas seriais (UART), I2C, conversor
  analógico‑digital (ADC), entre outros. São eles que "falam" com os sensores.

### Os chips em volta do módulo
- **Chip USB‑serial CH343/CH9102**: faz a ponte entre o cabo USB e o ESP32. É por ele que o
  computador grava o programa e que o Monitor Serial mostra as mensagens. No Linux ele aparece
  como `/dev/ttyACM0`.
- **Regulador de tensão**: recebe 5 V (do USB ou do pino VIN) e gera os **3,3 V** que o ESP32 usa.
  Essa tensão também sai no pino **3V3**, que alimenta o DHT22 e o BMP280.
- **Botões EN e BOOT**: EN reinicia a placa (por isso, apertá‑lo mostra o começo das mensagens no
  Monitor Serial); BOOT é usado na gravação.

### A regra mais importante: o ESP32 trabalha com 3,3 V
Os pinos de entrada do ESP32 suportam **no máximo 3,3 V**. Qualquer sinal de 5 V ligado direto pode
danificar a placa. É por isso que:

- DHT22 e BMP280 são alimentados com **3,3 V** (assim os sinais deles também são de 3,3 V);
- o MQ135, que precisa de 5 V, passa por um **divisor de tensão** antes de chegar ao ESP32;
- o PMS7003, que também precisa de 5 V, já envia seus dados em níveis de 3,3 V, então pode ligar direto.

### Os pinos escolhidos e por quê
| Pino | Uso | Motivo da escolha |
|---|---|---|
| GPIO16 (RX2) / GPIO17 (TX2) | PMS7003 | É a segunda porta serial (UART2), livre. A primeira (GPIO1/3) é usada pelo USB |
| GPIO21 (SDA) / GPIO22 (SCL) | BMP280 | São os pinos padrão do I2C no ESP32 |
| GPIO4 | DHT22 | Pino digital comum, sem função especial na inicialização |
| GPIO34 | MQ135 | Entrada analógica do grupo **ADC1**, que continua funcionando com o Wi‑Fi ligado (o grupo ADC2 para de funcionar quando o Wi‑Fi está ativo) |

Pinos **evitados**: GPIO0, 2, 12 e 15 (decidem como a placa liga e grava; um sensor ali pode travar
a inicialização), GPIO6 a 11 (ligados à memória interna) e GPIO1 e 3 (ocupados pelo USB).

---

## 3. Os sensores, um por um

### 3.1 DHT22: temperatura e umidade *(montado e funcionando)*

**O que mede:** temperatura (‑40 a 80 °C, precisão de cerca de ±0,5 °C) e umidade relativa
(0 a 100 %, precisão de cerca de ±2 a 5 %).

**Como mede:** dentro da caixinha branca há dois elementos:
- um **sensor de umidade capacitivo**: uma camada de material que absorve vapor d'água do ar.
  Quanto mais úmido o ar, mais água essa camada absorve e mais muda sua capacitância (a capacidade de
  guardar carga elétrica);
- um **termistor**: um resistor cuja resistência muda com a temperatura.

Um pequeno microcontrolador dentro do próprio DHT22 lê esses dois elementos, converte em números e
os envia ao ESP32.

**Como conversa com o ESP32: protocolo de 1 fio.** Só existe um fio de dados (perna 2 → GPIO4), e
os dois lados se revezam nele:

1. Em repouso, o fio fica em nível alto (3,3 V), graças ao **resistor de 4,7 kΩ** ligado ao 3V3
   (chamado *pull‑up*). Sem ele o fio ficaria "solto" e a comunicação falharia; por isso a falta do
   resistor é a causa mais comum de `ERRO: falha ao ler o DHT22`.
2. O ESP32 "chama" o sensor puxando o fio para 0 V por cerca de 1 milissegundo e depois solta.
3. O DHT22 responde e envia **40 bits** (zeros e uns). O que diferencia um 0 de um 1 é a **duração**
   de cada pulso: um pulso alto curto (~26 µs) é 0, e um longo (~70 µs) é 1.
4. Os 40 bits são: 16 de umidade, 16 de temperatura e 8 de **verificação** (*checksum*). O ESP32 soma
   os primeiros e confere com a verificação: se não bater, a leitura é descartada.

A biblioteca **DHT sensor library** (Adafruit) faz tudo isso por nós com `dht.readTemperature()` e
`dht.readHumidity()`.

**Limitação importante:** o DHT22 só aceita **uma leitura a cada 2 segundos**. Por isso os programas
esperam 2,5 s entre leituras. Leituras falhas ocasionais (por mau contato ou interferência) são
normais em protoboard e o programa final vai ignorá‑las.

### 3.2 BMP280: pressão e temperatura *(guia pronto, aguardando teste)*

**O que mede:** pressão atmosférica (300 a 1100 hPa, sensível a variações de cerca de 0,12 hPa,
o equivalente a ~1 m de altitude) e temperatura. **Não mede umidade** (quem mede é o BME280, um
"irmão" com chip quadrado; o seu tem chip retangular).

**Como mede:** dentro do chip há uma **membrana microscópica de silício** com resistores gravados
nela (sensor *piezorresistivo*). A pressão do ar deforma a membrana, a deformação muda a resistência
e o chip converte isso em número. O sensor de temperatura interno serve principalmente para
**corrigir** a medida de pressão, que varia com o calor.

Cada BMP280 sai de fábrica com **coeficientes de calibração** gravados na memória do chip. A
biblioteca **Adafruit BMP280** lê esses coeficientes e aplica as fórmulas de correção do fabricante
para entregar a pressão em pascal (o programa divide por 100 para mostrar em hPa).

**Como conversa com o ESP32: barramento I2C.** São só dois fios de dados:
- **SCL** (GPIO22): o "relógio", gerado pelo ESP32, que marca o ritmo de cada bit;
- **SDA** (GPIO21): os dados, que vão e voltam.

O I2C é um **barramento**: vários dispositivos podem dividir os mesmos dois fios, porque cada um tem
um **endereço**. O BMP280 usa **0x76** ou **0x77**. O programa da Etapa 5:
1. "chama" todos os endereços possíveis e lista quem respondeu (o *scanner I2C*);
2. lê o registrador de identificação (0xD0) do chip encontrado: **0x58 = BMP280**, 0x60 = BME280;
3. inicia o sensor e passa a ler pressão e temperatura.

**Para que serve a pressão numa estação de qualidade do ar:** a pressão muda com o tempo
meteorológico (pressão caindo costuma indicar chegada de chuva/frentes; pressão alta e estável
costuma vir com ar parado, que favorece o acúmulo de poluição). Ela também depende da altitude:
cerca de 1013 hPa ao nível do mar, caindo ~12 hPa a cada 100 m.

**Por que a temperatura dele difere da do DHT22:** são sensores diferentes, em lugares diferentes
da protoboard, e o BMP280 fica perto dos fios e da placa, que aquecem um pouco. Diferenças de até
~1 °C são normais.

### 3.3 PMS7003: partículas no ar *(ainda vai ser construído)*

Este é o sensor mais importante da estação, porque **material particulado** (especialmente o PM2.5)
é um dos poluentes mais ligados a efeitos na saúde.

**O que mede:** a concentração de partículas suspensas no ar, em **µg/m³** (microgramas por metro
cúbico), em três faixas de tamanho:
- **PM1.0**: partículas menores que 1 micrômetro (fumaça de combustão, fuligem);
- **PM2.5**: menores que 2,5 µm (fumaça, queimadas, poluição de veículos). Entram fundo nos pulmões;
- **PM10**: menores que 10 µm (poeira, pólen, partículas maiores).

Para comparação, um fio de cabelo tem cerca de 70 µm de espessura.

Ele também informa a **contagem** de partículas por tamanho (quantas partículas acima de 0,3, 0,5,
1, 2,5, 5 e 10 µm existem em 0,1 litro de ar).

**Como mede: espalhamento de laser.**
1. Um **pequeno ventilador** dentro do sensor puxa o ar para uma câmara escura.
2. Um **feixe de laser** atravessa essa câmara.
3. Quando uma partícula passa pelo feixe, ela **espalha a luz**, gerando um pequeno brilho.
4. Um **fotodetector** capta esses brilhos. A quantidade de brilhos indica quantas partículas
   passaram, e a intensidade de cada um indica o tamanho.
5. O processador interno usa essas contagens e um modelo de densidade das partículas para estimar a
   massa em µg/m³.

**Como conversa com o ESP32: serial (UART).** A UART manda bytes em sequência por um fio, num ritmo
combinado (aqui, **9600 bits por segundo**). Os fios são **cruzados**: o que um transmite (TX) o
outro recebe (RX):
- TX da adaptadora → **RX2 (GPIO16)** do ESP32;
- RX da adaptadora → **TX2 (GPIO17)** do ESP32 (usado para mandar comandos ao sensor, como dormir).

No modo normal o PMS7003 envia sozinho, a cada um ou dois segundos aproximadamente, um **pacote de
32 bytes**:
- começa sempre com os bytes **0x42 0x4D** (as letras "BM"), que marcam o início do pacote;
- no meio vêm os valores de PM1.0, PM2.5 e PM10 e as contagens por tamanho;
- termina com uma **soma de verificação**. O programa soma os bytes recebidos e compara: se não
  bater, o pacote veio corrompido e é descartado.

O programa planejado vai ficar "escutando" a GPIO16, achar o início "BM", juntar os 32 bytes,
conferir a soma e extrair os números.

**Alimentação:** o ventilador e o laser precisam de **5 V** (cerca de 100 mA), vindos do pino **VIN**.
Os sinais de dados são de 3,3 V, por isso podem ir direto ao ESP32.

**A placa adaptadora:** o conector original do PMS7003 é minúsculo. A adaptadora tem três conectores:
P1 (onde o sensor encaixa), P2 (cabo branco, não usado) e **P3** (4 pinos na ordem TX, RX, VCC, GND),
que é o que liga ao ESP32. Os pinos SET e RESET não são ligados: o sensor já fica ativo por padrão.

**Cuidados de uso:**
- Depois de ligar, o sensor precisa de **cerca de 30 segundos** para o ventilador estabilizar o fluxo
  de ar. As primeiras leituras devem ser ignoradas.
- O laser tem vida útil limitada (o fabricante indica alguns anos de uso contínuo). Uma opção para o
  programa final é colocar o sensor para **dormir** entre medições, por comando enviado pelo fio RX.
  Isso ainda será decidido.
- As entradas e saídas de ar do sensor não podem ser bloqueadas.

### 3.4 MQ135: gases *(ainda vai ser construído)*

**O que detecta:** o MQ135 reage a vários gases ao mesmo tempo: amônia, óxidos de nitrogênio,
álcool, benzeno, fumaça e, em parte, CO₂. É um bom indicador de "**o ar está ficando pior ou melhor**",
por exemplo num ambiente fechado com muitas pessoas, cheiro de solvente ou fumaça.

**Como detecta: semicondutor aquecido.**
1. Dentro da cápsula metálica há uma camada de **dióxido de estanho (SnO₂)** e um **aquecedor**.
2. O aquecedor mantém o SnO₂ quente (por isso o sensor fica morno; é normal).
3. Em ar limpo, o SnO₂ quente tem **resistência alta**. Quando aparecem gases como os citados,
   eles reagem com o oxigênio na superfície do material e a **resistência cai**.
4. No módulo, o sensor forma um divisor com um resistor fixo da plaquinha (o *resistor de carga*).
   Quando a resistência do sensor cai, a **tensão na saída AO sobe**. Ou seja:
   **mais gás → tensão maior em AO**.

**Como conversa com o ESP32: sinal analógico.** Não há "mensagem" aqui: é uma tensão contínua, entre
0 e quase 5 V. O ESP32 a mede com seu **conversor analógico‑digital (ADC)** na GPIO34, que
transforma a tensão num número de **0 a 4095** (12 bits). O programa vai usar a função
`analogReadMilliVolts()`, que já aplica a calibração interna do ESP32 e devolve a tensão em milivolts.

**O divisor de tensão (proteção do ESP32):**

```
AO do MQ135 ──[ 4,7 kΩ ]──┬──[ 4,7 kΩ ]── GND
                          │
                          └──> GPIO34 do ESP32
```

Com dois resistores iguais, a tensão no meio é **exatamente a metade** da tensão em AO. Se AO chegar
a 5 V, a GPIO34 recebe 2,5 V, abaixo do limite de 3,3 V. O programa multiplica a leitura por 2 para
saber a tensão real em AO.

A fórmula geral do divisor é: `V_saída = V_entrada × R2 / (R1 + R2)` = `V × 4,7 / (4,7 + 4,7)` = `V / 2`.

**O que o número significa (e o que não significa):** o MQ135 **não mede um gás específico com
precisão**. Ele não sabe se a mudança veio de álcool ou de amônia, e a leitura também muda com a
temperatura e a umidade. Por isso, o plano é tratá‑lo assim:
- **Índice relativo**: comparar a leitura atual com a leitura em **ar limpo** (feita ao ar livre ou
  com a janela aberta). Essa leitura de referência, chamada de **R0**, será registrada numa etapa de
  calibração.
- Estimativas em "ppm" são possíveis com as curvas do fabricante, mas devem ser vistas como
  aproximadas. A calibração em ar limpo também compensa o pequeno efeito do divisor sobre a saída do
  módulo.

**Cuidados de uso:**
- **Queima inicial (*burn‑in*)**: na primeira vez, o sensor deve ficar **24 a 48 horas ligado** para
  estabilizar.
- A cada vez que é ligado, precisa de alguns minutos de aquecimento antes de as leituras ficarem
  estáveis.
- O aquecedor consome cerca de **150 mA a 5 V** e esquenta. Na caixa final, ele deve ficar **longe do
  DHT22 e do BMP280**, para não aquecer os sensores de temperatura.
- O pino **DO** (saída digital ligada a um limite ajustado no potenciômetro do módulo) não será usado.

---

## 4. A energia: de onde vem e para onde vai

```
   USB do computador (testes)          Fonte 5 V / 3 A (estação final)
             │                                    │
             │      NUNCA OS DOIS AO MESMO TEMPO   │
             └───────────────┬────────────────────┘
                             ▼
                     ┌───────────────┐
                     │   pino VIN    │ ~5 V
                     └──┬─────┬──────┘
                        │     ├────────────► PMS7003 (ventilador + laser)  ~100 mA
                        │     └────────────► MQ135 (aquecedor)             ~150 mA
                        ▼
              regulador da placa
                        │ 3,3 V
                        ├────────────► o próprio ESP32 (com Wi‑Fi: picos de ~250 mA)
                        └── pino 3V3 ─► DHT22 + resistor 4,7 kΩ  (~1,5 mA)
                                      └► BMP280                   (menos de 1 mA)

          GND: todos os GND ligados juntos (terra comum)
```

**Consumo total estimado:** abaixo de 0,6 A nos picos. A fonte de 3 A tem folga de sobra.

**Por que o terra comum é obrigatório:** tensão é sempre medida "em relação a" alguma referência.
Se o GND do sensor não estiver ligado ao GND do ESP32, os dois não concordam sobre o que é "0 V", e
os sinais viram ruído sem sentido (ou pior, correntes por caminhos errados).

**Por que nunca USB e fonte juntos:** seriam duas fontes de 5 V ligadas uma contra a outra. Pequenas
diferenças de tensão entre elas fazem corrente circular entre as duas, o que pode danificar a porta
USB do computador ou a placa.

---

## 5. Os quatro "idiomas" de comunicação, lado a lado

| Sensor | Tipo de sinal | Fios de dados | Como o ESP32 sabe que a leitura é confiável |
|---|---|---|---|
| DHT22 | Digital, 1 fio, temporizado | 1 (GPIO4) | Byte de verificação no fim dos 40 bits |
| BMP280 | Digital, I2C | 2 (GPIO21, GPIO22) | Endereço e ID do chip conferidos; valores dentro da faixa |
| PMS7003 | Digital, serial (UART 9600) | 2 (GPIO16, GPIO17) | Início "BM" + soma de verificação no fim do pacote |
| MQ135 | Analógico (tensão) | 1 (GPIO34) | Não há verificação; o programa faz **média de várias leituras** para reduzir ruído |

Usar um tipo diferente para cada sensor não foi uma escolha nossa: é como cada fabricante projetou
o seu. O papel do ESP32 é justamente "falar todas essas línguas".

---

## 6. O programa: como o software funciona

### O que já existe
Cada etapa tem um programa de teste (em [`firmware/`](../firmware/)) que liga **só os sensores já
montados** e imprime os valores no Monitor Serial:

- `etapa03_teste_esp32`: mostra os dados do chip e pisca o LED azul (GPIO2). Prova que a placa e a
  gravação funcionam.
- `etapa04_dht22`: lê temperatura e umidade a cada 2,5 s.
- `etapa05_bmp280`: procura o BMP280 no I2C, confirma o tipo de chip e mostra BMP280 e DHT22 juntos.

Todo programa Arduino tem duas partes:
- **`setup()`**: roda **uma vez** quando a placa liga ou o botão EN é apertado. Inicia a porta
  serial, os barramentos e os sensores.
- **`loop()`**: roda **para sempre, repetidamente**. É onde ficam as leituras.

As **bibliotecas** (DHT sensor library, Adafruit BMP280, etc.) são códigos prontos que cuidam dos
detalhes de cada protocolo. Elas são instaladas pela IDE e incluídas com `#include`.

### O programa final *(ainda vai ser construído)*
O plano é juntar os testes num programa só, que funcionará mais ou menos assim:

```
setup():
  iniciar Monitor Serial (115200)
  iniciar UART2 para o PMS7003 (9600, GPIO16/17)
  iniciar I2C para o BMP280 (GPIO21/22)
  iniciar DHT22 (GPIO4)
  configurar GPIO34 como entrada analógica do MQ135
  esperar o aquecimento do PMS7003 (~30 s) e informar que o MQ135 ainda está esquentando

loop() — repete sem parar:
  - receber e conferir pacotes do PMS7003 assim que chegarem
  - a cada intervalo de leitura (ex.: alguns segundos):
      ler DHT22 (descartar se falhar)
      ler BMP280
      ler MQ135 várias vezes e tirar a média
  - a cada intervalo de relatório (ex.: 1 minuto):
      calcular médias do período
      mostrar tudo no Monitor Serial (e, no futuro, enviar pela rede)
```

Pontos importantes desse desenho:
- **Sem `delay()` longos travando tudo**: em vez de "esperar 2,5 s parado", o programa olha o relógio
  interno (`millis()`) e faz cada tarefa quando chega a hora dela. Assim ele não perde pacotes do
  PMS7003 enquanto espera o DHT22.
- **Médias**: leituras individuais oscilam. Médias de 1 minuto são mais estáveis e mais úteis.
- **Leituras inválidas descartadas**: pacote com soma errada, DHT22 com erro, valores fora da faixa
  física possível.

O que **ainda não foi decidido** e será combinado nas próximas etapas: se os dados serão enviados
por Wi‑Fi (e para onde: um painel na rede local, um serviço na internet, uma planilha), se serão
gravados num cartão/memória, e o intervalo exato das medições.

---

## 7. Como as medidas se ajudam

As medidas não são independentes; umas ajudam a entender as outras:

- **Umidade × partículas:** com umidade alta (acima de ~70 a 80 %), as partículas absorvem água e
  "incham". O PMS7003 vê partículas maiores e pode **superestimar** o PM2.5 e o PM10 (em neblina,
  por exemplo). A umidade do DHT22 permite marcar essas leituras como menos confiáveis.
- **Temperatura e umidade × MQ135:** a resistência do MQ135 também varia com o calor e a umidade.
  As leituras do DHT22 podem ser usadas para corrigir o índice de gases.
- **Pressão × tempo:** pressão alta e estável costuma vir com pouco vento e ar parado, situação em
  que a poluição se acumula. A pressão ajuda a explicar por que o PM2.5 subiu num dia sem nenhuma
  fonte nova de poluição.
- **Dois sensores de temperatura:** DHT22 e BMP280 servem para conferir um ao outro. Se um deles
  começar a divergir muito, há algo errado (mau contato ou aquecimento perto dele).

---

## 8. Como interpretar os números

### Partículas (PMS7003)
Valores de referência da **Organização Mundial da Saúde (diretrizes de 2021)**:

| Poluente | Média de 24 horas | Média anual |
|---|---|---|
| PM2.5 | até **15 µg/m³** | até **5 µg/m³** |
| PM10 | até **45 µg/m³** | até **15 µg/m³** |

Essas referências valem para **médias longas** (um dia, um ano). Um pico de alguns minutos (alguém
cozinhando, passando um carro) é normal e não significa, sozinho, que o ar é ruim. Sensores de baixo
custo como o PMS7003 são ótimos para ver **tendências e comparações**, mas não têm a exatidão de uma
estação oficial.

### Gases (MQ135)
Leia como **comparação com o ar limpo**: perto do valor de referência, o ar está bom; subindo muito,
há algo no ar (fumaça, solvente, ambiente fechado sem ventilação).

### Temperatura, umidade e pressão
- Conforto em ambiente interno: em geral **20 a 26 °C** e **40 a 60 %** de umidade.
- Pressão: compare com ela mesma ao longo do tempo; o valor absoluto depende da altitude do local.

---

## 9. Resumo do caminho até a estação completa

1. ✓ Inventário das peças
2. ✓ Mapa de ligações
3. ✓ IDE configurada e ESP32 testado
4. ✓ DHT22 funcionando
5. ⧗ BMP280 (guia pronto, aguardando teste)
6. ○ PMS7003 (partículas)
7. ○ MQ135 (gases, com divisor de tensão, queima inicial e calibração em ar limpo)
8. ○ Programa final juntando tudo
9. ○ Alimentação pela fonte de 5 V, caixa e posicionamento dos sensores
10. ○ Envio dos dados (a definir)

A ordem e os detalhes das etapas 6 em diante são conduzidos na thread "Montagem passo a passo da
estação", sempre testando cada peça antes de acrescentar a próxima.
