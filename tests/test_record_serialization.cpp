#include <msgbeginrequest.h>
#include <msgendrequest.h>
#include <msggetvalues.h>
#include <msggetvaluesresult.h>
#include <msgparams.h>
#include <msgunknowntype.h>
#include <streamdatamessage.h>
#include <record.h>
#include <encoder.h>
#include <errors.h>
#include <namevalue.h>
#include <gtest/gtest.h>
#include <functional>

namespace{

void convertRecordTest(const fcgi::Record& record, fcgi::Record& resultRecord)
{
    auto output = std::ostringstream{};
    record.toStream(output);
    auto recordData = output.str();

    auto input = std::istringstream{recordData};
    auto recordSize = resultRecord.fromStream(input, recordData.size());
    ASSERT_TRUE(recordSize != 0);
    ASSERT_TRUE(record == resultRecord);
}

}


TEST(RecordSerialization, MsgBeginRequest)
{
    auto msg = fcgi::MsgBeginRequest{
                   fcgi::Role::Responder,
                   fcgi::ResultConnectionState::KeepOpen};
    auto record = fcgi::Record{msg, 1};
    auto resultRecord = fcgi::Record{};
    convertRecordTest(record, resultRecord);

    auto& readMsg = resultRecord.getMessage<fcgi::MsgBeginRequest>();
    ASSERT_EQ(msg.role(), readMsg.role());
    ASSERT_EQ(msg.resultConnectionState(), readMsg.resultConnectionState());
}

TEST(RecordSerialization, MsgEndRequest)
{
    auto msg = fcgi::MsgEndRequest{77, fcgi::ProtocolStatus::Overloaded};
    auto record = fcgi::Record{msg, 1};
    auto resultRecord = fcgi::Record{};
    convertRecordTest(record, resultRecord);

    auto readMsg = resultRecord.getMessage<fcgi::MsgEndRequest>();
    ASSERT_EQ(msg.appStatus(), readMsg.appStatus());
    ASSERT_EQ(msg.protocolStatus(), readMsg.protocolStatus());

}

TEST(RecordSerialization, MsgGetValues)
{
    auto msg = fcgi::MsgGetValues{};
    auto record = fcgi::Record{msg, 1};
    auto resultRecord = fcgi::Record{};
    convertRecordTest(record, resultRecord);

    auto readMsg = resultRecord.getMessage<fcgi::MsgGetValues>();
    ASSERT_EQ(msg.requestList(), readMsg.requestList());

    msg = fcgi::MsgGetValues{};
    msg.requestValue(fcgi::ValueRequest::MaxReqs);
    msg.requestValue(fcgi::ValueRequest::MpxsConns);
    {
        auto record = fcgi::Record{msg, 1};
        auto resultRecord = fcgi::Record{};
        convertRecordTest(record, resultRecord);

        readMsg = resultRecord.getMessage<fcgi::MsgGetValues>();
        ASSERT_EQ(msg.requestList(), readMsg.requestList());
    }
}

TEST(RecordSerialization, MsgGetValuesResult)
{
    auto msg = fcgi::MsgGetValuesResult{};
    auto record = fcgi::Record{msg, 1};
    auto resultRecord = fcgi::Record{};
    convertRecordTest(record, resultRecord);

    auto readMsg = resultRecord.getMessage<fcgi::MsgGetValuesResult>();
    ASSERT_EQ(msg.requestList(), readMsg.requestList());

    msg = fcgi::MsgGetValuesResult{};
    msg.setRequestValue(fcgi::ValueRequest::MaxReqs, "10");
    msg.setRequestValue(fcgi::ValueRequest::MpxsConns, "2");
    {
        auto record = fcgi::Record{msg, 1};
        auto resultRecord = fcgi::Record{};
        convertRecordTest(record, resultRecord);

        readMsg = resultRecord.getMessage<fcgi::MsgGetValuesResult>();
        ASSERT_EQ(msg.requestList(), readMsg.requestList());
        for(auto request : msg.requestList())
            ASSERT_EQ(msg.requestValue(request), readMsg.requestValue(request));
    }
}

