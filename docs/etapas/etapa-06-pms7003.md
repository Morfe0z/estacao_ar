# Etapa 6: Terceiro sensor, PMS7003 (material particulado)

O DHT22 e o BME280 **continuam ligados**. Agora acrescentamos o PMS7003, pela plaquinha adaptadora (conector **P3**, ordem **TX, RX, VCC, GND**).
Ele mede a poeira fina do ar: **PM1.0, PM2.5 e PM10** (em µg/m³).

Não precisa instalar biblioteca nesta etapa.

## 6.1 Montar (com o USB DESCONECTADO)
| Pino da adaptadora (P3) | Liga em | Observação |
|---|---|---|
| **TX** | **RX2 / GPIO16** | cruzado: TX do sensor no RX do ESP32 |
| **RX** | **TX2 / GPIO17** | cruzado |
| **VCC** | **VIN** do ESP32 | **5 V** (é o único sensor até aqui que vai no VIN!) |
| **GND** | **GND** do ESP32 | |

⚠️ **Atenção redobrada no VCC:** ele vai no **VIN**, não no 3V3. E nunca troque VCC com GND.
Use jumper **vermelho no VCC** e **preto no GND**, se tiver.

## 6.2 Conferir antes de ligar (multímetro em continuidade)
1. **VIN com GND**: **não pode apitar**.
2. **3V3 com GND**: **não pode apitar**.
3. **VCC da adaptadora com VIN**: tem que apitar.
4. **GND da adaptadora com GND**: tem que apitar.

## 6.3 Ligar e observar
1. Conecte o USB. Nos primeiros segundos, a **ventoinha do PMS7003 deve começar a girar** (um zumbido baixinho; dá para sentir o ar saindo).
2. Se notar cheiro de queimado ou algo esquentando, **desconecte na hora**.

## 6.4 Gravar e testar
1. **File → New**, cole [`firmware/etapa06_pms7003/etapa06_pms7003.ino`](../../firmware/etapa06_pms7003/etapa06_pms7003.ino), salve como `etapa06_pms7003` e grave.
2. **Serial Monitor** em **115200**.

Esperado (cerca de uma linha por segundo):
```
=== Etapa 6: teste do PMS7003 ===
Aguardando dados do sensor (as leituras estabilizam depois de ~30 s)...
PM1.0: 5  |  PM2.5: 8  |  PM10: 9 ug/m3   |  >0,3um: 1020  >2,5um: 2   [ok: 1 / ruins: 0]
PM1.0: 5  |  PM2.5: 8  |  PM10: 10 ug/m3   |  >0,3um: 1032  >2,5um: 2   [ok: 2 / ruins: 0]
```
- Em ambiente interno limpo, o **PM2.5** costuma ficar entre **0 e 35 µg/m³**.
- O contador **ruins** deve ficar em **0** (ou crescer muito raramente).
- Ignore os primeiros ~30 s: é o tempo da ventoinha estabilizar o fluxo de ar.

**Teste de vida:** sacuda um pano empoeirado (ou bata palmas com as mãos sujas de pó/farinha) a uns 30 cm do sensor. O PM2.5 e o PM10 devem subir bastante e depois voltar devagar.

## Problemas comuns
| Sintoma | Causa provável | Solução |
|---|---|---|
| `Nenhum dado do PMS7003 ha 5 s` e a ventoinha **não gira** | Sem alimentação: VCC/GND soltos ou no pino errado | Conferir VCC → VIN e GND |
| `Nenhum dado...` mas a ventoinha **gira** | TX do sensor não chega no GPIO16 (ou TX/RX invertidos) | Trocar os fios TX e RX entre si (não queima nada) |
| Contador **ruins** subindo sempre | Mau contato no fio TX | Apertar os jumpers |
| Valores sempre 0 por mais de 1 minuto | Ar muito limpo ou entrada de ar bloqueada | Fazer o teste do pano |
