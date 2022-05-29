#include <gtest/gtest.h>
#include <streamdatamessage.h>
#include <msgparams.h>
#include <requestregistry.h>
#include <request.h>


TEST(Request, Data)
{
     auto request = fcgi::Request{{}, "Hello world"};
     ASSERT_EQ("Hello world", request.stdIn());
}

TEST(Request, Params)
{
    auto params = std::vector<std::pair<std::string, std::string>>{
            {"foo", "123"},
            {"bar", "Hello world"}
    };
    auto request = fcgi::Request{params, {}};

    ASSERT_EQ("Hello world", request.param("bar"));
    ASSERT_EQ(true, request.hasParam("foo"));
    ASSERT_EQ(false, request.hasParam("baz"));
    ASSERT_EQ("", request.param("baz"));
}
