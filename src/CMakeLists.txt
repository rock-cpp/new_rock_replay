rock_find_qt5(Widgets)

qt5_add_resources(VIZ_RESOURCES ressources.qrc)

rock_library(rock_replay
    SOURCES
        ReplayHandler.cpp
        LogTask.cpp
        LogTaskManager.cpp
        LogFileHelper.cpp
    HEADERS
        ReplayHandler.hpp
        LogTask.hpp
        LogTaskManager.hpp
        LogFileHelper.hpp
    DEPS_PKGCONFIG
        base-logging
        orocos_cpp
        pocolog_cpp
    LIBS
        gcov
)

rock_library(arg_parser
    SOURCES
        ArgParser.cpp
    HEADERS
        ArgParser.hpp
    LIBS
        boost_program_options
        gcov
)

rock_executable(rock-replay2
    SOURCES
        Main.cpp
        ReplayGui.cpp
        ${VIZ_RESOURCES}
    UI
        main.ui
    MOC
        ReplayGui.h
    DEPS
        rock_replay
        arg_parser
        Qt5::Widgets
)
