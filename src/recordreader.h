#pragma once
#include "constants.h"
#include "datareaderstream.h"
#include <functional>
#include <memory>
#include <sstream>
#include <string>

namespace fcgi {

class Record;

class RecordReader {
    enum class ReadResultAction {
        ContinueReading,
        StopReading
    };

public:
    explicit RecordReader(
            std::function<void(Record&)> recordReadHandler,
            std::function<void(std::uint8_t)> invalidRecordTypeHandler = {});
    void read(const char* data, std::size_t size);
    void setErrorInfoHandler(const std::function<void(const std::string&)>& errorInfoHandler);

private:
    ReadResultAction doRead(const char* data, std::size_t size);
    void findRecords(const char* data, std::size_t size);
    void notifyAboutError(const std::string& errorInfo);
    void clear();
    void skipBrokenRecord(std::size_t recordSize);

private:
    std::function<void(Record&)> recordReadHandler_;
    std::function<void(std::uint8_t)> invalidRecordTypeHandler_;
    std::function<void(const std::string&)> errorInfoHandler_;
    DataReaderStream dataStream_;
    std::string leftover_;
    std::size_t readRecordsSize_ = 0;
};

} //namespace fcgi
