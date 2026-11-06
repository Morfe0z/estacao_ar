# KiCad K1: instalar o KiCad e criar o projeto da placa

Nesta primeira etapa só instalamos o programa e criamos o projeto vazio. Nada de desenho ainda.
No fim há uma lista de conferência: me mande o que aparecer e só então seguimos para a K2.

## O que é o KiCad (em 1 minuto)
O KiCad é um programa gratuito para desenhar placas de circuito. O trabalho tem sempre duas partes:

1. **Esquemático**: o desenho "lógico". Diz *o que liga em quê* (o GPIO4 liga no DHT22 e no resistor de 4,7 kΩ...). Não importa a posição real das peças.
2. **Placa (PCB)**: o desenho "físico". Cada peça vira um **footprint** (a marca real dos furos no tamanho certo), e as ligações do esquemático viram **trilhas** de cobre.

O KiCad só deixa passar para a placa o que está no esquemático. Isso é o que evita erro: se uma ligação não existe no esquemático, a placa avisa.

Nossa placa vai ter o que já foi testado na protoboard (veja [`docs/estrutura-externa.md`](../estrutura-externa.md), seção 10):
- ESP32 de 30 pinos encaixado em 2 barras de pinos fêmea de 15;
- resistor de 4,7 kΩ (pull-up do DHT22) e divisor 4,7 kΩ / 4,7 kΩ do MQ135;
- capacitor de 470 µF / 16 V na entrada de energia;
- bornes de parafuso para: energia (2), BME280 (4), DHT22 (3), PMS7003 (4), MQ135 (3).

## Passo 1: instalar

Use a **versão 9 ou mais nova** do KiCad (o site sempre mostra a atual).

### No Windows (casa) — o caminho mais fácil
1. Abra https://www.kicad.org/download/windows/
2. Baixe o instalador (pode ser pelo link "GitHub" ou por um espelho).
3. Execute e aceite as opções padrão. Ele baixa as bibliotecas de peças junto (são uns 1,5 GB instalados; demora).

### No Linux do laboratório (Ubuntu, com sudo)
Use o repositório oficial do KiCad (PPA), que traz a versão atual:

```bash
sudo add-apt-repository ppa:kicad/kicad-9.0-releases
sudo apt update
sudo apt install --install-recommends kicad
```

Depois abra pelo menu de aplicativos (KiCad) ou digitando `kicad` no terminal.
Se o `sudo` for recusado, a alternativa sem sudo é o Flatpak só para o seu usuário:

```bash
flatpak --user remote-add --if-not-exists flathub https://dl.flathub.org/repo/flathub.flatpakrepo
flatpak --user install flathub org.kicad.KiCad
flatpak run org.kicad.KiCad
```

O projeto é só uma pasta de arquivos: dá para levar num pendrive ou no Drive e abrir nos dois computadores (lab e casa), desde que as versões sejam iguais (as duas 9.x).

## Passo 2: primeira abertura
Na primeira vez o KiCad pergunta sobre as **tabelas de bibliotecas** (símbolos e footprints).
Escolha **"Copiar tabela de bibliotecas global padrão (recomendado)"** e clique OK.

Se aparecer em inglês e você preferir português: **Preferences → Set Language → Português (Brasil)**.
(Daqui em diante vou escrever o nome em português e, entre parênteses, o nome em inglês.)

## Passo 3: criar o projeto
1. **Arquivo → Novo projeto** (*File → New Project*).
2. Escolha uma pasta sua, por exemplo `Documentos/estacao_placa`.
3. Nome do projeto: **`estacao_placa`**
4. Deixe marcada a opção **"Criar uma nova pasta para o projeto"**.

A janela principal vai mostrar três arquivos:

| Arquivo | Para que serve |
|---|---|
| `estacao_placa.kicad_pro` | O projeto (configurações) |
| `estacao_placa.kicad_sch` | O esquemático |
| `estacao_placa.kicad_pcb` | A placa |

## Passo 4: abrir os dois editores (só para conhecer)
1. Dê dois cliques em `estacao_placa.kicad_sch`. Abre o **Editor de Esquemático**: uma folha em branco com um quadro no canto inferior direito.
   - Gire a roda do mouse: dá zoom. Segure a roda (botão do meio) e arraste: move a folha.
   - Feche sem salvar nada.
2. Dê dois cliques em `estacao_placa.kicad_pcb`. Abre o **Editor de Placa**: fundo preto, vazio.
   - Feche sem salvar.

## Conferência antes da K2 (me mande isto)
- [ ] Em qual computador instalou (Windows ou Linux) e a versão: **Ajuda → Sobre o KiCad** (*Help → About KiCad*), a primeira linha, por exemplo `9.0.x`.
- [ ] Um print da janela principal com os 3 arquivos do projeto aparecendo.
- [ ] Os dois editores abriram sem mensagem de erro? (Se apareceu algum aviso de biblioteca, mande o print.)

## O que vem depois
- **K2**: desenhar o esquemático da estação (os mesmos fios da [`etapa-02-mapa-de-ligacoes.md`](../etapas/etapa-02-mapa-de-ligacoes.md)) e rodar a verificação automática (ERC).
- **K3**: escolher o footprint de cada peça.
- **K4**: posicionar as peças e desenhar as trilhas, com verificação automática (DRC).
- **K5**: gerar os arquivos para fabricar (ou o desenho furo a furo para a placa perfurada) e conferir antes de soldar.
