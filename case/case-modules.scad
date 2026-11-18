// Корпус адаптера для навесного монтажа: Arduino Nano и модуль MCP2515 стоят прямо в корпусе
// на угловых штырьках (без макетной платы). Корпус собирается винтами, крышка прижимает модули.
// Выходы: торец с USB Nano и торец с двумя проводами CAN (у клеммника MCP2515).
//
// Размеры модулей — типовые, перед печатью сверить со своими платами (особенно высоту клеммника
// MCP2515 и положение USB Nano). Все параметры ниже.
//
// Экспорт:
//   openscad -D "part=\"base\"" -o base.stl  case-modules.scad
//   openscad -D "part=\"lid\""  -o lid.stl   case-modules.scad
//   openscad -D "part=\"base\"" -o base.3mf  case-modules.scad   // для OrcaSlicer
//   openscad -D "part=\"lid_tall\"" -o lid-tall.stl case-modules.scad // крышка с бортиком на старое основание

part = "assembly";  // [assembly, base, lid, lid_tall]

/* [Модули] */
// Arduino Nano: габариты платы. Плата ложится краями на торцы угловых стоек, чип/детали — снизу.
nano = [46.0, 18.0, 1.6];
nano_tall = 4.0;      // детали над платой (штырьки гребёнки и т.п.) — для высоты полости
// Окно USB (micro): ширина с запасом; по высоте — от уровня опоры стоек вниз и вверх.
nano_usb_w = 12.0;    // ширина окна
nano_usb_below = 6.0; // окно ниже уровня стоек (разъём уходит под плату)
nano_usb_above = 2.0; // окно выше уровня стоек
// Модуль MCP2515: габариты платы и высота самого высокого элемента (клеммник).
mcp = [40.0, 28.0, 1.6];
mcp_tall = 11.0;     // клеммник/кварц над платой
mcp_term_w = 10.0;   // ширина окна под провода CAN у клеммника

/* [Посадка] */
standoff = 7.0;      // высота опоры: плата лежит на торцах стоек, под ней место под чип/детали/провода
wire_above = 25.0;   // просвет над уровнем стоек: дюпон «мама» на гребёнке ~16.5 над платой + изгиб провода ~5
base_wire_above = 20.0; // просвет уже напечатанного основания — для part="lid_tall" (крышка с бортиком)
post_d = 2.4;        // диаметр углового штырька
post_play = 0.4;     // зазор штырьков от края платы (XY-фиксация, с запасом под разброс платы)
ledge = 1.6;         // ширина уголка-опоры под плату
gap = 4.0;           // зазор между модулями и до стенок
usb_gap = 1.0;       // от торца Nano (с USB под платой) до стенки — разъём у самого окна
press_d = 3.0;       // колонна крышки над угловой стойкой
tab_over = 1.2;      // язычок колонны заходит на угол платы
finger = [1.6, 3.0]; // палец прижима у бобышки: вдоль торца платы X × поперёк Y

/* [Корпус] */
wall = 2.0;
floor_t = 2.0;
lid_t = 2.0;
corner_r = 2.5;
boss = 5.5;          // бобышка под винт в углах
screw_pilot = 2.5;   // отверстие под самонарез в бобышке (винт M3/2,9)
screw_clear = 3.2;   // отверстие в крышке
screw_head = 6.0;    // потай под головку
fit = 0.25;          // зазор посадки крышки
ears = true;         // ушки под стяжки
ear = [12, 8, 2.5];
tie_slot = [6, 2.5];

$fn = 48;
eps = 0.01;

// ---------- Раскладка ----------
// Модули вдоль X: Nano в своей дорожке (USB у X=0), MCP2515 — в соседней (клеммник у X=max).
// Начало координат — внутренний угол полости на дне.
corner_clear = boss + 0.5;                           // платы не заходят на угловые бобышки винтов
nano_pos = [usb_gap, corner_clear];                  // угол платы Nano (X,Y): торец с USB вплотную к стенке
mcp_pos  = [gap, nano_pos.y + nano.y + gap];         // угол платы MCP2515
inner = [
  max(nano_pos.x + nano.x, mcp_pos.x + mcp.x) + gap,
  mcp_pos.y + mcp.y + corner_clear,
  standoff + max(wire_above, nano.z + nano_tall + 2, mcp.z + mcp_tall + 2)
];
outer = [inner.x + 2 * wall, inner.y + 2 * wall];
board_rect = [[nano_pos, nano], [mcp_pos, mcp]];

