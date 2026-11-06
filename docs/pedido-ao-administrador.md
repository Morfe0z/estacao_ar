# Pedido ao administrador do laboratório (Ubuntu 24.04)

Para gravar programas numa placa ESP32 via USB, o usuário precisa de acesso às portas seriais USB.
Por favor, rodar (trocando USUARIO pelo nome de usuário, ex.: o seu login):

```
sudo usermod -aG dialout USUARIO
```

Se a placa usar o chip CH340 (aparece como "QinHeng Electronics" no `lsusb`), o serviço brltty
toma a porta no Ubuntu 24.04. Se ninguém usa display braille nesta máquina:

```
sudo apt remove brltty
```

Depois disso, o usuário precisa sair da sessão e entrar de novo (ou reiniciar).
Verificação: `groups` deve listar `dialout`, e com a placa conectada `ls -l /dev/ttyUSB0` deve existir.

Observação: a Arduino IDE foi instalada como snap pelo App Center. Se o menu de portas da IDE não listar
`/dev/ttyUSB0` mesmo após o passo acima, conectar a interface USB do snap
(o nome do snap aparece em `snap list`, ex.: `arduino`):

```
sudo snap connect arduino:raw-usb
```
