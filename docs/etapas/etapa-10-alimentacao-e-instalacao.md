# Etapa 10: Alimentação pela fonte 5 V e instalação da estação

O programa final já está gravado e **não muda**. Ao ligar na energia, o ESP32 começa a funcionar sozinho (não precisa de computador).
Esta etapa troca o USB do computador pela **fonte 5 V / 3 A** e define **onde e como** posicionar a estação.

## 10.1 Descobrir o tipo da sua fonte
Olhe a ponta do cabo da fonte:

### Caso A: fonte com saída **USB** (tipo carregador de celular)
É o caso mais simples e seguro: use um cabo **micro‑USB** da fonte direto no conector do ESP32, exatamente como fazia com o computador.
Nenhum fio muda. Pule para o 10.3.

### Caso B: fonte com **plugue redondo (P4)** ou **fios soltos**
Vai ser ligada nos pinos **VIN** (positivo) e **GND** (negativo) do ESP32. Siga o 10.2 com calma.

## 10.2 Só para o Caso B: conferir a polaridade ANTES de ligar
1. **Desconecte o cabo USB do ESP32.** ⚠️ **Nunca** use a fonte no VIN e o USB ao mesmo tempo.
2. Ligue a fonte na tomada, **sem encostar no circuito**.
3. Multímetro em **tensão contínua (20 V)**: ponta vermelha no fio/ponto que você acha que é o positivo, preta no negativo.
   - Deve mostrar entre **+4,8 V e +5,3 V**.
   - Se mostrar um valor **negativo** (ex.: -5,0 V), as pontas estão invertidas: o positivo é o outro fio. Anote qual é qual.
   - Se mostrar **acima de 5,5 V**, **não use** essa fonte.
4. Tire a fonte da tomada.
5. Ligue **positivo → VIN** e **negativo → GND** (na protoboard, nas mesmas fileiras que já têm VIN e GND).
6. Ligue a fonte na tomada.

## 10.3 Conferir funcionando
1. O LED vermelho do ESP32 acende, a ventoinha do PMS7003 gira e o MQ135 esquenta.
2. Meça **VIN com GND** com tudo ligado: deve ficar entre **4,6 V e 5,3 V**.
3. No celular, conecte na rede **EstacaoAr** e abra **http://192.168.4.1**: todos os sensores devem estar **ok**.
4. Se a rede do celular (hotspot) estiver ligada, confira no ThingSpeak que os dados continuam chegando.

## 10.4 Onde posicionar a estação
Esses cuidados fazem diferença na qualidade das medições:
- **MQ135 longe do BME280 e do DHT22.** O aquecedor do MQ135 esquenta o ar em volta e falseia a temperatura e a umidade. Deixe pelo menos **10 cm** de distância, com o MQ135 **acima** ou do lado oposto (o ar quente sobe).
- **BME280 e DHT22 longe do ESP32** também (alguns cm), porque o ESP32 esquenta com o Wi‑Fi ligado.
- **PMS7003 com a entrada e a saída de ar livres.** Não encoste essas aberturas em paredes ou caixas fechadas.
- **Ventilação.** Se for usar uma caixa, ela precisa ter **furos ou aberturas** para o ar circular; caixa fechada mede o ar de dentro da caixa.
- **Longe de fontes diretas** (janela de exaustor, bancada com solventes, sol direto), a não ser que seja isso que você quer medir.
- **Ambiente externo:** precisa de proteção contra chuva e sol (abrigo ventilado). Me avise se for esse o caso, que eu detalho.

## 10.5 Deixar ligada 24 a 48 horas (preparação para a calibração)
O MQ135 só estabiliza depois de **24 a 48 horas ligado sem parar** (queima inicial).
Deixe a estação ligada nesse período. Depois disso faremos a **Etapa 11: calibração do MQ135**, que transforma a tensão em um índice de gases comparável.

## Problemas comuns
| Sintoma | Causa provável | Solução |
|---|---|---|
| Nada liga | Polaridade invertida ou mau contato | Desligar na hora e refazer o 10.2 |
| ESP32 reinicia sozinho | Tensão baixa no VIN (< 4,5 V) ou fio fino/longo | Medir VIN; usar fios mais curtos |
| Temperatura alguns graus acima do normal | Sensor perto do MQ135 ou do ESP32 | Afastar (10.4) |
| Página abre, mas ThingSpeak parou | O hotspot do celular foi desligado | Normal: volta a enviar sozinho quando a rede voltar |
