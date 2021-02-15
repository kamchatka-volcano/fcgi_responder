#include <responder.h>
#include <record.h>
#include <msgbeginrequest.h>
#include <msgendrequest.h>
#include <msggetvalues.h>
#include <msggetvaluesresult.h>
#include <msgunknowntype.h>
#include <msgparams.h>
#include <msgabortrequest.h>
#include <streamdatamessage.h>
#include <encoder.h>
#include <constants.h>
#include <requesteditor.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace fcgi;

class MockResponder : public Responder{
public:
    MOCK_METHOD1(sendData, void(const std::string& data));
    MOCK_METHOD0(disconnect, void());
    MOCK_METHOD1(processRequest, Response(const Request& request));

    void receive(const std::string& data)
    {
        Responder::receiveData(data.c_str(), data.size());
    }
};

class MockResponderWithTestProcessor : public Responder{
public:
    MOCK_METHOD1(sendData, void(const std::string& data));
    MOCK_METHOD0(disconnect, void());
    Response processRequest(const Request& request)
    {
        auto testMsg = request.stdIn();
        std::reverse(testMsg.begin(), testMsg.end());
        auto response = Response{};
        response.setData(testMsg);
        return response;
    }
    void receive(const std::string& data)
    {
        Responder::receiveData(data.c_str(), data.size());
    }
};

namespace{
    template<typename... Args>
    std::string messageData(Args&&... args)
    {
        auto record = fcgi::Record{std::forward<Args>(args)...};
        auto recordStream = std::ostringstream{};
        record.toStream(recordStream);
        return recordStream.str();
    }
}

template <typename ResponderType>
class BaseTestResponder : public ::testing::TestWithParam<bool>{
public:
    BaseTestResponder()
    {
        responder_.setErrorInfoHandler([&errorInfo = errorInfo_](const std::string& errorMsg){
            errorInfo += errorMsg + "\n";
        });
    }
protected:
    void receiveData(const std::string& data)
    {
        responder_.receive(data);
    }
    void receiveMessage(std::unique_ptr<fcgi::Message> msg, uint16_t requestId = 0)
    {
        responder_.receive(messageData(std::move(msg), requestId));
    }
    void receiveMessage(RecordType recordType, uint16_t requestId = 0)
    {
        responder_.receive(messageData(recordType, requestId));
    }
    void receiveMessage(const std::string& recordData)
    {
        responder_.receive(recordData);
    }
    void expectMessageToBeSent(std::unique_ptr<fcgi::Message> msg, uint16_t requestId = 0)
    {
        EXPECT_CALL(responder_, sendData(messageData(std::move(msg), requestId)));
    }
    void expectNoMessagesToBeSent()
    {
        EXPECT_CALL(responder_, sendData(::testing::_)).Times(0);
    }
    void checkConnectionState()
    {
        auto disconnectOnEnd = GetParam();
        if (disconnectOnEnd)
            EXPECT_CALL(responder_, disconnect());
        else
            EXPECT_CALL(responder_, disconnect()).Times(0);
    }
    ResultConnectionState resultConnectionState()
    {
        auto disconnectOnEnd = GetParam();
        return disconnectOnEnd ? ResultConnectionState::Close
                               : ResultConnectionState::KeepOpen;
    }

    ResponderType responder_;
    std::string errorInfo_;
};

using TestResponder = BaseTestResponder<MockResponder>;
using TestResponderWithTestProcessor = BaseTestResponder<MockResponderWithTestProcessor>;

TEST_F(TestResponder, UnknownType)
{
    expectMessageToBeSent(std::make_unique<MsgUnknownType>(99), 0);

    auto output = std::ostringstream{};
    auto encoder = Encoder(output);
    encoder  << cProtocolVersion
             << static_cast<uint8_t>(99)
             << static_cast<uint16_t>(1)
             << static_cast<uint16_t>(0)
             << static_cast<uint8_t>(0);
    encoder.addPadding(1);
    receiveMessage(output.str());
    EXPECT_EQ(errorInfo_, "Record type \"99\" is invalid.\n");
}

