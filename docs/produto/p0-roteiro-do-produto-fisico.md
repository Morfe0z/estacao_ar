# Produto físico da estação: roteiro (P0)

Objetivo: transformar o protótipo da protoboard numa **estação externa acabada**, que aguenta sol, chuva e meses ligada sem mexer.
Base: o desenho de [estrutura externa](../estrutura-externa.md) (três partes separadas) e a placa de 100 × 75 mm do KiCad.

```
  A. CAIXA IP65 (ESP32 + placa)   B. ABRIGO DE RADIAÇÃO (BME280 + DHT22)   C. "U" DE PVC 75 mm (PMS7003 + MQ135)
        seca e fechada                 sombra + ventilação, impresso 3D          ar de fora, aberturas para baixo
```

> **Atualização 12/11/2026:** a placa própria (KiCad) ficou de lado por enquanto; o produto inicial usa uma **placa expansora para ESP32 de 30 pinos**. Entraram **anemômetro, cata-vento (direção) e pluviômetro**. Plano e compras em [P2](p2-nova-fase-vento-chuva-e-compras.md).

Cada etapa só começa quando a anterior foi conferida (foto, medida ou teste).

| Etapa | O que fazemos | Como sabemos que deu certo |
|---|---|---|
| **P1** | Levantar medidas reais das peças, da impressora e do local de instalação | Tabela da P1 preenchida + fotos com régua |
| **P2** | Nova fase (expansora, vento, direção, chuva) e lista de compras; caixa IP65 no tamanho certo | Lista com medidas e quantidades fechadas |
| **P3** | Desenho 3D das peças impressas (OpenSCAD, arquivos `.scad` + `.stl`): anemômetro, cata-vento, pluviômetro, abrigo de radiação, suporte dos sensores, suporte interno do "U", placa de montagem da caixa | Peça de teste impressa encaixa nas peças reais |
| **P4** | Impressão definitiva em PETG/ASA branco | Peças sem empeno, furos no lugar |
| **P5** | Cabos: cortar, identificar, terminais, comprimentos | Continuidade conferida no multímetro, fio a fio |
| **P6** | Caixa A: furação, prensa-cabos, dreno, placa de montagem | Teste de água (borrifador/mangueira fraca) sem entrar água |
| **P7** | Montagem completa e **teste de bancada de 48 h** dentro de casa/lab | Página e ThingSpeak sem falhas; temperatura no abrigo coerente |
| **P8** | Suporte/braço e instalação externa (face sul, 1,5–3 m, ≥ 1 m da parede) | Fotos do local; dados chegando por 1 semana |
| **P9** | Documentação final no GitHub (fotos, STL, lista de materiais, manual de manutenção) | README atualizado |

Dependências com outras frentes:
- **Placa (KiCad)**: pausada por decisão da equipe; o produto inicial usa a placa expansora.
- **Calibração do MQ135 (Etapa 11)** pode acontecer em paralelo, na bancada.

## Correção em relação ao rascunho anterior
O rascunho da estrutura externa cita o PMS7003 com **50 × 38 × 21 mm**: essa é a medida do **PMS5003**.
O **PMS7003** é menor e mais fino: **48 × 37 × 12 mm** (folha de dados). Vamos confirmar com a régua na P1, junto com a placa adaptadora e o cabo, que ocupam mais espaço.
