// Адаптер между комфортной CAN-шиной PSA (Citroën C4) и магнитолой на Android.
// Что делает:
//  - кнопки руля -> события `KEY ...` в USB и (полный вариант) сопротивление на проводе KEY;
//  - задний ход и подсветка -> строки `STATE ...` в USB и (полный вариант) выходы +12 В;
//  - выбранные пакеты шины -> USB для приложения PSADisplay;
//  - команды из порта -> кнопки бортового компьютера, настройка, отладка.
// Протокол порта описан в docs/PROTOCOL.md, состав железа и пины — в Config.h.

#include <SPI.h>
#include <mcp2515.h>
#include <PsaPanelButtons.h>

#include "Config.h"
#include "FrameForwarder.h"
#include "Keys.h"
#include "Output.h"
#include "ResistiveKeys.h"

MCP2515 can(PIN_CAN_CS);
PsaPanelButtons panel(&can);
Keys keys;
FrameForwarder forwarder;
#if USE_RESISTIVE_KEYS
ResistiveKeys resistive;
#endif

uint8_t debugLevel;  // 0 — тихо, 1 — события текстом, 2 — ещё и вся шина
bool logAll;         // вся шина в порт (команда L)
bool configMode;     // обучение магнитолы: шина не обрабатывается, кнопки нажимаются командами
bool reverseGear;    // задняя передача по шине
bool forceReverse;   // камера включена вручную
bool trailerMode;    // режим прицепа: задний ход на магнитолу не передаётся
bool illumination;   // подсветка по шине
bool reverseOut;     // сигнал «задний ход» для магнитолы
bool illuminationOut;  // сигнал «подсветка» для магнитолы
bool resistiveKeys = USE_RESISTIVE_KEYS;  // нажимать кнопки на проводе KEY (команды K / k)

void setup() {
#if USE_OUTPUTS
  // Выходы выключены до первого пакета шины
  digitalWrite(PIN_REVERSE, OUTPUT_OFF);
  digitalWrite(PIN_ILLUMINATION, OUTPUT_OFF);
  pinMode(PIN_REVERSE, OUTPUT);
  pinMode(PIN_ILLUMINATION, OUTPUT);
#endif

  out.begin();

  can.reset();
  can.setBitrate(CAN_SPEED, CAN_CLOCK);
  can.setNormalMode();

#if USE_RESISTIVE_KEYS
  resistive.begin();
#endif
  panel.SetState(EMULATE_PANEL_BUTTONS);

  printInfo();
}

void loop() {
  unsigned long now = millis();

  int command = out.read();
  if (command >= 0) {
    handleCommand((char)command, now);
  }

  // Буферов приёма у MCP2515 всего два — забираем всё, что накопилось
  struct can_frame frame;
  for (uint8_t i = 0; i < 4 && can.readMessage(&frame) == MCP2515::ERROR_OK; ++i) {
    handleFrame(frame, now);
  }

#if USE_RESISTIVE_KEYS
  resistive.tick(now);
#endif
  forwarder.flush(now);
  panel.DoWork(now);
}

// ---------- Шина ----------

void handleFrame(const struct can_frame& frame, unsigned long now) {
  if (logAll || debugLevel > 1) {
    out.writeFrame(frame, false);
  }
  if (configMode) {
    return;
  }

  forwarder.onFrame(frame);

  switch (frame.can_id) {
    case ID_WHEEL_KEYS:
      keys.onWheelFrame(frame.data, frame.can_dlc, now);
      break;
    case ID_MENU_KEYS:
      keys.onMenuFrame(frame.data, frame.can_dlc, now);
      break;
    case ID_REVERSE:
      if (frame.can_dlc == 8) {
        reverseGear = frame.data[7] & 0x80;
        applyReverse();
      }
      break;
    case ID_LAMPS:
      if (frame.can_dlc > 4) {
        illumination = frame.data[4] & ILLUMINATION_MASK;
        setIllumination(illumination);
      }
      break;
  }
}

// Сигналы магнитоле: строка STATE в порт при изменении и выход +12 В в полном варианте.

void applyReverse() {
  bool on = forceReverse || (reverseGear && !trailerMode);
  if (on != reverseOut) {
    reverseOut = on;
    printSwitch(F("STATE REVERSE"), on);
  }
  setOutput(PIN_REVERSE, on);
}

void setIllumination(bool on) {
  if (on != illuminationOut) {
    illuminationOut = on;
    printSwitch(F("STATE ILLUMINATION"), on);
  }
  setOutput(PIN_ILLUMINATION, on);
}

