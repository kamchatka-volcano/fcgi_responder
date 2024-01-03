#pragma once
#include <cstdint>
#include <limits>

namespace fcgi {
namespace hardcoded {
const std::uint8_t protocolVersion = 1;
const std::uint8_t keepConnectionMask = 1;
const std::uint8_t headerSize = 8;
const std::uint32_t maxDataMessageSize = std::numeric_limits<std::uint16_t>::max();
const std::uint32_t maxRecordPaddingSize = std::numeric_limits<std::uint8_t>::max();
const std::uint32_t maxRecordSize = headerSize + maxDataMessageSize + maxRecordPaddingSize;
} //namespace hardcoded
} //namespace fcgi
