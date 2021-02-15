#include "recordreader.h"
#include "record.h"
#include <algorithm>

namespace fcgi{

RecordReader::RecordReader(std::function<void(Record&)> recordReadedHandler)
    : recordReadedHandler_(recordReadedHandler)
    , stream_(&buffer_)
{    
}

void RecordReader::read(const char *data, std::size_t size)
{
    buffer_ = InputStreamDualBuffer{leftover_.c_str(), leftover_.size(), data, size};
    stream_.rdbuf(&buffer_);
    readedRecordsSize_ = 0;
    findRecords(data, size);
}

void RecordReader::findRecords(const char *data, std::size_t size)
{
    auto record = Record{};
    const auto dataSize = leftover_.size() + size;
    auto recordSize = record.fromStream(stream_, dataSize - readedRecordsSize_);
    while (recordSize){        
        leftover_.clear();
        readedRecordsSize_ += recordSize;
        recordReadedHandler_(record);
        recordSize = record.fromStream(stream_, dataSize - readedRecordsSize_);
    }
    if (readedRecordsSize_ < dataSize){
        if (leftover_.empty()){
            const auto leftoverSize = dataSize - readedRecordsSize_;
            leftover_ = std::string{data + size - leftoverSize, data + size};
        }
        else
            leftover_ += std::string{data, data + size};
    }
}

void RecordReader::removeBrokenRecord(std::size_t recordSize, const char *data, std::size_t size)
{
    readedRecordsSize_ += recordSize;
    findRecords(data, size);
}

void RecordReader::clear()
{
    leftover_.clear();
    readedRecordsSize_ = 0;
}

}
