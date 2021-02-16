#include "LogFileHelper.hpp"

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(TestWhiteListStreamWithoutRegEx)
{
    const std::vector<std::string> whitelist = {"trajectory_follower.state"};
    const std::string streamName = "trajectory_follower.state";

    BOOST_TEST(LogFileHelper::isWhiteListed(streamName, whitelist));
}

BOOST_AUTO_TEST_CASE(TestWhiteListStreamWithRegEx)
{
    const std::vector<std::string> whitelist = {".*state"};
    const std::string streamName = "trajectory_follower.state";

    BOOST_TEST(LogFileHelper::isWhiteListed(streamName, whitelist));
}

BOOST_AUTO_TEST_CASE(TestMultiMatchAndMismatch)
{
    const std::vector<std::string> whitelist = {".*state", ".*moti.*"};
    const std::vector<std::string> passingStreams = {"trajectory_follower.state", "trajectory_follower.motion_command"};
    const std::vector<std::string> rejectedStreams = {"trajectory_follower.follower_data"};

    for(const auto& entry : passingStreams)
    {
        BOOST_TEST(LogFileHelper::isWhiteListed(entry, whitelist));
    }

    for(const auto& entry : rejectedStreams)
    {
        BOOST_TEST(!LogFileHelper::isWhiteListed(entry, whitelist));
    }
}