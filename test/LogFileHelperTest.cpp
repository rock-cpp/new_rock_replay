#include <boost/test/unit_test.hpp>

#include "LogFileHelper.hpp"
#include <iostream>

void testParser(const std::vector<std::string>& args, uint64_t size)
{
    auto fileNames = LogFileHelper::parseFileNames(args);
    
    BOOST_TEST(fileNames.size() == size); 
}

void testStreamSplit(const std::string& input, const std::string& firstPart, const std::string& secondPart)
{
    const auto namePair = LogFileHelper::splitStreamName(input);
    
    BOOST_TEST(namePair.first == firstPart);
    BOOST_TEST(namePair.second == secondPart);
}

BOOST_AUTO_TEST_CASE(TestLogfileParserFilenames)
{
    std::vector<std::string> cmdLineArgs = {"../logs/trajectory_follower_Logger.0.log", "non_existing"};
    testParser(cmdLineArgs, 1);
}

BOOST_AUTO_TEST_CASE(TestLogfileParserFolder)
{
    std::vector<std::string> cmdLineArgs = {"../logs"};
    testParser(cmdLineArgs, 4);
}

BOOST_AUTO_TEST_CASE(TestLogfileParserEmpty)
{
    std::vector<std::string> cmdLineArgs = {""};
    testParser(cmdLineArgs, 0);
}

BOOST_AUTO_TEST_CASE(TestStreamNameSplitSingleDot)
{
    const std::string first = "trajectory_follower";
    const std::string second = "my_port";
    
    testStreamSplit(first + "." + second, first, second);
}

BOOST_AUTO_TEST_CASE(TestStreamNameSplitMultiDot)
{
    const std::string first = "trajectory_follower.my_port";
    const std::string second = "my_member";
    
    testStreamSplit(first + "." + second, first, second);
}

BOOST_AUTO_TEST_CASE(TestStreamNameSplitSlash)
{
    const std::string first = "trajectory_follower.my_port/nested";
    const std::string second = "my_member";
    
    testStreamSplit(first + "." + second, first, second);
}

BOOST_AUTO_TEST_CASE(TestStreamNameEmpty)
{
    testStreamSplit("", "", "");
}

