# P1: medidas reais das peças, da impressora e do local

Antes de desenhar qualquer peça em 3D ou comprar a caixa, precisamos das **medidas reais** do que você tem.
Medidas de folha de dados variam de fabricante para fabricante (principalmente as plaquinhas do BME280 e do MQ135), e 1 mm errado faz a peça não encaixar.

**Ferramenta:** paquímetro, se tiver (o melhor); senão, régua milimetrada.
**Fotos:** tire cada peça **de cima, com a régua ao lado**, em fundo liso. Isso me deixa conferir as medidas.

⚠️ Pode desligar a estação para medir. Não precisa desmontar a protoboard: dá para medir as peças no lugar.

## 1. Sensores

| Peça | Medir | Valor típico (para comparar) |
|---|---|---|
| **BME280** (plaquinha) | comprimento × largura × espessura da placa | ~13 × 10 mm ou ~15 × 12 mm |
|  | tem furo de fixação? diâmetro e distância do furo à borda | muitas têm 1 furo de ~3 mm, outras nenhum |
|  | lado em que saem os 4 pinos | borda curta |
| **DHT22** | comprimento × largura × espessura do corpo branco (sem as pernas) | 25 × 15 × 7,7 mm |
|  | tem o furo/orelha de fixação em cima? | sim, furo ~3 mm |
| **MQ135** (módulo azul/preto) | comprimento × largura da placa | ~32 × 22 mm |
|  | altura total do chão da placa ao topo da tela metálica | ~25 mm |
|  | furos de fixação: quantos, diâmetro, distância entre centros | 1 furo de ~3 mm, às vezes 2 |
| **PMS7003** | comprimento × largura × altura do sensor | 48 × 37 × 12 mm |
|  | onde ficam a **entrada** e a **saída** de ar (foto mostrando) | as duas na mesma face |
| **Placa adaptadora do PMS** | comprimento × largura, e altura com o cabo encaixado | varia |

## 2. ESP32 encaixado
| Medir | Para quê |
|---|---|
| Altura do ESP32 sobre a protoboard, do topo da protoboard ao topo do módulo metálico | estimar a altura dentro da caixa |

## 3. Impressora 3D
| Pergunta | Exemplo |
|---|---|
| Marca e modelo | Ender 3 V2, Bambu A1… |
| Área de impressão (X × Y × Z) | 220 × 220 × 250 mm |
| Filamentos disponíveis (material e cor) | PETG branco, PLA preto… |
| Bico (diâmetro) | 0,4 mm |
| Fatiador que vocês usam | Cura, PrusaSlicer, Bambu Studio |

Se não houver PETG ou ASA **branco**, me avise: dá para planejar a compra (1 rolo de 1 kg sobra).

## 4. Local de instalação
| Pergunta | Por quê |
|---|---|
| Onde vai ficar: casa, laboratório, telhado, poste, muro? | define o suporte |
| Fixação em **parede** ou **poste/cano**? (se for cano, o diâmetro) | define braço ou abraçadeira |
| Altura possível do chão | ideal 1,5 a 3 m |
| Para que lado a parede fica virada (use a bússola do celular) | ideal face **sul** |
| Distância até a **tomada** mais próxima (caminho do cabo) | dimensiona o cabo 2 × 0,75 mm² |
| O Wi-Fi do local alcança esse ponto? (teste com o celular lá) | sem Wi-Fi a fila guarda só 4 h |
| Foto do local, de longe | conferir sombra, sol, exaustores, telhado embaixo |

## 5. O que já foi comprado
Marque o que já existe da [lista de compras](../estrutura-externa.md#8-lista-de-compras-estimativa) (caixa IP65, prensa-cabos, joelhos de PVC, cabos…).
**Não compre a caixa IP65 ainda:** na P2 vamos escolher o tamanho com a placa de 100 × 75 mm e as medidas acima.

## Quando terminar
Mande as medidas (pode ser a tabela copiada com os valores) e as fotos. Eu confiro e começamos a P2.
