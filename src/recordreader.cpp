#include "recordreader.h"
#include "record.h"
#include <algorithm>

namespace fcgi{

RecordReader::RecordReader(
        std::function<void(Record&)> recordReadHandler,
        std::function<void(uint8_t)> invalidRecordTypeHandler)
    : recordReadHandler_{std::move(recordReadHandler)}
    , invalidRecordTypeHandler_{std::move(invalidRecordTypeHandler)}
{
    dataStream_.exceptions(std::istream::failbit | std::istream::badbit | std::istream::eofbit);
}

void RecordReader::read(const char *data, std::size_t size)
{
    readRecordsSize_ = 0;
    while(doRead(data, size) == ReadResultAction::ContinueReading);
}

void RecordReader::setErrorInfoHandler(const std::function<void(const std::string&)>& errorInfoHandler)
{
    errorInfoHandler_ = errorInfoHandler;
}

void RecordReader::findRecords(const char *data, std::size_t size)
{
    auto record = Record{};
    const auto dataSize = leftover_.size() + size;
    auto recordSize = record.fromStream(dataStream_, dataSize - readRecordsSize_);
    while (recordSize){
        leftover_.clear();
        readRecordsSize_ += recordSize;
        recordReadHandler_(record);
        recordSize = record.fromStream(dataStream_, dataSize - readRecordsSize_);
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

void RecordReader::skipBrokenRecord(std::size_t recordSize)
{
    readRecordsSize_ += recordSize - leftover_.size();
    leftover_.clear();
}

RecordReader::ReadResultAction RecordReader::doRead(const char* data, std::size_t size)
{
    dataStream_ = DataReaderStream{leftover_, std::string_view{data + readRecordsSize_, size}};
    try {
        findRecords(data, size);
    }
    catch(const InvalidRecordType& e){
        invalidRecordTypeHandler_(e.recordType());
        notifyAboutError(e.what());
        clear();
    }
    catch(const RecordMessageReadError& e){
        notifyAboutError(e.what());
        skipBrokenRecord(e.recordSize());
        return ReadResultAction::ContinueReading;
    }
    catch(const std::exception& e){
        notifyAboutError(e.what());
        clear();
    }
    return ReadResultAction::StopReading;
}

void RecordReader::clear()
{
    readRecordsSize_ = 0;
    leftover_.clear();
}

void RecordReader::notifyAboutError(const std::string& errorInfo)
{
    if (errorInfoHandler_)
        errorInfoHandler_(errorInfo);
}

}
