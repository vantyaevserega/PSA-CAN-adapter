"""Генератор схем подключения: python gen_schematic.py — пишет schematic-full.svg и schematic-lite.svg.

Стиль «метки цепей»: блок — модуль, у каждого вывода подписана цепь; одинаковые метки соединены.
"""
import html
import pathlib

COLORS = {
    "pwr": "#c62828",
    "gnd": "#263238",
    "sig": "#1565c0",
    "can": "#2e7d32",
    "out": "#e65100",
    "nc": "#9e9e9e",
}
ROW = 26
STUB = 34


class Svg:
    def __init__(self, width, height, title):
        self.parts = [
            f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {width} {height}" '
            f'width="{width}" height="{height}" font-family="Segoe UI, Arial, sans-serif">',
            f'<rect width="{width}" height="{height}" fill="#ffffff"/>',
        ]
        self.text(24, 34, title, 20, "#111", weight="600")

    def text(self, x, y, value, size=13, fill="#222", anchor="start", weight="400"):
        self.parts.append(
            f'<text x="{x}" y="{y}" font-size="{size}" fill="{fill}" text-anchor="{anchor}" '
            f'font-weight="{weight}">{html.escape(value)}</text>'
        )

    def block(self, x, y, width, title, left=(), right=(), notes=()):
        """Модуль с выводами слева и справа; вывод — (имя, цепь, вид цепи)."""
        rows = max(len(left), len(right))
        height = 34 + rows * ROW + (8 + 17 * len(notes) if notes else 0) + 8
        self.parts.append(
            f'<rect x="{x}" y="{y}" width="{width}" height="{height}" rx="6" fill="#f5f7fa" '
            f'stroke="#455a64" stroke-width="1.5"/>'
        )
        self.parts.append(f'<path d="M{x} {y + 28}H{x + width}" stroke="#455a64" stroke-width="1"/>')
        self.text(x + width / 2, y + 19, title, 13.5, "#111", "middle", "600")

        for is_left, pins in ((True, left), (False, right)):
            for i, (name, net, kind) in enumerate(pins):
                py = y + 34 + i * ROW + ROW / 2
                color = COLORS[kind]
                edge = x if is_left else x + width
                end = edge - STUB if is_left else edge + STUB
                self.parts.append(f'<path d="M{edge} {py}H{end}" stroke="{color}" stroke-width="2.2"/>')
                self.parts.append(f'<circle cx="{end}" cy="{py}" r="3.2" fill="{color}"/>')
                if is_left:
                    self.text(x + 7, py + 4.5, name, 12.5)
                    self.text(end - 7, py + 4.5, net, 12.5, color, "end", "600")
                else:
                    self.text(x + width - 7, py + 4.5, name, 12.5, anchor="end")
                    self.text(end + 7, py + 4.5, net, 12.5, color, "start", "600")

        for i, note in enumerate(notes):
            self.text(x + 8, y + 34 + rows * ROW + 20 + i * 17, note, 11.5, "#546e7a")

    def legend(self, x, y, lines):
        for i, line in enumerate(lines):
            self.text(x, y + i * 18, line, 12.5, "#37474f")

    def save(self, path):
        path.write_text("\n".join(self.parts + ["</svg>"]) + "\n", encoding="utf-8", newline="\n")


NANO_SPI = [
    ("5V", "+5V", "pwr"),
    ("GND", "GND", "gnd"),
    ("D10", "CAN_CS", "sig"),
    ("D11 (MOSI)", "MOSI", "sig"),
    ("D12 (MISO)", "MISO", "sig"),
    ("D13 (SCK)", "SCK", "sig"),
]
MCP_SPI = [
    ("VCC", "+5V", "pwr"),
    ("GND", "GND", "gnd"),
    ("CS", "CAN_CS", "sig"),
    ("SI", "MOSI", "sig"),
    ("SO", "MISO", "sig"),
    ("SCK", "SCK", "sig"),
]