TEST_F(TestResponder, GetValuesAllDefaultSettings)
{
    auto resultMsg = std::make_unique<MsgGetValuesResult>();
    resultMsg->setRequestValue(ValueRequest::MaxReqs, std::to_string(responder_.maximumRequestsNumber()));
    resultMsg->setRequestValue(ValueRequest::MaxConns, std::to_string(responder_.maximumConnectionsNumber()));
    resultMsg->setRequestValue(ValueRequest::MpxsConns, std::to_string(responder_.isMultiplexingEnabled() ? 1 : 0));
    expectMessageToBeSent(std::move(resultMsg));

    auto requestMsg = std::make_unique<MsgGetValues>();
    requestMsg->requestValue(ValueRequest::MaxReqs);
    requestMsg->requestValue(ValueRequest::MaxConns);
    requestMsg->requestValue(ValueRequest::MpxsConns);
    receiveMessage(std::move(requestMsg));
}

TEST_F(TestResponder, GetValueChangedSettings)
{
    responder_.setMaximumRequestsNumber(99);
    auto resultMsg = std::make_unique<MsgGetValuesResult>();
    resultMsg->setRequestValue(ValueRequest::MaxReqs, std::to_string(responder_.maximumRequestsNumber()));
    resultMsg->setRequestValue(ValueRequest::MpxsConns, std::to_string(responder_.isMultiplexingEnabled() ? 1 : 0));
    expectMessageToBeSent(std::move(resultMsg));

    auto requestMsg = std::make_unique<MsgGetValues>();
    requestMsg->requestValue(ValueRequest::MaxReqs);
    requestMsg->requestValue(ValueRequest::MpxsConns);
    receiveMessage(std::move(requestMsg));
}

TEST_P(TestResponder, AbortRequest)
{
    expectMessageToBeSent(std::make_unique<MsgEndRequest>(0, ProtocolStatus::RequestComplete), 1);
    checkConnectionState();

    receiveMessage(std::make_unique<MsgBeginRequest>(Role::Responder, resultConnectionState()), 1);
    receiveMessage(RecordType::AbortRequest, 1);
}

TEST_P(TestResponder, UnknownRole)
{
    expectMessageToBeSent(std::make_unique<MsgEndRequest>(0, ProtocolStatus::UnknownRole));
    checkConnectionState();

    receiveMessage(std::make_unique<MsgBeginRequest>(Role::Authorizer, resultConnectionState()));
}

TEST_P(TestResponder, CantMultiplex)
{
    responder_.setMultiplexingEnabled(false);
    expectMessageToBeSent(std::make_unique<MsgEndRequest>(0, ProtocolStatus::CantMpxConn), 2);
    checkConnectionState();

    receiveMessage(std::make_unique<MsgBeginRequest>(Role::Responder, resultConnectionState()), 1);
    receiveMessage(std::make_unique<MsgBeginRequest>(Role::Responder, resultConnectionState()), 2);
}

TEST_P(TestResponder, Overloaded)
{
    responder_.setMaximumRequestsNumber(2);
    expectMessageToBeSent(std::make_unique<MsgEndRequest>(0, ProtocolStatus::Overloaded), 3);
    checkConnectionState();

    receiveMessage(std::make_unique<MsgBeginRequest>(Role::Responder, resultConnectionState()), 1);
    receiveMessage(std::make_unique<MsgBeginRequest>(Role::Responder, resultConnectionState()), 2);
    receiveMessage(std::make_unique<MsgBeginRequest>(Role::Responder, resultConnectionState()), 3);
}

namespace fcgi{
bool operator==(const Request& lhs, const Request& rhs)
{
    auto equal =
           lhs.paramList() == rhs.paramList() &&
           lhs.stdIn() == rhs.stdIn();

    if (!equal)
        return false;
    for (const auto& paramName : lhs.paramList())
        if (lhs.param(paramName) != rhs.param(paramName))
            return false;
    return true;
}
}

