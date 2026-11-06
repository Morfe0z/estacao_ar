# Fotos e capturas de tela

## Montagem
| Foto | O que mostra |
|---|---|
| ![Protótipo visto de cima](prototipo-protoboard-vista-de-cima.jpg) | Protótipo completo na protoboard, visto de cima: ESP32 (esquerda), DHT22 (branco, em cima), BME280 (embaixo à esquerda), MQ135 (direita, LED vermelho), PMS7003 (verde, embaixo) e os resistores do divisor e do pull-up |
| ![Protótipo de lado](prototipo-protoboard-lateral.jpg) | O mesmo protótipo de lado, ligado e funcionando |
| ![PMS7003 com adaptadora](pms7003-adaptadora-e-cabo.jpg) | PMS7003 com a placa adaptadora e o cabo |

<!-- Fotos da montagem completa entram aqui: protoboard com todos os sensores, detalhe do divisor do MQ135,
     estação ligada na fonte, e depois a placa soldada e a caixa. -->

## Resultados de cada etapa (Monitor Serial e páginas)
| Etapa | Captura |
|---|---|
| 3. ESP32 sozinho | ![Etapa 3](capturas/etapa03-esp32-monitor-serial.png) |
| 4. DHT22 | ![Etapa 4](capturas/etapa04-dht22.png) |
| 5. Sensor I2C identificado como BME280 (ID 0x60) | ![Etapa 5](capturas/etapa05-bme280-identificado.png) |
| 6. PMS7003 | ![Etapa 6](capturas/etapa06-pms7003.png) |
| 7. MQ135 aquecendo | ![Etapa 7](capturas/etapa07-mq135-aquecendo.png) |
| 8. Estação integrada (com o teste de tirar o fio do PMS) | ![Etapa 8](capturas/etapa08-estacao-integrada.png) |
| 9. Página da estação no celular (http://192.168.4.1) | ![Etapa 9](capturas/etapa09-pagina-wifi-celular.png) |
| 9. Erro 400 do ThingSpeak (incidente de 09/10, ver [solução de problemas](../docs/solucao-de-problemas.md)) | ![Erro 400](capturas/etapa09-erro-400-thingspeak.png) |
| Painel do ThingSpeak | ![Painel](capturas/painel-thingspeak.png) |
