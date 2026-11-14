# PSA-CAN-adapter

Прослойка между комфортной CAN-шиной автомобиля PSA (Citroën C4) и магнитолой на Android (или любой другой).

Что делает адаптер:

- кнопки руля (пакеты 0x21F, 0x0A2) → события в USB и сопротивление на проводе KEY магнитолы;
- задняя передача (0x0F6) и подсветка (0x128) → события в USB и выходы +12 В на магнитолу;
- сообщения и данные бортового компьютера, двери, скорость и др. → USB для приложения
  [PSADisplay](https://github.com/vantyaevserega/PSADisplay);
- команды из порта → кнопки бортового компьютера на штатном дисплее (эмуляция панели магнитолы, 0x3E5).

## Варианты

| | FULL | LITE |
|---|---|---|
| Состав | Nano, MCP2515, DC-DC, 2 оптопары, X9C104 | Nano, MCP2515 |
| Кнопки, задний ход, подсветка | USB + провода KEY / BACK / ILL | только USB |
| Питание | +12 В автомобиля | USB магнитолы |

Схемы, перечень деталей, раскладка плат и корпуса (OpenSCAD + STL) — в [hardware/README.md](hardware/README.md).

## Сборка прошивки

Железо: Arduino Nano, MCP2515 с кварцем 8 МГц, шина 125 кбит/с, CS на D10. Вариант и остальные настройки —
в [`PsaCanAdapter/Config.h`](PsaCanAdapter/Config.h).

Репозиторий устроен как папка скетчей Arduino: в Arduino IDE достаточно указать его корень в
*File → Preferences → Sketchbook location* (или скопировать содержимое `libraries/` в свою папку библиотек)
и открыть `PsaCanAdapter/PsaCanAdapter.ino`.

Из командной строки:

```
arduino-cli compile --fqbn arduino:avr:nano --libraries libraries PsaCanAdapter
arduino-cli compile --fqbn arduino:avr:nano --libraries libraries --build-property "build.extra_flags=-DVARIANT=2" PsaCanAdapter   # LITE
arduino-cli upload  --fqbn arduino:avr:nano -p COM5 PsaCanAdapter
```

Для Nano со старым загрузчиком — `--fqbn arduino:avr:nano:cpu=atmega328old`.

## Порт

USB, 115200 бод. Адаптер пишет строки `CAN …`, `KEY …`, `STATE …`, `MODE …` и принимает односимвольные
команды — см. [docs/PROTOCOL.md](docs/PROTOCOL.md). Переход с прошивки 1.x и изменения для приложения —
[docs/MIGRATION.md](docs/MIGRATION.md).

## Состав репозитория

| Путь | Что |
|---|---|
| `PsaCanAdapter/` | прошивка адаптера |
| `TestScreen/` | стенд для проверки штатного дисплея без автомобиля (эмуляция BSI и магнитолы) |
| `libraries/PsaEmulators/` | эмуляторы узлов шины: кнопки панели, BSI, магнитола |
| `libraries/autowp-mcp2515/` | библиотека MCP2515 ([autowp/arduino-mcp2515](https://github.com/autowp/arduino-mcp2515)) |
| `hardware/` | схемы и перечень деталей |
| `case/` | корпус: `case.scad`, STL; `корпус-v1.skp` — прежний корпус (SketchUp) |
| `docs/` | протокол порта, переход с 1.x |
| `tools/` | настройки Serial Bluetooth Terminal (команды под прошивку 1.x, 9600 бод) |
