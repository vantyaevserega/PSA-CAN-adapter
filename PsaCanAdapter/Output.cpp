#include "Output.h"

#include "Config.h"

#if USE_BLUETOOTH
#include <SoftwareSerial.h>
static SoftwareSerial bt(PIN_BT_RX, PIN_BT_TX);
#endif

// "CAN " + 8 цифр ID + TAB + DLC + 8 x (TAB + 2 цифры) + CRLF
static const uint8_t FRAME_LINE_MAX = 4 + 8 + 2 + 8 * 3 + 2;

Output out;

void Output::begin() {
  Serial.begin(USB_BAUD);
#if USE_BLUETOOTH
  bt.begin(BT_BAUD);
#endif
}

size_t Output::write(uint8_t c) {
#if USE_BLUETOOTH
  bt.write(c);
#endif
  return Serial.write(c);
}

size_t Output::write(const uint8_t* buffer, size_t size) {
#if USE_BLUETOOTH
  bt.write(buffer, size);
#endif
  return Serial.write(buffer, size);
}

int Output::read() {
  if (Serial.available() > 0) {
    return Serial.read();
  }
#if USE_BLUETOOTH
  if (bt.available() > 0) {
    return bt.read();
  }
#endif
  return -1;
}

bool Output::hasRoomForFrame() {
  return Serial.availableForWrite() >= FRAME_LINE_MAX;
}

static char* putHex(char* p, uint32_t value) {
  bool started = false;
  for (int8_t shift = 28; shift >= 0; shift -= 4) {
    uint8_t digit = (value >> shift) & 0x0F;
    if (digit != 0 || started || shift == 0) {
      *p++ = digit < 10 ? '0' + digit : 'A' + digit - 10;
      started = true;
    }
  }
  return p;
}

void Output::writeFrame(const struct can_frame& frame, bool tagged) {
  char line[FRAME_LINE_MAX];
  char* p = line;
  if (tagged) {
    *p++ = 'C';
    *p++ = 'A';
    *p++ = 'N';
    *p++ = ' ';
  }
  p = putHex(p, frame.can_id & CAN_EFF_MASK);
  *p++ = '\t';
  uint8_t dlc = frame.can_dlc > CAN_MAX_DLEN ? CAN_MAX_DLEN : frame.can_dlc;
  p = putHex(p, dlc);
  for (uint8_t i = 0; i < dlc; ++i) {
    *p++ = '\t';
    p = putHex(p, frame.data[i]);
  }
  *p++ = '\r';
  *p++ = '\n';
  write((const uint8_t*)line, p - line);
}
