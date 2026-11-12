# KiCad K4a: placa, parte 1 — regras, contorno e posição das peças

Versão: **KiCad 10**. Agora saímos do esquemático e vamos para o **Editor de Placa** (*PCB Editor*).
Nesta parte: configurar as regras de placa caseira, trazer as peças, desenhar o contorno e posicionar.
As **trilhas** ficam para a K4b.

## 0. Antes de começar
- Abra o projeto pelo AppImage e clique em **PCB Editor** (fundo escuro).
- Unidade em **milímetros**: na barra da esquerda, o botão `mm` / `mils` deve estar em **mm**.
- Teclas novas: **M** move, **R** gira a peça sob o mouse, **F** vira a peça para o outro lado (não use agora), **Alt+3** abre a vista 3D, **Ctrl+S** salva.

## 1. Regras de placa caseira (File → Board Setup…)
Estas regras fazem o KiCad **proibir** trilhas finas demais ou coladas demais, que o toner e o percloreto não conseguem fazer.

### 1.1 Design Rules → Constraints
| Campo | Valor |
|---|---|
| Minimum clearance | **0.5 mm** |
| Minimum track width | **0.8 mm** |
| Minimum annular width | **0.4 mm** |
| Minimum through hole | **0.8 mm** |
| Copper to edge clearance | **1 mm** |

### 1.2 Design Rules → Net Classes
1. Na linha **Default**: *Clearance* **0.5**, *Track Width* **1.0**.
2. Clique no **+** para criar uma classe nova chamada **`Energia`**: *Clearance* **0.5**, *Track Width* **2.0**.
3. Mais abaixo, em **Netclass assignments**, clique no **+** três vezes e preencha:

| Pattern | Net Class |
|---|---|
| `+5V` | Energia |
| `GND` | Energia |
| `+3V3` | Energia |

Assim as trilhas de alimentação saem com 2 mm automaticamente, e as de sinal com 1 mm.

Clique **OK**.

## 2. Trazer as peças do esquemático
1. **Tools → Update PCB from Schematic…** (tecla **F8**).
2. Clique em **Update PCB**. A janela lista as 12 peças sendo adicionadas, sem erros em vermelho.
3. **Close**. As peças vêm "grudadas" no mouse: clique em qualquer lugar fora do quadro da folha para soltá-las.

Você vai ver as peças amontoadas, ligadas por **linhas finas brancas** (o *ratsnest*, ou "ninho de rato"). Essas linhas **não são trilhas**: só mostram o que ainda precisa ser ligado.

## 3. Contorno da placa: 100 × 75 mm
1. No painel de camadas à direita, clique em **Edge.Cuts** (fica selecionada).
2. **Place → Draw Rectangle** (ou o ícone de retângulo na barra da direita).
3. Clique em qualquer ponto, depois em outro, para criar um retângulo.
4. Aperte **Esc**, passe o mouse sobre a borda do retângulo e aperte **E**. Digite:
   - Start X **100**, Start Y **60**
   - End X **200**, End Y **135**
5. **OK**. Pronto: placa de 100 × 75 mm (cabe na caixa IP65 de ~150 × 110 mm).

## 4. Posicionar as peças
Para colocar uma peça numa posição exata: passe o mouse sobre ela, aperte **E** e preencha **Position X / Y** e **Orientation**.

### 4.1 O ESP32 (o mais importante: tem que bater com a placa real)
| Peça | X | Y | Orientação |
|---|---|---|---|
| **J1** (Esp32_esquerda) | **137.3** | **90** | **0** |
| **J2** (Esp32_direita) | **162.7** | **90** | **0** |

162,7 − 137,3 = **25,4 mm**, exatamente a distância medida entre as fileiras do seu ESP32.
Com orientação 0, o pino 1 (EN / D23) fica **em cima** e o 15 (VIN / 3V3) **embaixo**: o **USB fica para a borda de baixo** da placa, acessível para gravar.

### 4.2 Bornes na borda de cima
Os bornes ficam numa fileira na borda de cima, com a **abertura dos fios virada para fora** da placa.

| Peça | X | Y |
|---|---|---|
| J3 Energia | 105 | 68 |
| J4 BME280 | 118 | 68 |
| J5 DHT22 | 140 | 68 |
| J6 PMS7003 | 157 | 68 |
| J7 MQ135 | 179 | 68 |

Depois de posicionar, abra a vista 3D (**Alt+3**) e veja de que lado ficou a **boca dos bornes** (onde entra o fio). Se estiver virada para dentro da placa, volte, passe o mouse em cada borne e aperte **R** duas vezes (gira 180°). Ajuste a posição para eles continuarem dentro do contorno.

### 4.3 Resistores e capacitor
Não precisam de posição exata. Arraste com **M** para perto de quem eles ligam (as linhas brancas ajudam):
- **R1** (pull-up do DHT22): à direita do J2, perto da altura do pino 11.
- **R2 e R3** (divisor do MQ135): no espaço **entre J1 e J2**, perto da altura do pino 4 do J1 (o D34). Pode ficar embaixo do ESP32: o resistor deitado tem ~2,5 mm de altura e o ESP32 fica ~8 mm acima da placa, apoiado nas barras fêmea.
- **C1** (470 µF): perto do **J3 Energia**, no canto de cima à esquerda.

Regra de ouro: **as linhas brancas devem ficar curtas e cruzar o mínimo possível**. Gire as peças com **R** até as linhas "desembaraçarem".

### 4.4 Furos de fixação (opcional, recomendado)
**Place → Add Footprint** (tecla **A**), procure `MountingHole_3.2mm_M3` e coloque um em cada canto, a ~4 mm das bordas: (104, 64), (196, 64), (104, 131), (196, 131).
Cuidado para não ficarem em cima dos bornes; se ficarem, mova o furo um pouco.

## 5. Conferir e salvar
1. **Alt+3**: confira na vista 3D se o ESP32 (as duas barras) e os bornes estão do jeito que você imagina a placa montada.
2. **Ctrl+S**.

## 6. Me mande
- [ ] Print do Editor de Placa com tudo posicionado (aperte **Home** para caber).
- [ ] Print da vista 3D.

Na **K4b** desenhamos as trilhas **só na camada de baixo (B.Cu)**, já que a placa é de face simples, e aumentamos as ilhas para facilitar a furação manual.
