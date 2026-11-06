# Etapa 10b: versão de campo (mais robusta)

Programa: [`firmware/estacao_campo/estacao_campo.ino`](../../firmware/estacao_campo/estacao_campo.ino)
Ligações: **as mesmas** da versão final. Não precisa mexer em nenhum fio.

## O que muda em relação à versão final
| Situação | Versão final | Versão de campo |
|---|---|---|
| Internet ou ThingSpeak fora do ar | a leitura daquele momento se perde | as leituras ficam numa **fila** (até 4 h) e sobem todas juntas, cada uma com a sua hora certa, quando a conexão volta |
| Falha de envio que não passa | tenta para sempre do mesmo jeito | 10 min: **reinicia o Wi-Fi** do zero. 30 min: **salva a fila na memória flash e reinicia o ESP32**. A fila volta sozinha depois de ligar. |
| Programa travado | fica travado | o **watchdog** reinicia o ESP32 depois de 60 s travado |
| Número de um dígito (ex.: PM2.5 = 3) | o ESP32 escrevia " 3", com espaço, e o ThingSpeak recusava com erro 400 (foi a parada de 2026-10-09) | o espaço é removido antes do envio |
| Envio | `http://` | `https://` (conexão criptografada) |
| Erro de envio | só o código | o texto que o servidor respondeu aparece na página |
| Página | básica | visual novo, com mínimo, média e máximo de 2 h, tendência da última hora, sinal do Wi-Fi, fila, reinícios e o motivo do último reinício |

Se a energia cair, a fila guardada na memória RAM se perde (o salvamento na flash só acontece antes de um reinício automático). Para não perder nada nem nesse caso, o próximo passo seria o cartão microSD.

## Passo 1: preparar a configuração (secrets.h)
1. Baixe a pasta [`firmware/estacao_campo/`](../../firmware/estacao_campo/) e, dentro dela, copie
   `secrets.exemplo.h` para um arquivo novo chamado **`secrets.h`**.
2. Abra `estacao_campo.ino` na Arduino IDE (**File > Open**). O `secrets.h` aparece numa aba ao lado.
3. Na aba `secrets.h`, preencha `WIFI_NOME`, `WIFI_SENHA`, `THINGSPEAK_CHAVE` e o **novo**:
   ```
   const char* THINGSPEAK_CANAL = "1234567";   // o "Channel ID" do seu canal
   ```
   O Channel ID é o número que aparece no topo da página do canal no ThingSpeak. Ele é necessário para enviar as leituras atrasadas.

Para várias redes (casa, laboratório, eduroam), use a variante [`firmware/estacao_campo_redes/`](../../firmware/estacao_campo_redes/):
ela procura a rede conhecida de melhor sinal. Mesma ideia: copie o `secrets.exemplo.h` e preencha a lista `REDES`.

## Passo 2: gravar
1. Tire o cabo da fonte e ligue o ESP32 no computador.
2. Placa **ESP32 Dev Module**, porta `/dev/ttyACM0`.
3. Em **Tools > Partition Scheme**, deixe **"Default 4MB with spiffs"** (é a padrão; a fila usa esse espaço).
4. **Upload**.
   Se aparecer *"Sketch too big"*, troque o Partition Scheme para **"Huge APP (3MB No OTA/1MB SPIFFS)"** e grave de novo.

## Passo 3: conferir no Monitor Serial (115200)
Logo ao ligar deve aparecer:
```
Inicio  : ligada na energia ou botão EN
Memoria : pronta para guardar a fila
Watchdog: ligado (reinicia se travar por 60 s)
```
E a cada 5 s, além das leituras, uma linha assim:
```
ThingSpeak  : 3 envios ok, 0 falhas  |  fila: 0 leituras esperando
```

## Passo 4: conferir na página
Volte para a fonte, conecte o celular na rede **EstacaoAr** (senha do `AP_SENHA`) e abra http://192.168.4.1.
- No topo, a etiqueta deve ficar verde, com **"enviando"**.
- Em **Situação da estação**: "Último envio aceito há poucos segundos" e "Fila: 0 leituras".

## Passo 5 (opcional): testar a fila
1. Desligue o roteador por uns 3 minutos (ou mude a senha do Wi-Fi no código e grave).
2. A página mostra a **fila crescendo** (1 leitura a cada 20 s) e a etiqueta **"sem enviar"**.
3. Volte o roteador: em até 1 minuto a fila zera, e no ThingSpeak os pontos desses 3 minutos aparecem **sem buraco**.