function boss_centers() = [
  for (x = [boss / 2, inner.x - boss / 2], y = [boss / 2, inner.y - boss / 2]) [x, y]
];

module rounded_box(size, r) {
  hull() for (x = [r, size.x - r], y = [r, size.y - r])
    translate([x, y, 0]) cylinder(r = r, h = size.z);
}

// Четыре угловых штырька + уголки-опоры под одну плату. pos — угол платы [x,y], b — габариты платы.
module board_mount(pos, b) {
  for (cx = [0, 1], cy = [0, 1]) {
    px = pos.x + (cx ? b.x + post_d / 2 + post_play : -post_d / 2 - post_play);
    py = pos.y + (cy ? b.y + post_d / 2 + post_play : -post_d / 2 - post_play);
    translate([px, py, 0]) cylinder(d = post_d, h = standoff + b.z);   // до верха платы — сверху ложится колонна крышки
    // уголок-опора: плата ложится на него на высоте standoff
    lx = pos.x + (cx ? b.x - ledge : 0);
    ly = pos.y + (cy ? b.y - ledge : 0);
    translate([lx, ly, 0]) cube([ledge, ledge, standoff]);
  }
}

// Вырез в торцевой стенке над платой: at_max_x — у стенки X=max, иначе X=0.
module end_notch(at_max_x, y_center, width, z0, z1) {
  x = at_max_x ? inner.x - eps : -wall - eps;
  translate([x, y_center - width / 2, standoff + z0])
    cube([wall + 2 * eps, width, z1 - z0 + eps]);
}

module base() {
  difference() {
    union() {
      difference() {
        translate([-wall, -wall, -floor_t]) rounded_box([outer.x, outer.y, floor_t + inner.z], corner_r);
        cube([inner.x, inner.y, inner.z + eps]);
      }
      for (c = boss_centers()) translate([c.x - boss / 2, c.y - boss / 2, 0]) cube([boss, boss, inner.z]);
      for (r = board_rect) board_mount(r[0], r[1]);
      // разгрузка CAN-провода у выхода (X=max)
      tie_saddle(inner.x - 7, mcp_pos.y + mcp.y / 2);
      if (ears) for (side = [0, 1])
        translate([inner.x / 2 - ear.x / 2, side ? inner.y + wall - eps : -wall - ear.y + eps, -floor_t])
          difference() {
            cube(ear);
            translate([(ear.x - tie_slot.x) / 2, (ear.y - tie_slot.y) / 2, -eps])
              cube([tie_slot.x, tie_slot.y, ear.z + 2 * eps]);
          }
    }
    for (c = boss_centers()) translate([c.x, c.y, 2]) cylinder(d = screw_pilot, h = inner.z);
    // USB Nano у X=0: окно 12 мм, от 6 ниже уровня опоры до 2 выше
    end_notch(false, nano_pos.y + nano.y / 2, nano_usb_w, -nano_usb_below, nano_usb_above);
    // Провода CAN у X=max, у клеммника MCP2515
    end_notch(true, mcp_pos.y + mcp.y / 2, mcp_term_w, 0, mcp_tall);
  }
}

// Разгрузка натяжения: напольный мостик под стяжку у выхода CAN. Провода лежат сверху,
// zip-tie проходит сквозь поперечный туннель, охватывает провода — рывок за кабель идёт в корпус.
module tie_saddle(x_center, y_center) {
  sd = 6; sw = 10; sh = 8;       // след (X×Y) и высота мостика
  slot_w = 3.2; slot_h = 2.6;    // туннель под стяжку
  bridge = 1.4;                  // перемычка сверху над туннелем
  translate([x_center - sd / 2, y_center - sw / 2, 0]) difference() {
    cube([sd, sw, sh]);
    // туннель вдоль Y насквозь, у верха (стяжка проходит под перемычкой)
    translate([sd / 2 - slot_w / 2, -eps, sh - bridge - slot_h])
      cube([slot_w, sw + 2 * eps, slot_h]);
  }
}

