# Retomada na máquina nova (antes da Etapa 5)

Este guia substitui só a **preparação do computador** da Etapa 3. A montagem e os programas
das Etapas 1 a 4 continuam valendo exatamente como estão.

## O que já está pronto
| Etapa | Situação |
|---|---|
| 1. Inventário | ESP32 30 pinos (chip USB CH343, porta `/dev/ttyACM0`), BMP280, 1 DHT22 solto, MQ135, PMS7003 com adaptadora |
| 2. Mapa de ligações | Definido em [`etapa-02-mapa-de-ligacoes.md`](etapas/etapa-02-mapa-de-ligacoes.md) |
| 3. IDE + teste do ESP32 | Funcionou na máquina antiga (refazer a preparação aqui) |
| 4. DHT22 | Funcionou (25,6 °C, 50 %); revalidar na máquina nova |
| 5. BMP280 | Guia e programa prontos, **ainda não testado** |

## R1. Permissão da porta USB
Com a placa conectada pelo cabo USB, no Terminal:
```
lsusb | grep -i qinheng
ls -l /dev/ttyACM*
```
Esperado: uma linha `QinHeng Electronics ... 1a86:55d4` e o arquivo `/dev/ttyACM0`.

Libere a porta para esta placa (vale para sempre, inclusive depois de reiniciar):
```
echo 'SUBSYSTEM=="tty", ATTRS{idVendor}=="1a86", ATTRS{idProduct}=="55d4", MODE="0666"' | sudo tee /etc/udev/rules.d/99-esp32.rules
sudo udevadm control --reload-rules && sudo udevadm trigger
```
Desconecte e reconecte a placa e confira com `ls -l /dev/ttyACM0`: o começo tem que ser `crw-rw-rw-`.

## R2. Pacote do ESP32 na IDE
1. **File → Preferences → Additional boards manager URLs**, cole:
   `https://espressif.github.io/arduino-esp32/package_esp32_index.json`
2. **Tools → Board → Boards Manager**, pesquise **esp32** (by Espressif Systems) e instale a **versão mais recente**.
   (A versão 2.0.17 da máquina antiga era só um contorno da IDE antiga; aqui não é necessária.)
3. **Tools → Board → esp32 → ESP32 Dev Module** e **Tools → Port → /dev/ttyACM0**.

## R3. Bibliotecas
Em **Tools → Manage Libraries** (ou o ícone de livros na lateral):
1. **DHT sensor library** (Adafruit) → quando perguntar, **Install all** (traz a "Adafruit Unified Sensor").
2. **Adafruit BMP280 Library** → **Install all** (traz a "Adafruit BusIO").

## R4. Revalidar o DHT22
Mesma montagem e mesmo programa da Etapa 4 ([`firmware/etapa04_dht22/etapa04_dht22.ino`](../firmware/etapa04_dht22/etapa04_dht22.ino)), num **sketch novo**
(File → New). Monitor Serial em **115200**. Esperado: uma linha a cada 2,5 s com temperatura e umidade,
e a umidade sobe quando você fecha a mão em volta do sensor.

Depois disso, seguir para [`etapa-05-bmp280.md`](etapas/etapa-05-bmp280.md) (pode pular o passo 5.1, a biblioteca já foi instalada em R3).
