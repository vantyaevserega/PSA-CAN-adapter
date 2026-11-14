#include "FrameForwarder.h"

#include "Config.h"
#include "Output.h"

struct Rule {
  uint16_t id;
  uint16_t minIntervalMs;
};

static const Rule RULES[] = { FORWARDED_FRAMES };
static const uint8_t RULE_COUNT = sizeof(RULES) / sizeof(RULES[0]);

struct Slot {
  uint8_t data[CAN_MAX_DLEN];
  uint8_t dlc;
  bool changed;  // содержимое отличается от отправленного
  bool fresh;    // пакет приходил с шины после последней отправки
  uint16_t sentAt;
};

static Slot slots[RULE_COUNT];

void FrameForwarder::onFrame(const struct can_frame& frame) {
  for (uint8_t i = 0; i < RULE_COUNT; ++i) {
    if (RULES[i].id != frame.can_id) {
      continue;
    }
    Slot& slot = slots[i];
    uint8_t dlc = frame.can_dlc > CAN_MAX_DLEN ? CAN_MAX_DLEN : frame.can_dlc;
    if (slot.dlc != dlc || memcmp(slot.data, frame.data, dlc) != 0) {
      slot.dlc = dlc;
      memcpy(slot.data, frame.data, dlc);
      slot.changed = true;
    }
    slot.fresh = true;
    return;
  }
}

void FrameForwarder::flush(unsigned long now) {
  uint16_t now16 = (uint16_t)now;
  for (uint8_t i = 0; i < RULE_COUNT; ++i) {
    Slot& slot = slots[i];
    if (!slot.fresh) {
      continue;  // шина молчит — устаревшее не повторяем
    }
    uint16_t elapsed = now16 - slot.sentAt;
    bool due = slot.changed ? elapsed >= RULES[i].minIntervalMs : elapsed >= FORWARD_REFRESH_MS;
    if (!due) {
      continue;
    }
    if (!out.hasRoomForFrame()) {
      return;  // остальное уйдёт в следующих циклах
    }

    struct can_frame frame;
    frame.can_id = RULES[i].id;
    frame.can_dlc = slot.dlc;
    memcpy(frame.data, slot.data, slot.dlc);
    out.writeFrame(frame, true);

    slot.changed = false;
    slot.fresh = false;
    slot.sentAt = now16;
  }
}
