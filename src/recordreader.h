#pragma once
#include "inputstreamdualbuffer.h"
#include "constants.h"
#include <string>
#include <functional>
#include <sstream>
#include <memory>

namespace fcgi{

class Record;

class RecordReader{
public:
    explicit RecordReader(std::function<void(Record&)> recordReadHandler);
    void read(const char* data, std::size_t size);
    void clear();
    void removeBrokenRecord(std::size_t recordSize, const char *data, std::size_t size);

private:
    void findRecords(const char* data, std::size_t size);    

private:    
    std::function<void(Record&)> recordReadHandler_;
    InputStreamDualBuffer buffer_;
    std::istream stream_;
    std::string leftover_;
    std::size_t readRecordsSize_ = 0;
};

}

