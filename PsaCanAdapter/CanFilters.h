// Аппаратный фильтр приёма MCP2515: до Nano доходят только нужные пакеты.
// Значения — в Config.h (CAN_RX_GROUP_MASK / CAN_RX_GROUP_FILTERS), проверка покрытия — при сборке.
#pragma once

#include <mcp2515.h>

#include "Config.h"

namespace canfilters {

struct Rule {
  uint16_t id;
  uint16_t minIntervalMs;
};

constexpr Rule FORWARDED[] = { FORWARDED_FRAMES };
constexpr uint8_t FORWARDED_COUNT = sizeof(FORWARDED) / sizeof(FORWARDED[0]);
constexpr uint16_t GROUP[] = { CAN_RX_GROUP_FILTERS };
constexpr uint8_t GROUP_COUNT = sizeof(GROUP) / sizeof(GROUP[0]);
static_assert(GROUP_COUNT == 4, "У буфера 1 MCP2515 ровно 4 фильтра: CAN_RX_GROUP_FILTERS — четыре значения");

constexpr bool inGroup(uint16_t id, uint8_t i = 0) {
  return i < GROUP_COUNT &&
         (((id & CAN_RX_GROUP_MASK) == (GROUP[i] & CAN_RX_GROUP_MASK)) || inGroup(id, i + 1));
}

constexpr bool allForwardedPass(uint8_t i = 0) {
  return i >= FORWARDED_COUNT || (inGroup(FORWARDED[i].id) && allForwardedPass(i + 1));
}

static_assert(allForwardedPass(), "Пакет из FORWARDED_FRAMES не проходит аппаратный фильтр — поправьте CAN_RX_GROUP_FILTERS в Config.h");
static_assert(inGroup(ID_REVERSE) && inGroup(ID_LAMPS), "ID_REVERSE / ID_LAMPS не проходят аппаратный фильтр");

// all = true — пропускать всю шину (лог), false — только нужное. Оставляет чип в рабочем режиме.
inline void apply(MCP2515& can, bool all) {
  can.setFilterMask(MCP2515::MASK0, false, all ? 0 : 0x7FF);
  can.setFilter(MCP2515::RXF0, false, ID_WHEEL_KEYS);
  can.setFilter(MCP2515::RXF1, false, ID_MENU_KEYS);
  can.setFilterMask(MCP2515::MASK1, false, all ? 0 : CAN_RX_GROUP_MASK);
  can.setFilter(MCP2515::RXF2, false, GROUP[0]);
  can.setFilter(MCP2515::RXF3, false, GROUP[1]);
  can.setFilter(MCP2515::RXF4, false, GROUP[2]);
  can.setFilter(MCP2515::RXF5, false, GROUP[3]);
  can.setNormalMode();
}

}  // namespace canfilters
