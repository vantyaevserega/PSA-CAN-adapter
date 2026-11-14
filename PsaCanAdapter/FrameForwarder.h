// Пересылка выбранных пакетов шины в USB: только изменения и редкое обновление,
// без ожидания порта — чтобы вывод не задерживал приём шины.
#pragma once

#include <Arduino.h>
#include <mcp2515.h>

class FrameForwarder {
 public:
  // Запомнить принятый пакет, если его ID есть в списке FORWARDED_FRAMES.
  void onFrame(const struct can_frame& frame);

  // Отправить то, что пора отправить и что помещается в буфер порта.
  void flush(unsigned long now);
};
