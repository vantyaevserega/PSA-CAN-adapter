// Эмуляция штатной магнитолы на столе: пакеты, по которым дисплей считает её включённой.
#ifndef _PsaRadioEmulator_H_
#define _PsaRadioEmulator_H_

#include <mcp2515.h>

class PsaRadioEmulator {
 public:
  explicit PsaRadioEmulator(MCP2515* sender);

  void DoWork(unsigned long now);

  // Разовый пакет инициализации (0x5E0).
  void Init();

  void SetState(bool inWork);

 private:
  bool active;
  unsigned long last100;
  unsigned long last500;
  struct can_frame radioState;     // 0x165
  struct can_frame radioLevel;     // 0x1A5
  struct can_frame radioSettings;  // 0x1E5
  struct can_frame radioPresence;  // 0x39B
  MCP2515* sender;
};

#endif
