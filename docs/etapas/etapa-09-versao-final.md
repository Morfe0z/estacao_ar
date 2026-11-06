# Etapa 9: Versão final (página Wi-Fi própria + ThingSpeak)

**Nenhum fio muda.** Programa: [`firmware/estacao_final/estacao_final.ino`](../../firmware/estacao_final/estacao_final.ino).
Ele faz tudo o que a Etapa 8 fazia e acrescenta:
- **Rede Wi-Fi própria** chamada **EstacaoAr** (senha definida em `AP_SENHA`, no `secrets.h`). Conectando nela, a página **http://192.168.4.1** mostra os valores, a classificação do ar, **gráficos das últimas 2 horas** e um link para **baixar o histórico em CSV**. Funciona sem internet.
- **Envio ao ThingSpeak** a cada 20 s, quando houver uma rede com internet configurada.

A etapa tem duas partes. A Parte A já funciona sem configurar nada.

---

## Parte A: página Wi-Fi própria

### A.1 Gravar
1. Baixe a pasta [`firmware/estacao_final/`](../../firmware/estacao_final/) inteira e, dentro dela, copie
   `secrets.exemplo.h` para um arquivo novo chamado **`secrets.h`** (veja o [guia do firmware](../../firmware/README.md)).
   Abra o `estacao_final.ino` na Arduino IDE (**File → Open**): o `secrets.h` aparece numa aba ao lado. Grave.
   (Não precisa de biblioteca nova: Wi-Fi, servidor web e HTTP já vêm com o pacote do ESP32.)
2. No **Serial Monitor** (115200) deve aparecer:
   ```
   === Estacao de Qualidade do Ar - VERSAO FINAL ===
   BME280  : iniciado
   ...
   Wi-Fi   : rede propria "EstacaoAr" criada. Pagina em http://192.168.4.1
   Wi-Fi   : rede com internet nao configurada (ThingSpeak desligado)
   Pagina  : servidor iniciado
   ```

### A.2 Abrir a página
1. No **celular**, abra as configurações de Wi-Fi e conecte na rede **EstacaoAr**, com a senha que você definiu em `AP_SENHA`.
   - O celular pode avisar "rede sem internet": escolha **manter conectado**.
   - Se o celular insistir em trocar de rede, desligue os **dados móveis** durante o teste.
2. No navegador do celular, abra **http://192.168.4.1** (digite exatamente assim, com `http://`).
3. Esperado: cartões com PM2.5, PM10, temperatura, umidade, pressão e gases, a faixa colorida de qualidade do ar, e a seção "Situação" com todos os sensores **ok**.
   Os valores se atualizam sozinhos a cada 5 s. Os gráficos ganham um ponto a cada 30 s (o primeiro traço aparece depois de ~1 minuto).

### Para mudar o nome ou a senha da rede
Na aba **`secrets.h`**:
```cpp
const char* AP_NOME  = "EstacaoAr";
const char* AP_SENHA = "troque-esta-senha";   // minimo 8 caracteres
```

---

## Parte B: ThingSpeak (nuvem)

### B.1 Criar a conta e o canal (uma vez)
1. Acesse **thingspeak.mathworks.com** e crie uma conta gratuita (**Sign Up**).
2. **Channels → My Channels → New Channel**.
3. Nome: `Estacao Qualidade do Ar`. Marque e nomeie os **8 campos** exatamente nesta ordem:
   | Campo | Nome |
   |---|---|
   | Field 1 | Temperatura (C) |
   | Field 2 | Umidade (%) |
   | Field 3 | Pressao (hPa) |
   | Field 4 | PM1.0 (ug/m3) |
   | Field 5 | PM2.5 (ug/m3) |
   | Field 6 | PM10 (ug/m3) |
   | Field 7 | MQ135 (V) |
   | Field 8 | Umidade DHT22 (%) |
4. **Save Channel**.
5. Abra a aba **API Keys** e copie a **Write API Key** (16 letras/números).

### B.2 Configurar o programa
Na aba **`secrets.h`**, preencha:
```cpp
const char* WIFI_NOME  = "nome-da-rede-com-internet";
const char* WIFI_SENHA = "senha-da-rede";
const char* THINGSPEAK_CHAVE = "SUA_WRITE_API_KEY";
```
Grave de novo. No Serial Monitor, a linha **Rede** deve passar a mostrar `internet: conectada` e `ThingSpeak: N envios ok, 0 falhas`.
No site do ThingSpeak, aba **Private View**, os gráficos começam a receber pontos.

⚠️ O ESP32 só conecta em redes de **2,4 GHz** com **senha simples** (WPA2 pessoal). Redes que pedem **usuário e senha pessoais** (como a eduroam) ou página de login no navegador **não funcionam** desse jeito. Nesse caso, um roteador de celular (hotspot) em 2,4 GHz resolve.

## Problemas comuns
| Sintoma | Causa provável | Solução |
|---|---|---|
| A rede EstacaoAr não aparece | Programa antigo gravado | Gravar o `estacao_final` |
| Página não abre | Celular saiu da rede EstacaoAr ou endereço digitado errado | Reconectar; usar `http://192.168.4.1` |
| `internet: desconectada` sempre | Nome/senha errados, rede de 5 GHz ou rede com login | Conferir B.2 e o aviso acima |
| `ThingSpeak recusou` | Write API Key errada (ou a Read Key no lugar) | Copiar de novo a **Write** API Key |
| ESP32 reinicia sozinho | Wi-Fi aumenta o consumo; USB fraco | Outra porta USB, ou a fonte 5 V (etapa seguinte) |
