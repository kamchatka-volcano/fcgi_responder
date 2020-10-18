#include "recordreader.h"
#include "record.h"
#include <sstream>

using namespace fcgi;

RecordReader::RecordReader(std::function<void(Record&)> recordReadedHandler)
    : recordReadedHandler_(recordReadedHandler)
{
}

void RecordReader::addData(const std::string &data)
{
    if (readedBytes_ > 0){
        buffer_.erase(buffer_.begin(), buffer_.begin() + static_cast<int32_t>(readedBytes_));
        readedBytes_ = 0;
    }
    buffer_ += data;
    auto record = Record{};
    auto bufferStream = std::istringstream{buffer_};
    auto recordSize = record.fromStream(bufferStream, buffer_.size());
    while (recordSize){
        readedBytes_ += recordSize;
        recordReadedHandler_(record);
        recordSize = record.fromStream(bufferStream, buffer_.size() - readedBytes_);
    }
}

void RecordReader::removeBrokenRecord(std::size_t recordSize)
{
    readedBytes_ = recordSize;
    addData({});
}

void RecordReader::clear()
{
    buffer_.clear();
}
