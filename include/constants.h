#pragma once
#include <cstdint>

namespace fcgi{

    const uint8_t cProtocolVersion = 1;
    const uint8_t cHeaderSize = 8;
    const uint8_t cKeepConnectionMask = 1;
    const int cDefaultMaxConnectionsNumber = 1;
    const int cDefaultMaxRequestsNumber = 10;
    const bool cDefaultMultiplexingEnabled = true;

}
