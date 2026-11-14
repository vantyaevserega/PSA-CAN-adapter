#include "PsaBsiEmulator.h"

#include <string.h>

static void fill(struct can_frame& frame, canid_t id, uint8_t dlc, const uint8_t* data) {
  frame.can_id = id;
  frame.can_dlc = dlc;
  memcpy(frame.data, data, dlc);
}

PsaBsiEmulator::PsaBsiEmulator(MCP2515* sender)
  : active(false), cycleMessages(false), message(-1), last100(0), last500(0),
    previousMessage(0), previousCycle(0), sender(sender) {
  static const uint8_t ignitionData[] = { 0x0E, 0x00, 0x05, 0x2F, 0x21, 0x80, 0x00, 0xA0 };
  static const uint8_t engineData[] = { 0x08, 0x32, 0x00, 0x1F, 0x00, 0x0D, 0x40, 0x01 };
  static const uint8_t clockData[] = { 0x95, 0x03, 0x05, 0x08, 0x10, 0x1B, 0x10 };
  static const uint8_t infoData[] = { 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00 };
  fill(ignition, 0x036, 8, ignitionData);
  fill(engine, 0x0F6, 8, engineData);
  fill(clock, 0x276, 7, clockData);
  fill(infoMessage, 0x1A1, 8, infoData);
}

void PsaBsiEmulator::StartMessages(unsigned long now) {
  SetMessage(now, 0);
  cycleMessages = true;
  previousCycle = now;
}

void PsaBsiEmulator::StopMessages(unsigned long now) {
  SetMessage(now, -1);
  cycleMessages = false;
}

void PsaBsiEmulator::SetMessage(unsigned long now, int _message) {
  bool valid = _message > -1 && _message < 256;
  previousMessage = now;
  infoMessage.data[0] = valid ? 0x80 : 0;
  infoMessage.data[1] = valid ? _message : 0;
  message = _message;
}

void PsaBsiEmulator::DoWork(unsigned long now) {
  if (!active) {
    return;
  }

  if (cycleMessages && now - previousCycle > 5000) {
    SetMessage(now, ++message);
    previousCycle = now;
    if (message > 255) {
      cycleMessages = false;
    }
  }

  // Через 4 секунды сообщение гасится: сначала «скрыть», затем пусто
  if (now - previousMessage > 4000) {
    infoMessage.data[0] = infoMessage.data[0] == 0x80 ? 0x7F : 0;
  }

  if (now - last100 >= 100) {
    sender->sendMessage(&infoMessage);
    sender->sendMessage(&ignition);
    last100 = now;
  }

  if (now - last500 >= 500) {
    sender->sendMessage(&engine);
    sender->sendMessage(&clock);
    last500 = now;
  }
}

void PsaBsiEmulator::SetState(bool inWork) {
  active = inWork;
}
