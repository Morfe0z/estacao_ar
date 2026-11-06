# Etapa 3: Configurar a Arduino IDE e testar o ESP32 sozinho

**Situação:** Ubuntu 24.04 (computador do laboratório, sem sudo), Arduino IDE 2 em ZIP (a do App Center não serve).
**Nesta etapa nenhum sensor fica ligado ao ESP32.**

> Nesta placa o chip USB é o **CH343/CH9102**, então a porta aparece como **`/dev/ttyACM0`** (e não `/dev/ttyUSB0`). No Windows, instale o driver CH343 e use a porta COM. Para preparar um Linux com sudo, veja [preparar-computador-linux.md](../preparar-computador-linux.md).

A etapa tem duas partes:
- **Parte A** (não precisa da placa nem de permissão): configurar a IDE e **compilar** o programa. Isso prova que a IDE está pronta.
- **Parte B** (precisa da permissão de porta USB): conectar a placa e **gravar**.

---

## Parte A: configurar a IDE e compilar

### A.1 Instalar a Arduino IDE 2 (ZIP, sem sudo)
⚠️ **Não use a IDE do App Center** (1.8.19 em snap): o Python embutido nela é antigo e causa o erro
`SyntaxError: future feature annotations is not defined` com o pacote atual do ESP32.

1. Em **arduino.cc/en/software**, baixe **"Linux ZIP file 64 bits (X86-64)"** para a pasta Downloads.
2. No Terminal (Ctrl+Alt+T):
   ```
   cd ~/Downloads
   unzip arduino-ide_*_Linux_64bit.zip
   cd arduino-ide_*_Linux_64bit
   ./arduino-ide --no-sandbox
   ```
   O `--no-sandbox` é necessário no Ubuntu 24.04 quando não se tem sudo; sem ele a janela não abre.
3. Para abrir de novo depois, repita os dois últimos comandos.

### A.2 Adicionar o suporte ao ESP32
1. Abra a Arduino IDE.
2. **File → Preferences** (Arquivo → Preferências).
3. No campo **"Additional boards manager URLs"** (URLs adicionais para Gerenciadores de Placas), cole:
   ```
   https://espressif.github.io/arduino-esp32/package_esp32_index.json
   ```
4. **OK**.
5. **Tools → Board → Boards Manager** (Ferramentas → Placa → Gerenciador de Placas).
6. Pesquise **esp32** e instale **"esp32 by Espressif Systems"**. É grande; espere terminar.

### A.3 Escolher a placa
**Tools → Board → esp32 → "ESP32 Dev Module"**.

### A.4 Colar o programa de teste
1. **File → New** (Arquivo → Novo). Apague o que estiver na janela.
2. Cole o código do arquivo [`firmware/etapa03_teste_esp32/etapa03_teste_esp32.ino`](../../firmware/etapa03_teste_esp32/etapa03_teste_esp32.ino).
3. **File → Save** com o nome `etapa03_teste_esp32`.

### A.5 Compilar (sem a placa)
Clique no botão **✓ (Verify / Verificar)**, à esquerda da seta.
- **Sucesso:** aparece algo como `Sketch uses 280000 bytes (21%) of program storage space`.
- **Erro:** me mande o texto vermelho que apareceu (ou um print).

✅ **Se compilou, a IDE está 100% pronta.** Falta só a parte da porta USB.

---

## Parte B: conectar e gravar

### B.1 Verificar a placa e a permissão
Conecte o ESP32 pelo cabo micro‑USB (**sem sensores**) e rode:
```
lsusb
ls -l /dev/ttyUSB*
groups
```
Me mande o resultado dos três.
- `lsusb` deve mostrar uma linha com **CP210x** (Silicon Labs) ou **CH340** (QinHeng). Se nenhuma das duas aparecer, o cabo provavelmente é só de carga.
- `ls -l /dev/ttyUSB*` deve mostrar **/dev/ttyUSB0**.
- `groups` precisa ter **dialout** para gravar. Hoje mostra só "LabMachine", então a gravação será bloqueada até o administrador resolver
  (pedido pronto em [`pedido-ao-administrador.md`](../pedido-ao-administrador.md)).

### B.2 Gravar (quando a permissão estiver resolvida)
1. **Tools → Port → `/dev/ttyUSB0`**.
2. Clique na **seta (→) Upload**.
3. Se ficar travado em `Connecting........_____`, **segure o botão BOOT** da placa até aparecer `Writing at 0x...` e solte.
4. Termina com `Hard resetting via RTS pin...`.

### B.3 Conferir
1. **Tools → Serial Monitor**, velocidade **115200 baud**.
2. Aperte **EN** na placa.
3. Esperado: LED azul piscando a cada 1 segundo, e no Monitor Serial:
   ```
   === Etapa 3: teste do ESP32 ===
   Modelo do chip: ESP32-D0WD-V3
   ...
   Se voce esta lendo isto, a placa esta funcionando!
   LED aceso
   LED apagado
   ```

## Problemas comuns
| Sintoma | Causa provável | Solução |
|---|---|---|
| Menu **Port** cinza ou vazio | Placa não detectada, ou snap sem acesso USB | Me mande o resultado do B.1 |
| `Permission denied` ao gravar | Usuário fora do grupo dialout | Administrador (pedido pronto) |
| `/dev/ttyUSB0` aparece e some | brltty pegou a porta (chip CH340) | Administrador remove o brltty |
| `Failed to connect to ESP32` | Placa fora do modo de gravação | Segurar BOOT durante o `Connecting...` |
| Caracteres estranhos no Monitor | Velocidade errada | 115200 baud |
