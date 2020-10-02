rock_testsuite(
    test_suite
        Main.cpp
        LogFileHelperTest.cpp
        LogTaskManagerTest.cpp
        ReplayHandlerTest.cpp
    DEPS 
        rock_replay
)

target_include_directories(test_suite PRIVATE "../src")
