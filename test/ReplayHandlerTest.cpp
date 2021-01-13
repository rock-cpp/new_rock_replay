#include "ReplayHandler.hpp"

#include "LogFileHelper.hpp"

#include <boost/test/unit_test.hpp>

ReplayHandler replayHandler;
const auto fileNames = LogFileHelper::parseFileNames({"../logs/trajectory_follower_Logger.0.log"});

BOOST_AUTO_TEST_CASE(TestInit)
{
    replayHandler.init(fileNames, "");

    BOOST_TEST(replayHandler.getMaxIndex() == 848);
    BOOST_TEST(replayHandler.getCurIndex() == 0);
    BOOST_TEST(replayHandler.getCurrentSpeed() == 0);
    BOOST_TEST(replayHandler.getReplayFactor() == 1.);
    BOOST_TEST(replayHandler.getCurSamplePortName() == "trajectory_follower.follower_data");
    BOOST_TEST(!replayHandler.isPlaying());
    BOOST_TEST(!replayHandler.hasFinished());
    BOOST_TEST(replayHandler.getCurTimeStamp() == "20161118-11:08:20:786284+0000");
    BOOST_TEST(replayHandler.canSampleBeReplayed());
}

BOOST_AUTO_TEST_CASE(TestSetSpeed)
{
    replayHandler.setReplaySpeed(0);
    BOOST_TEST(replayHandler.getReplayFactor() == 0.01f);

    replayHandler.setReplaySpeed(0.5);
    BOOST_TEST(replayHandler.getReplayFactor() == 0.5);
}

BOOST_AUTO_TEST_CASE(TestMinSpan)
{
    replayHandler.setMinSpan(10);
    replayHandler.setSampleIndex(9);

    BOOST_TEST(replayHandler.getMinSpan() == 10);
    BOOST_TEST(replayHandler.getCurIndex(), 9);
}

BOOST_AUTO_TEST_CASE(TestMaxSpan)
{
    replayHandler.setMaxSpan(100);
    replayHandler.setSampleIndex(101);

    BOOST_TEST(replayHandler.getMaxSpan() == 100);
    BOOST_TEST(replayHandler.getCurIndex(), 100);
}

BOOST_AUTO_TEST_CASE(TestNext)
{
    replayHandler.setMaxSpan(848);
    replayHandler.setSampleIndex(50);

    replayHandler.next();
    BOOST_TEST(replayHandler.getCurIndex() == 51);

    for(size_t i = 0; i < 800; i++)
    {
        replayHandler.next();
        BOOST_TEST(replayHandler.getCurIndex() <= 848);
    }
}

BOOST_AUTO_TEST_CASE(TestPrevious)
{
    replayHandler.setMinSpan(0);
    replayHandler.setSampleIndex(50);

    replayHandler.previous();
    BOOST_TEST(replayHandler.getCurIndex() == 49);

    for(size_t i = 0; i < 50; i++)
    {
        replayHandler.previous();
        BOOST_TEST(replayHandler.getCurIndex() >= 0);
    }
}

BOOST_AUTO_TEST_CASE(TestTaskCollection)
{
    auto tasksWithPortNames = replayHandler.getTaskNamesWithPorts();
    BOOST_TEST(tasksWithPortNames.size() == 1);
    BOOST_TEST(tasksWithPortNames.at("trajectory_follower").size() == 3);
}

BOOST_AUTO_TEST_CASE(TestPlay)
{
    replayHandler.setSampleIndex(0);
    replayHandler.play();

    std::this_thread::sleep_for(std::chrono::seconds(5));

    BOOST_TEST(replayHandler.isPlaying());
    BOOST_TEST(replayHandler.getCurrentSpeed() > 0);
    BOOST_TEST(replayHandler.canSampleBeReplayed());

    auto intermediateIndex = replayHandler.getCurIndex();
    replayHandler.stop();

    BOOST_TEST(!replayHandler.isPlaying());
    BOOST_TEST(!replayHandler.hasFinished());
    BOOST_TEST(replayHandler.getCurIndex() != intermediateIndex);
}

BOOST_AUTO_TEST_CASE(TestPause)
{
    replayHandler.setSampleIndex(0);
    replayHandler.play();

    std::this_thread::sleep_for(std::chrono::seconds(5));
    replayHandler.pause();

    auto intermediateIndex = replayHandler.getCurIndex();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    BOOST_TEST(replayHandler.getCurIndex() == intermediateIndex);
    BOOST_TEST(intermediateIndex > 0);
    BOOST_TEST(!replayHandler.isPlaying());
    BOOST_TEST(!replayHandler.hasFinished());
}

BOOST_AUTO_TEST_CASE(TestPlayThrough)
{
    replayHandler.setSampleIndex(0);
    replayHandler.setReplaySpeed(10.);
    replayHandler.play();

    std::this_thread::sleep_for(std::chrono::seconds(10));

    BOOST_TEST(replayHandler.getCurIndex() == replayHandler.getMaxIndex());
    BOOST_TEST(replayHandler.hasFinished());
    BOOST_TEST(!replayHandler.isPlaying());
}

BOOST_AUTO_TEST_CASE(TestDeinit)
{
    replayHandler.deinit();
    BOOST_TEST(!replayHandler.isPlaying());
}
