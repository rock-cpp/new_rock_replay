rock_testsuite(
    test_suite
        Main.cpp
        ReplayHandlerTest.cpp
        LogFileHelperTest.cpp
    DEPS 
        rock_replay
)

target_include_directories(test_suite PRIVATE "../src")
