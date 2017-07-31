add_definitions(-DBUILD_QXT)

rock_library(span_slider
    SOURCES 
        qxtspanslider.cpp
    HEADERS 
        qxtglobal.h
        qxtnamespace.h
        qxtspanslider.h 
        qxtspanslider_p.h
    DEPS_PKGCONFIG
        QtCore
        QtGui
    MOC
        qxtnamespace.h
        qxtspanslider_p.h
        qxtspanslider.h
)