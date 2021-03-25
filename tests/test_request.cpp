#include <gtest/gtest.h>
#include <streamdatamessage.h>
#include <msgparams.h>
#include <request.h>

namespace fcgi{
class RequestMaker{
public:
    RequestMaker(Request& request)
        : request_(request)
    {}
    void addParams(const std::vector<std::pair<std::string, std::string>>& params)
    {
        request_.addParams(params);
    }
    void addParams(const fcgi::MsgParams& msg)
    {
        request_.addParams(msg);
    }
    void addData(const fcgi::MsgStdIn& msg)
    {
        request_.addData(msg);
    }

private:
    Request& request_;
};
}


TEST(Request, Data)
{
     auto msg = fcgi::MsgStdIn{"Hello world"};
     auto request = fcgi::Request{};
     auto requestMaker = fcgi::RequestMaker{request};
     requestMaker.addData(msg);
     ASSERT_EQ("Hello world", request.stdIn());
}

TEST(Request, ParamsMsg)
{
    auto msg = fcgi::MsgParams{};
    msg.setParam("foo", "123");
    msg.setParam("bar", "Hello world");
    auto request = fcgi::Request{};
    auto requestMaker = fcgi::RequestMaker{request};
    requestMaker.addParams(msg);

    ASSERT_EQ("Hello world", request.param("bar"));
    ASSERT_EQ(true, request.hasParam("foo"));
    ASSERT_EQ(false, request.hasParam("baz"));
    ASSERT_EQ("", request.param("baz"));
    ASSERT_EQ((std::vector<std::string>{"bar", "foo"}), request.paramList());
}

TEST(Request, ParamsVector)
{
    auto params = std::vector<std::pair<std::string, std::string>>{
        {"foo", "123"},
        {"bar", "Hello world"}
    };
    auto request = fcgi::Request{};
    auto requestMaker = fcgi::RequestMaker{request};
    requestMaker.addParams(params);

    ASSERT_EQ("Hello world", request.param("bar"));
    ASSERT_EQ(true, request.hasParam("foo"));
    ASSERT_EQ(false, request.hasParam("baz"));
    ASSERT_EQ("", request.param("baz"));
    ASSERT_EQ((std::vector<std::string>{"bar", "foo"}), request.paramList());
}
