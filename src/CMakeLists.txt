find_package(Threads)
find_package(Qt4 REQUIRED)
find_path(QWT_INCLUDE_DIR
    NAMES qwt_plot.h
    HINTS ${QT_INCLUDE_DIR}
    PATH_SUFFIXES qwt-qt4
)

find_library(QWT_LIBRARIES
        NAMES qwt-qt4
)

QT4_ADD_RESOURCES(VIZ_RESC ressources.qrc)

INCLUDE_DIRECTORIES(${QWT_INCLUDE_DIR})

add_subdirectory(ext)
include_directories(ext)

rock_library(rock_replay
    SOURCES ReplayHandler.cpp LogTask.cpp
        ReplayGUI.cpp
        ReplayGUIBase.cpp
        ${VIZ_RESC}
    MOC
        ReplayGUI.h
        ReplayGUIBase.h
    HEADERS 
        ReplayHandler.hpp 
        LogTask.hpp
        ReplayGUI.h
        ReplayGUIBase.h
    DEPS_PKGCONFIG 
        orocos_cpp
        pocolog_cpp
        rtt_typelib-${OROCOS_TARGET}
        orocos-rtt-${OROCOS_TARGET}
        orocos-rtt-mqueue-${OROCOS_TARGET}
        orocos-rtt-corba-${OROCOS_TARGET}
        QtCore
        QtGui
        vizkit3d
    DEPS
        span_slider
    UI
        main.ui
)

target_link_libraries(rock_replay pthread)
TARGET_LINK_LIBRARIES(rock_replay ${QWT_LIBRARIES})

install(FILES ${CMAKE_BINARY_DIR}/src/ui_main.h DESTINATION include/rock_replay/)



rock_executable(rock-replay2 
    SOURCES
        Main.cpp
    DEPS
        rock_replay
)



