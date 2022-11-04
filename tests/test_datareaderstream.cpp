#include <gtest/gtest.h>
#include <datareaderstream.h>

using namespace fcgi;

TEST(DataReaderStream, Empty)
{
    auto stream = DataReaderStream{};
    auto res =  std::string{};
    stream >> res;
    EXPECT_EQ(res, "");
}

TEST(DataReaderStream, OnePart)
{
    auto part = std::string{"hello"};
    auto stream = DataReaderStream{part};
    auto res =  std::string{};
    stream >> res;
    EXPECT_EQ(res, "hello");
}

TEST(DataReaderStream, TwoParts)
{
    auto part = std::string{"hello"};
    auto part2 = std::string{"world"};
    auto stream = DataReaderStream{part, part2};
    auto res =  std::string{};
    stream >> res;
    EXPECT_EQ(res, "helloworld");
}

TEST(DataReaderStream, TwoPartsWithFirstEmpty)
{
    auto part = std::string{""};
    auto part2 = std::string{"world"};
    auto stream = DataReaderStream{part, part2};
    auto res =  std::string{};
    stream >> res;
    EXPECT_EQ(res, "world");
}


TEST(DataReaderStream, ThreeParts)
{
    auto part = std::string{"hello"};
    auto part2 = std::string{"world"};
    auto part3 = std::string{"!!!"};
    auto stream = DataReaderStream{part, part2, part3};
    auto res =  std::string{};
    stream >> res;
    EXPECT_EQ(res, "helloworld!!!");
}

TEST(DataReaderStream, ThreePartsWithFirstEmpty)
{
    auto part = std::string{""};
    auto part2 = std::string{"world"};
    auto part3 = std::string{"!!!"};
    auto stream = DataReaderStream{part, part2, part3};
    auto res =  std::string{};
    stream >> res;
    EXPECT_EQ(res, "world!!!");
}

TEST(DataReaderStream, ThreePartsWithFirstTwoEmpty)
{
    auto part = std::string{""};
    auto part2 = std::string{""};
    auto part3 = std::string{"!!!"};
    auto stream = DataReaderStream{part, part2, part3};
    auto res =  std::string{};
    stream >> res;
    EXPECT_EQ(res, "!!!");
}