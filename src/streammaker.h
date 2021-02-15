#pragma once
#include "record.h"
#include "types.h"
#include "constants.h"
#include <vector>

namespace fcgi{

class StreamMaker{
public:
    StreamMaker(std::size_t maxDataMessageSize = cMaxDataMessageSize);
    std::vector<Record> makeStream(RecordType type, uint16_t requestId, std::string&& data);

private:
    std::size_t maxDataMessageSize_;
};

}

