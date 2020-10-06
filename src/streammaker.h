#pragma once
#include "record.h"
#include "types.h"
#include <vector>

namespace fcgi{

class StreamMaker{
public:
    StreamMaker(std::size_t maxDataMessageSize = 4096);
    std::vector<Record> makeStream(RecordType type, uint16_t requestId, const std::string& data);

private:
    std::size_t maxDataMessageSize_;
};

}

