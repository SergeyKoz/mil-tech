# Тестування ДЗ11 на Raspberry Pi 4 / 5

Обидві програми (ваша + чекер) крутяться на одній малині (зайдіть по ssh, зручно tmux
або кілька терміналів). START/DROP — реальні піни (перемички «вихід студента → вхід чекера»).
UART — або віртуальний (socat), або два апаратних порти.

## 0. Який бінар брати

- Pi 4 / Pi 5 з 64-біт ОС → **checker_pi_arm64**
- Перевірте розрядність: `uname -m` (aarch64 = 64-біт).

## 1. Номер gpiochip (важливо!)

```
gpiodetect
```
- **Pi 4** — головна гребінка це **gpiochip0**
- **Pi 5** — це **gpiochip4** (гребінка через чип RP1), НЕ gpiochip0

Далі в командах підставляйте свій `gpiochipN`.

## 2. Перемички START / DROP (однаково для Pi 4 і Pi 5)

Дві перемички на 40-піновій гребінці, вихід вашої програми → вхід чекера:

| Сигнал | Ваш вихід (BCM / пін) | → | Вхід чекера (BCM / пін) |
|--------|----------------------|---|-------------------------|
| START  | GPIO24 / пін 18      | → | GPIO27 / пін 13         |
| DROP   | GPIO23 / пін 16      | → | GPIO22 / пін 15         |

Спільна земля не потрібна — одна плата.

---

## Варіант А — UART через socat (простий, рекомендований)

Нічого не вмикати в config.txt, працює однаково на Pi 4 і Pi 5.

```
# термінал 1 — віртуальна пара портів:
socat -d -d pty,raw,echo=0,link=/tmp/ttyA pty,raw,echo=0,link=/tmp/ttyB

# термінал 2 — чекер (GPIO реальні, UART віртуальний):
sudo ./checker_pi_arm64 1 --hw --uart /tmp/ttyB --gpiochip gpiochipN --start-line 27 --drop-line 22

# термінал 3 — ваша програма:
sudo ./student --uart /tmp/ttyA --gpiochip gpiochipN --start-line 24 --drop-line 23
```

Порядок: спершу socat, потім чекер (він чекає START), потім ваша програма.

---

## Варіант Б — два апаратних UART (повністю залізний тракт, за бажанням)

### Увімкнути другий UART

У `/boot/firmware/config.txt` додати рядок і перезавантажитись:

- **Pi 4:** `dtoverlay=uart3`  → другий порт на GPIO4(TX, пін 7)/GPIO5(RX, пін 29)
- **Pi 5:** `dtoverlay=uart2-pi5` → другий порт на GPIO4(TX, пін 7)/GPIO5(RX, пін 29)

Також звільнити головний порт від консолі: `sudo raspi-config` →
Interface Options → Serial Port → «login shell over serial?» **No**, «serial hardware enabled?» **Yes**.

Після ребуту перевірити, які зʼявились:
```
ls /dev/ttyAMA*
```

### Зʼєднати два UART навхрест (TX↔RX)

Чекер сидить на головному UART (GPIO14 TX пін 8 / GPIO15 RX пін 10),
ваша програма — на другому (GPIO4 TX пін 7 / GPIO5 RX пін 29):

| Напрям | З (пін) | → | На (пін) |
|--------|---------|---|----------|
| чекер → студент | чекер TX GPIO14 / пін 8 | → | студ RX GPIO5 / пін 29 |
| студент → чекер | студ TX GPIO4 / пін 7   | → | чекер RX GPIO15 / пін 10 |

Разом з перемичками START/DROP виходить 4 проводи.

### Запуск

```
sudo ./checker_pi_arm64 1 --hw --uart /dev/ttyAMA0 --gpiochip gpiochipN --start-line 27 --drop-line 22
sudo ./student --uart /dev/ttyAMA1 --gpiochip gpiochipN --start-line 24 --drop-line 23
```
(імена портів — з `ls /dev/ttyAMA*`; на Pi 5 другий може називатись інакше — беріть із виводу.)

---

## Дрібниці

- `sudo` обовʼязковий для доступу до /dev/gpiochip і апаратного UART (або додайте себе в групи `gpio`, `dialout`).
- Швидкість UART у коді має збігатися з боку чекера і студента (115200).
- Якщо чекер пише «Chekayu START» і не рухається — перевірте перемичку START і що драйвите правильний gpiochip (на Pi 5 це gpiochip4).
