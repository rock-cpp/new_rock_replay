rock_testsuite(
    test_suite
        Main.cpp
        LogFileHelperTest.cpp
        LogTaskManagerTest.cpp
        LogTaskTest.cpp
        ReplayHandlerTest.cpp
    DEPS 
        rock_replay
    DEPS_PKGCONFIG
        trajectory_follower
        orocos_cpp
)

target_include_directories(test_suite PRIVATE "../src")