def full(path):
    g = Svg(1240, 800, "PSA-CAN адаптер — полный вариант (FULL)")

    g.block(150, 60, 170, "Автомобиль",
            right=[("+12 В (ACC)", "+12V_IN", "pwr"), ("Масса", "GND", "gnd"),
                   ("CAN-H", "CAN_H", "can"), ("CAN-L", "CAN_L", "can")],
            notes=["комфортная шина, 125 кбит/с"])
    g.block(150, 250, 170, "Защита",
            left=[("F1 → D1", "+12V_IN", "pwr"), ("D2 на массу", "GND", "gnd")],
            right=[("выход", "+12V", "pwr")],
            notes=["F1 — предохранитель 1 А", "D1 — SS14 (от переполюсовки)", "D2 — TVS P6KE18A"])
    g.block(150, 420, 170, "DC-DC Mini360",
            left=[("IN+", "+12V", "pwr"), ("IN−", "GND", "gnd")],
            right=[("OUT+", "+5V", "pwr"), ("OUT−", "GND", "gnd")],
            notes=["до пайки выставить 5,0 В"])

    g.block(540, 60, 190, "Arduino Nano",
            left=NANO_SPI,
            right=[("USB", "USB магнитолы", "out"), ("D2", "POT_CS", "sig"), ("D3", "POT_INC", "sig"),
                   ("D4", "POT_UD", "sig"), ("D8", "REV_N", "sig"), ("D9", "ILL_N", "sig")],
            notes=["D5 / D6 — HC-05 (TXD / RXD),", "только если USE_BLUETOOTH = 1"])
    g.block(540, 330, 190, "Модуль MCP2515 (8 МГц)",
            left=MCP_SPI,
            right=[("H", "CAN_H", "can"), ("L", "CAN_L", "can"), ("INT", "не подключён", "nc")],
            notes=["перемычку J1 (120 Ом) снять"])
    g.block(540, 590, 190, "Модуль X9C104",
            left=[("VCC", "+5V", "pwr"), ("GND", "GND", "gnd"), ("CS", "POT_CS", "sig"),
                  ("INC", "POT_INC", "sig"), ("U/D", "POT_UD", "sig")],
            right=[("VW", "KEY", "out"), ("VL", "GND", "gnd"), ("VH", "не подключён", "nc")])

    optos = (("OK1 PC817 — задний ход", "REV_N", "REVERSE", "R1", "R3"),
             ("OK2 PC817 — подсветка", "ILL_N", "ILL", "R2", "R4"))
    for i, (title, control, output, led_resistor, pull_down) in enumerate(optos):
        g.block(960, 60 + i * 200, 170, title,
                left=[(f"1 анод ← {led_resistor}", "+5V", "pwr"), ("2 катод", control, "sig"),
                      ("4 коллектор", "+12V", "pwr"), ("3 эмиттер", output, "out")],
                notes=[f"{led_resistor} 330 Ом — между +5V и выв. 1", f"{pull_down} 10 кОм — выв. 3 на GND"])
    g.block(960, 470, 170, "Магнитола",
            left=[("USB", "USB магнитолы", "out"), ("KEY (кнопки руля)", "KEY", "out"),
                  ("BACK (задний ход)", "REVERSE", "out"), ("ILL (подсветка)", "ILL", "out"),
                  ("Масса", "GND", "gnd")])

    g.legend(24, 640, [
        "Одинаковые метки цепей соединяются между собой.",
        "Выходы REVERSE и ILL — сигнальные (+12 В, до 5 мА).",
        "Если от линии заднего хода питается камера,",
        "вместо OK1 нужен ключ на P-MOSFET или реле.",
        "D8 / D9: низкий уровень = выход включён",
        "(совместимо со старым релейным модулем).",
    ])
    g.save(path)


def lite(path):
    g = Svg(1240, 420, "PSA-CAN адаптер — облегчённый вариант (LITE)")

    g.block(150, 70, 170, "Автомобиль",
            right=[("CAN-H", "CAN_H", "can"), ("CAN-L", "CAN_L", "can")],
            notes=["комфортная шина, 125 кбит/с"])
    g.block(150, 230, 170, "Магнитола",
            right=[("USB", "USB магнитолы", "out")],
            notes=["масса — общая с автомобилем", "через корпус магнитолы"])
    g.block(540, 70, 190, "Arduino Nano",
            left=NANO_SPI,
            right=[("USB", "USB магнитолы", "out")],
            notes=["питание — от USB магнитолы"])
    g.block(960, 70, 170, "Модуль MCP2515 (8 МГц)",
            left=MCP_SPI + [("H", "CAN_H", "can"), ("L", "CAN_L", "can")],
            notes=["перемычку J1 (120 Ом) снять", "INT не подключён"])

    g.legend(24, 380, [
        "Одинаковые метки цепей соединяются между собой. Кнопки руля, задний ход и подсветка "
        "уходят в USB строками KEY / STATE (docs/PROTOCOL.md).",
    ])
    g.save(path)


if __name__ == "__main__":
    here = pathlib.Path(__file__).parent
    full(here / "schematic-full.svg")
    lite(here / "schematic-lite.svg")