TEST(RecordSerialization, MsgParams)
{
    auto msg = fcgi::MsgParams{};

    auto record = fcgi::Record{msg, 1};
    auto resultRecord = fcgi::Record{};
    convertRecordTest(record, resultRecord);

    auto readMsg = resultRecord.getMessage<fcgi::MsgParams>();
    ASSERT_EQ(msg.paramList(), readMsg.paramList());

    msg = fcgi::MsgParams{};
    msg.setParam("Hello", "World");
    {
        auto record = fcgi::Record{msg, 1};
        auto resultRecord = fcgi::Record{};
        convertRecordTest(record, resultRecord);

        readMsg = resultRecord.getMessage<fcgi::MsgParams>();
        ASSERT_EQ(msg.paramList(), readMsg.paramList());
        for(const auto& param : msg.paramList())
            ASSERT_EQ(msg.paramValue(param), readMsg.paramValue(param));
    }
}

TEST(RecordSerialization, MsgUnkownType)
{
    auto msg = fcgi::MsgUnknownType{77};
    auto record = fcgi::Record{msg, 1};
    auto resultRecord = fcgi::Record{};
    convertRecordTest(record, resultRecord);

    auto readMsg = resultRecord.getMessage<fcgi::MsgUnknownType>();
    ASSERT_EQ(msg.unknownTypeValue(), readMsg.unknownTypeValue());
}

TEST(RecordSerialization, MsgStdIn)
{
    auto msg = fcgi::MsgStdIn{"Hello world"};
    auto record = fcgi::Record{msg, 1};
    auto resultRecord = fcgi::Record{};
    convertRecordTest(record, resultRecord);

    auto readMsg = resultRecord.getMessage<fcgi::MsgStdIn>();
    ASSERT_EQ(msg.data(), readMsg.data());
}

template<typename ExceptionType>
void assert_exception(std::function<void()> throwingCode, std::function<void(const ExceptionType&)> exceptionContentChecker)
{
    try{
        throwingCode();
        FAIL() << "exception wasn't thrown!";
    }
    catch(const ExceptionType& e){
        exceptionContentChecker(e);
    }
    catch(...){
        FAIL() << "Unexpected exception was thrown";
    }
}

TEST(RecordSerializationError, MsgBeginRequestInvalidRole)
{
    auto output = std::ostringstream{};
    auto encoder = fcgi::Encoder(output);
    encoder << static_cast<uint16_t>(99)
            << static_cast<uint8_t>(1);
    encoder.addPadding(5);

    auto msgData = output.str();
    auto input = std::istringstream{msgData};
    auto msg =  fcgi::MsgBeginRequest{};
    assert_exception<fcgi::InvalidValue>(
        [&](){msg.read(input, 8);},
        [](const fcgi::InvalidValue& e){
            EXPECT_EQ(e.type(), fcgi::InvalidValueType::Role);
            EXPECT_EQ(e.asInt(), 99);
        });
}

TEST(RecordSerializationError, MsgBeginRequestCutoffInput)
{
    auto output = std::ostringstream{};
    auto encoder = fcgi::Encoder(output);
    encoder << static_cast<uint16_t>(fcgi::Role::Responder)
            << static_cast<uint8_t>(1);
    encoder.addPadding(4); //should be 5

    auto msgData = output.str();
    auto input = std::istringstream{msgData};
    auto msg =  fcgi::MsgBeginRequest{};
    assert_exception<std::runtime_error>(
        [&](){msg.read(input, 8);},
        [](const std::runtime_error& e){
            EXPECT_EQ(std::string(e.what()), "basic_ios::clear: iostream error");
        });
}

TEST(RecordSerializationError, MsgEndRequestCutoffInput)
{
    auto output = std::ostringstream{};
    auto encoder = fcgi::Encoder(output);
    encoder << static_cast<uint32_t>(0)
            << static_cast<uint8_t>(0);
    encoder.addPadding(2); //should be 3

    auto msgData = output.str();
    auto input = std::istringstream{msgData};
    auto msg =  fcgi::MsgEndRequest{};
    assert_exception<std::runtime_error>(
        [&](){msg.read(input, 8);},
        [](const std::runtime_error& e){
            EXPECT_EQ(std::string(e.what()), "basic_ios::clear: iostream error");
        });
}

TEST(RecordSerializationError, MsgGetValueInvalidName)
{
    auto output = std::ostringstream{};
    auto nameValue = fcgi::NameValue("wrongName", "0");
    nameValue.toStream(output);

    auto msgData = output.str();
    auto input = std::istringstream{msgData};
    auto msg =  fcgi::MsgGetValues{};
    assert_exception<fcgi::InvalidValue>(
        [&](){msg.read(input, msgData.size());},
        [](const fcgi::InvalidValue& e){
            EXPECT_EQ(e.type(), fcgi::InvalidValueType::ValueRequest);
            EXPECT_EQ(e.asString(), "wrongName");
        });
}

