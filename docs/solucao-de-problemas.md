# Solução de problemas: o que já aconteceu e como foi resolvido

Cada etapa em [etapas/](etapas/) tem a sua tabela de "Problemas comuns". Aqui ficam os casos reais que apareceram neste projeto.

## Computador e gravação
| Problema | Causa | Solução |
|---|---|---|
| Arduino IDE do App Center (snap 1.8.19) dá `SyntaxError: future feature annotations` | Python antigo dentro do snap | IDE 2 em ZIP (`./arduino-ide --no-sandbox`) ou pacote esp32 2.0.17 |
| `Permission denied` na porta | usuário fora do grupo `dialout` | regra udev ([preparar-computador-linux.md](preparar-computador-linux.md)) ou pedido ao administrador ([pedido-ao-administrador.md](pedido-ao-administrador.md)) |
| Porta é `/dev/ttyACM0`, não `/dev/ttyUSB0` | o chip USB da placa é CH343/CH9102 (1a86:55d4) | normal nesta placa |
| IDE com placa "ESP32-WROOM-DA Module" | escolha errada no menu | usar **ESP32 Dev Module** |

## Sensores
| Problema | Causa | Solução |
|---|---|---|
| "BMP280" respondeu com ID 0x60 | o módulo é um **BME280** (mede umidade também) | biblioteca Adafruit BME280, programa `etapa05b_bme280` |
| DHT22 ~8 % de umidade acima do BME280 | diferença normal entre sensores baratos | BME280 é a referência; DHT22 fica como segunda opinião |
| PMS7003 em 0 | ar limpo de verdade (em casa); não é defeito | conferir com o teste do pano empoeirado |
| MQ135 grudado em ~4,9 V (noite de 09/10) | mau contato na protoboard | reencaixar; motivo a mais para a placa soldada |

## Envio ao ThingSpeak: o erro 400 de 09/10/2026
**Sintoma:** às 6h o ThingSpeak começou a responder `400 Bad Request` (página do nginx) a todos os envios. No navegador, a mesma chave funcionava.

**Investigação:** os programas em [firmware/diagnostico/](../firmware/diagnostico/) testaram HTTP, HTTPS, GET, POST,
DNS e a rede própria ligada junto. Tudo funcionava com números de teste.

**Causa:** no ESP32, `String(valor, casas)` usa `dtostrf` com largura fixa e coloca um **espaço à esquerda** em números curtos.
De madrugada o PM2.5 do laboratório caiu abaixo de 10, o valor virou `" 3"`, e o espaço dentro da URL fazia o servidor recusar o pedido.
Voltou "sozinho" às 11h porque o PM subiu de novo.

**Correção:** cada número passa por `trim()` antes de entrar no envio (função `numeroTexto()` na versão de campo). Está em `estacao_final`, `estacao_campo` e `estacao_campo_redes`.

**Lição:** a versão de campo ganhou fila de 4 h, reinício do Wi-Fi após 10 min de falha, reinício do ESP32 após 30 min e o texto da resposta do servidor na página.
