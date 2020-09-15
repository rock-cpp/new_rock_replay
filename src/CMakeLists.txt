find_package(Threads)
find_package(Qt4 REQUIRED)

QT4_ADD_RESOURCES(VIZ_RESC ressources.qrc)

rock_library(rock_replay
    SOURCES 
        ReplayHandler.cpp 
        LogTask.cpp
        LogTaskManager.cpp
        LogFileHelper.cpp
        ReplayGui.cpp
        ${VIZ_RESC}
    MOC
        ReplayGui.h
    HEADERS 
        ReplayHandler.hpp 
        LogTask.hpp
        LogTaskManager.hpp
        LogFileHelper.hpp
        ReplayGui.h
    DEPS_PKGCONFIG 
        orocos_cpp
        pocolog_cpp 
        QtCore
        QtGui
        vizkit3d
    UI
        main.ui
)

target_link_libraries(rock_replay pthread boost_program_options)

install(FILES ${CMAKE_BINARY_DIR}/src/ui_main.h DESTINATION include/rock_replay/)



rock_executable(rock-replay2 
    SOURCES
        Main.cpp
    DEPS
        rock_replay
)