void setOutput(uint8_t pin, bool on) {
#if USE_OUTPUTS
  digitalWrite(pin, on ? OUTPUT_ON : OUTPUT_OFF);
#else
  (void)pin;
  (void)on;
#endif
}

// ---------- Кнопки руля ----------

void onKey(Key key, bool isLong) {
  const KeyInfo& info = KEY_INFO[key];

  out.print(F("KEY "));
  out.print(info.name);
  out.println(isLong ? F(" LONG") : F(""));

#if USE_RESISTIVE_KEYS
  uint8_t position = isLong ? info.potLong : info.potShort;
  if (resistiveKeys && position != 0 && !resistive.busy()) {
    resistive.press(position, RESISTIVE_PRESS_MS, millis());
  }
#endif

  // Служебные режимы: долгое OK — режим прицепа, долгое ESC — камера вручную
  if (isLong && key == KEY_OK) {
    trailerMode = !trailerMode;
    applyReverse();
    printSwitch(F("MODE TRAILER"), trailerMode);
  } else if (isLong && key == KEY_ESC) {
    forceReverse = !forceReverse;
    printSwitch(F("MODE REVERSE"), forceReverse);
    applyReverse();
  }
}

// ---------- Команды порта ----------

void handleCommand(char command, unsigned long now) {
  if (command >= '1' && command <= '9') {
    panel.SetButton((PsaPanelButtons::Button)(command - '0'));
    debug(F("panel button"));
    return;
  }

  switch (command) {
    case '?':
      printInfo();
      break;
    case 'S':
      debugLevel = 0;
      break;
    case 'D':
      debugLevel = 1;
      break;
    case 'F':
      debugLevel = 2;
      break;
    case 'L':
    case 'l':
      logAll = command == 'L';
      debug(logAll ? F("logging on") : F("logging off"));
      break;
    case 'R':
    case 'r':
      forceReverse = command == 'R';
      printSwitch(F("MODE REVERSE"), forceReverse);
      applyReverse();
      break;
    case 'K':
    case 'k':
      resistiveKeys = USE_RESISTIVE_KEYS && command == 'K';
      printSwitch(F("MODE KEYWIRE"), resistiveKeys);
      break;
    case 's':
      configMode = true;
      keys.reset();
      debug(F("configuration started"));
      break;
    case 'f':
      configMode = false;
#if USE_RESISTIVE_KEYS
      resistive.release();
#endif
      setIllumination(illumination);
      debug(F("configuration finished"));
      break;
    case 'I':
    case 'i':
      if (configMode) {
        setIllumination(command == 'I');
      }
      break;
    default:
      if (configMode) {
        pressConfigKey(command, now);
      }
      break;
  }
}

// Обучение магнитолы: держим сопротивление кнопки дольше обычного.
void pressConfigKey(char command, unsigned long now) {
#if USE_RESISTIVE_KEYS
  for (uint8_t key = 0; key < KEY_COUNT; ++key) {
    const KeyInfo& info = KEY_INFO[key];
    uint8_t position = 0;
    if (info.configShort == command) {
      position = info.potShort;
    } else if (info.configLong == command) {
      position = info.potLong;
    }
    if (position != 0) {
      resistive.press(position, RESISTIVE_CONFIG_MS, now);
      debug(info.name);
      return;
    }
  }
#else
  (void)command;
  (void)now;
#endif
}

// ---------- Служебный вывод ----------

// Ответ на `?` и строка при старте: версия, вариант и все текущие состояния.
void printInfo() {
  out.println(F("ADAPTER PSA-CAN " FW_VERSION
#if VARIANT == VARIANT_LITE
                " LITE"
#else
                " FULL"
#endif
  ));
  printSwitch(F("MODE TRAILER"), trailerMode);
  printSwitch(F("MODE REVERSE"), forceReverse);
  printSwitch(F("MODE KEYWIRE"), resistiveKeys);
  printSwitch(F("STATE REVERSE"), reverseOut);
  printSwitch(F("STATE ILLUMINATION"), illuminationOut);
}

void printSwitch(const __FlashStringHelper* name, bool on) {
  out.print(name);
  out.println(on ? F(" ON") : F(" OFF"));
}

void debug(const __FlashStringHelper* text) {
  if (debugLevel > 0) {
    out.print(F("# "));
    out.println(text);
  }
}

void debug(const char* text) {
  if (debugLevel > 0) {
    out.print(F("# "));
    out.println(text);
  }
}
