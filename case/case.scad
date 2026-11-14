// Корпус адаптера PSA-CAN, два варианта (раскладка плат — в hardware/README.md):
//   full — макетная плата 50x70 мм: Arduino Nano, MCP2515, DC-DC Mini360, оптопары, X9C104;
//   lite — макетная плата 50x48 мм (5x7, обрезанная по длине): только Arduino Nano и MCP2515.
//
// Плата лежит на полках, с торцов упирается в бобышки, сверху её прижимают стойки крышки.
// Крышка крепится четырьмя саморезами 2,2x9,5 мм (или M2.5x8) в бобышки по углам.
//
// Экспорт: openscad -D "variant=\"full\"" -D "part=\"base\"" -o base-full.stl case.scad
//          (и то же для part "lid" и variant "lite")

variant = "full";  // [full, lite]
part = "assembly"; // [assembly, base, lid]
lite = variant == "lite";

/* [Плата] */
board = lite ? [48, 50, 1.6] : [70, 50, 1.6];
board_gap = 0.4;       // зазор платы до стенок
standoff = 4;          // высота под платой (пайка)
// высота над платой: full — Nano в панельках и клеммник MCP2515; lite — модули впаяны
headroom = lite ? 16 : 20;

/* [Корпус] */
wall = 2;
floor_t = 2;
lid_t = 2;
corner_r = 2.5;
boss = 5;              // сторона квадратной бобышки под саморез
screw_pilot = 1.9;     // отверстие под саморез 2,2 мм
screw_clear = 2.6;     // отверстие в крышке
screw_head = 4.8;      // потай под головку
fit = 0.25;            // зазор посадки крышки

/* [Разъём USB Nano — торец X=0] */
usb_y = lite ? 10.5 : 14.5;  // центр разъёма от края платы
// центр разъёма над платой: в панельке 11 + плата Nano 1,6 + 2; впаянная Nano — 2,5 + 1,6 + 2
usb_z = lite ? 6.1 : 14.6;
usb_w = 13;            // окно под корпус штекера mini-USB
usb_h = 9;

/* [Вывод проводов — торец X=max] */
cable_y = lite ? 35.5 : 25;  // центр от края платы
cable_z = lite ? 6 : 8;      // низ окна над платой
cable_w = lite ? 10 : 14;    // lite — только CAN-H / CAN-L
cable_h = lite ? 5 : 6;

/* [Стойки крышки, прижимающие плату: координаты от угла платы] */
post_d = 4;
post_inset = 2.5;
posts = lite
  ? [[board.x - post_inset, post_inset], [post_inset, board.y - post_inset]]
  : [for (x = [post_inset, board.x - post_inset], y = [post_inset, board.y - post_inset]) [x, y]];

/* [Крепление] */
ears = true;           // ушки под стяжки
ear = [12, 8, 2.5];
tie_slot = [6, 2.5];

/* [Вид] */
show_parts = true;     // показать платы в сборке

$fn = 48;
eps = 0.01;

// ---------- Производные размеры (начало координат — внутренний угол полости на дне) ----------
margin = boss + 0.3;                              // от торца полости до платы
inner = [board.x + 2 * margin, board.y + 2 * board_gap, standoff + board.z + headroom];
outer = [inner.x + 2 * wall, inner.y + 2 * wall];
board_pos = [margin, board_gap, standoff];        // угол платы
board_top = standoff + board.z;

function boss_centers() = [
  for (x = [boss / 2, inner.x - boss / 2], y = [boss / 2, inner.y - boss / 2]) [x, y]
];

module rounded_box(size, r) {
  hull() for (x = [r, size.x - r], y = [r, size.y - r])
    translate([x, y, 0]) cylinder(r = r, h = size.z);
}

// Вырез в торцевой стенке: от z0 (над платой) до верха стенки
module end_notch(at_max_x, y_center, width, z0) {
  x = at_max_x ? inner.x - eps : -wall - eps;
  translate([x, board_pos.y + y_center - width / 2, board_top + z0])
    cube([wall + 2 * eps, width, inner.z]);
}

