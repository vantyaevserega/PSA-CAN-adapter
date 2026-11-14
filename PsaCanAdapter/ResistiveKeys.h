// Резистивные кнопки магнитолы (провод KEY) на цифровом потенциометре X9C104.
#pragma once

#include <Arduino.h>

class ResistiveKeys {
 public:
  void begin();

  // Выставить положение потенциометра на durationMs, затем вернуть «не нажато».
  void press(uint8_t position, unsigned int durationMs, unsigned long now);

  // true, пока предыдущее нажатие ещё удерживается.
  bool busy() const { return pressed; }

  void release();
  void tick(unsigned long now);

 private:
  void moveTo(uint8_t position);
  void step(bool up, uint8_t count);

  uint8_t current;
  bool pressed;
  unsigned long releaseAt;
};