TEST_P(TestResponder, Request)
{
    auto params = std::make_unique<MsgParams>();
    params->setParam("test", "hello world");
    params->setParam("foo", "bar");

    auto inStream = std::make_unique<MsgStdIn>();
    inStream->setData("HELLO WORLD");

    auto expectedRequest = Request{};
    RequestEditor(expectedRequest).addParamsMsg(*params);
    RequestEditor(expectedRequest).addStdInMsg(*inStream);

    ::testing::InSequence seq;
    EXPECT_CALL(responder_, processRequest(expectedRequest));
    expectMessageToBeSent(std::make_unique<MsgStdOut>(), 1);
    expectMessageToBeSent(std::make_unique<MsgStdErr>(), 1);
    expectMessageToBeSent(std::make_unique<MsgEndRequest>(0, ProtocolStatus::RequestComplete), 1);
    checkConnectionState();

    receiveMessage(std::make_unique<MsgBeginRequest>(Role::Responder, resultConnectionState()), 1);
    receiveMessage(std::move(params), 1);
    receiveMessage(std::make_unique<MsgParams>(), 1);
    receiveMessage(std::move(inStream), 1);
    receiveMessage(std::make_unique<MsgStdIn>(), 1);
}

#include "streammaker.h"
#include <iostream>

TEST_P(TestResponder, ReceivingMessagesInLargeChunks)
{
    auto streamRecordData = std::string(cMaxDataMessageSize, '0');
    auto streamData = std::string{};
    auto expectedRequest = Request{};
    auto requestId = static_cast<uint16_t>(1);
    for (auto i = 0; i < 3; ++i){
        auto inStream = std::make_unique<MsgStdIn>();
        inStream->setData(streamRecordData);
        RequestEditor{expectedRequest}.addStdInMsg(*inStream);
        streamData += messageData(std::move(inStream), requestId);
        std::cout << streamData.size() << std::endl;
    }
    streamData += messageData(std::make_unique<MsgStdIn>(), requestId);
    std::cout << streamData.size() << std::endl;


    ::testing::InSequence seq;
    EXPECT_CALL(responder_, processRequest(expectedRequest));
    expectMessageToBeSent(std::make_unique<MsgStdOut>(), requestId);
    expectMessageToBeSent(std::make_unique<MsgStdErr>(), requestId);
    expectMessageToBeSent(std::make_unique<MsgEndRequest>(0, ProtocolStatus::RequestComplete), requestId);
    checkConnectionState();

    receiveMessage(std::make_unique<MsgBeginRequest>(Role::Responder, resultConnectionState()), requestId);
    auto halfSize = streamData.size() / 2;
    receiveData(streamData.substr(0, halfSize));
    receiveData(streamData.substr(halfSize, streamData.size() - halfSize));
}

TEST_F(TestResponder, Multiplexing)
{
    auto params = std::make_unique<MsgParams>();
    params->setParam("test", "hello world");
    params->setParam("foo", "bar");

    auto inStream = std::make_unique<MsgStdIn>();
    inStream->setData("HELLO WORLD");

    auto params2 = std::make_unique<MsgParams>();
    params2->setParam("msg", "param");

    auto expectedRequest = Request{};
    RequestEditor(expectedRequest).addParamsMsg(*params);
    RequestEditor(expectedRequest).addStdInMsg(*inStream);

    auto expectedRequest2 = Request{};
    RequestEditor(expectedRequest2).addParamsMsg(*params2);

    ::testing::InSequence seq;
    EXPECT_CALL(responder_, processRequest(expectedRequest2));
    expectMessageToBeSent(std::make_unique<MsgStdOut>(), 2);
    expectMessageToBeSent(std::make_unique<MsgStdErr>(), 2);
    expectMessageToBeSent(std::make_unique<MsgEndRequest>(0, ProtocolStatus::RequestComplete), 2);
    EXPECT_CALL(responder_, processRequest(expectedRequest));
    expectMessageToBeSent(std::make_unique<MsgStdOut>(), 1);
    expectMessageToBeSent(std::make_unique<MsgStdErr>(), 1);
    expectMessageToBeSent(std::make_unique<MsgEndRequest>(0, ProtocolStatus::RequestComplete), 1);
    EXPECT_CALL(responder_, disconnect()).Times(0);

    receiveMessage(std::make_unique<MsgBeginRequest>(Role::Responder, ResultConnectionState::KeepOpen), 1);
    receiveMessage(std::move(params), 1);
    receiveMessage(std::make_unique<MsgBeginRequest>(Role::Responder, ResultConnectionState::KeepOpen), 2);
    receiveMessage(std::move(params2), 2);
    receiveMessage(std::make_unique<MsgParams>(), 1);
    receiveMessage(std::move(inStream), 1);
    receiveMessage(std::make_unique<MsgParams>(), 2);
    receiveMessage(std::make_unique<MsgStdIn>(), 2);
    receiveMessage(std::make_unique<MsgStdIn>(), 1);
}

