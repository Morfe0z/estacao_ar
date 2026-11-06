# Painel remoto da estação (dentro do ThingSpeak)

> Arquivo do painel: [`painel/painel_thingspeak.html`](../painel/painel_thingspeak.html).
> A mesma página está publicada para qualquer pessoa em **https://morfe0z.github.io/estacao_ar/** (o `index.html` da raiz do repositório).

O painel lê os dados que a estação envia ao ThingSpeak e funciona **de qualquer lugar**, pelo celular ou pelo computador.
Ele fica guardado dentro da sua conta do ThingSpeak como um "Plugin" e aparece na página do canal.

## O que ele mostra
- Situação da estação: **no ar** ou **sem envio**, com a hora do último envio.
- PM2.5 atual com a classe (BOA, MODERADA...) e a posição na escala de cores.
- 8 mostradores: valor atual; **mín, méd e máx** do período escolhido; tendência da última hora.
- Gráficos de partículas (PM1.0, PM2.5 e PM10), temperatura, umidade (BME280 e DHT22), pressão e gases.
  Toque ou passe o mouse sobre um gráfico para ver o valor exato daquele horário.
- Períodos de 6 h, 24 h, 3 dias e 7 dias.
- **Saúde da estação:** quantos registros chegaram, cobertura (%), maior buraco sem dados e se algum sensor parou.
- Aviso quando a umidade passa de 85 % (as partículas ficam superestimadas).
- Link para baixar os dados dos últimos 7 dias em CSV.
- Atualiza sozinho a cada minuto.

## Passo 1: pegar o número do canal e a chave de leitura
1. Entre em https://thingspeak.mathworks.com com a sua conta.
2. **Channels > My Channels** e abra o canal da estação.
3. Anote o **Channel ID** (número no topo da página).
4. Aba **API Keys** e copie a **Read API Key** (é a de **leitura**, não a de escrita).

## Passo 2: preencher o painel
1. Abra o arquivo `painel_thingspeak.html` num editor de texto (Bloco de Notas, gedit etc.).
2. Procure a parte **CONFIGURACAO** (perto do começo do JavaScript) e preencha:
   ```
   const CANAL = "1234567";              // seu Channel ID
   const CHAVE_LEITURA = "ABCDEF123456"; // sua Read API Key
   ```
3. Salve.

## Passo 3: criar o plugin no ThingSpeak
1. No ThingSpeak, menu **Apps > Plugins** e clique em **New**.
2. Escolha **Custom (no starting code)** e clique em **Create**.
3. Em **Name**, escreva `Painel da estação`.
4. Na caixa **HTML**: apague tudo e cole **o arquivo inteiro** já preenchido.
5. Nas caixas **CSS** e **JavaScript**: apague tudo e deixe vazias (o painel já traz tudo dentro do HTML).
6. Em **Add this Visualization to a Channel**, marque o canal da estação.
7. Clique em **Save**.

## Passo 4: abrir de qualquer lugar
- Entre no ThingSpeak, abra o canal e vá na aba **Private View**: o painel aparece junto com os gráficos.
- Os plugins só aparecem na visão privada, então é preciso estar logado na sua conta.

## Plano B: abrir direto no computador
O mesmo arquivo, já preenchido, também funciona aberto direto no navegador do computador (duplo clique).
Ele busca os dados no ThingSpeak do mesmo jeito, de qualquer rede com internet.

## Se aparecer um aviso vermelho no painel
- **"Falta configurar"**: o CANAL ou a CHAVE_LEITURA ficaram vazios.
- **"canal ou chave de leitura incorretos"**: confira o número do canal e se copiou a chave de **leitura**.
- **"sem envio"** no topo: a estação parou de mandar dados. Veja em "Saúde da estação" desde quando.
