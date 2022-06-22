#pragma once
#include <cstdint>

namespace fcgi{
namespace hardcoded {
const uint8_t protocolVersion = 1;
const uint8_t headerSize = 8;
const uint8_t keepConnectionMask = 1;
const uint32_t maxRecordSize = 65551;
const uint32_t maxDataMessageSize = 65528;
}
}
