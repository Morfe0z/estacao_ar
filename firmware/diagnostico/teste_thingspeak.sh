#!/bin/bash
# Teste do envio ao ThingSpeak a partir do computador do laboratorio.
# Uso: bash teste_thingspeak.sh
# Cada teste grava 1 ponto de teste no canal (espera 20 s entre eles).
# Uso: THINGSPEAK_CHAVE=SUA_WRITE_API_KEY bash teste_thingspeak.sh
CHAVE="${THINGSPEAK_CHAVE:?defina THINGSPEAK_CHAVE com a Write API Key do canal}"
BASE="https://api.thingspeak.com/update?api_key=$CHAVE"
ESP_UA="ESP32HTTPClient"
ESP_ENC="identity;q=1,chunked;q=0.1,*;q=0"

echo "== 0) Endereco de api.thingspeak.com nesta rede =="
getent hosts api.thingspeak.com
echo

echo "== 1) So o campo 1, como o navegador =="
curl -s -m 15 -w "   [HTTP %{http_code}]\n" "$BASE&field1=1"
sleep 20

echo "== 2) Igual ao ESP32: todos os campos, com PM em zero =="
curl -s -m 15 -w "   [HTTP %{http_code}]\n" "$BASE&field1=22.4&field2=46.5&field3=1012.5&field4=0&field5=0&field6=0&field7=0.640&field8=50.2"
sleep 20

echo "== 3) Igual ao ESP32: so o campo 1, mas com os cabecalhos do ESP32 =="
curl -s -m 15 -w "   [HTTP %{http_code}]\n" -A "$ESP_UA" -H "Connection: keep-alive" -H "Accept-Encoding: $ESP_ENC" "$BASE&field1=1"
sleep 20

echo "== 4) Igual ao ESP32 em tudo: todos os campos e os cabecalhos do ESP32 =="
curl -s -m 15 -w "   [HTTP %{http_code}]\n" -A "$ESP_UA" -H "Connection: keep-alive" -H "Accept-Encoding: $ESP_ENC" "$BASE&field1=22.4&field2=46.5&field3=1012.5&field4=0&field5=0&field6=0&field7=0.640&field8=50.2"
echo
echo "Fim. Um numero (ex.: 1730) = aceito. 0 = recusado. Pagina <html> com 400 = mesmo erro do ESP32."
