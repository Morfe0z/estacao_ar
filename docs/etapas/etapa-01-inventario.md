# Etapa 1: Inventário e identificação das peças

> **Resultado desta etapa:** ESP32 DevKit 30 pinos (micro-USB, chip USB CH343), módulo I2C que depois se revelou **BME280**,
> **um** DHT22 solto, MQ135 em módulo, PMS7003 com adaptadora. Os resistores usados foram **4,7 kΩ** (no lugar dos de 10 kΩ citados abaixo).
> Lista final em [hardware/lista-de-materiais.md](../../hardware/lista-de-materiais.md).

Nesta etapa **não ligamos nada na energia**. O objetivo é saber exatamente o que temos em mãos,
porque cada detalhe (versão do módulo, número de pinos, tensão) muda a montagem das próximas etapas.

## 1. Placa ESP32-WROOM-32
- Conte os pinos da placa: as mais comuns são **DevKit V1 de 30 pinos** ou **DevKitC de 38 pinos**.
- Veja o chip USB‑serial perto do conector (geralmente **CP2102** ou **CH340**); isso define o driver no computador.
- Confira o conector: micro‑USB ou USB‑C.

## 2. PMS7003 (partículas PM1.0 / PM2.5 / PM10)
- Precisa de **5 V** para o ventilador interno (~100 mA), mas os sinais de dados são **3,3 V**, compatíveis com o ESP32.
- O conector original é bem pequeno (passo de 1,27 mm). Veja se veio uma **plaquinha adaptadora** com cabo; sem ela fica difícil ligar na protoboard.

## 3. BME280 ou BMP280 (precisamos confirmar)
- **BME280** mede temperatura, **umidade** e pressão. **BMP280** mede só temperatura e pressão.
- Como diferenciar olhando o chip metálico no módulo (use lupa ou foto com zoom):
  - **BME280**: chip **quadrado** (2,5 × 2,5 mm).
  - **BMP280**: chip **retangular** (2,0 × 2,5 mm).
- Conte os pinos do módulo: **4 pinos** (VCC, GND, SCL, SDA) ou **6 pinos** (inclui CSB e SDO).
- Veja se o pino de alimentação se chama **VIN** (geralmente aceita 5 V, tem regulador) ou **3V3/VCC** (só 3,3 V).
- Na etapa de testes vamos confirmar por software também: o chip responde com um código diferente (0x60 = BME280, 0x58 = BMP280).

## 4. Dois DHT22 (temperatura e umidade)
- Veja se são **sensores soltos de 4 pernas** ou **módulos de 3 pinos** (numa plaquinha). O módulo já costuma ter o resistor de 10 kΩ necessário; o sensor solto precisa de um resistor externo.
- Pergunta: qual a ideia de usar dois? (ex.: um dentro e um fora da caixa, ou comparação entre eles)

## 5. MQ135 (gases / qualidade do ar)
- Normalmente vem em módulo com 4 pinos: **VCC, GND, DO, AO**. Confirme.
- Ponto crítico: ele funciona a **5 V** e a saída analógica (AO) pode chegar perto de 5 V. **O ESP32 só aceita até 3,3 V** nas entradas; ligar direto pode danificar a placa. Vamos usar um **divisor de tensão** com dois resistores (detalhado na etapa de ligação).
- O aquecedor interno consome ~150 mA e esquenta; é normal.
- Ele precisa de **24 a 48 horas ligado** na primeira vez ("queima inicial") para as leituras estabilizarem.

## 6. Material de apoio (confira o que você tem)
- Protoboard (de preferência grande, 830 furos) e jumpers macho‑macho e macho‑fêmea.
- Resistores: **10 kΩ** (para DHT22 solto) e um par para o divisor do MQ135 (ex.: **10 kΩ + 20 kΩ**, ou 10 kΩ + 15 kΩ).
- Cabo USB **de dados** (alguns cabos só carregam e não funcionam para programar).
- Fonte de alimentação 5 V de pelo menos **1 A** para a estação completa (o USB do computador serve para os testes iniciais, sensor por sensor).
- **Multímetro**: muito recomendado, vamos usá-lo para conferir tensões antes de ligar cada sensor.

## Regra de segurança para todas as etapas
Sempre montar e mexer nos fios **com tudo desligado**, conferir com calma, e só então conectar o USB.
