# KiCad K2a: esquemático, parte 1 — colocar as peças na folha

Versão usada: **KiCad 10**. Nesta parte só **colocamos as peças** na folha, sem ligar nada.
As ligações (fios e etiquetas) ficam para a K2b, e a verificação automática (ERC) para a K2c.

## 0. Antes: o nome do projeto
Seu projeto ficou como `placa_estação`. O KiCad aceita, mas **acentos e "ç" no nome costumam dar problema depois**, quando enviamos os arquivos para fabricar (sites de fábrica e arquivos .zip às vezes estragam esses caracteres).
Como a folha ainda está vazia, é a hora mais barata de trocar:

1. No KiCad, **File → Close Project**.
2. **File → New Project** → template **default** → OK → pasta `Documents` → nome **`placa_estacao`** (sem acento e sem ç) → Save.
3. A pasta antiga `Documents/placa_estação` pode ser apagada.

## 1. Abrir o editor e preencher o quadro da folha
1. Clique em **Schematic Editor** (ou dois cliques no `.kicad_sch`).
2. **File → Page Settings…**
   - *Title*: `Estacao de Qualidade do Ar - placa principal`
   - *Revision*: `1`
   - *Date*: clique no botão de data de hoje
   - OK. O quadro no canto inferior direito da folha mostra esses dados.

## 2. Teclas que vamos usar (o mouse precisa estar sobre a folha)
| Tecla | Faz |
|---|---|
| **A** | Adicionar peça (símbolo) |
| **P** | Adicionar símbolo de alimentação (GND, 3V3, 5V) |
| **R** | Girar a peça que está sob o mouse |
| **M** | Mover a peça que está sob o mouse |
| **E** | Editar as propriedades (valor, nome) da peça sob o mouse |
| **Esc** | Cancelar o que está fazendo |
| **Ctrl+S** | Salvar (salve sempre!) |
| Roda do mouse / botão do meio | Zoom / arrastar a folha |

Para colocar uma peça: aperte **A**, digite o nome na busca, escolha, **OK**, e clique na folha onde quer deixá-la. Depois de colocar uma, o KiCad oferece outra igual: aperte **Esc** se não quiser.

## 3. As peças da nossa placa
Coloque uma de cada vez, nesta ordem. Na busca, digite exatamente o que está na coluna "Buscar".
O KiCad dá os nomes (J1, R1, C1…) sozinho, na ordem em que você coloca; se ficar diferente da tabela, não tem problema.

| Nome | Buscar (A) | Quantas | Valor (tecla E → Value) | O que é |
|---|---|---|---|---|
| J1 | `Conn_01x15` (biblioteca *Connector_Generic*) | 1 | `ESP32_esquerda` | barra fêmea do lado esquerdo do ESP32 |
| J2 | `Conn_01x15` (*Connector_Generic*) | 1 | `ESP32_direita` | barra fêmea do lado direito do ESP32 |
| J3 | `Screw_Terminal_01x02` (*Connector*) | 1 | `ENERGIA` | borne da fonte (VIN, GND) |
| J4 | `Screw_Terminal_01x04` (*Connector*) | 1 | `BME280` | borne do BME280 |
| J5 | `Screw_Terminal_01x03` (*Connector*) | 1 | `DHT22` | borne do DHT22 |
| J6 | `Screw_Terminal_01x04` (*Connector*) | 1 | `PMS7003` | borne do PMS7003 |
| J7 | `Screw_Terminal_01x03` (*Connector*) | 1 | `MQ135` | borne do MQ135 |
| R1 | `R` (biblioteca *Device*, a primeira, só "R") | 1 | `4.7k` | pull-up do DHT22 |
| R2, R3 | `R` (*Device*) | 2 | `4.7k` | divisor do MQ135 |
| C1 | `C_Polarized` (*Device*) | 1 | `470uF 16V` | capacitor da entrada de energia |

Por que o ESP32 virou "duas barras de 15 pinos": a placa não leva o ESP32 soldado, leva **as barras fêmea onde ele encaixa**. Então, para o KiCad, o que existe na placa são duas fileiras de 15 furos. Na K2b vamos escrever em cada pino o nome dele (GPIO4, 3V3, GND…).

Dica de arrumação (não é obrigatório, só ajuda a ler):
- J1 e J2 no meio da folha, lado a lado, J1 à esquerda.
- Bornes dos sensores (J4–J7) à direita; energia (J3) e C1 à esquerda, embaixo.
- R1 perto de J5 (DHT22); R2 e R3 perto de J7 (MQ135). Gire os resistores com **R** se quiser deitá-los.

## 4. Símbolos de alimentação (tecla P)
Coloque alguns de cada; na K2b ligamos os fios até eles. Pode colocar mais depois.

| Buscar (P) | Quantos, por enquanto | Para |
|---|---|---|
| `GND` | 4 | o terra comum |
| `+5V` | 2 | a linha VIN (5 V da fonte) |
| `+3V3` | 2 | a saída 3,3 V do ESP32 |

## 5. Salvar e conferir
**Ctrl+S**. Me mande:
- [ ] Um print da folha inteira (aperte **Home** para caber tudo na tela).
- [ ] **Um dado da sua placa ESP32, para a K2b**: olhando o ESP32 por cima, com o **conector USB para baixo**, me diga o que está escrito ao lado dos pinos, **de cima para baixo**, no lado esquerdo e no lado direito (ou mande uma foto nítida da placa de cima onde dê para ler).
  Isso é importante porque existem ESP32 de 30 pinos com a ordem dos pinos diferente; não vamos supor, vamos copiar da sua.
- [ ] **A distância entre as duas fileiras de pinos do ESP32**, medida com régua de centro a centro do pino (costuma ser 22,9 mm ou 25,4 mm). É para a placa encaixar certinho.
- [ ] A ordem dos 4 pinos do seu **BME280** (o que está escrito na plaquinha, por exemplo `VIN GND SCL SDA`).

Com isso, a K2b liga tudo exatamente como está funcionando hoje na protoboard.
