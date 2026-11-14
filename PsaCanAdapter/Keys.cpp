#include "Keys.h"

#include "Config.h"

// Положения X9C первых шести кнопок совпадают с прежней прошивкой — магнитолу переучивать не нужно.
const KeyInfo KEY_INFO[KEY_COUNT] = {
  { "VOL_DOWN", 2, 18, 'v', 'q' },
  { "VOL_UP", 4, 24, 'V', 'Q' },
  { "MUTE", 6, 30, 'm', 'w' },
  { "SEEK_FWD", 8, 36, 'n', 'W' },
  { "SEEK_BACK", 14, 44, 'b', 'B' },
  { "SOURCE", 54, 66, 'o', 'O' },
  { "SCROLL_DOWN", 10, 0, 't', 0 },
  { "SCROLL_UP", 12, 0, 'T', 0 },
  { "MODE", 0, 0, 0, 0 },
  { "MENU", 0, 0, 0, 0 },
  { "ESC", 0, 0, 0, 0 },
  { "OK", 0, 0, 0, 0 },
};

// Пакет 0x21F: байт 0 — кнопки, байт 1 — счётчик колёсика.
void Keys::onWheelFrame(const uint8_t* data, uint8_t dlc, unsigned long now) {
  if (dlc < 2) {
    return;
  }

  uint8_t buttons = data[0];
  bool mute = (buttons & 0x0C) == 0x0C;  // обе кнопки громкости сразу
  set(KEY_VOL_DOWN, !mute && (buttons & 0x04), now);
  set(KEY_VOL_UP, !mute && (buttons & 0x08), now);
  set(KEY_MUTE, mute, now);
  set(KEY_SEEK_FWD, buttons & 0x80, now);
  set(KEY_SEEK_BACK, buttons & 0x40, now);
  set(KEY_SOURCE, buttons & 0x02, now);

  // Счётчик колёсика переполняется через 255 — разница как знаковый байт это учитывает.
  int8_t delta = scrollKnown ? (int8_t)(data[1] - lastScroll) : 0;
  lastScroll = data[1];
  scrollKnown = true;
  if (delta > 0) {
    onKey(KEY_SCROLL_UP, false);
  } else if (delta < 0) {
    onKey(KEY_SCROLL_DOWN, false);
  }
}

// Пакет 0x0A2 (5 байт): байт 1 — кнопки меню.
void Keys::onMenuFrame(const uint8_t* data, uint8_t dlc, unsigned long now) {
  if (dlc != 5) {
    return;
  }

  uint8_t buttons = data[1];
  set(KEY_MODE, buttons & 0x04, now);
  set(KEY_MENU, buttons & 0x08, now);
  set(KEY_ESC, buttons & 0x10, now);
  set(KEY_OK, buttons & 0x20, now);
}

// Событие приходит при отпускании: по времени удержания оно короткое или долгое.
void Keys::set(Key key, bool down, unsigned long now) {
  State& state = states[key];
  if (down && !state.down) {
    state.down = true;
    state.downAt = now;
  } else if (!down && state.down) {
    state.down = false;
    unsigned long threshold = key >= KEY_MODE ? MENU_LONG_MS : WHEEL_LONG_MS;
    onKey(key, now - state.downAt >= threshold);
  }
}

void Keys::reset() {
  for (uint8_t key = 0; key < KEY_COUNT; ++key) {
    states[key].down = false;
  }
  scrollKnown = false;
}
