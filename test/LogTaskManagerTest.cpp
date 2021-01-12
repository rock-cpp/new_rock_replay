#include "LogTaskManager.hpp"

#include "LogFileHelper.hpp"

#include <boost/test/unit_test.hpp>
#include <set>

const auto fileNames = LogFileHelper::parseFileNames({"../logs/trajectory_follower_Logger.0.log"});
LogTaskManager manager;

BOOST_AUTO_TEST_CASE(TestSpawnTrajectoryFollower)
{
    const std::set<std::pair<std::string, std::string>> portsWithTypes = {
        {"follower_data", "/trajectory_follower/FollowerData"}, {"motion_command", "/base/commands/Motion2D"}, {"state", "int"}};

    manager.init(fileNames, "");

    auto tasks = manager.getTaskCollection();
    auto trajectoryFollowerTask = tasks.at("trajectory_follower");
    auto orderedPortsWithType = std::set<std::pair<std::string, std::string>>(trajectoryFollowerTask.begin(), trajectoryFollowerTask.end());

    BOOST_TEST(manager.getNumSamples() == 849);
    BOOST_TEST(tasks.size() == 1);
    BOOST_TEST(orderedPortsWithType == portsWithTypes);
}

BOOST_AUTO_TEST_CASE(TestTaskMetaData)
{
    auto metadata = manager.setIndex(250);

    BOOST_TEST(metadata.valid);
    BOOST_TEST(metadata.portName == "trajectory_follower.motion_command");
    BOOST_TEST(!metadata.timeStamp.isNull());
}

BOOST_AUTO_TEST_CASE(TestInvalidTaskMetaData)
{
    auto metadata = manager.setIndex(1000);

    BOOST_TEST(!metadata.valid);
    BOOST_TEST(metadata.portName == "");
    BOOST_TEST(metadata.timeStamp.isNull());
}

BOOST_AUTO_TEST_CASE(TestTaskActivateReplayForPort)
{
    manager.setIndex(250);
    bool replayedSampleActivated = manager.replaySample();

    manager.activateReplayForPort("trajectory_follower", "motion_command", false);
    manager.setIndex(250);
    bool replayedSampleDeactivated = manager.replaySample();

    BOOST_TEST(replayedSampleActivated);
    BOOST_TEST(!replayedSampleDeactivated);
}

BOOST_AUTO_TEST_CASE(TestReplayWithException)
{
    manager.init(LogFileHelper::parseFileNames({"../logs/slam3d_Logger.0.log"}), "");
    bool canReplay = manager.replaySample();
    
    BOOST_TEST(!canReplay);
}
// BOOST_AUTO_TEST_CASE(TestReplay)
// {
//     orocos_cpp::OrocosCpp orocos;
//     auto task = orocos.getTaskContext("trajectory_follower");
// }