TEST(RecordSerializationError, MsgGetValueMisalignedNameValue)
{
    auto output = std::ostringstream{};
    auto nameValue = fcgi::NameValue("FCGI_MAX_CONNS", "1");
    nameValue.toStream(output);

    auto msgData = output.str();
    auto input = std::istringstream{msgData};
    auto msg =  fcgi::MsgGetValues{};
    auto wrongSize = msgData.size() - 1;
    assert_exception<fcgi::UnrecoverableProtocolError>(
        [&](){msg.read(input, wrongSize);},
        [](const fcgi::UnrecoverableProtocolError& e){
            EXPECT_EQ(std::string{e.what()}, "Misaligned name-value");
        });
}

TEST(RecordSerializationError, MsgGetValueCutoffInput)
{
    auto output = std::ostringstream{};
    auto nameValue = fcgi::NameValue("wrongName", "0");
    nameValue.toStream(output);

    auto msgData = output.str();
    msgData.resize(msgData.size() - 1);
    auto input = std::istringstream{msgData};
    auto msg =  fcgi::MsgGetValues{};
    assert_exception<std::runtime_error>(
        [&](){msg.read(input, msgData.size());},
        [](const std::runtime_error& e){
             EXPECT_EQ(std::string(e.what()), "basic_ios::clear: iostream error");
        });
}

TEST(RecordSerializationError, MsgGetValueResultInvalidName)
{
    auto output = std::ostringstream{};
    auto nameValue = fcgi::NameValue("wrongName", "0");
    nameValue.toStream(output);

    auto msgData = output.str();
    auto input = std::istringstream{msgData};
    auto msg =  fcgi::MsgGetValuesResult{};
    assert_exception<fcgi::InvalidValue>(
        [&](){msg.read(input, msgData.size());},
        [](const fcgi::InvalidValue& e){
            EXPECT_EQ(e.type(), fcgi::InvalidValueType::ValueRequest);
            EXPECT_EQ(e.asString(), "wrongName");
        });
}

TEST(RecordSerializationError, MsgGetValueResultMisalignedNameValue)
{
    auto output = std::ostringstream{};
    auto nameValue = fcgi::NameValue("FCGI_MAX_CONNS", "1");
    nameValue.toStream(output);

    auto msgData = output.str();
    auto input = std::istringstream{msgData};
    auto msg =  fcgi::MsgGetValuesResult{};
    auto wrongSize = msgData.size() - 1;
    assert_exception<fcgi::UnrecoverableProtocolError>(
        [&](){msg.read(input, wrongSize);},
        [](const fcgi::UnrecoverableProtocolError& e){
            EXPECT_EQ(std::string{e.what()}, "Misaligned name-value");
        });
}

TEST(RecordSerializationError, MsgGetValueResultCutoffInput)
{
    auto output = std::ostringstream{};
    auto nameValue = fcgi::NameValue("wrongName", "0");
    nameValue.toStream(output);

    auto msgData = output.str();
    msgData.resize(msgData.size() - 1);
    auto input = std::istringstream{msgData};
    auto msg =  fcgi::MsgGetValuesResult{};
    assert_exception<std::runtime_error>(
        [&](){msg.read(input, msgData.size());},
        [](const std::runtime_error& e){
             EXPECT_EQ(std::string(e.what()), "basic_ios::clear: iostream error");
        });
}

TEST(RecordSerializationError, MsgParamsMisalignedNameValue)
{
    auto output = std::ostringstream{};
    auto nameValue = fcgi::NameValue("foo", "bar");
    nameValue.toStream(output);

    auto msgData = output.str();
    auto input = std::istringstream{msgData};
    auto msg =  fcgi::MsgParams{};
    auto wrongSize = msgData.size() - 1;
    assert_exception<fcgi::UnrecoverableProtocolError>(
        [&](){msg.read(input, wrongSize);},
        [](const fcgi::UnrecoverableProtocolError& e){
            EXPECT_EQ(std::string{e.what()}, "Misaligned name-value");
        });
}

TEST(RecordSerializationError, MsgParamsCutoffInput)
{
    auto output = std::ostringstream{};
    auto nameValue = fcgi::NameValue("foo", "bar");
    nameValue.toStream(output);

    auto msgData = output.str();
    msgData.resize(msgData.size() - 1);
    auto input = std::istringstream{msgData};
    auto msg =  fcgi::MsgParams{};
    assert_exception<std::runtime_error>(
        [&](){msg.read(input, msgData.size());},
        [](const std::runtime_error& e){
             EXPECT_EQ(std::string(e.what()), "basic_ios::clear: iostream error");
        });
}

