find_package(Threads)

rock_library(rock_replay
    SOURCES ReplayHandler.cpp LogTask.cpp
    HEADERS ReplayHandler.hpp LogTask.hpp
    DEPS_PKGCONFIG 
        orocos_cpp
        pocolog_cpp
        rtt_typelib-${OROCOS_TARGET}
        orocos-rtt-${OROCOS_TARGET}
        orocos-rtt-mqueue-${OROCOS_TARGET}
        orocos-rtt-corba-${OROCOS_TARGET}        
)

target_link_libraries(rock_replay pthread)



find_package(Qt4 REQUIRED)
find_path(QWT_INCLUDE_DIR
    NAMES qwt_plot.h
    HINTS ${QT_INCLUDE_DIR}
    PATH_SUFFIXES qwt qwt-qt3 qwt-qt4 qwt-qt5
)

find_library(QWT_LIBRARIES
        NAMES qwt qwt-qt4
)

QT4_ADD_RESOURCES(VIZ_RESC ressources.qrc)

INCLUDE_DIRECTORIES(${QWT_INCLUDE_DIR})

rock_executable(viz 
    SOURCES
        Main.cpp
        ReplayGUI.cpp
        ${VIZ_RESC}
    HEADERS
        ReplayGUI.h
    MOC
        ReplayGUI.h
        ressources.qrc
    DEPS_PKGCONFIG
        QtCore
        QtGui
        vizkit3d
        QxtWidgets
    DEPS
        rock_replay
    UI
        main.ui
)

TARGET_LINK_LIBRARIES(viz ${QWT_LIBRARIES})


