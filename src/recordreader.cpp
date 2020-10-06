#include "recordreader.h"
#include "record.h"
#include <sstream>

using namespace fcgi;

RecordReader::RecordReader(std::function<void(const Record&)> recordReadedHandler)
    : recordReadedHandler_(recordReadedHandler)
{
}

void RecordReader::addData(const std::string &data)
{
    buffer_ += data;
    auto record = Record{};
    auto bufferStream = std::istringstream{buffer_};
    auto recordSize = record.fromStream(bufferStream);
    while (recordSize){
        recordReadedHandler_(record);
        buffer_.erase(buffer_.begin(), buffer_.begin() + recordSize);
        recordSize = record.fromStream(bufferStream);
    }
}

void RecordReader::clear()
{
    buffer_.clear();
}
