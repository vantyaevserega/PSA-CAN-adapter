#include "PsaPanelButtons.h"

#include <string.h>

PsaPanelButtons::PsaPanelButtons(MCP2515* sender)
  : active(false), last100(0), current(NONE), sender(sender) {
  frame.can_id = 0x3E5;
  frame.can_dlc = 6;
  memset(frame.data, 0, sizeof(frame.data));
}

void PsaPanelButtons::DoWork(unsigned long now) {
  if (!active || now - last100 < 100) {
    return;
  }

  memset(frame.data, 0, sizeof(frame.data));
  switch (current) {
    case UP:
      frame.data[5] = 0x40;
      break;
    case DOWN:
      frame.data[5] = 0x10;
      break;
    case LEFT:
      frame.data[5] = 0x01;
      break;
    case RIGHT:
      frame.data[5] = 0x04;
      break;
    case OK:
      frame.data[2] = 0x40;
      break;
    case ESCAPE:
      frame.data[2] = 0x10;
      break;
    case DARK:
      frame.data[2] = 0x04;
      break;
    case MENU:
      frame.data[0] = 0x40;
      break;
    case MODE:
      frame.data[1] = 0x10;
      break;
    case NONE:
      break;
  }

  current = NONE;
  sender->sendMessage(&frame);
  last100 = now;
}

void PsaPanelButtons::SetState(bool inWork) {
  active = inWork;
}

void PsaPanelButtons::SetButton(Button button) {
  if (button == NONE) {
    return;
  }

  current = button;
}
