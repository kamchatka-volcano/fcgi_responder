#include "recordreader.h"
#include "record.h"
#include <algorithm>

namespace fcgi{

RecordReader::RecordReader(std::function<void(Record&)> recordReadedHandler)
    : recordReadedHandler_(recordReadedHandler)
{    
    stream_.rdbuf()->pubsetbuf(&buffer_[0], static_cast<std::streamsize>(buffer_.size()));
    begReceivedData_ = &buffer_[0];
    endReceivedData_ = begReceivedData_;
}

void RecordReader::addData(const char* data, std::size_t dataSize)
{
    std::copy(data, data + dataSize, endReceivedData_);
    std::advance(endReceivedData_, dataSize);
    stream_.seekg(std::distance(&buffer_[0], begReceivedData_));
    auto record = Record{};
    auto recordSize = record.fromStream(stream_, receivedDataSize());
    while(recordSize){
        begReceivedData_ += recordSize;
        recordReadedHandler_(record);
        recordSize = record.fromStream(stream_, receivedDataSize());
    }
    if (std::distance(&buffer_[0], begReceivedData_) > cMaxRecordSize){
        std::copy(begReceivedData_, endReceivedData_, &buffer_[0]);
        endReceivedData_ = &buffer_[0] + receivedDataSize();
        begReceivedData_ = &buffer_[0];
    }
}

std::size_t RecordReader::receivedDataSize() const
{
    return static_cast<std::size_t>(std::distance(begReceivedData_, endReceivedData_));
}

void RecordReader::removeBrokenRecord(std::size_t recordSize)
{
    std::advance(begReceivedData_, recordSize);
    addData(nullptr, 0);
}

void RecordReader::clear()
{
    begReceivedData_ = &buffer_[0];
    endReceivedData_ = begReceivedData_;
}

}
