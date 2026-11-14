// Эмуляция кнопок панели штатной магнитолы PSA (пакет 0x3E5, раз в 100 мс):
// ими листается и настраивается бортовой компьютер на штатном дисплее.
#ifndef _PsaPanelButtons_H_
#define _PsaPanelButtons_H_

#include <mcp2515.h>

class PsaPanelButtons {
 public:
  // Номера совпадают с командами 1-9 последовательного порта.
  enum Button {
    NONE,
    MODE,
    DOWN,
    DARK,
    LEFT,
    OK,
    RIGHT,
    ESCAPE,
    UP,
    MENU
  };

  explicit PsaPanelButtons(MCP2515* sender);

  void DoWork(unsigned long now);

  void SetState(bool inWork);

  // Нажать кнопку: уйдёт в ближайшем пакете. NONE ожидающее нажатие не отменяет.
  void SetButton(Button button);

 private:
  bool active;
  unsigned long last100;
  Button current;
  struct can_frame frame;
  MCP2515* sender;
};

#endif
