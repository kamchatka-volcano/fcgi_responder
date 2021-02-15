#pragma once
#include <cstdint>

namespace fcgi{
    
    const uint8_t cProtocolVersion = 1;
    const uint8_t cHeaderSize = 8;
    const uint8_t cKeepConnectionMask = 1;
    const uint32_t cMaxRecordSize = 65551;
    const uint32_t cMaxDataMessageSize = 65528;
}
