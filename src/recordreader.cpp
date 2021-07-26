#include "recordreader.h"
#include "record.h"
#include <algorithm>

namespace fcgi{

RecordReader::RecordReader(std::function<void(Record&)> recordReadHandler)
    : recordReadHandler_(std::move(recordReadHandler))
    , stream_(&buffer_)
{    
}

void RecordReader::read(const char *data, std::size_t size)
{
    buffer_ = InputStreamDualBuffer{leftover_.c_str(), leftover_.size(), data, size};
    stream_.rdbuf(&buffer_);
    readRecordsSize_ = 0;
    findRecords(data, size);
}

void RecordReader::findRecords(const char *data, std::size_t size)
{
    auto record = Record{};
    const auto dataSize = leftover_.size() + size;
    auto recordSize = record.fromStream(stream_, dataSize - readRecordsSize_);
    while (recordSize){        
        leftover_.clear();
        readRecordsSize_ += recordSize;
        recordReadHandler_(record);
        recordSize = record.fromStream(stream_, dataSize - readRecordsSize_);
    }
    if (readRecordsSize_ < dataSize){
        if (leftover_.empty()){
            const auto leftoverSize = dataSize - readRecordsSize_;
            leftover_ = std::string{data + size - leftoverSize, data + size};
        }
        else
            leftover_ += std::string{data, data + size};
    }
}

void RecordReader::removeBrokenRecord(std::size_t recordSize, const char *data, std::size_t size)
{
    readRecordsSize_ += recordSize;
    findRecords(data, size);
}

void RecordReader::clear()
{
    leftover_.clear();
    readRecordsSize_ = 0;
}

}