module base() {
  difference() {
    union() {
      difference() {
        translate([-wall, -wall, -floor_t]) rounded_box([outer.x, outer.y, floor_t + inner.z], corner_r);
        cube([inner.x, inner.y, inner.z + eps]);
      }
      // бобышки под саморезы — они же упоры платы с торцов
      for (c = boss_centers()) translate([c.x - boss / 2, c.y - boss / 2, 0]) cube([boss, boss, inner.z]);
      // полки под длинные края платы
      for (y = [0, inner.y - 1.4]) translate([margin, y, 0]) cube([board.x, 1.4, standoff]);
      if (ears) for (side = [0, 1])
        translate([inner.x / 2 - ear.x / 2, side ? inner.y + wall - eps : -wall - ear.y + eps, -floor_t])
          difference() {
            cube(ear);
            translate([(ear.x - tie_slot.x) / 2, (ear.y - tie_slot.y) / 2, -eps])
              cube([tie_slot.x, tie_slot.y, ear.z + 2 * eps]);
          }
    }
    for (c = boss_centers()) translate([c.x, c.y, inner.z - 12]) cylinder(d = screw_pilot, h = 12 + eps);
    end_notch(false, usb_y, usb_w, usb_z - usb_h / 2);
    end_notch(true, cable_y, cable_w, cable_z);
  }
}

// Крышка в положении «на корпусе»: нижняя плоскость пластины на z = inner.z
module lid() {
  lip_h = 3;
  lip_t = 1.6;
  usb_top = usb_z + usb_h / 2;
  cable_top = cable_z + cable_h;

  difference() {
    union() {
      translate([-wall, -wall, inner.z]) rounded_box([outer.x, outer.y, lid_t], corner_r);
      // бортик вдоль длинных стенок (между бобышками)
      for (y = [fit, inner.y - fit - lip_t])
        translate([boss + fit, y, inner.z - lip_h]) cube([inner.x - 2 * (boss + fit), lip_t, lip_h + eps]);
      // стойки, прижимающие плату
      for (p = posts)
        translate([board_pos.x + p.x, board_pos.y + p.y, board_top + 0.3])
          cylinder(d = post_d, h = headroom - 0.3 + eps);
      // языки, закрывающие вырезы в торцах над окнами
      lid_tongue(false, usb_y, usb_w, usb_top);
      lid_tongue(true, cable_y, cable_w, cable_top);
    }
    for (c = boss_centers()) {
      translate([c.x, c.y, inner.z - eps]) cylinder(d = screw_clear, h = lid_t + 2 * eps);
      translate([c.x, c.y, inner.z + lid_t - 1]) cylinder(d = screw_head, h = 1 + eps);
    }
  }
}

module lid_tongue(at_max_x, y_center, width, z_top) {
  h = headroom - z_top;
  if (h > 0.6)
    translate([at_max_x ? inner.x + fit / 2 : -wall, board_pos.y + y_center - width / 2 + fit, board_top + z_top])
      cube([wall - fit / 2, width - 2 * fit, h + eps]);
}

// Условные габариты плат — только для просмотра сборки
module parts() {
  nano_z = usb_z - 3.6;  // низ платы Nano над макетной платой
  translate(board_pos) {
    color("darkgreen") cube(board);
    translate([0, 0, board.z]) {
      color("royalblue") translate([-3, usb_y - 9, nano_z]) cube([43, 18, 1.6]);       // Nano
      color("silver") translate([-4.5, usb_y - 3.9, nano_z + 1.6]) cube([9, 7.8, 4]);  // mini-USB
      if (lite) {
        color("steelblue") translate([5, 21.5, 2.5]) cube([40, 28, 1.6]);              // MCP2515
        color("seagreen") translate([37, 28, 4.1]) cube([8, 15, 10]);                  // клеммник CAN
      } else {
        color("steelblue") translate([42, 5, 9]) cube([28, 40, 1.6]);                  // MCP2515
        color("seagreen") translate([44, 35, 10.6]) cube([15, 8, 10]);                 // клеммник CAN
        color("orange") translate([2, 34, 2]) cube([17, 11, 4]);                       // Mini360
        color("purple") translate([10, 27, 8]) cube([28, 5, 1.6]);                     // X9C104 (на ребре)
        color("dimgray") for (i = [0, 1]) translate([24 + i * 8, 38, 0]) cube([5, 7, 4]); // PC817
      }
    }
  }
}

if (part == "base") {
  base();
} else if (part == "lid") {
  // для печати: пластиной вниз
  translate([0, inner.y, inner.z + lid_t]) rotate([180, 0, 0]) lid();
} else {
  base();
  if (show_parts) parts();
  %lid();
}
