#pragma once
#include "constants.h"
#include <string>
#include <functional>
#include <sstream>

namespace fcgi{

class Record;

class RecordReader{
public:
    RecordReader(std::function<void(Record&)> recordReadedHandler);
    void addData(const char* data, std::size_t size);
    void clear();
    void removeBrokenRecord(std::size_t recordSize);

private:
    std::size_t receivedDataSize() const;

private:
    std::array<char, cMaxRecordSize * 2> buffer_;
    std::istringstream stream_;
    char* endReceivedData_ = nullptr;
    char* begReceivedData_ = nullptr;
    std::function<void(Record&)> recordReadedHandler_;
};

}

