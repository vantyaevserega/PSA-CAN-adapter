// Эмуляция BSI на столе: минимум пакетов комфортной шины, чтобы штатный дисплей
// включился, плюс перебор сообщений бортового компьютера (пакет 0x1A1).
#ifndef _PsaBsiEmulator_H_
#define _PsaBsiEmulator_H_

#include <mcp2515.h>

class PsaBsiEmulator {
 public:
  explicit PsaBsiEmulator(MCP2515* sender);

  void DoWork(unsigned long now);

  void SetState(bool inWork);

  // Показать сообщение с кодом 0..255; другое значение убирает сообщение.
  void SetMessage(unsigned long now, int message);

  // Перебор всех сообщений по одному раз в 5 секунд.
  void StartMessages(unsigned long now);

  void StopMessages(unsigned long now);

 private:
  bool active;
  bool cycleMessages;
  int message;
  unsigned long last100;
  unsigned long last500;
  unsigned long previousMessage;
  unsigned long previousCycle;
  struct can_frame ignition;     // 0x036
  struct can_frame engine;       // 0x0F6
  struct can_frame clock;        // 0x276
  struct can_frame infoMessage;  // 0x1A1
  MCP2515* sender;
};

#endif
