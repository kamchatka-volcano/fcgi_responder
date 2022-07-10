#pragma once
#include <limits>
#include <cstdint>

namespace fcgi{
namespace hardcoded{
const uint8_t protocolVersion = 1;
const uint8_t keepConnectionMask = 1;
const uint8_t headerSize = 8;
const uint32_t maxDataMessageSize = std::numeric_limits<uint16_t>::max();
const uint32_t maxRecordPaddingSize = std::numeric_limits<uint8_t>::max();
const uint32_t maxRecordSize = headerSize + maxDataMessageSize + maxRecordPaddingSize;
}
}
