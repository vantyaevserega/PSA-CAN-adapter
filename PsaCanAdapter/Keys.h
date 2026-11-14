// Кнопки руля: разбор пакетов 0x21F и 0x0A2 в события «короткое / долгое нажатие».
#pragma once

#include <Arduino.h>

enum Key : uint8_t {
  // подрулевой блок магнитолы, пакет 0x21F
  KEY_VOL_DOWN,
  KEY_VOL_UP,
  KEY_MUTE,
  KEY_SEEK_FWD,
  KEY_SEEK_BACK,
  KEY_SOURCE,
  KEY_SCROLL_DOWN,
  KEY_SCROLL_UP,
  // блок кнопок меню, пакет 0x0A2
  KEY_MODE,
  KEY_MENU,
  KEY_ESC,
  KEY_OK,
  KEY_COUNT
};

struct KeyInfo {
  const char* name;   // имя в строке `KEY <имя>` протокола USB
  uint8_t potShort;   // положение X9C для короткого нажатия, 0 — нет
  uint8_t potLong;    // положение X9C для долгого нажатия, 0 — нет
  char configShort;   // символ команды в режиме обучения магнитолы, 0 — нет
  char configLong;
};

extern const KeyInfo KEY_INFO[KEY_COUNT];

// Событие кнопки; определяется в скетче. Приходит при отпускании кнопки:
// долгое — если её держали не меньше WHEEL_LONG_MS (кнопки меню — MENU_LONG_MS).
void onKey(Key key, bool isLong);

class Keys {
 public:
  void onWheelFrame(const uint8_t* data, uint8_t dlc, unsigned long now);
  void onMenuFrame(const uint8_t* data, uint8_t dlc, unsigned long now);

  // Забыть все удерживаемые кнопки без событий.
  void reset();

 private:
  struct State {
    bool down;
    unsigned long downAt;
  };

  void set(Key key, bool down, unsigned long now);

  State states[KEY_COUNT];
  uint8_t lastScroll;
  bool scrollKnown;
};
