#include <gtest/gtest.h>
#include <multipartstream.h>

using namespace sfun::stream_utils;

TEST(MultipartStream, Empty)
{
    auto stream = MultiPartStream{{}};
    auto res =  std::string{};
    stream >> res;
    EXPECT_EQ(res, "");
}

TEST(MultipartStream, OnePart)
{
    auto part = std::string{"hello"};
    auto stream = MultiPartStream{{part}};
    auto res =  std::string{};
    stream >> res;
    EXPECT_EQ(res, "hello");
}

TEST(MultipartStream, TwoParts)
{
    auto part = std::string{"hello"};
    auto part2 = std::string{"world"};
    auto stream = MultiPartStream{{part, part2}};
    auto res =  std::string{};
    stream >> res;
    EXPECT_EQ(res, "helloworld");
}

TEST(MultipartStream, ThreeParts)
{
    auto part = std::string{"hello"};
    auto part2 = std::string{"world"};
    auto part3 = std::string{"!!!"};
    auto stream = MultiPartStream{{part, part2, part3}};
    auto res =  std::string{};
    stream >> res;
    EXPECT_EQ(res, "helloworld!!!");
}