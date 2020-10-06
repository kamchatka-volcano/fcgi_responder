#include <msgbeginrequest.h>
#include <msgendrequest.h>
#include <msggetvalues.h>
#include <msggetvaluesresult.h>
#include <msgparams.h>
#include <msgunknowntype.h>
#include <streamdatamessage.h>
#include <record.h>
#include <recordreader.h>
#include <streammaker.h>
#include <gtest/gtest.h>
#include <sstream>

namespace{

bool operator==(const fcgi::Record& lhs, const fcgi::Record& rhs)
{
    return lhs.type() == rhs.type() &&
           lhs.requestId() == rhs.requestId() &&
           lhs.messageData() == rhs.messageData();
}

}

TEST(Utils, RecordReader)
{
    auto msg = fcgi::MsgParams{};
    msg.setParam("TEST", "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.");
    msg.setParam("HELLO", "WORLD");
    auto record = fcgi::Record{msg, 1};

    auto recordData = std::string{};
    auto recordStream = std::ostringstream{};
    record.toStream(recordStream);
    recordData = recordStream.str();

    auto readedRecordList = std::vector<fcgi::Record>{};
    auto recordReadHandler = [&readedRecordList](const fcgi::Record& record)
    {
        readedRecordList.push_back(record);
    };

    auto recordReader = fcgi::RecordReader{recordReadHandler};

    auto chunk = std::string{};
    for (auto byte : recordData){
        chunk.push_back(byte);
        if (chunk.size() == 10){
            recordReader.addData(chunk);
            chunk.clear();
        }
    }
    if (!chunk.empty())
        recordReader.addData(chunk);

    ASSERT_EQ(readedRecordList.size(), 1);
    ASSERT_TRUE(record == readedRecordList.front());
}

TEST(Utils, StreamMaker)
{
    auto str = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.";
    auto streamMaker = fcgi::StreamMaker(16);
    auto recordList = streamMaker.makeStream(fcgi::RecordType::StdOut, 1, str);

    auto resultStr = std::string{};
    for (const auto& record : recordList)
        resultStr += record.getMessage<fcgi::MsgStdOut>().data();

    ASSERT_EQ(str, resultStr);
}