TEST(RecordSerializationError, MsgUnkownTypeCutoffInput)
{
    auto output = std::ostringstream{};
    auto encoder = fcgi::Encoder(output);
    encoder << static_cast<uint8_t>(0);
    encoder.addPadding(6); //should be 7

    auto msgData = output.str();
    auto input = std::istringstream{msgData};
    auto msg =  fcgi::MsgUnknownType{};
    assert_exception<std::runtime_error>(
        [&](){msg.read(input, 8);},
        [](const std::runtime_error& e){
            EXPECT_EQ(std::string(e.what()), "basic_ios::clear: iostream error");
        });
}


TEST(RecordSerializationError, MsgStreamDataCutoffInput)
{
    auto output = std::ostringstream{};
    auto encoder = fcgi::Encoder(output);
    encoder << "Test";

    auto msgData = output.str();
    auto input = std::istringstream{msgData};
    auto msg =  fcgi::MsgStdIn{};
    auto wrongSize = msgData.size() + 1;
    assert_exception<std::runtime_error>(
        [&](){msg.read(input, wrongSize);},
        [](const std::runtime_error& e){
            EXPECT_EQ(std::string(e.what()), "basic_ios::clear: iostream error");
        });
}

TEST(RecordSerializationError, RecordUsupportedProtocolVersion)
{
    auto output = std::ostringstream{};
    auto encoder = fcgi::Encoder(output);
    encoder  << static_cast<uint8_t>(10) // wrong protocol version
             << static_cast<uint8_t>(fcgi::RecordType::AbortRequest)
             << static_cast<uint16_t>(1)
             << static_cast<uint16_t>(0)
             << static_cast<uint8_t>(0);
    encoder.addPadding(1);

    auto recordData = output.str();
    auto input = std::istringstream{recordData};
    auto record =  fcgi::Record{};
    assert_exception<fcgi::UnsupportedVersion>(
        [&](){record.fromStream(input, recordData.size());},
        [](const fcgi::UnsupportedVersion& e){
            EXPECT_EQ(e.protocolVersion(), 10);
            EXPECT_EQ(std::string{e.what()}, "Protocol version \"10\" isn't supported.");
        });
}

TEST(RecordSerializationError, RecordInvalidType)
{
    auto output = std::ostringstream{};
    auto encoder = fcgi::Encoder(output);
    encoder  << static_cast<uint8_t>(1)
             << static_cast<uint8_t>(99) // wrong record type
             << static_cast<uint16_t>(1)
             << static_cast<uint16_t>(0)
             << static_cast<uint8_t>(0);
    encoder.addPadding(1);

    auto recordData = output.str();
    auto input = std::istringstream{recordData};
    auto record =  fcgi::Record{};
    assert_exception<fcgi::InvalidRecordType>(
        [&](){record.fromStream(input, recordData.size());},
        [](const fcgi::InvalidRecordType& e){
            EXPECT_EQ(e.recordType(), 99);
            EXPECT_EQ(e.recordSize(), 8);
            EXPECT_EQ(std::string{e.what()}, "Record type \"99\" is invalid.");
        });
}

TEST(RecordSerializationError, RecordInvalidMessageReadError)
{
    auto output = std::ostringstream{};
    auto encoder = fcgi::Encoder(output);
    encoder  << static_cast<uint8_t>(1)
             << static_cast<uint8_t>(fcgi::RecordType::BeginRequest)
             << static_cast<uint16_t>(1)
             << static_cast<uint16_t>(8) //message size
             << static_cast<uint8_t>(0);
    encoder.addPadding(1);
    //MsgBeginRequest
    encoder << static_cast<uint16_t>(99) // wrong role
            << static_cast<uint8_t>(1);
    encoder.addPadding(5);

    auto recordData = output.str();
    auto input = std::istringstream{recordData};
    auto record =  fcgi::Record{};
    assert_exception<fcgi::RecordReadError>(
        [&](){record.fromStream(input, recordData.size());},
        [](const fcgi::RecordReadError& e){
            EXPECT_EQ(e.recordSize(), 16);
            EXPECT_EQ(std::string{e.what()}, "Role value \"99\" is invalid.");
        });
}
