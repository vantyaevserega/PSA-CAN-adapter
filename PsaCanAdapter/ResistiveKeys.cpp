#include "ResistiveKeys.h"

#include "Config.h"

#if USE_RESISTIVE_KEYS

static const uint8_t X9C_STEPS = 99;

void ResistiveKeys::begin() {
  pinMode(PIN_X9C_INC, OUTPUT);
  pinMode(PIN_X9C_UD, OUTPUT);
  pinMode(PIN_X9C_CS, OUTPUT);
  digitalWrite(PIN_X9C_INC, HIGH);
  digitalWrite(PIN_X9C_CS, HIGH);

  // Положение после включения неизвестно: уходим в ноль и поднимаемся в «не нажато».
  step(false, X9C_STEPS);
  current = 0;
  moveTo(RESISTIVE_IDLE);

  // Один раз запоминаем «не нажато» в памяти X9C (подъём CS при высоком INC),
  // чтобы до старта прошивки магнитола не видела нажатой кнопки.
  digitalWrite(PIN_X9C_INC, HIGH);
  digitalWrite(PIN_X9C_CS, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_X9C_CS, HIGH);
  delay(20);
}

void ResistiveKeys::press(uint8_t position, unsigned int durationMs, unsigned long now) {
  moveTo(position);
  pressed = true;
  releaseAt = now + durationMs;
}

void ResistiveKeys::release() {
  moveTo(RESISTIVE_IDLE);
  pressed = false;
}

void ResistiveKeys::tick(unsigned long now) {
  if (pressed && (long)(now - releaseAt) >= 0) {
    release();
  }
}

void ResistiveKeys::moveTo(uint8_t position) {
  if (position > X9C_STEPS) {
    position = X9C_STEPS;
  }
  if (position > current) {
    step(true, position - current);
  } else if (position < current) {
    step(false, current - position);
  }
  current = position;
}

// Движок сдвигается по спаду INC. CS поднимается при низком INC — так X9C не пишет
// положение в свою память: её ресурс 100 000 записей, а нажатий за жизнь адаптера больше.
void ResistiveKeys::step(bool up, uint8_t count) {
  if (count == 0) {
    return;
  }
  digitalWrite(PIN_X9C_UD, up ? HIGH : LOW);
  digitalWrite(PIN_X9C_INC, HIGH);
  digitalWrite(PIN_X9C_CS, LOW);
  delayMicroseconds(2);
  while (true) {
    digitalWrite(PIN_X9C_INC, LOW);
    delayMicroseconds(2);
    if (--count == 0) {
      break;
    }
    digitalWrite(PIN_X9C_INC, HIGH);
    delayMicroseconds(2);
  }
  digitalWrite(PIN_X9C_CS, HIGH);
  digitalWrite(PIN_X9C_INC, HIGH);
}

#endif