module lid() {
  difference() {
    union() {
      translate([-wall, -wall, inner.z]) rounded_box([outer.x, outer.y, lid_t], corner_r);
      // прижим по углам: колонна над угловой стойкой + язычок на угол платы (там нет пинов),
      // середина над гребёнками свободна под дюпоны
      for (r = board_rect) {
        pos = r[0]; b = r[1];
        top = standoff + b.z;              // верх платы = верх стойки
        h = inner.z - top;
        for (cx = [0, 1], cy = [0, 1]) {
          px = pos.x + (cx ? b.x + post_d / 2 + post_play : -post_d / 2 - post_play);
          py = pos.y + (cy ? b.y + post_d / 2 + post_play : -post_d / 2 - post_play);
          // колонна внутри полости, не в стенке
          qx = min(max(px, press_d / 2 + fit), inner.x - press_d / 2 - fit);
          qy = min(max(py, press_d / 2 + fit), inner.y - press_d / 2 - fit);
          // у бобышки винта колонну не ставим — она бы упёрлась в бобышку
          near_boss = len([for (c = boss_centers())
            if (abs(qx - c.x) < boss / 2 + press_d / 2 + fit && abs(qy - c.y) < boss / 2 + press_d / 2 + fit) 1]) > 0;
          if (!near_boss) {
            translate([qx, qy, top]) cylinder(d = press_d, h = h + eps);
            // язычок над углом платы (угол свободен от пинов)
            tx = cx ? pos.x + b.x - tab_over : pos.x;
            ty = cy ? pos.y + b.y - tab_over : pos.y;
            translate([min(qx, tx), min(qy, ty), top])
              cube([abs(qx - tx) + tab_over, abs(qy - ty) + tab_over, 2]);
          } else {
            // у бобышки — узкий палец над торцевой полоской платы (до первых пинов ~2.7 мм)
            fx = cx ? pos.x + b.x - finger.x : pos.x;
            fy = cy ? pos.y + b.y - finger.y : pos.y;
            translate([fx, fy, top]) cube([finger.x, finger.y, h + eps]);
          }
        }
      }
    }
    for (c = boss_centers()) {
      translate([c.x, c.y, inner.z - eps]) cylinder(d = screw_clear, h = lid_t + 2 * eps);
      translate([c.x, c.y, inner.z + lid_t - 1]) cylinder(d = screw_head, h = 1 + eps);
    }
  }
}

// Условные платы — для просмотра сборки
module parts() {
  translate([nano_pos.x, nano_pos.y, standoff]) {
    color("royalblue") cube(nano);
    color("silver") translate([-1.5, nano.y / 2 - nano_usb_w / 2, -nano_usb_below]) cube([4, nano_usb_w, nano_usb_below + nano_usb_above]);
  }
  translate([mcp_pos.x, mcp_pos.y, standoff]) {
    color("steelblue") cube(mcp);
    color("seagreen") translate([mcp.x - 8, mcp.y / 2 - mcp_term_w / 2, mcp.z]) cube([8, mcp_term_w, mcp_tall]);
  }
}

// Бортик крышки: надставка стенок и бобышек на (wire_above - base_wire_above) мм.
// Нужна, чтобы поставить высокую крышку на основание, уже напечатанное с меньшим просветом.
module lid_skirt(h) {
  translate([0, 0, inner.z - h]) difference() {
    union() {
      difference() {
        translate([-wall, -wall, 0]) rounded_box([outer.x, outer.y, h + eps], corner_r);
        translate([0, 0, -eps]) cube([inner.x, inner.y, h + 3 * eps]);
      }
      for (c = boss_centers()) translate([c.x - boss / 2, c.y - boss / 2, 0]) cube([boss, boss, h + eps]);
    }
    for (c = boss_centers()) translate([c.x, c.y, -eps]) cylinder(d = screw_clear, h = h + 3 * eps);
  }
}

if (part == "base") {
  base();
} else if (part == "lid") {
  translate([0, inner.y, inner.z + lid_t]) rotate([180, 0, 0]) lid();  // печать пластиной вниз
} else if (part == "lid_tall") {
  // крышка с бортиком под основание, напечатанное с base_wire_above
  translate([0, inner.y, inner.z + lid_t]) rotate([180, 0, 0]) {
    lid();
    lid_skirt(wire_above - base_wire_above);
  }
} else {
  base();
  parts();
  %lid();
}
