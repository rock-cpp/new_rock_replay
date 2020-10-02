#include <boost/test/unit_test.hpp>
#include "LogTaskManager.hpp"
#include "LogFileHelper.hpp"

const auto fileNames = LogFileHelper::parseFileNames({"../logs/trajectory_follower_Logger.0.log"});

BOOST_AUTO_TEST_CASE(TestSpawnTrajectoryFollower)
{
    LogTaskManager manager;
    manager.init(fileNames, "");
}
