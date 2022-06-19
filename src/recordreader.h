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
    enum class ReadResultAction{
        ContinueReading,
        StopReading
    };

public:
    RecordReader(std::function<void(Record&)> recordReadHandler,
                 std::function<void(uint8_t)> invalidRecordTypeHandler = {});
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
    std::function<void(uint8_t)> invalidRecordTypeHandler_;
    std::function<void(const std::string&)> errorInfoHandler_;
    InputStreamDualBuffer buffer_;
    std::istream stream_;
    std::string leftover_;
    std::size_t readRecordsSize_ = 0;
};

}