TEST_P(TestResponderWithTestProcessor, Request)
{
    auto params = std::make_unique<MsgParams>();
    params->setParam("test", "hello world");
    params->setParam("foo", "bar");

    auto inStream = std::make_unique<MsgStdIn>();
    inStream->setData("HELLO WORLD");

    ::testing::InSequence seq;
    expectMessageToBeSent(std::make_unique<MsgStdOut>("DLROW OLLEH"), 1);
    expectMessageToBeSent(std::make_unique<MsgStdOut>(), 1);
    expectMessageToBeSent(std::make_unique<MsgStdErr>(), 1);
    expectMessageToBeSent(std::make_unique<MsgEndRequest>(0, ProtocolStatus::RequestComplete), 1);
    checkConnectionState();

    receiveMessage(std::make_unique<MsgBeginRequest>(Role::Responder, resultConnectionState()), 1);
    receiveMessage(std::move(params), 1);
    receiveMessage(std::make_unique<MsgParams>(), 1);
    receiveMessage(std::move(inStream), 1);
    receiveMessage(std::make_unique<MsgStdIn>(), 1);
}

TEST_F(TestResponder, UnexpectedRecord)
{
    expectNoMessagesToBeSent();
    receiveMessage(std::make_unique<MsgAbortRequest>(), 1);
    EXPECT_EQ(errorInfo_, "Received unexpected record, RecordType = 2, requestId = 1\n");
}

TEST_P(TestResponder, RecordReadError)
{
    expectMessageToBeSent(std::make_unique<MsgEndRequest>(0, ProtocolStatus::RequestComplete), 1);
    checkConnectionState();

    auto output = std::ostringstream{};
    auto encoder = Encoder(output);
    auto nameValue = fcgi::NameValue("wrongName", "0");
    encoder  << cProtocolVersion
             << static_cast<uint8_t>(RecordType::GetValues)
             << static_cast<uint16_t>(1)
             << static_cast<uint16_t>(nameValue.size())
             << static_cast<uint8_t>(0);
    encoder.addPadding(1);
    nameValue.toStream(output);

    auto receivedData = output.str();
    receivedData += messageData(std::make_unique<MsgBeginRequest>(Role::Responder, resultConnectionState()), static_cast<uint16_t>(1));
    receivedData += messageData(std::make_unique<MsgAbortRequest>(), static_cast<uint16_t>(1));
    receiveMessage(receivedData);

    EXPECT_EQ(errorInfo_, "Value request value \"wrongName\" is invalid.\n");
}

TEST_P(TestResponder, RecordReadErrorMisalignedNameValue)
{
    expectNoMessagesToBeSent();
    auto output = std::ostringstream{};
    auto encoder = Encoder(output);
    auto nameValue = fcgi::NameValue("FCGI_MAX_CONNS", "");
    encoder  << cProtocolVersion
             << static_cast<uint8_t>(RecordType::GetValues)
             << static_cast<uint16_t>(1)
             << static_cast<uint16_t>(nameValue.size() - 1) //wrong size
             << static_cast<uint8_t>(0);
    encoder.addPadding(1);
    nameValue.toStream(output);

    auto receivedData = output.str();
    receivedData += messageData(std::make_unique<MsgBeginRequest>(Role::Responder, resultConnectionState()), static_cast<uint16_t>(1));
    receivedData += messageData(std::make_unique<MsgAbortRequest>(), static_cast<uint16_t>(1));
    receiveMessage(receivedData);
    EXPECT_EQ(errorInfo_, "Misaligned name-value\n");
}

INSTANTIATE_TEST_CASE_P(WithConnectionStateCheck, TestResponder, ::testing::Values(false, true));
INSTANTIATE_TEST_CASE_P(WithConnectionStateCheck, TestResponderWithTestProcessor, ::testing::Values(false, true));

