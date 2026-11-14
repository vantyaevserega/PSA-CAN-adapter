#include "PsaRadioEmulator.h"

#include <string.h>

static void fill(struct can_frame& frame, canid_t id, uint8_t dlc, const uint8_t* data) {
  frame.can_id = id;
  frame.can_dlc = dlc;
  memcpy(frame.data, data, dlc);
}

PsaRadioEmulator::PsaRadioEmulator(MCP2515* sender)
  : active(false), last100(0), last500(0), sender(sender) {
  static const uint8_t stateData[] = { 0xCC, 0x80, 0x40, 0x00 };
  static const uint8_t levelData[] = { 0xEF };
  static const uint8_t settingsData[] = { 0x3F, 0x48, 0x3F, 0x3F, 0x3F, 0x07, 0x0F };
  static const uint8_t presenceData[] = { 0xF4, 0x05, 0x0C, 0x14, 0x1E };
  fill(radioState, 0x165, 4, stateData);
  fill(radioLevel, 0x1A5, 1, levelData);
  fill(radioSettings, 0x1E5, 7, settingsData);
  fill(radioPresence, 0x39B, 5, presenceData);
}

void PsaRadioEmulator::DoWork(unsigned long now) {
  if (!active) {
    return;
  }

  if (now - last100 >= 100) {
    sender->sendMessage(&radioState);
    last100 = now;
  }

  if (now - last500 >= 500) {
    sender->sendMessage(&radioSettings);
    sender->sendMessage(&radioLevel);
    sender->sendMessage(&radioPresence);
    last500 = now;
  }
}

void PsaRadioEmulator::Init() {
  // Прежний код дважды присваивал байт 6 (0x20, затем 0x11) и не задавал байт 7;
  // здесь то, что фактически уходило в шину, с нулём в последнем байте.
  static const uint8_t initData[] = { 0x20, 0x01, 0x02, 0x06, 0x12, 0x1F, 0x11, 0x00 };
  struct can_frame radioInit;
  fill(radioInit, 0x5E0, 8, initData);
  sender->sendMessage(&radioInit);
}

void PsaRadioEmulator::SetState(bool inWork) {
  active = inWork;
}
