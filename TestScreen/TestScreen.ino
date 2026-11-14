// Стенд для проверки штатного дисплея PSA без автомобиля: эмулирует BSI, магнитолу
// и кнопки панели. Управление — джойстик и кнопки на пинах либо команды в порт:
//   1-9 — кнопка панели (см. PsaPanelButtons::Button), A — перебор сообщений БК,
//   B / C — включить / выключить эмуляцию BSI.
// Всё, что приходит с шины, печатается в порт.

#include <SPI.h>
#include <mcp2515.h>
#include <PsaBsiEmulator.h>
#include <PsaPanelButtons.h>
#include <PsaRadioEmulator.h>

#define PIN_VRX A0
#define PIN_VRY A1
#define PIN_OK 2
#define PIN_ESC 4
#define PIN_MENU 5
#define PIN_MODE 6
#define PIN_DARK 7
#define PIN_CAN_CS 10

#define JOYSTICK_CENTER 512
#define JOYSTICK_DEADZONE 100

struct PinButton {
  uint8_t pin;
  uint8_t mode;
  PsaPanelButtons::Button button;
  bool last;
};

PinButton pinButtons[] = {
  { PIN_OK, INPUT_PULLUP, PsaPanelButtons::OK, LOW },
  { PIN_ESC, INPUT, PsaPanelButtons::ESCAPE, LOW },
  { PIN_MODE, INPUT, PsaPanelButtons::MODE, LOW },
  { PIN_DARK, INPUT, PsaPanelButtons::DARK, LOW },
  { PIN_MENU, INPUT, PsaPanelButtons::MENU, LOW },
};

MCP2515 can(PIN_CAN_CS);
PsaRadioEmulator radio(&can);
PsaBsiEmulator car(&can);
PsaPanelButtons buttons(&can);

// Чтение пина с защитой от дребезга.
bool debounce(uint8_t pin, bool last) {
  bool current = digitalRead(pin);
  if (current != last) {
    delay(5);
    current = digitalRead(pin);
  }
  return current;
}

void printFrame(const struct can_frame& frame) {
  Serial.print(frame.can_id, HEX);
  Serial.print('\t');
  Serial.print(frame.can_dlc, HEX);
  for (uint8_t i = 0; i < frame.can_dlc; ++i) {
    Serial.print('\t');
    Serial.print(frame.data[i], HEX);
  }
  Serial.println();
}

void setup() {
  for (PinButton& b : pinButtons) {
    pinMode(b.pin, b.mode);
  }

  Serial.begin(115200);
  Serial.println(F("Start work"));

  can.reset();
  can.setBitrate(CAN_125KBPS, MCP_8MHZ);
  can.setNormalMode();

  radio.SetState(true);
  buttons.SetState(true);
  radio.Init();
}

void loop() {
  unsigned long now = millis();
  PsaPanelButtons::Button pressed = PsaPanelButtons::NONE;

  int x = analogRead(PIN_VRX);
  int y = analogRead(PIN_VRY);
  if (x < JOYSTICK_CENTER - JOYSTICK_DEADZONE) {
    pressed = PsaPanelButtons::LEFT;
  } else if (x > JOYSTICK_CENTER + JOYSTICK_DEADZONE) {
    pressed = PsaPanelButtons::RIGHT;
  } else if (y < JOYSTICK_CENTER - JOYSTICK_DEADZONE) {
    pressed = PsaPanelButtons::DOWN;
  } else if (y > JOYSTICK_CENTER + JOYSTICK_DEADZONE) {
    pressed = PsaPanelButtons::UP;
  }

  for (PinButton& b : pinButtons) {
    bool current = debounce(b.pin, b.last);
    if (b.last == LOW && current == HIGH) {
      pressed = b.button;
    }
    b.last = current;
  }

  if (Serial.available() > 0) {
    char command = (char)Serial.read();
    if (command >= '1' && command <= '9') {
      car.StopMessages(now);
      pressed = (PsaPanelButtons::Button)(command - '0');
    } else if (command == 'A') {
      car.StartMessages(now);
    } else if (command == 'B') {
      car.SetState(true);
    } else if (command == 'C') {
      car.SetState(false);
    }
  }

  buttons.SetButton(pressed);
  radio.DoWork(now);
  car.DoWork(now);
  buttons.DoWork(now);

  struct can_frame frame;
  if (can.readMessage(&frame) == MCP2515::ERROR_OK) {
    printFrame(frame);
  }
}
