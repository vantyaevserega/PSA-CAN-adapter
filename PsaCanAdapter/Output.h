// Порт связи с магнитолой: USB и (по желанию) его копия в Bluetooth.
#pragma once

#include <Arduino.h>
#include <mcp2515.h>

class Output : public Print {
 public:
  void begin();

  using Print::write;
  size_t write(uint8_t c) override;
  size_t write(const uint8_t* buffer, size_t size) override;

  // Очередной принятый символ команды или -1.
  int read();

  // true, если строка пакета уйдёт в USB без ожидания (цикл не затормозится).
  bool hasRoomForFrame();

  // Строка пакета: `[CAN ]ID<TAB>DLC<TAB>B0<TAB>B1...` — HEX без ведущих нулей.
  void writeFrame(const struct can_frame& frame, bool tagged);
};

extern Output out;
