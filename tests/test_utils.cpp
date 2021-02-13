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

TEST(Utils, RecordReader)
{
    auto msg = fcgi::MsgParams{};
    msg.setParam("TEST", "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.");
    msg.setParam("HELLO", "WORLD");
    auto record = fcgi::Record{std::move(msg), 1};

    auto recordData = std::string{};
    auto recordStream = std::ostringstream{};
    record.toStream(recordStream);
    recordData = recordStream.str();

    auto readedRecordList = std::vector<fcgi::Record>{};
    auto recordReadHandler = [&readedRecordList](fcgi::Record& record)
    {
        readedRecordList.push_back(record);
    };

    auto recordReader = std::make_unique<fcgi::RecordReader>(recordReadHandler);

    auto chunk = std::string{};
    for (auto byte : recordData){
        chunk.push_back(byte);
        if (chunk.size() == 10){
            recordReader->read(chunk.c_str(), chunk.size());
            chunk.clear();
        }
    }
    if (!chunk.empty())
        recordReader->read(chunk.c_str(), chunk.size());

    ASSERT_EQ(readedRecordList.size(), 1);
    ASSERT_TRUE(record == readedRecordList.front());
}


TEST(Utils, StreamMaker)
{
    auto str = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.";
    auto streamMaker = fcgi::StreamMaker(16);
    auto recordList = streamMaker.makeStream<fcgi::MsgStdOut>(1, str);

    auto resultStr = std::string{};
    for (const auto& record : recordList)
        resultStr += record.getMessage<fcgi::MsgStdOut>().data();

    ASSERT_EQ(str, resultStr);
}

